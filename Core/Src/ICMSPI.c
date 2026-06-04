/*
 * ICMSPI.c  —  ICM-45686 6-axis IMU driver (SPI, polling)
 *
 * Created : Jun 2, 2026
 * Author  : simil
 *
 * All register addresses and bit fields verified against:
 *   TDK Datasheet DS-000577 Rev 1.0
 *
 * Configuration baked into this driver (suited for sounding rocketry):
 *   Accel  ±32 g      @ 400 Hz LN   — handles >25 g launch loads
 *   Gyro   ±2000 dps  @ 400 Hz LN   — handles moderate roll rates
 *
 * Data endianness:
 *   ICM-45686 outputs Little Endian by default (DS §15).
 *   Burst read from address 0x00 returns:
 *     [AX_L][AX_H][AY_L][AY_H][AZ_L][AZ_H]
 *     [GX_L][GX_H][GY_L][GY_H][GZ_L][GZ_H]
 *     [TEMP_L][TEMP_H]
 *   Assembly: raw = (H_byte << 8) | L_byte
 */

#include "ICMSPI.h"

/* -----------------------------------------------------------------------
 * Register addresses — User Bank 0  (DS §16.1)
 * ----------------------------------------------------------------------- */
#define REG_ACCEL_DATA_X   0x00U  /* burst start; 12 bytes accel+gyro, then 2 bytes temp */
#define REG_PWR_MGMT0      0x10U  /* [3:2]=GYRO_MODE  [1:0]=ACCEL_MODE                  */
#define REG_INT1_CONFIG0   0x16U  /* [2]=DRDY interrupt enable on INT1                  */
#define REG_INT1_CONFIG2   0x18U  /* [2]=drive  [1]=mode  [0]=polarity                  */
#define REG_INT1_STATUS0   0x19U  /* [2]=DRDY flag (R/C — reading clears it)            */
#define REG_ACCEL_CONFIG0  0x1BU  /* [6:4]=ACCEL_UI_FS_SEL  [3:0]=ACCEL_ODR            */
#define REG_GYRO_CONFIG0   0x1CU  /* [7:4]=GYRO_UI_FS_SEL   [3:0]=GYRO_ODR             */
#define REG_WHO_AM_I       0x72U  /* always reads 0xE9                                  */
#define REG_MISC2          0x7FU  /* [1]=SOFT_RST (self-clearing)                       */

/* -----------------------------------------------------------------------
 * Register values — verified from DS bit-field tables
 *
 *  PWR_MGMT0 = 0x0F:
 *    GYRO_MODE  bits[3:2] = 11 → Low-Noise
 *    ACCEL_MODE bits[1:0] = 11 → Low-Noise
 *
 *  ACCEL_CONFIG0 = 0x07:
 *    ACCEL_UI_FS_SEL bits[6:4] = 000 → ±32 g
 *    ACCEL_ODR       bits[3:0] = 0111 → 400 Hz
 *
 *  GYRO_CONFIG0 = 0x17:
 *    GYRO_UI_FS_SEL  bits[7:4] = 0001 → ±2000 dps
 *    GYRO_ODR        bits[3:0] = 0111 → 400 Hz
 *
 *  INT1_CONFIG2 = 0x01:
 *    INT1_DRIVE      bit[2]    = 0    → push-pull
 *    INT1_MODE       bit[1]    = 0    → pulse (auto-clears)
 *    INT1_POLARITY   bit[0]    = 1    → active-high
 *    (configure PC13 EXTI as RISING edge in CubeMX)
 *
 *  INT1_CONFIG0 = 0x04:
 *    INT1_STATUS_EN_DRDY bit[2] = 1   → route DRDY to INT1 pin
 * ----------------------------------------------------------------------- */
#define VAL_PWR_MGMT0      0x0FU
#define VAL_ACCEL_CONFIG0  0x07U  /* ±32 g, 400 Hz */
#define VAL_GYRO_CONFIG0   0x17U  /* ±2000 dps, 400 Hz */
#define VAL_INT1_CONFIG2   0x01U  /* push-pull, pulse, active-high */
#define VAL_INT1_CONFIG0   0x04U  /* enable DRDY on INT1 */

#define SPI_READ_BIT       0x80U
#define SPI_TIMEOUT_MS     5U     /* bounded timeout — never HAL_MAX_DELAY in flight code */

/* -----------------------------------------------------------------------
 * Module-level state
 * ----------------------------------------------------------------------- */
static SPI_HandleTypeDef *_hspi;
static GPIO_TypeDef      *_csPort;
static uint16_t           _csPin;

static ICM45686_RawData  _raw;
static ICM45686_Data     _data;

