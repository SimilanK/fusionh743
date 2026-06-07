/*
 * Kalman2State.h  —  2-state altimeter Kalman filter (altitude + vertical velocity)
 *
 *   State : x = [ altitude_m ; velocity_ms ]
 *   Input : u = accelLongAxis - gravity        (accelerometer drives the PREDICT)
 *   Meas. : z = barometric altitude            (barometer drives the CORRECT)
 *
 * Self-contained. No matrix library, no malloc, no exceptions, no FreeRTOS.
 * float throughout (assumes the H7 hardware FPU). One scalar measurement →
 * the Kalman gain is a DIVISION, never a matrix inverse.
 *
 * Why a 2x2 covariance run LIVE in float (and not AltOS-style baked gains):
 *   AltOS (ao_kalman.c) precomputes steady-state gains offline and stores them
 *   in fixed-point — perfect for an FPU-less 8-bit part. We have a 480 MHz
 *   Cortex-M7 with a single-precision FPU, so running the real 2x2 covariance
 *   costs ~30 float ops (~0.2 us) but buys: (a) honest adaptation to a VARIABLE
 *   dt, (b) one intuitive tuning knob (sigmaA), (c) readable/debuggable math.
 *   Measured budget impact is negligible — see the .c file header.
 *
 * USAGE (3 lines):
 *   Kalman2State kf;
 *   Kalman2State_Init(&kf, padAltitude_m);           // once, on the pad
 *   ... each cycle:
 *   Kalman2State_Predict(&kf, accelZ_mss, dt_s);     // at IMU rate
 *   Kalman2State_UpdateBaro(&kf, baroAlt_m);         // when a baro sample arrives
 *   float h = Kalman2State_Altitude(&kf);            // fused outputs
 *   float v = Kalman2State_Velocity(&kf);
 *
 * Author: airbrake project, 2026.  Standalone — depends on nothing in firmware.
 */

#ifndef KALMAN2STATE_H
#define KALMAN2STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ───────────────────────────────────────────────────────────────────────────
 *  TUNE  — defaults baked here; override per-flight via Kalman2State_SetTuning()
 *          after tuning on logged data in the MATLAB clone (no firmware rebuild).
 * ───────────────────────────────────────────────────────────────────────────
 *
 * R  [m^2]   : barometer measurement-noise VARIANCE. From variance_extractor.m
 *              on a calm static segment (std ~0.15 m -> var ~0.02). Smaller R =
 *              "trust the baro more"; larger R = "smooth harder, trust accel".
 *
 * sigmaA [m/s^2] : accelerometer process-noise STD — the ONE main knob.
 *              Set ABOVE the raw accel noise floor (~0.03) to also cover
 *              vibration, gravity-subtraction error and tilt. Try 0.05–0.2.
 *              Bigger sigmaA = filter believes the accel less, leans on baro.
 *
 * gravity [m/s^2] : 1 g along the rocket's LONG axis, measured on the pad.
 *              PREDICT subtracts this so the input u is true linear accel.
 *
 * baroDistrustSpeed [m/s] : above this |velocity|, the baro is inflated because
 *              dynamic-pressure / shock errors grow ~ v^2 (same idea AltOS uses
 *              when it stops trusting the baro at speed). 50 m/s is conservative.
 */
#define K2S_DEFAULT_R                   0.02f
#define K2S_DEFAULT_SIGMA_A             0.10f
#define K2S_DEFAULT_GRAVITY             9.81f
#define K2S_DEFAULT_BARO_DISTRUST_SPEED 50.0f

/* ───────────────────────────────────────────────────────────────────────────
 *  ROBUSTNESS GUARDS (numerical hygiene, NOT tuning) — see .c for why each.
 * ─────────────────────────────────────────────────────────────────────────── */
#define K2S_S_MIN     1e-9f   /* skip update if innovation covariance S < this  */
#define K2S_P_FLOOR   1e-9f   /* keep diagonal P entries from collapsing to <=0 */
#define K2S_DT_MAX    1.0f    /* ignore absurd dt (stale tick / first sample)   */

/* ───────────────────────────────────────────────────────────────────────────
 *  Filter object. One explicit "self" pointer per call (reentrant, no globals).
 *  Layout: state (2), covariance (4), tunables (4) = 10 floats = 40 bytes.
 * ─────────────────────────────────────────────────────────────────────────── */
typedef struct {
    /* State vector x = [altitude ; velocity] */
    float x0;   /* altitude  [m]   (AGL once init'd to pad)   */
    float x1;   /* velocity  [m/s] (positive = upward)        */

    /* Covariance P (symmetric 2x2): [ P00 P01 ; P10 P11 ] */
    float P00;  /* var(altitude)              */
    float P01;  /* cov(altitude, velocity)    */
    float P10;  /* cov(velocity, altitude)    */
    float P11;  /* var(velocity)              */

    /* Active tunables (seeded from K2S_DEFAULT_* in Init) */
    float R;                  /* baro meas-noise variance [m^2]   */
    float sigmaA;             /* accel process-noise std  [m/s^2] */
    float gravity;            /* long-axis 1 g            [m/s^2] */
    float baroDistrustSpeed;  /* baro-distrust threshold  [m/s]   */
} Kalman2State;

/* ───────────────────────────────────────────────────────────────────────────
 *  PUBLIC API (exactly the small surface the spec asks for, plus SetTuning).
 * ─────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialise on the pad. Sets altitude=alt0, velocity=0, loads default
 *        tunables, and seeds P with moderate uncertainty so early baro/accel
 *        samples are weighted sensibly.
 * @param kf    filter object
 * @param alt0  starting altitude [m] (pass the pad altitude to get AGL output)
 */
void Kalman2State_Init(Kalman2State *kf, float alt0);

/**
 * @brief Override the four tunables AFTER Init (use MATLAB-tuned values).
 *        Pass a non-positive value to keep the current setting for that field.
 */
void Kalman2State_SetTuning(Kalman2State *kf,
                            float R, float sigmaA,
                            float gravity, float baroDistrustSpeed);

/**
 * @brief PREDICT step. Propagates state & covariance forward by dt using the
 *        accelerometer as the control input. Removes gravity internally.
 * @param kf             filter object
 * @param accelLongAxis  SIGNED long-axis accel [m/s^2] (sign carries up/down!)
 * @param dt             time since previous predict [s]; ignored if <=0 or huge
 */
void Kalman2State_Predict(Kalman2State *kf, float accelLongAxis, float dt);

/**
 * @brief CORRECT step with a barometric altitude measurement. Applies speed-
 *        based distrust (inflates R when |velocity| > baroDistrustSpeed).
 * @param kf  filter object
 * @param z   barometric altitude [m] (same datum as alt0 — i.e. AGL if you
 *            subtract pad pressure-altitude before calling)
 */
void Kalman2State_UpdateBaro(Kalman2State *kf, float z);

/** @return fused altitude estimate [m]. */
float Kalman2State_Altitude(const Kalman2State *kf);

/** @return fused vertical-velocity estimate [m/s] (positive = upward). */
float Kalman2State_Velocity(const Kalman2State *kf);

#ifdef __cplusplus
}
#endif

#endif /* KALMAN2STATE_H */
