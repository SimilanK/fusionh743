/*
 * Kalman2State.c  —  implementation of the 2-state altimeter Kalman filter.
 *
 * ── COMPUTATIONAL BUDGET (why this is safe next to the controller) ──────────
 *   Target: STM32H7 @ 480 MHz, single-precision FPU (verified: profiling report
 *   timer = 4.8e8 ticks/s). Your existing control model's budgets, measured:
 *       controlTID0 (1 kHz loop): avg 11.6 us, max 18.2 us  (1.8% of 1000 us)
 *       NAP (apogee predictor)  : avg 0.44 ms, max 2.5 ms   (in the 50 Hz task)
 *       overall CPU: 2.4% avg, 64% peak.
 *
 *   This filter per full cycle (Predict + UpdateBaro):
 *       ~22 mul, ~18 add, 1 div, 1 fabsf  ≈ ~40 float ops.
 *       On an M7 FPU (~1 op/cycle pipelined, div ~14 cyc) that is roughly
 *       60–90 cycles ≈ 0.15 us. Even called at the full 1 kHz it adds
 *       ~0.015% CPU. Called at the IMU/baro rate it is unmeasurable.
 *   Conclusion: zero threat to the 1 kHz cadence or the NAP peak. (cost = est.,
 *   from op-count; the proof test below measures correctness, not wall-clock.)
 *
 * ── THE MATH (discrete-time, exactly the spec) ──────────────────────────────
 *   F = [1 dt ; 0 1]      B = [0.5 dt^2 ; dt]      H = [1 0]
 *   Predict: x = F x + B u ;  P = F P F' + Q
 *   Update : y = z - H x ;  S = H P H' + R ;  K = P H' / S ;
 *            x = x + K y    ;  P = (I - K H) P
 *   Because H = [1 0], "H P H'" collapses to P00 and "P H'" to [P00 ; P10],
 *   so S is a scalar and the gain is a plain divide.
 */

#include "Kalman2State.h"

#include <math.h>    /* fabsf — the ONLY external dependency, and it's intrinsic
                      *         on the M7 (compiles to a VABS instruction).      */

/* ─────────────────────────────────────────────────────────────────────────── */
void Kalman2State_Init(Kalman2State *kf, float alt0)
{
    /* State: start at the given altitude, assume at rest. */
    kf->x0 = alt0;
    kf->x1 = 0.0f;

    /* Covariance seed. We KNOW the start altitude fairly well (we just measured
     * the pad), and we KNOW velocity is zero on the pad — so seed both variances
     * modestly and the cross-terms at zero. The filter converges within a few
     * samples regardless; this just avoids a wild first correction. */
    kf->P00 = 1.0f;    /* +/-1 m^2 altitude uncertainty   */
    kf->P01 = 0.0f;
    kf->P10 = 0.0f;
    kf->P11 = 1.0f;    /* +/-1 (m/s)^2 velocity uncertainty */

    /* Load default tunables. Override later with Kalman2State_SetTuning(). */
    kf->R                 = K2S_DEFAULT_R;
    kf->sigmaA            = K2S_DEFAULT_SIGMA_A;
    kf->gravity           = K2S_DEFAULT_GRAVITY;
    kf->baroDistrustSpeed = K2S_DEFAULT_BARO_DISTRUST_SPEED;
}

/* ─────────────────────────────────────────────────────────────────────────── */
void Kalman2State_SetTuning(Kalman2State *kf,
                            float R, float sigmaA,
                            float gravity, float baroDistrustSpeed)
{
    /* Non-positive => "leave this one as-is". Lets you set just one knob. */
    if (R                 > 0.0f) kf->R                 = R;
    if (sigmaA            > 0.0f) kf->sigmaA            = sigmaA;
    if (gravity           > 0.0f) kf->gravity           = gravity;
    if (baroDistrustSpeed > 0.0f) kf->baroDistrustSpeed = baroDistrustSpeed;
}

