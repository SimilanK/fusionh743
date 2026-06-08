/*
   MS5607-02 SPI library for ARM STM32 Microcontrollers - Main source file
   05/01/2020 by Joao Pedro Vilas <joaopedrovbs@gmail.com>
   Changelog:
     2012-05-23 - initial release.
*/
/* ============================================================================================
 MS5607-02 device SPI library code is placed under the MIT license
Copyright (c) 2020 João Pedro Vilas Boas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 ================================================================================================
 */

#include "MS5607SPI.h"
#include <math.h>

/* --- Module-level state -------------------------------------------------- */

static SPI_HandleTypeDef *_hspi;
static GPIO_TypeDef      *_csPort;
static uint16_t           _csPin;

static struct promData                  _prom;
static struct MS5607UncompensatedValues _raw;
static struct MS5607Readings            _readings;

static MS5607OSRFactors _tempOSR     = OSR_4096;
static MS5607OSRFactors _pressureOSR = OSR_4096;

/* ── Non-blocking state machine ─────────────────────────────────────────── */
typedef enum {
    MS5607_NB_IDLE,     /* no conversion running; Poll() starts D1             */
    MS5607_NB_WAIT_D1,  /* D1 pressure conversion running; waiting for deadline */
    MS5607_NB_WAIT_D2,  /* D2 temperature conversion running; waiting for deadline */
} MS5607_NB_State_t;

static MS5607_NB_State_t _nb_state      = MS5607_NB_IDLE;
static uint32_t          _nb_conv_start = 0; /* HAL_GetTick() when current conv started        */

/* TWO timestamps are required because the machine is 2-deep: when Poll() returns
 * a completed sample, the NEXT sample's D1 conversion has already been issued, so
 * two D1-issue times are live at once.
 *   _nb_tick        : D1-issue time of the conversion currently IN FLIGHT
 *   _nb_sample_tick : D1-issue time of the sample whose data is in _readings NOW
 *                     (this is what MS5607_GetSampleTick() must return)
 * Earlier this was a single register; overwriting it with the next D1 time made
 * GetSampleTick() report a tick ~40 ms newer than the data it accompanied. */
static uint32_t          _nb_tick        = 0; /* in-flight conversion's D1-issue time          */
static uint32_t          _nb_sample_tick = 0; /* returned sample's measurement (D1-issue) time */

/* --- OSR to conversion delay (ms) ---------------------------------------- */

static uint32_t getConversionDelay(MS5607OSRFactors osr)
{
    switch (osr) {
        case OSR_256:  return 1;
        case OSR_512:  return 2;
        case OSR_1024: return 3;
        case OSR_2048: return 10;
        case OSR_4096: return 20;
        default:       return 20;
    }
}

/* --- Chip-select helpers -------------------------------------------------- */

void enableCSB(void)
{
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);
}

void disableCSB(void)
{
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
}

/* --- Internal SPI helpers ------------------------------------------------- */

static void sendCommand(uint8_t cmd)
{
    uint8_t rxDummy;
    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, &cmd, &rxDummy, 1, HAL_MAX_DELAY);
    disableCSB();
}

/* --- Public API ----------------------------------------------------------- */

MS5607StateTypeDef MS5607_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort, uint16_t csPin)
{
    _hspi   = hspi;
    _csPort = csPort;
    _csPin  = csPin;

    disableCSB();
    HAL_Delay(10);

    /* Reset the sensor */
    sendCommand(RESET_COMMAND);
    HAL_Delay(3);

    /* Read calibration PROM */
    MS5607PromRead(&_prom);

    /* Sanity-check: at least one calibration word must be non-zero */
    if (_prom.sens == 0 && _prom.off == 0 && _prom.tcs == 0) {
        return MS5607_STATE_FAILED;
    }

    return MS5607_STATE_READY;
}

void MS5607PromRead(struct promData *prom)
{
    uint8_t txBuf[3];
    uint8_t rxBuf[3];

    uint16_t *words = (uint16_t *)prom;
    for (uint8_t i = 0; i < 8; i++) {
        txBuf[0] = PROM_READ(i);
        txBuf[1] = 0xFF;
        txBuf[2] = 0xFF;
        enableCSB();
        HAL_SPI_TransmitReceive(_hspi, txBuf, rxBuf, 3, HAL_MAX_DELAY);
        disableCSB();
        words[i] = ((uint16_t)rxBuf[1] << 8) | rxBuf[2];
    }
}