/* -----------------------------------------------------------------------
 * Chip-select helpers
 * ----------------------------------------------------------------------- */
static inline void cs_low(void)  { HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET); }
static inline void cs_high(void) { HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET); }

/* -----------------------------------------------------------------------
 * SPI error recovery
 * Called when any SPI transfer fails (timeout, error, busy).
 * Resets the HAL state machine so the peripheral is usable again.
 * Does NOT reconfigure the sensor — caller handles that if needed.
 * ----------------------------------------------------------------------- */
static void spi_recover(void)
{
    cs_high();
    HAL_SPI_Abort(_hspi);
    HAL_SPI_Init(_hspi);   /* clears HAL_SPI_STATE_ERROR, re-enables peripheral */
}

/* -----------------------------------------------------------------------
 * Low-level SPI access — all return HAL_StatusTypeDef
 * ----------------------------------------------------------------------- */
static HAL_StatusTypeDef reg_write(uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = { reg & 0x7FU, val };
    uint8_t rx[2];
    HAL_StatusTypeDef status;
    cs_low();
    status = HAL_SPI_TransmitReceive(_hspi, tx, rx, 2, SPI_TIMEOUT_MS);
    cs_high();
    return status;
}

static uint8_t reg_read(uint8_t reg)
{
    uint8_t tx[2] = { reg | SPI_READ_BIT, 0x00U };
    uint8_t rx[2] = { 0U, 0U };
    cs_low();
    HAL_SPI_TransmitReceive(_hspi, tx, rx, 2, SPI_TIMEOUT_MS);
    cs_high();
    return rx[1];
}

/*
 * Burst read: returns HAL_OK on success, error code on failure.
 * rxbuf[0] is the dummy byte; real data starts at rxbuf[1].
 * Caller must size rxbuf to at least (datalen + 1).
 */
static HAL_StatusTypeDef burst_read(uint8_t reg, uint8_t *rxbuf, uint16_t datalen)
{
    uint8_t txbuf[16] = { 0U };
    HAL_StatusTypeDef status;
    txbuf[0] = reg | SPI_READ_BIT;
    cs_low();
    status = HAL_SPI_TransmitReceive(_hspi, txbuf, rxbuf, datalen + 1U, SPI_TIMEOUT_MS);
    cs_high();
    return status;
}

/* -----------------------------------------------------------------------
 * Sensor reconfiguration — writes all config registers without soft reset.
 * Called after a connection loss where the sensor lost power and its
 * registers reverted to reset defaults.
 * ----------------------------------------------------------------------- */
static void sensor_reconfig(void)
{
    reg_write(REG_ACCEL_CONFIG0, VAL_ACCEL_CONFIG0);
    reg_write(REG_GYRO_CONFIG0,  VAL_GYRO_CONFIG0);
    reg_write(REG_PWR_MGMT0,     VAL_PWR_MGMT0);
    HAL_Delay(50);   /* gyro LN settle time */
    reg_write(REG_INT1_CONFIG2,  VAL_INT1_CONFIG2);
    reg_write(REG_INT1_CONFIG0,  VAL_INT1_CONFIG0);
}

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

ICM45686_State ICM45686_Init(SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef      *csPort,
                              uint16_t           csPin)
{
    _hspi   = hspi;
    _csPort = csPort;
    _csPin  = csPin;

    cs_high();
    HAL_Delay(10);

    /* Soft reset via REG_MISC2 bit1 (DS §17.84).  Self-clears after reset. */
    reg_write(REG_MISC2, 0x02U);
    HAL_Delay(5);   /* DS: wait for reset to complete before accessing registers */

    /* Verify device identity before writing any configuration */
    if (reg_read(REG_WHO_AM_I) != ICM45686_WHO_AM_I_VAL) {
        return ICM45686_FAILED;
    }

    /* Configure full-scale range and ODR */
    reg_write(REG_ACCEL_CONFIG0, VAL_ACCEL_CONFIG0); /* ±32 g,    400 Hz */
    reg_write(REG_GYRO_CONFIG0,  VAL_GYRO_CONFIG0);  /* ±2000 dps, 400 Hz */

    /* Power on gyro and accel in Low-Noise mode */
    reg_write(REG_PWR_MGMT0, VAL_PWR_MGMT0);
    HAL_Delay(50);  /* DS: gyro needs ~45 ms to settle from off to LN mode */

    /* INT1: push-pull, pulsed, active-high (change PC13 EXTI to RISING) */
    reg_write(REG_INT1_CONFIG2, VAL_INT1_CONFIG2);

    /* Route Data-Ready event to INT1 pin */
    reg_write(REG_INT1_CONFIG0, VAL_INT1_CONFIG0);

    return ICM45686_READY;
}