/* ─────────────────────────────────────────────────────────────────────────── */
void Kalman2State_Predict(Kalman2State *kf, float accelLongAxis, float dt)
{
    /* Guard: a non-positive dt (first call, repeated tick) or an absurd dt
     * (stale/garbage timestamp) would inject nonsense — just skip the step.
     * State is left unchanged; the next valid dt resumes cleanly. */
    if (dt <= 0.0f || dt > K2S_DT_MAX) {
        return;
    }

    /* Control input = measured long-axis accel minus 1 g. The SIGN matters:
     * it tells the filter whether we are accelerating up or down. (Using a
     * magnitude here would be a bug — it erases the direction of motion.) */
    const float u = accelLongAxis - kf->gravity;

    /* ── State propagate:  x = F x + B u  ────────────────────────────────────
     *   altitude += velocity*dt + 0.5*u*dt^2   (constant-accel kinematics)
     *   velocity += u*dt
     * Order: update altitude using the OLD velocity FIRST, then the velocity. */
    const float dt2 = dt * dt;
    kf->x0 += kf->x1 * dt + 0.5f * u * dt2;
    kf->x1 += u * dt;

    /* ── Process-noise Q from the single knob sigmaA ─────────────────────────
     * Q models "how much could an unmodelled acceleration have changed the
     * state over dt". With acceleration noise variance s2 = sigmaA^2 entering
     * through B, the standard result is:
     *     Q = s2 * [ 0.25 dt^4   0.5 dt^3 ;
     *                0.5  dt^3       dt^2  ]
     */
    const float s2  = kf->sigmaA * kf->sigmaA;
    const float dt3 = dt2 * dt;
    const float dt4 = dt2 * dt2;
    const float Q00 = 0.25f * dt4 * s2;
    const float Q01 = 0.5f  * dt3 * s2;   /* == Q10 (symmetric) */
    const float Q11 =          dt2 * s2;

    /* ── Covariance propagate:  P = F P F' + Q  ─────────────────────────────
     * Expanding F P F' with F = [1 dt ; 0 1] gives the closed form below.
     * CRITICAL: every right-hand side uses the OLD P entries. We read P01,P10,
     * P11 before overwriting P00/P01/P10, so capture P11 contribution first.
     *
     *   P00' = P00 + dt(P01+P10) + dt^2 P11 + Q00
     *   P01' = P01 + dt P11               + Q01
     *   P10' = P10 + dt P11               + Q10
     *   P11' = P11                        + Q11
     */
    const float P00 = kf->P00, P01 = kf->P01, P10 = kf->P10, P11 = kf->P11;

    kf->P00 = P00 + dt * (P01 + P10) + dt2 * P11 + Q00;
    kf->P01 = P01 + dt * P11 + Q01;
    kf->P10 = P10 + dt * P11 + Q01;   /* Q10 == Q01 */
    kf->P11 = P11 + Q11;

    /* Symmetry hygiene: float rounding can nudge P01 != P10 over many steps.
     * Force them equal (average) so the matrix stays a valid covariance. */
    const float sym = 0.5f * (kf->P01 + kf->P10);
    kf->P01 = sym;
    kf->P10 = sym;
}

/* ─────────────────────────────────────────────────────────────────────────── */
void Kalman2State_UpdateBaro(Kalman2State *kf, float z)
{
    /* ── Speed-based baro distrust ───────────────────────────────────────────
     * At low speed the baro is the truth anchor. As |v| climbs, dynamic-pressure
     * and shock effects corrupt the static-pressure reading; the error grows
     * ~ v^2, so inflate R by (|v|/threshold)^2 above the threshold. This is the
     * same instinct AltOS encodes when it stops trusting the baro at speed. */
    float Reff = kf->R;
    const float v = fabsf(kf->x1);
    if (v > kf->baroDistrustSpeed) {
        const float ratio = v / kf->baroDistrustSpeed;
        Reff = kf->R * ratio * ratio;
    }

    /* ── Innovation and its covariance ───────────────────────────────────────
     *   y = z - H x = z - altitude        (how far the baro is from our guess)
     *   S = H P H' + Reff = P00 + Reff     (scalar; expected spread of y)      */
    const float y = z - kf->x0;
    const float S = kf->P00 + Reff;

    /* Guard: S should always be > 0 (Reff >= R > 0). If a pathological tune or
     * corruption makes it tiny, skip rather than divide by ~0 and explode. */
    if (S < K2S_S_MIN) {
        return;
    }

    /* ── Kalman gain  K = P H' / S = [P00 ; P10] / S  (a divide, no inverse) ── */
    const float invS = 1.0f / S;
    const float K0 = kf->P00 * invS;   /* gain into altitude */
    const float K1 = kf->P10 * invS;   /* gain into velocity */

    /* ── State correction  x = x + K y ─────────────────────────────────────── */
    kf->x0 += K0 * y;
    kf->x1 += K1 * y;

    /* ── Covariance correction  P = (I - K H) P,  H = [1 0]  ─────────────────
     * Written out, with H picking the first column:
     *   P00' = P00 - K0 P00
     *   P01' = P01 - K0 P01
     *   P10' = P10 - K1 P00
     *   P11' = P11 - K1 P01
     * ORDER TRAP: P10' and P11' need the PRE-update P00 and P01. So snapshot
     * them before we overwrite P00/P01. */
    const float P00_old = kf->P00;
    const float P01_old = kf->P01;

    kf->P00 = P00_old      - K0 * P00_old;
    kf->P01 = kf->P01      - K0 * P01_old;
    kf->P10 = kf->P10      - K1 * P00_old;
    kf->P11 = kf->P11      - K1 * P01_old;

    /* Symmetry + positivity hygiene (rounding can break both near steady state). */
    const float sym = 0.5f * (kf->P01 + kf->P10);
    kf->P01 = sym;
    kf->P10 = sym;
    if (kf->P00 < K2S_P_FLOOR) kf->P00 = K2S_P_FLOOR;
    if (kf->P11 < K2S_P_FLOOR) kf->P11 = K2S_P_FLOOR;
}

/* ─────────────────────────────────────────────────────────────────────────── */
float Kalman2State_Altitude(const Kalman2State *kf) { return kf->x0; }
float Kalman2State_Velocity(const Kalman2State *kf) { return kf->x1; }
