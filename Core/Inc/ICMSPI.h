/*
 * ICMSPI.h  —  ICM-45686 6-axis IMU driver (SPI, polling)
 *
 * All register addresses and bit fields verified against:
 *   TDK Datasheet DS-000577 Rev 1.0
 *
 * SPI wiring (this project):
 *   SPI4: PE2=SCK, PE5=MISO, PE6=MOSI
 *   PE4  = ICM_CS  (active-LOW, software-controlled)
 *   PC13 = ICM_INT1 (EXTI, configure as RISING edge in CubeMX)
 *
 * !! SPI MODE — MUST change in CubeMX before use !!
 *   ICM-45686 requires Mode 3: CPOL=High, CPHA=2Edge
 *   (data latched on rising edge, clock idles high — DS §10.5)
 *   Current project has Mode 0. Change MX_SPI4_Init:
 *       CLKPolarity = SPI_POLARITY_HIGH
 *       CLKPhase    = SPI_PHASE_2EDGE
 *
 * Configured for sounding rocketry (>25 g launch, ≤2000 dps roll):
 *   Accel : ±32 g,    400 Hz, Low-Noise mode
 *   Gyro  : ±2000 dps, 400 Hz, Low-Noise mode
 *
 * Default data endianness on the ICM-45686 is Little Endian (DS §15).
 * The driver reads data in that default mode — no extra config needed.
 */

#ifndef ICMSPI_H
#define ICMSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

/* -----------------------------------------------------------------------
 * WHO_AM_I expected value (DS §17.79)
 * ----------------------------------------------------------------------- */
#define ICM45686_WHO_AM_I_VAL   0xE9U

/* -----------------------------------------------------------------------
 * Accel sensitivity: ±32 g → 32768 / 32 = 1024 LSB/g
 * Gyro  sensitivity: ±2000 dps → 32768 / 2000 = 16.384 LSB/dps
 * ----------------------------------------------------------------------- */
#define ICM45686_ACCEL_SENS     1024.0f      /* LSB / g              */
#define ICM45686_GYRO_SENS      16.384f      /* LSB / dps            */

/* -----------------------------------------------------------------------
 * Gravity compensation note:
 * The driver returns specific force in the BODY frame — exactly what the
 * sensor measures. Do NOT subtract gravity here.
 * Gravity (9.80665 m/s²) always acts in the GLOBAL frame (straight down).
 * To remove it you must know the rocket's attitude and rotate the gravity
 * vector into the body frame first. Do that in your navigation layer.
 * ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 * Data structures
 * ----------------------------------------------------------------------- */

/* Raw 16-bit two's-complement output straight from registers */
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t temperature;
} ICM45686_RawData;

/* Scaled physical units */
typedef struct {
    float accel_x_mss;    /* m/s²  (positive = sensor +X direction) */
    float accel_y_mss;
    float accel_z_mss;
    float gyro_x_rads;   /* rad/s (right-hand positive)             */
    float gyro_y_rads;
    float gyro_z_rads;
    float temperature_c; /* °C                                      */
} ICM45686_Data;

/* -----------------------------------------------------------------------
 * Driver status
 * ----------------------------------------------------------------------- */
typedef enum {
    ICM45686_FAILED = 0,
    ICM45686_READY  = 1,
} ICM45686_State;

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

/**
 * @brief  Soft-reset and initialise the ICM-45686.
 *         Verifies WHO_AM_I before writing any config.
 * @param  hspi    Pointer to SPI handle  (use &hspi4 in this project)
 * @param  csPort  GPIO port of chip-select pin  (GPIOE)
 * @param  csPin   GPIO pin  of chip-select pin  (GPIO_PIN_4)
 * @retval ICM45686_READY  — sensor responded correctly and is configured
 *         ICM45686_FAILED — WHO_AM_I mismatch (check wiring / SPI mode)
 */
ICM45686_State ICM45686_Init(SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef      *csPort,
                              uint16_t           csPin);

/**
 * @brief  Poll INT1_STATUS0 for the Data-Ready flag.
 *         Reading the status register clears the flag (R/C register).
 * @retval 1 = new sample available,  0 = not yet
 */
uint8_t ICM45686_DataReady(void);

/**
 * @brief  Burst-read all sensor outputs in a single SPI transaction and
 *         update the internal data cache.
 *         Call this when DataReady() == 1, or unconditionally each loop.
 */
void ICM45686_Update(void);

/* Scaled getters — all valid after ICM45686_Update() */
float ICM45686_GetAccelX_mss(void);
float ICM45686_GetAccelY_mss(void);
float ICM45686_GetAccelZ_mss(void);
float ICM45686_GetGyroX_rads(void);
float ICM45686_GetGyroY_rads(void);
float ICM45686_GetGyroZ_rads(void);
float ICM45686_GetTemperature_C(void);

/**
 * @brief  Copy the latest raw register values into caller's struct.
 *         Useful for logging or computing accel magnitude without the
 *         m/s² conversion overhead.
 */
void ICM45686_GetRaw(ICM45686_RawData *out);

/**
 * @brief  Copy all scaled values into caller's struct in one call.
 *         Most efficient way to snapshot a full IMU frame.
 */
void ICM45686_GetScaled(ICM45686_Data *out);

#ifdef __cplusplus
}
#endif

#endif /* ICMSPI_H */
