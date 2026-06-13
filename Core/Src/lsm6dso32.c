/*
 * lsm6dso32.c
 *
 * Polling-mode I2C driver for the ST LSM6DSO32.
 * See lsm6dso32.h for usage.
 */

#include "lsm6dso32.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── Register map (datasheet Table 17) ─────────────────────────────────── */
#define REG_WHO_AM_I   0x0F
#define REG_CTRL1_XL   0x10   /* accel ODR + FS                             */
#define REG_CTRL2_G    0x11   /* gyro  ODR + FS                             */
#define REG_CTRL3_C    0x12   /* SW_RESET | BDU | IF_INC                    */
#define REG_STATUS     0x1E
#define REG_OUT_TEMP_L 0x20   /* temperature LSB (burst base for temp only) */
#define REG_OUTX_L_G   0x22   /* gyro  X LSB — burst base for gyro + accel  */

/* STATUS_REG bits */
#define STATUS_XLDA    0x01   /* accel data available */
#define STATUS_GDA     0x02   /* gyro  data available */

/* CTRL3_C bits */
#define CTRL3_SW_RESET 0x01
#define CTRL3_IF_INC   0x04   /* auto-increment register address on burst    */
#define CTRL3_BDU      0x40   /* block data update — prevents torn reads     */

#define LSM6_I2C_TIMEOUT  10u

/* ── mg/LSB for each accel FS (datasheet Table 2) ─────────────────────── */
static const float XL_SENS_MG[4] = {
    0.122f,   /* ±4g  → LSM6_FS_XL_4G  = 0 */
    0.976f,   /* ±32g → LSM6_FS_XL_32G = 1 */
    0.244f,   /* ±8g  → LSM6_FS_XL_8G  = 2 */
    0.488f,   /* ±16g → LSM6_FS_XL_16G = 3 */
};

/* mdps/LSB for gyro FS values 0–3; index 4 = 125 dps special case */
static const float G_SENS_MDPS[5] = {
    8.75f,    /* ±250  dps → LSM6_FS_G_250DPS  = 0 */
    17.50f,   /* ±500  dps → LSM6_FS_G_500DPS  = 1 */
    35.0f,    /* ±1000 dps → LSM6_FS_G_1000DPS = 2 */
    70.0f,    /* ±2000 dps → LSM6_FS_G_2000DPS = 3 */
    4.375f,   /* ±125  dps → LSM6_FS_G_125DPS  = 4 (bit 4 set in CTRL2_G)  */
};

/* ── Low-level helpers ──────────────────────────────────────────────────── */

static LSM6_Status reg_write(LSM6DSO32_t *dev, uint8_t reg, uint8_t val)
{
    if (HAL_I2C_Mem_Write(dev->hi2c, dev->addr, reg,
                          I2C_MEMADD_SIZE_8BIT, &val, 1,
                          LSM6_I2C_TIMEOUT) != HAL_OK)
        return LSM6_ERR_I2C;
    return LSM6_OK;
}

