/*
 * mpl3115a2.h
 *
 * Polling-mode driver for the NXP MPL3115A2 I2C pressure/altitude sensor.
 * Target: STM32 HAL (tested against STM32H7). No interrupts, no DMA.
 *
 * Usage:
 *   MPL3115A2_t baro;
 *   if (MPL3115A2_Init(&baro, &hi2c1, MPL_MODE_ALTIMETER, MPL_OSR_16) != MPL_OK) {
 *       // hardware fault: pull-ups / clock / wiring
 *   }
 *   float alt, temp;
 *   if (MPL3115A2_ReadAltitude(&baro, &alt, &temp) == MPL_OK) { ... }
 */

#ifndef MPL3115A2_H
#define MPL3115A2_H

#include "stm32h7xx_hal.h"   /* change to your series, e.g. stm32f4xx_hal.h */
#include <stdint.h>

/* 7-bit address 0x60, shifted to the 8-bit form HAL expects */
#define MPL3115A2_I2C_ADDR   (0x60 << 1)
#define MPL3115A2_WHOAMI_VAL  0xC4

typedef enum {
    MPL_OK = 0,
    MPL_ERR_I2C,        /* bus transaction failed */
    MPL_ERR_WHOAMI      /* device did not return 0xC4 */
} MPL_Status;

typedef enum {
    MPL_MODE_BAROMETER = 0,   /* output in Pascals  */
    MPL_MODE_ALTIMETER = 1    /* output in meters   */
} MPL_Mode;

/* Oversample ratio. Higher = less noise, slower conversion.
 * Approx min time between samples (from datasheet Table 46):
 *   OSR_1  ~6ms    OSR_16  ~66ms
 *   OSR_2  ~10ms   OSR_32  ~130ms
 *   OSR_4  ~18ms   OSR_64  ~258ms
 *   OSR_8  ~34ms   OSR_128 ~512ms
 */
typedef enum {
    MPL_OSR_1   = 0,
    MPL_OSR_2   = 1,
    MPL_OSR_4   = 2,
    MPL_OSR_8   = 3,
    MPL_OSR_16  = 4,
    MPL_OSR_32  = 5,
    MPL_OSR_64  = 6,
    MPL_OSR_128 = 7
} MPL_OSR;

typedef struct {
    I2C_HandleTypeDef *hi2c;   /* bus handle, e.g. &hi2c1 */
    MPL_Mode mode;
    MPL_OSR  osr;
} MPL3115A2_t;

/* Verify WHO_AM_I, configure mode + OSR in standby, then go active. */
MPL_Status MPL3115A2_Init(MPL3115A2_t *dev, I2C_HandleTypeDef *hi2c,
                          MPL_Mode mode, MPL_OSR osr);

/* Returns MPL_OK and sets *ready=1 if new data is waiting (PTDR flag). */
MPL_Status MPL3115A2_DataReady(MPL3115A2_t *dev, uint8_t *ready);

/* Read altitude (m) + temperature (C). Requires MPL_MODE_ALTIMETER.
 * Returns MPL_ERR_I2C if no new data was ready when polled. */
MPL_Status MPL3115A2_ReadAltitude(MPL3115A2_t *dev, float *altitude_m, float *temp_c);

/* Read pressure (Pa) + temperature (C). Requires MPL_MODE_BAROMETER. */
MPL_Status MPL3115A2_ReadPressure(MPL3115A2_t *dev, float *pressure_pa, float *temp_c);

/* Change OSR at runtime (handles the required standby->active cycle). */
MPL_Status MPL3115A2_SetOSR(MPL3115A2_t *dev, MPL_OSR osr);

/* Switch altimeter/barometer at runtime (standby->active cycle). */
MPL_Status MPL3115A2_SetMode(MPL3115A2_t *dev, MPL_Mode mode);

/* Set equivalent sea-level pressure for altitude calc, in Pascals.
 * Default is 101326 Pa. Adjust to local METAR to zero your AGL reference.
 * Register is 2 Pa/LSB, so the value is rounded to the nearest 2 Pa. */
MPL_Status MPL3115A2_SetSeaLevelPressure(MPL3115A2_t *dev, uint32_t pressure_pa);
/* ISA altitude (m) from pressure (Pa). Pure math — no I2C, no device state. */
float MPL3115A2_AltitudeFromPressure(float pressure_pa);

#endif /* MPL3115A2_H */