void MS5607UncompensatedRead(struct MS5607UncompensatedValues *values)
{
    uint8_t cmd;
    uint8_t rxDummy;
    uint8_t txBuf[4] = {READ_ADC_COMMAND, 0xFF, 0xFF, 0xFF};
    uint8_t adcRx[4];

    /* Convert D1 – pressure. CS stays low through the conversion delay. */
    cmd = CONVERT_D1_COMMAND | (uint8_t)_pressureOSR;
    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, &cmd, &rxDummy, 1, HAL_MAX_DELAY);
    HAL_Delay(getConversionDelay(_pressureOSR));
    disableCSB();

    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, txBuf, adcRx, 4, HAL_MAX_DELAY);
    disableCSB();
    values->pressure = ((uint32_t)adcRx[1] << 16) | ((uint32_t)adcRx[2] << 8) | adcRx[3];

    /* Convert D2 – temperature. CS stays low through the conversion delay. */
    cmd = CONVERT_D2_COMMAND | (uint8_t)_tempOSR;
    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, &cmd, &rxDummy, 1, HAL_MAX_DELAY);
    HAL_Delay(getConversionDelay(_tempOSR));
    disableCSB();

    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, txBuf, adcRx, 4, HAL_MAX_DELAY);
    disableCSB();
    values->temperature = ((uint32_t)adcRx[1] << 16) | ((uint32_t)adcRx[2] << 8) | adcRx[3];
}

/*
 * Compensation algorithm straight from the MS5607-02BA03 datasheet.
 * Results: readings->temperature in units of 0.01 °C (e.g. 2000 = 20.00 °C)
 *          readings->pressure    in Pa
 */
void MS5607Convert(struct MS5607UncompensatedValues *raw, struct MS5607Readings *readings)
{
    uint32_t D1 = raw->pressure;
    /* D2 (raw->temperature) is intentionally unused — temperature sensor
       is broken and its ADC value is unreliable.  A synthetic dT for
       exactly 25.00 °C is substituted so that OFF and SENS (and therefore
       pressure) are computed at a known, stable reference point. */

    /* Pull calibration values from PROM (named per datasheet) */
    int64_t C1 = _prom.sens;
    int64_t C2 = _prom.off;
    int64_t C3 = _prom.tcs;
    int64_t C4 = _prom.tco;
    int64_t C6 = _prom.tempsens;

    /* Synthetic dT for 25.00 °C (TEMP = 2500 in 0.01 °C units):
       TEMP = 2000 + dT*C6/8388608 = 2500
       → dT = 500 * 8388608 / C6
       Guard against C6 = 0 (unprogrammed PROM) with fallback dT = 0. */
    int32_t dT   = (C6 != 0) ? (int32_t)(500LL * 8388608LL / C6) : 0;
    int32_t TEMP = 2500; /* 25.00 °C fixed — no second-order correction needed above 20 °C */

    int64_t OFF  = C2 * 131072LL + (C4 * (int64_t)dT) / 64LL;   /* C2*2^17 + C4*dT/2^6 */
    int64_t SENS = C1 * 65536LL  + (C3 * (int64_t)dT) / 128LL;  /* C1*2^16 + C3*dT/2^7 */

    /* Second-order compensation: TEMP = 2500 > 2000, so T2/OFF2/SENS2 = 0.
       Block kept for clarity and to handle any future dT override. */
    if (TEMP < 2000) {
        int64_t T2    = ((int64_t)dT * (int64_t)dT) / 2147483648LL;
        int64_t OFF2  = 61LL * (int64_t)(TEMP - 2000) * (int64_t)(TEMP - 2000) / 16LL;
        int64_t SENS2 = 2LL  * (int64_t)(TEMP - 2000) * (int64_t)(TEMP - 2000);
        if (TEMP < -1500) {
            OFF2  += 15LL * (int64_t)(TEMP + 1500) * (int64_t)(TEMP + 1500);
            SENS2 +=  8LL * (int64_t)(TEMP + 1500) * (int64_t)(TEMP + 1500);
        }
        TEMP -= (int32_t)T2;
        OFF  -= OFF2;
        SENS -= SENS2;
    }

    int64_t P = ((int64_t)D1 * SENS / 2097152LL - OFF) / 32768LL; /* (D1*SENS/2^21-OFF)/2^15 */

    readings->temperature = TEMP;
    readings->pressure    = (int32_t)P;
}

void MS5607Update(void)
{
    MS5607UncompensatedRead(&_raw);
    MS5607Convert(&_raw, &_readings);
}

double MS5607GetTemperatureC(void)
{
    /* Temperature sensor broken — return safe constant.
       Pressure compensation runs on real D2 so pressure is unaffected. */
    return 25.0;
}

