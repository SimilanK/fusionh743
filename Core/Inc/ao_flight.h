/*
 * ao_flight.h  —  flight state machine  (STARTUP → PAD → BOOST)
 *
 * Names and transition thresholds adapted from AltOS ao_flight.c / ao_flight.h.
 * Reference: github.com/ajtowns/altos  src/kernel/ao_flight.c
 *
 * ── State diagram (3-state first slice) ──────────────────────────────────────
 *
 *   AO_FLIGHT_STARTUP ──(N cal samples done)──► AO_FLIGHT_PAD
 *   AO_FLIGHT_PAD     ──(launch detected)──────► AO_FLIGHT_BOOST
 *
 * ── Data flow ────────────────────────────────────────────────────────────────
 *
 *   Every baro sample:
 *     sensors → main.c reads raw values
 *             → ao_flight_update() called
 *                   ├─ STARTUP: accumulate cal samples
 *                   │           on Nth sample: init Kalman, → PAD  (state→filter)
 *                   ├─ PAD:     Kalman predict + updateBaro         (state drives filter)
 *                   │           read kf outputs, check thresholds   (filter → state)
 *                   └─ BOOST:   Kalman predict + updateBaro         (same as PAD for now)
 *             → main.c reads ao_flight_state + kf outputs for logging / actuation
 *
 * ── Naming vs AltOS ──────────────────────────────────────────────────────────
 *   AO_FLIGHT_STARTUP  = AltOS ao_flight_startup  (collecting calibration samples)
 *   AO_FLIGHT_PAD      = AltOS ao_flight_pad       (calibrated, armed, watching launch)
 *   AO_FLIGHT_BOOST    = AltOS ao_flight_boost      (launch detected, motor burning)
 *   ao_ground_height   = AltOS ao_ground_height     (absolute pad baro altitude [m])
 *   ao_ground_accel    = AltOS ao_ground_accel      (measured 1g on vertical axis [m/s²])
 *
 *   NOTE: AltOS also has ao_flight_idle (USB/download mode — NOT the same as
 *   STARTUP here). We skip idle for now; this slice begins at STARTUP.
 */

#ifndef AO_FLIGHT_H
#define AO_FLIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Kalman2State.h"

/* ── State enum (AltOS-aligned names) ───────────────────────────────────────── */
typedef enum {
    AO_FLIGHT_STARTUP = 0,  /* collecting calibration samples; Kalman NOT yet live  */
    AO_FLIGHT_PAD,          /* calibrated, armed, stationary — watching for launch   */
    AO_FLIGHT_BOOST,        /* launch confirmed, motor burning — airbrake LOCKED     */
} ao_flight_state_t;

/* ── Public state (read by main loop for logging and actuation) ─────────────── */
extern ao_flight_state_t ao_flight_state;
extern float ao_ground_height;  /* absolute baro altitude measured on the pad [m]   */
extern float ao_ground_accel;   /* 1g on the vertical axis measured on the pad [m/s²] */

/* ── Tuning ─────────────────────────────────────────────────────────────────────
 * All thresholds from AltOS ao_flight.c — edit here before compile.
 *
 * AO_FLIGHT_STARTUP_SAMPLES:  how many baro+IMU samples to average for calibration.
 *   AltOS uses 512; at our ~25 Hz baro rate, 100 ≈ 4 s (quick bench) — increase
 *   to 250 (~10 s) before flight for a more stable reference.
 *
 * Launch detect — AltOS condition (ao_flight.c):
 *   height > 20 m  OR  (accel > 20 m/s²  AND  speed > 5 m/s)
 *   The AND prevents a table-bump (spike accel, no sustained speed) from
 *   falsely triggering BOOST.  Both values come from AltOS unchanged.
 */
#define AO_FLIGHT_STARTUP_SAMPLES   100     /* cal sample count                          */
#define AO_BOOST_HEIGHT_THRESHOLD   20.0f   /* KF altitude AGL [m]  — AO_M_TO_HEIGHT(20) */
#define AO_BOOST_ACCEL_THRESHOLD    20.0f   /* net upward accel [m/s²] — AO_MSS_TO_ACCEL */
#define AO_BOOST_SPEED_THRESHOLD     5.0f   /* KF velocity [m/s]    — AO_MS_TO_SPEED(5)  */

/* ── Calibration sanity bands (mirrors AltOS ao_flight.c startup validation) ──────
 * If the averaged pad readings fall outside these, calibration is rejected and
 * STARTUP restarts (accumulators reset). Catches a moving board, an SPI glitch
 * returning 0/garbage, or a baro blow-out — none of which should ever arm.
 *
 * Accel band: expect ~1g (9.81) on the vertical axis. Generous margin allows
 *   modest pad tilt but rejects 0 / NaN / absurd values. Positive band assumes
 *   nose-up mounting (az reads +9.81); flip if your axis convention differs.
 * Height band: AltOS uses [-1000, 7000] m; widened top for high-altitude sites. */
#define AO_CAL_ACCEL_MIN   7.0f      /* [m/s²] reject if |g_cal| below this  */
#define AO_CAL_ACCEL_MAX  12.0f      /* [m/s²] reject if g_cal above this    */
#define AO_CAL_HEIGHT_MIN (-1000.0f) /* [m]    reject baro below this        */
#define AO_CAL_HEIGHT_MAX  9000.0f   /* [m]    reject baro above this        */

/* Number of consecutive calibration rejections to report (for visibility).
 * Read by main.c to surface a warning; does not stop retrying.               */
extern uint32_t ao_cal_reject_count;

/* ── API ─────────────────────────────────────────────────────────────────────── */

/**
 * @brief  Call once after hardware init, before the main loop.
 *         Resets the state machine to AO_FLIGHT_STARTUP.
 */
void ao_flight_init(void);

/**
 * @brief  Call every baro sample (inside the if(MS5607_Poll()) block in main.c).
 *         Internally runs the Kalman filter for PAD and BOOST states.
 *         Advances the state machine when transition conditions are met.
 *
 * @param kf            Pointer to the Kalman2State object (owned by main.c).
 *                      Not used in STARTUP — safe to pass before Init is called.
 * @param accel_long    Signed long-axis acceleration [m/s²].
 *                      Use az_mss if the board is nose-up (reads ~+9.81 at rest).
 *                      TODO: change to correct axis if board is mounted sideways.
 * @param dt            Time since previous call [s].
 *                      Passed through to Kalman2State_Predict; ignored if <=0.
 * @param baro_alt_abs  Absolute barometric altitude [m] — pass MS5607GetAltitudeM().
 *                      ao_flight_update subtracts ao_ground_height internally.
 */
void ao_flight_update(Kalman2State *kf,
                      float accel_long,
                      float dt,
                      float baro_alt_abs);

#ifdef __cplusplus
}
#endif

#endif /* AO_FLIGHT_H */
