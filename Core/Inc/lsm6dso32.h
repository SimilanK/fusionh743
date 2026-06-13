/*
 * lsm6dso32.h
 *
 * Polling-mode I2C driver for the ST LSM6DSO32 6-axis IMU
 * (±32 g accel + ±2000 dps gyro).
 * Target: STM32 HAL.  No interrupts, no DMA.
 *
 * Usage:
 *   LSM6DSO32_t imu;
 *   if (LSM6DSO32_Init(&imu, &hi2c2, LSM6DSO32_ADDR_6B,
 *                      LSM6_ODR_104HZ,
 *                      LSM6_FS_XL_32G,
 *                      LSM6_FS_G_2000DPS) != LSM6_OK) { Error_Handler(); }
 *
 *   if (LSM6DSO32_Update(&imu) == LSM6_OK) {
 *       float az = imu.accel_z_mss;
 *       float gz = imu.gyro_z_rads;
 *   }
 */

#ifndef LSM6DSO32_H
#define LSM6DSO32_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

/* 7-bit I2C addresses (shifted to 8-bit form HAL expects).
 * SA0/SDO pin selects: GND → 0x6A, VDD → 0x6B (Adafruit breakout default). */
#define LSM6DSO32_ADDR_6A  (0x6A << 1)
#define LSM6DSO32_ADDR_6B  (0x6B << 1)

#define LSM6DSO32_WHOAMI_VAL  0x6C

typedef enum {
    LSM6_OK = 0,
    LSM6_ERR_I2C,
    LSM6_ERR_WHOAMI
} LSM6_Status;

/* Output data rate — applied to both accel and gyro. */
typedef enum {
    LSM6_ODR_OFF    = 0x00,
    LSM6_ODR_12HZ   = 0x01,
    LSM6_ODR_26HZ   = 0x02,
    LSM6_ODR_52HZ   = 0x03,
    LSM6_ODR_104HZ  = 0x04,   /* 104 Hz — matches 100 Hz loop with margin */
    LSM6_ODR_208HZ  = 0x05,
    LSM6_ODR_416HZ  = 0x06,
    LSM6_ODR_833HZ  = 0x07,
} LSM6_ODR;

/* Accelerometer full-scale range.
 * LSM6DSO32 FS encoding: 00=±4g  01=±32g  10=±8g  11=±16g  (not sequential) */
typedef enum {
    LSM6_FS_XL_4G  = 0x00,   /* 0.122 mg/LSB */
    LSM6_FS_XL_32G = 0x01,   /* 0.976 mg/LSB — recommended for rockets      */
    LSM6_FS_XL_8G  = 0x02,   /* 0.244 mg/LSB */
    LSM6_FS_XL_16G = 0x03,   /* 0.488 mg/LSB */
} LSM6_FS_XL;

/* Gyroscope full-scale range. */
typedef enum {
    LSM6_FS_G_250DPS  = 0x00,   /* 8.75  mdps/LSB */
    LSM6_FS_G_500DPS  = 0x01,   /* 17.50 mdps/LSB */
    LSM6_FS_G_1000DPS = 0x02,   /* 35.0  mdps/LSB */
    LSM6_FS_G_2000DPS = 0x03,   /* 70.0  mdps/LSB — recommended for rockets  */
    LSM6_FS_G_125DPS  = 0x10,   /* 4.375 mdps/LSB (bit 4 set in CTRL2_G)     */
} LSM6_FS_G;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t            addr;    /* 8-bit HAL address (LSM6DSO32_ADDR_6A/6B)  */
    LSM6_ODR           odr;
    LSM6_FS_XL         fs_xl;
    LSM6_FS_G          fs_g;

    /* Last burst-read outputs — updated by LSM6DSO32_Update() */
    float accel_x_mss;
    float accel_y_mss;
    float accel_z_mss;
    float gyro_x_rads;
    float gyro_y_rads;
    float gyro_z_rads;
    float temp_c;
} LSM6DSO32_t;

/* Verify WHO_AM_I, SW-reset, configure ODR + FS, enable BDU + auto-increment. */
LSM6_Status LSM6DSO32_Init(LSM6DSO32_t *dev, I2C_HandleTypeDef *hi2c,
                            uint8_t addr, LSM6_ODR odr,
                            LSM6_FS_XL fs_xl, LSM6_FS_G fs_g);

/* Poll STATUS_REG.  Sets *ready=1 if both XLDA and GDA are set. */
LSM6_Status LSM6DSO32_DataReady(LSM6DSO32_t *dev, uint8_t *ready);

/* Burst-read gyro + accel (12 bytes), convert to SI, store in dev fields.
 * Returns LSM6_ERR_I2C if the sensor reports no new data. */
LSM6_Status LSM6DSO32_Update(LSM6DSO32_t *dev);

#endif /* LSM6DSO32_H */