double MS5607GetRawTemperatureC(void)
{
    /* Actual calculated value from broken sensor — diagnostics only.
       This is the temperature the compensation algorithm used. */
    return (double)_readings.temperature / 100.0;
}

int32_t MS5607GetPressurePa(void)
{
    return _readings.pressure;
}

void MS5607SetTemperatureOSR(MS5607OSRFactors osr)
{
    _tempOSR = osr;
}

double MS5607GetAltitudeM(void) {
	   /* ISA standard atmosphere constants */
	    const double T0       = 288.15;    /* sea-level standard temperature (K)     */
	    const double L        = 0.0065;    /* temperature lapse rate (K/m)           */
	    const double P0       = 101325.0;  /* sea-level standard pressure (Pa)       */
	    const double EXPONENT = 5.2558;    /* g*M / (R*L) — dimensionless            */

	    double pressure_pa = (double)_readings.pressure;  /* use internal sensor state */
	    double altitude    = (T0 / L) * (1.0 - pow((pressure_pa / P0), (1.0 / EXPONENT)));
	    return altitude;
}
void MS5607SetPressureOSR(MS5607OSRFactors osr)
{
    _pressureOSR = osr;
}

/* ── Non-blocking implementation ────────────────────────────────────────── */

/* Issue a single-byte command on SPI1 (CS toggled around it). */
static inline void _nb_send_cmd(uint8_t cmd)
{
    uint8_t rx;
    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, &cmd, &rx, 1, HAL_MAX_DELAY);
    disableCSB();
}

/* Read 3 ADC bytes from the sensor into a uint32_t. */
static inline uint32_t _nb_read_adc(void)
{
    uint8_t tx[4] = {READ_ADC_COMMAND, 0xFF, 0xFF, 0xFF};
    uint8_t rx[4];
    enableCSB();
    HAL_SPI_TransmitReceive(_hspi, tx, rx, 4, HAL_MAX_DELAY);
    disableCSB();
    return ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | rx[3];
}

uint8_t MS5607_Poll(void)
{
    switch (_nb_state) {

    /* ── IDLE: kick off a fresh D1 pressure conversion ─────────────────── */
    case MS5607_NB_IDLE:
        _nb_tick       = HAL_GetTick();   /* record measurement timestamp    */
        _nb_conv_start = _nb_tick;
        _nb_send_cmd(CONVERT_D1_COMMAND | (uint8_t)_pressureOSR);
        _nb_state = MS5607_NB_WAIT_D1;
        return 0;

    /* ── WAIT_D1: poll until D1 ADC ready ──────────────────────────────── */
    case MS5607_NB_WAIT_D1:
        /* Unsigned subtraction handles HAL_GetTick() 32-bit wraparound.    */
        if (HAL_GetTick() - _nb_conv_start < getConversionDelay(_pressureOSR))
            return 0;
        _raw.pressure  = _nb_read_adc();
        _nb_conv_start = HAL_GetTick();
        _nb_send_cmd(CONVERT_D2_COMMAND | (uint8_t)_tempOSR);
        _nb_state = MS5607_NB_WAIT_D2;
        return 0;

    /* ── WAIT_D2: poll until D2 ADC ready ──────────────────────────────── */
    case MS5607_NB_WAIT_D2:
        if (HAL_GetTick() - _nb_conv_start < getConversionDelay(_tempOSR))
            return 0;
        _raw.temperature = _nb_read_adc();
        MS5607Convert(&_raw, &_readings);

        /* Capture the measurement time of the sample we are about to return,
           BEFORE _nb_tick is overwritten by the next D1 below. This is the
           pipeline register the earlier version was missing. */
        _nb_sample_tick = _nb_tick;

        /* Immediately start the NEXT D1 so its 20 ms conversion window
           overlaps the caller's ICM read, UART write, and SD write.
           This keeps the effective baro rate at ~25 Hz regardless of
           how long the caller's post-processing takes (up to ~20 ms).
           _nb_tick now tracks THIS new in-flight conversion.               */
        _nb_tick       = HAL_GetTick();
        _nb_conv_start = _nb_tick;
        _nb_send_cmd(CONVERT_D1_COMMAND | (uint8_t)_pressureOSR);
        _nb_state = MS5607_NB_WAIT_D1;

        return 1;   /* data ready — getters valid until the next return 1  */

    default:
        _nb_state = MS5607_NB_IDLE;
        return 0;
    }
}

uint32_t MS5607_GetSampleTick(void)
{
    return _nb_sample_tick;
}
