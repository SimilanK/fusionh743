/*
 * mpl3115a2.c
 *
 *  Created on: Jun 13, 2026
 *      Author: simil
 */

/*
 * mpl3115a2.c
 *
 * Polling-mode driver for the NXP MPL3115A2. See mpl3115a2.h for usage.
 */

#include "mpl3115a2.h"
#include <math.h>
/* ---- Register map (datasheet Table 10) ---- */
#define REG_STATUS      0x00
#define REG_OUT_P_MSB   0x01   /* auto-increments through OUT_T_LSB (0x05) */
#define REG_WHO_AM_I    0x0C
#define REG_PT_DATA_CFG 0x13
#define REG_BAR_IN_MSB  0x14
#define REG_BAR_IN_LSB  0x15
#define REG_CTRL_REG1   0x26

/* STATUS / DR_STATUS bits */
#define STATUS_PTDR     0x08   /* pressure/temp data ready */

/* PT_DATA_CFG: DREM | PDEFE | TDEFE -> enable all data-ready event flags */
#define PT_DATA_CFG_ALL 0x07

/* CTRL_REG1 bits */
#define CTRL1_SBYB      0x01   /* 0=standby, 1=active */
#define CTRL1_ALT       0x80   /* 0=barometer, 1=altimeter */

#define MPL_I2C_TIMEOUT 100u

/* ---- low-level helpers ---- */

static MPL_Status reg_write(MPL3115A2_t *dev, uint8_t reg, uint8_t val) {
    if (HAL_I2C_Mem_Write(dev->hi2c, MPL3115A2_I2C_ADDR, reg,
                          I2C_MEMADD_SIZE_8BIT, &val, 1, MPL_I2C_TIMEOUT) != HAL_OK)
        return MPL_ERR_I2C;
    return MPL_OK;
}

static MPL_Status reg_read(MPL3115A2_t *dev, uint8_t reg, uint8_t *buf, uint16_t n) {
    if (HAL_I2C_Mem_Read(dev->hi2c, MPL3115A2_I2C_ADDR, reg,
                         I2C_MEMADD_SIZE_8BIT, buf, n, MPL_I2C_TIMEOUT) != HAL_OK)
        return MPL_ERR_I2C;
    return MPL_OK;
}

/* Build CTRL_REG1 value for current mode+osr. SBYB added by caller. */
static uint8_t ctrl1_base(MPL3115A2_t *dev) {
    uint8_t v = (uint8_t)(dev->osr) << 3;   /* OS[2:0] -> bits 5:3 */
    if (dev->mode == MPL_MODE_ALTIMETER) v |= CTRL1_ALT;
    return v;
}

/* Apply mode/osr: must drop to standby to change ALT/OS, then re-activate. */
static MPL_Status apply_config(MPL3115A2_t *dev) {
    uint8_t base = ctrl1_base(dev);
    MPL_Status s;
    if ((s = reg_write(dev, REG_CTRL_REG1, base)) != MPL_OK) return s;          /* standby */
    if ((s = reg_write(dev, REG_PT_DATA_CFG, PT_DATA_CFG_ALL)) != MPL_OK) return s;
    if ((s = reg_write(dev, REG_CTRL_REG1, base | CTRL1_SBYB)) != MPL_OK) return s; /* active */
    return MPL_OK;
}

/* ---- public API ---- */

MPL_Status MPL3115A2_Init(MPL3115A2_t *dev, I2C_HandleTypeDef *hi2c,
                          MPL_Mode mode, MPL_OSR osr) {
    uint8_t id = 0;
    dev->hi2c = hi2c;
    dev->mode = mode;
    dev->osr  = osr;

    if (reg_read(dev, REG_WHO_AM_I, &id, 1) != MPL_OK) return MPL_ERR_I2C;
    if (id != MPL3115A2_WHOAMI_VAL) return MPL_ERR_WHOAMI;

    return apply_config(dev);
}

