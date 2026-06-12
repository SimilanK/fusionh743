#ifndef PSEUDO_TRAJ_H
#define PSEUDO_TRAJ_H

#include <stdint.h>
#include "rtwtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Loads and parses the pseudo_flight.csv file from the SD card.
 * Filters out leading rows with h < 100m.
 * Decimates the parsed data by storing every 50th valid row to RAM.
 * @return 1 on success, 0 on failure.
 */
uint8_t pseudo_traj_load(void);

/**
 * Interpolates trajectory variables linearly to 1 kHz base rate at time t.
 * @param t Current simulation time in seconds.
 * @param h Output pointer for MSL altitude in meters.
 * @param vz Output pointer for vertical velocity in m/s.
 * @param pitch Output pointer for pitch angle in radians.
 * @param logic Output pointer for control arm logic flag.
 * @return 1 if simulation has reached the end of the recording, 0 otherwise.
 */
uint8_t pseudo_sample(double t, double *h, double *vz, double *pitch, boolean_T *logic);

#ifdef __cplusplus
}
#endif

#endif /* PSEUDO_TRAJ_H */
