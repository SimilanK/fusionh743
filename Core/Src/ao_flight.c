/*
 * ao_flight.c  —  flight state machine implementation
 *
 * Three states only (first slice):
 *   AO_FLIGHT_STARTUP  collect cal samples non-blockingly, init Kalman
 *   AO_FLIGHT_PAD      Kalman running, watch for launch
 *   AO_FLIGHT_BOOST    launch confirmed
 *
 * Computational cost: a handful of float comparisons per call — negligible
 * against the Kalman's ~0.26 µs and the sensor read overhead.
 */

#include "ao_flight.h"
#include <math.h>   /* fabsf — FPU intrinsic on M7, no overhead */

/* ── Public state ────────────────────────────────────────────────────────────── */
ao_flight_state_t ao_flight_state = AO_FLIGHT_STARTUP;
float ao_ground_height = 0.0f;   /* absolute pad altitude [m] */
float ao_ground_accel  = 9.81f;  /* measured 1g on vertical axis [m/s²] */
uint32_t ao_cal_reject_count = 0;  /* incremented each time a cal batch is rejected */

/* ── Private calibration accumulator ────────────────────────────────────────── */
static float    _cal_height_sum = 0.0f;
static float    _cal_accel_sum  = 0.0f;
static uint32_t _cal_count      = 0;

/* Reset the calibration accumulators (start a fresh averaging batch). */
static void _cal_reset(void)
{
    _cal_height_sum = 0.0f;
    _cal_accel_sum  = 0.0f;
    _cal_count      = 0;
}

/* ─────────────────────────────────────────────────────────────────────────────── */
void ao_flight_init(void)
{
    ao_flight_state     = AO_FLIGHT_STARTUP;
    ao_ground_height    = 0.0f;
    ao_ground_accel     = 9.81f;
    ao_cal_reject_count = 0;
    _cal_reset();
}

/* ─────────────────────────────────────────────────────────────────────────────── */
void ao_flight_update(Kalman2State *kf,
                      float accel_long,
                      float dt,
                      float baro_alt_abs)
{
    switch (ao_flight_state) {

    /* ── STARTUP: accumulate calibration samples ──────────────────────────────
     * Non-blocking: called once per baro sample from main loop.
     * Accumulates height and accel readings until AO_FLIGHT_STARTUP_SAMPLES
     * have been collected, then:
     *   1. Computes ao_ground_height (AGL zero reference)
     *   2. Computes ao_ground_accel  (true 1g, replaces nominal 9.81)
     *   3. Initialises the Kalman filter with measured gravity
     *   4. Transitions to AO_FLIGHT_PAD
     *
     * Note: Kalman is NOT called here — it isn't initialised yet.
     * The kf pointer is valid but untouched until the transition.           */
    case AO_FLIGHT_STARTUP:
        /* Reject non-finite sensor reads outright (SPI glitch / NaN) without
         * polluting the running sums — drop this sample, keep collecting.    */
        if (!isfinite(baro_alt_abs) || !isfinite(accel_long)) {
            break;
        }

        _cal_height_sum += baro_alt_abs;
        _cal_accel_sum  += accel_long;
        _cal_count++;

        if (_cal_count >= AO_FLIGHT_STARTUP_SAMPLES) {
            float h = _cal_height_sum / (float)AO_FLIGHT_STARTUP_SAMPLES;
            float g = _cal_accel_sum  / (float)AO_FLIGHT_STARTUP_SAMPLES;

            /* ── Sanity gate (mirrors AltOS startup validation) ──────────────
             * If the averaged references are out of band, the board was moving,
             * a sensor glitched, or the baro blew out. Do NOT arm: reject the
             * batch, count it, and restart calibration. This is the safe
             * default — STARTUP simply repeats until a clean batch is seen.   */
            if (g < AO_CAL_ACCEL_MIN  || g > AO_CAL_ACCEL_MAX ||
                h < AO_CAL_HEIGHT_MIN || h > AO_CAL_HEIGHT_MAX ||
                !isfinite(g) || !isfinite(h))
            {
                ao_cal_reject_count++;
                _cal_reset();        /* throw out this batch, start over */
                break;
            }

            /* Clean batch — commit references */
            ao_ground_height = h;
            ao_ground_accel  = g;

            /* Initialise filter (state → filter, one-shot)
             * x0 = 0 (AGL starts at zero on the pad)
             * x1 = 0 (at rest)
             * gravity = ao_ground_accel (measured, not nominal)            */
            Kalman2State_Init(kf, 0.0f);
            Kalman2State_SetTuning(kf,
                                  -1.0f,            /* keep default R           */
                                  -1.0f,            /* keep default sigmaA      */
                                  ao_ground_accel,  /* measured 1g — key fix    */
                                  -1.0f);           /* keep default distrust spd */

            ao_flight_state = AO_FLIGHT_PAD;
        }
        break;

    /* ── PAD: Kalman running, watching for launch ──────────────────────────────
     * Filter runs every sample (normal predict + baro correct).
     * Launch detection mirrors AltOS ao_flight.c exactly:
     *
     *   BOOST if:  kf_altitude > 20 m AGL
     *   OR         (net_accel > 20 m/s²  AND  kf_velocity > 5 m/s)
     *
     * The AND prevents a table-bump (spike in accel but no sustained speed)
     * from false-triggering BOOST. Both values from AltOS, unchanged.
     *
     * Net accel = accel_long − ao_ground_accel: removes the 1g gravity bias so
     * 0 m/s² means "sitting still", positive means "accelerating upward".    */
    case AO_FLIGHT_PAD:
        /* Run the filter — baro is fully trusted at pad speeds */
        Kalman2State_Predict(kf, accel_long, dt);
        Kalman2State_UpdateBaro(kf, baro_alt_abs - ao_ground_height);

        {
            float kf_alt  = Kalman2State_Altitude(kf);
            float kf_vel  = Kalman2State_Velocity(kf);
            float net_acc = accel_long - ao_ground_accel; /* upward accel above 1g */

            /* AltOS launch detection (ao_flight.c):
             *   ao_height > AO_M_TO_HEIGHT(20)  OR
             *   (ao_accel > AO_MSS_TO_ACCEL(20)  AND  ao_speed > AO_MS_TO_SPEED(5)) */
            if (kf_alt > AO_BOOST_HEIGHT_THRESHOLD ||
               (net_acc > AO_BOOST_ACCEL_THRESHOLD && kf_vel > AO_BOOST_SPEED_THRESHOLD))
            {
                ao_flight_state = AO_FLIGHT_BOOST;
            }
        }
        break;

    /* ── BOOST: motor burning, climbing fast ──────────────────────────────────
     * Filter keeps running. Baro distrust is automatic — Kalman2State inflates
     * R when |velocity| > baroDistrustSpeed (50 m/s default). No manual config
     * needed here; the filter self-adjusts as velocity climbs.
     *
     * Actuator: airbrake MUST stay closed during boost (deploying would
     * destabilise the rocket). That lock is enforced in main.c, not here.
     *
     * Next transition (BOOST → COAST/FAST) not yet implemented — add here
     * when the state machine is extended.                                     */
    case AO_FLIGHT_BOOST:
        Kalman2State_Predict(kf, accel_long, dt);
        Kalman2State_UpdateBaro(kf, baro_alt_abs - ao_ground_height);
        /* TODO: add BOOST → next-state transition when extending the machine */
        break;

    default:
        ao_flight_state = AO_FLIGHT_STARTUP;
        break;
    }
}