uint8_t ICM45686_DataReady(void)
{
    /* INT1_STATUS0 (0x19) bit[2] = DRDY flag.
       Register is R/C — reading it clears the flag (DS §17.24). */
    return (reg_read(REG_INT1_STATUS0) & 0x04U) ? 1U : 0U;
}

void ICM45686_Update(void)
{
    /*
     * Single burst from 0x00 reads 14 bytes (accel + gyro + temp).
     * Default Little Endian layout from hardware (DS §15):
     *
     *   rxbuf[0]       dummy (response to address byte)
     *   rxbuf[1..2]    Accel X  [LSB, MSB]
     *   rxbuf[3..4]    Accel Y  [LSB, MSB]
     *   rxbuf[5..6]    Accel Z  [LSB, MSB]
     *   rxbuf[7..8]    Gyro  X  [LSB, MSB]
     *   rxbuf[9..10]   Gyro  Y  [LSB, MSB]
     *   rxbuf[11..12]  Gyro  Z  [LSB, MSB]
     *   rxbuf[13..14]  Temp     [LSB, MSB]
     *
     * Assembly: value = (MSB << 8) | LSB
     */
    uint8_t rxbuf[15];
    if (burst_read(REG_ACCEL_DATA_X, rxbuf, 14) != HAL_OK)
    {
        /* SPI failed — HAL state machine is stuck.
           1. Reset the SPI peripheral.
           2. Reconfigure the sensor (it may have lost power and reset).
           3. Skip this frame — data from a failed read is garbage. */
        spi_recover();
        sensor_reconfig();
        return;
    }

    _raw.accel_x     = (int16_t)((uint16_t)rxbuf[2]  << 8 | rxbuf[1]);
    _raw.accel_y     = (int16_t)((uint16_t)rxbuf[4]  << 8 | rxbuf[3]);
    _raw.accel_z     = (int16_t)((uint16_t)rxbuf[6]  << 8 | rxbuf[5]);
    _raw.gyro_x      = (int16_t)((uint16_t)rxbuf[8]  << 8 | rxbuf[7]);
    _raw.gyro_y      = (int16_t)((uint16_t)rxbuf[10] << 8 | rxbuf[9]);
    _raw.gyro_z      = (int16_t)((uint16_t)rxbuf[12] << 8 | rxbuf[11]);
    _raw.temperature = (int16_t)((uint16_t)rxbuf[14] << 8 | rxbuf[13]);

    /* Accel: raw / 1024 LSB/g → g → m/s² */
    const float a_scale = 9.80665f / ICM45686_ACCEL_SENS;
    _data.accel_x_mss = (float)_raw.accel_x * a_scale;
    _data.accel_y_mss = (float)_raw.accel_y * a_scale;
    _data.accel_z_mss = (float)_raw.accel_z * a_scale;

    /* NOTE: No gravity subtraction here.
       The sensor measures specific force in the BODY frame.
       Gravity acts in the GLOBAL frame.
       Subtracting a fixed 9.80665 from a body axis is only valid when
       that axis is perfectly aligned with global Z (rocket exactly vertical).
       Proper gravity compensation requires attitude knowledge — apply it
       in the navigation layer after rotating the gravity vector into the
       body frame using your attitude estimate. */

    /* Gyro: raw / 16.384 LSB/dps → dps → rad/s */
    const float g_scale = 0.01745329252f / ICM45686_GYRO_SENS;  /* π/180 / sens */
    _data.gyro_x_rads = (float)_raw.gyro_x * g_scale;
    _data.gyro_y_rads = (float)_raw.gyro_y * g_scale;
    _data.gyro_z_rads = (float)_raw.gyro_z * g_scale;

    /* Temperature: T(°C) = TEMP_DATA / 128 + 25  (DS §17.14) */
    _data.temperature_c = (float)_raw.temperature / 128.0f + 25.0f;
}

float ICM45686_GetAccelX_mss(void)    { return _data.accel_x_mss; }
float ICM45686_GetAccelY_mss(void)    { return _data.accel_y_mss; }
float ICM45686_GetAccelZ_mss(void)    { return _data.accel_z_mss; }
float ICM45686_GetGyroX_rads(void)    { return _data.gyro_x_rads; }
float ICM45686_GetGyroY_rads(void)    { return _data.gyro_y_rads; }
float ICM45686_GetGyroZ_rads(void)    { return _data.gyro_z_rads; }
float ICM45686_GetTemperature_C(void) { return _data.temperature_c; }

void ICM45686_GetRaw(ICM45686_RawData *out)    { if (out) { *out = _raw; } }
void ICM45686_GetScaled(ICM45686_Data *out)    { if (out) { *out = _data; } }