static LSM6_Status reg_read(LSM6DSO32_t *dev, uint8_t reg,
                             uint8_t *buf, uint16_t n)
{
    if (HAL_I2C_Mem_Read(dev->hi2c, dev->addr, reg,
                         I2C_MEMADD_SIZE_8BIT, buf, n,
                         LSM6_I2C_TIMEOUT) != HAL_OK)
        return LSM6_ERR_I2C;
    return LSM6_OK;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

LSM6_Status LSM6DSO32_Init(LSM6DSO32_t *dev, I2C_HandleTypeDef *hi2c,
                            uint8_t addr, LSM6_ODR odr,
                            LSM6_FS_XL fs_xl, LSM6_FS_G fs_g)
{
    dev->hi2c  = hi2c;
    dev->addr  = addr;
    dev->odr   = odr;
    dev->fs_xl = fs_xl;
    dev->fs_g  = fs_g;

    /* Verify device identity */
    uint8_t id = 0;
    LSM6_Status s;
    if ((s = reg_read(dev, REG_WHO_AM_I, &id, 1)) != LSM6_OK) return s;
    if (id != LSM6DSO32_WHOAMI_VAL) return LSM6_ERR_WHOAMI;

    /* Software reset — clears all registers to defaults */
    if ((s = reg_write(dev, REG_CTRL3_C, CTRL3_SW_RESET)) != LSM6_OK) return s;
    HAL_Delay(10);   /* datasheet: wait for reset to complete (~5 ms) */

    /* Enable auto-increment + block data update */
    if ((s = reg_write(dev, REG_CTRL3_C, CTRL3_IF_INC | CTRL3_BDU)) != LSM6_OK) return s;

    /* Gyro: ODR | FS[2:1] | FS_125(bit4)
     * CTRL2_G [7:4]=ODR, [3:2]=FS_G[1:0], [1]=FS_125, [0]=0  */
    uint8_t ctrl2;
    if (fs_g == LSM6_FS_G_125DPS) {
        ctrl2 = (uint8_t)((odr << 4) | 0x02);  /* FS[1:0]=00, FS_125=1 */
    } else {
        ctrl2 = (uint8_t)((odr << 4) | ((fs_g & 0x03) << 2));
    }
    if ((s = reg_write(dev, REG_CTRL2_G, ctrl2)) != LSM6_OK) return s;

    /* Accel: ODR | FS[1:0]
     * CTRL1_XL [7:4]=ODR, [3:2]=FS_XL, [1:0]=0 */
    uint8_t ctrl1 = (uint8_t)((odr << 4) | ((fs_xl & 0x03) << 2));
    if ((s = reg_write(dev, REG_CTRL1_XL, ctrl1)) != LSM6_OK) return s;

    return LSM6_OK;
}

LSM6_Status LSM6DSO32_DataReady(LSM6DSO32_t *dev, uint8_t *ready)
{
    uint8_t sta;
    LSM6_Status s = reg_read(dev, REG_STATUS, &sta, 1);
    if (s != LSM6_OK) return s;
    *ready = ((sta & (STATUS_XLDA | STATUS_GDA)) == (STATUS_XLDA | STATUS_GDA))
             ? 1u : 0u;
    return LSM6_OK;
}

LSM6_Status LSM6DSO32_Update(LSM6DSO32_t *dev)
{
    /* Check data-ready before committing to the burst read */
    uint8_t rdy = 0;
    LSM6_Status s = LSM6DSO32_DataReady(dev, &rdy);
    if (s != LSM6_OK) return s;
    if (!rdy) return LSM6_ERR_I2C;

    /* Burst-read: OUTX_L_G → OUTZ_H_A (0x22–0x2D), 12 bytes
     * Layout: [Gx_L Gx_H Gy_L Gy_H Gz_L Gz_H Ax_L Ax_H Ay_L Ay_H Az_L Az_H] */
    uint8_t buf[12];
    if ((s = reg_read(dev, REG_OUTX_L_G, buf, 12)) != LSM6_OK) return s;

    int16_t raw_gx = (int16_t)((buf[1]  << 8) | buf[0]);
    int16_t raw_gy = (int16_t)((buf[3]  << 8) | buf[2]);
    int16_t raw_gz = (int16_t)((buf[5]  << 8) | buf[4]);
    int16_t raw_ax = (int16_t)((buf[7]  << 8) | buf[6]);
    int16_t raw_ay = (int16_t)((buf[9]  << 8) | buf[8]);
    int16_t raw_az = (int16_t)((buf[11] << 8) | buf[10]);

    /* Accel sensitivity index is just fs_xl (0–3), table lookup */
    float xl_sens_mss = XL_SENS_MG[dev->fs_xl & 0x03] * 9.80665f / 1000.0f;

    dev->accel_x_mss = raw_ax * xl_sens_mss;
    dev->accel_y_mss = raw_ay * xl_sens_mss;
    dev->accel_z_mss = raw_az * xl_sens_mss;

    /* Gyro sensitivity: 125 dps special case uses index 4 */
    uint8_t g_idx = (dev->fs_g == LSM6_FS_G_125DPS) ? 4u : (uint8_t)(dev->fs_g & 0x03);
    float g_sens_rads = G_SENS_MDPS[g_idx] * (float)M_PI / (180000.0f);

    dev->gyro_x_rads = raw_gx * g_sens_rads;
    dev->gyro_y_rads = raw_gy * g_sens_rads;
    dev->gyro_z_rads = raw_gz * g_sens_rads;

    /* Temperature: separate 2-byte read (0x20–0x21).
     * 256 LSB/°C, 0 = 25 °C. */
    uint8_t tb[2];
    if (reg_read(dev, REG_OUT_TEMP_L, tb, 2) == LSM6_OK) {
        int16_t raw_t = (int16_t)((tb[1] << 8) | tb[0]);
        dev->temp_c = 25.0f + (float)raw_t / 256.0f;
    }

    return LSM6_OK;
}