MPL_Status MPL3115A2_DataReady(MPL3115A2_t *dev, uint8_t *ready) {
    uint8_t sta;
    MPL_Status s = reg_read(dev, REG_STATUS, &sta, 1);
    if (s != MPL_OK) return s;
    *ready = (sta & STATUS_PTDR) ? 1u : 0u;
    return MPL_OK;
}

MPL_Status MPL3115A2_ReadAltitude(MPL3115A2_t *dev, float *altitude_m, float *temp_c) {
    uint8_t sta, b[5];
    MPL_Status s = reg_read(dev, REG_STATUS, &sta, 1);
    if (s != MPL_OK) return s;
    if (!(sta & STATUS_PTDR)) return MPL_ERR_I2C;   /* no new sample yet */

    /* burst-read OUT_P_MSB..OUT_T_LSB (auto-increment), clears PTDR */
    s = reg_read(dev, REG_OUT_P_MSB, b, 5);
    if (s != MPL_OK) return s;

    /* Altitude: Q16.4 signed. b[0:1] = signed meters, b[2] high nibble = /16 */
    int16_t alt_int = (int16_t)((b[0] << 8) | b[1]);
    *altitude_m = (float)alt_int + ((b[2] >> 4) / 16.0f);

    /* Temperature: Q8.4 signed. b[3] = signed C, b[4] high nibble = /16 */
    *temp_c = (float)((int8_t)b[3]) + ((b[4] >> 4) / 16.0f);
    return MPL_OK;
}

MPL_Status MPL3115A2_ReadPressure(MPL3115A2_t *dev, float *pressure_pa, float *temp_c) {
    uint8_t sta, b[5];
    MPL_Status s = reg_read(dev, REG_STATUS, &sta, 1);
    if (s != MPL_OK) return s;
    if (!(sta & STATUS_PTDR)) return MPL_ERR_I2C;

    s = reg_read(dev, REG_OUT_P_MSB, b, 5);
    if (s != MPL_OK) return s;

    /* Pressure: Q18.2 unsigned. 18 integer bits across b[0],b[1],b[2][7:6];
     * b[2][5:4] = fractional quarters. */
    uint32_t p_int = ((uint32_t)b[0] << 10) | ((uint32_t)b[1] << 2) | (b[2] >> 6);
    float    p_frac = ((b[2] >> 4) & 0x03) / 4.0f;
    *pressure_pa = (float)p_int + p_frac;

    *temp_c = (float)((int8_t)b[3]) + ((b[4] >> 4) / 16.0f);
    return MPL_OK;
}

MPL_Status MPL3115A2_SetOSR(MPL3115A2_t *dev, MPL_OSR osr) {
    dev->osr = osr;
    return apply_config(dev);
}

MPL_Status MPL3115A2_SetMode(MPL3115A2_t *dev, MPL_Mode mode) {
    dev->mode = mode;
    return apply_config(dev);
}

MPL_Status MPL3115A2_SetSeaLevelPressure(MPL3115A2_t *dev, uint32_t pressure_pa) {
    /* Register is unsigned 16-bit in 2 Pa units. */
    uint16_t raw = (uint16_t)((pressure_pa + 1) / 2);   /* round to nearest 2 Pa */
    MPL_Status s;
    if ((s = reg_write(dev, REG_BAR_IN_MSB, (uint8_t)(raw >> 8))) != MPL_OK) return s;
    if ((s = reg_write(dev, REG_BAR_IN_LSB, (uint8_t)(raw & 0xFF))) != MPL_OK) return s;
    return MPL_OK;
}

float MPL3115A2_AltitudeFromPressure(float pressure_pa) {
    /* ISA standard atmosphere (matches MS5607 path) */
    const float T0       = 288.15f;    /* sea-level standard temperature (K) */
    const float L        = 0.0065f;    /* temperature lapse rate (K/m)       */
    const float P0       = 101325.0f;  /* sea-level standard pressure (Pa)   */
    const float EXPONENT = 5.2558f;    /* g*M / (R*L)                        */

    return (T0 / L) * (1.0f - powf(pressure_pa / P0, 1.0f / EXPONENT));
}
