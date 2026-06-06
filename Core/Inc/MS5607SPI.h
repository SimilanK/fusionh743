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

#ifndef _MS5607SPI_H_
#define _MS5607SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

/* MS5607 SPI COMMANDS */
#define RESET_COMMAND                 0x1E
#define PROM_READ(address)            (0xA0 | ((address) << 1))         // Macro to change values for the 8 PROM addresses
#define CONVERT_D1_COMMAND            0x40
#define CONVERT_D2_COMMAND            0x50
#define READ_ADC_COMMAND              0x00

/* MS5607 Oversampling Ratio Enumeration*/
typedef enum OSRFactors{
  OSR_256,
  OSR_512=0x02,
  OSR_1024=0x04,
  OSR_2048=0x06,
  OSR_4096=0x08
}MS5607OSRFactors;

/* MS5607 System States Enumeration*/
typedef enum MS5607States{
  MS5607_STATE_FAILED,
  MS5607_STATE_READY
}MS5607StateTypeDef;

/* MS5607 PROM Data Structure */
struct promData{
  uint16_t reserved;
  uint16_t sens;
  uint16_t off;
  uint16_t tcs;
  uint16_t tco;
  uint16_t tref;
  uint16_t tempsens;
  uint16_t crc;
};

/* Uncompensated digital Values */
struct MS5607UncompensatedValues{
  uint32_t  pressure;
  uint32_t  temperature;
};

/* Actual readings */
struct MS5607Readings{
  int32_t  pressure;
  int32_t  temperature;
};

/**
 * @brief  Initializes MS5607 Sensor
 * @param  SPI Handle address
 * @param  GPIO Port Definition
 * @param  GPIO Pin
 * @retval Initialization status:
 * - 0 or MS5607_STATE_FAILED: Was not abe to communicate with sensor
 * - 1 or MS5607_STATE_READY: Sensor initialized OK and ready to use
 */
MS5607StateTypeDef MS5607_Init(SPI_HandleTypeDef *, GPIO_TypeDef *, uint16_t);

/**
 * @brief  Reads MS5607 PROM Content
 * @note   Must be called only on device initialization
 * @param  Address of promData structure instance
 * @retval None
 */
void MS5607PromRead(struct promData *);

/**
 * @brief  Reads uncompensated content from the MS5607 ADC
 * @note   Must be called before every convertion
 * @param  Address of MS5607 UncompensatedValues Structure
 * @retval None
 */
void MS5607UncompensatedRead(struct MS5607UncompensatedValues *);

/**
 * @brief  Converts uncompensated values into real world values using data from @ref promData and @ref MS5607UncompensatedValues
 * @note   Must be called after @ref MS5607UncompensatedRead
 * @param  Address of MS5607UncompensatedValues Structure
 * @param  Address of MS5607Readings Structure
 * @retval None
 */
void MS5607Convert(struct MS5607UncompensatedValues *, struct MS5607Readings *);

/**
 * @brief  Updates the readings from the sensor
 * @note   This function must be called each time you want new values from the sensor
 * @param  None
 * @retval None
 */
void MS5607Update(void);

/**
 * @brief  Gets the temperature reading from the last sensor update
 * @note   This function must be called after an @ref MS5607Update()
 * @param  None
 * @retval Temperature in celsius
 */
/**
 * @note   The internal temperature sensor is broken on this unit.
 *         Always returns 25.0 °C as a safe constant.
 *         Pressure compensation still runs on the real raw D2 value
 *         so pressure readings remain accurate.
 */
double MS5607GetTemperatureC(void);

/**
 * @brief  Returns the ACTUAL calculated temperature from raw D2 data.
 * @note   Diagnostics only — sensor reads incorrectly on this unit.
 *         This is the value used internally for pressure compensation.
 * @retval Actual sensor temperature in °C (may be wrong/unstable)
 */
double MS5607GetRawTemperatureC(void);

/**
 * @brief  Gets the pressure reading from the last sensor update
 * @note   This function must be called after an @ref MS5607Update()
 * @param  None
 * @retval Pressure in Pascal
 */
int32_t MS5607GetPressurePa(void);

/**
 * @brief  Enables the chip select pin
 * @param  None
 * @retval None
 */
double MS5607GetAltitudeM(void);

void enableCSB(void);

/**
 * @brief  Disables the chip select pin
 * @param  None
 * @retval None
 */
void disableCSB(void);

/**
 * @brief  Sets the temperature reading oversamplig ratio
 * @param  OSR factor from 256 to 4096
 * @retval None
 */
void MS5607SetTemperatureOSR(MS5607OSRFactors);

/**
 * @brief  Sets the pressure reading oversamplig ratio
 * @param  OSR factor from 256 to 4096
 * @retval None
 */
void MS5607SetPressureOSR(MS5607OSRFactors);

/* ── Non-blocking (poll-based) interface ─────────────────────────────────────
 *
 * Use these instead of MS5607Update() when the main loop must not block.
 * The state machine advances each time MS5607_Poll() is called and uses
 * HAL_GetTick() deadlines instead of HAL_Delay(), so the CPU is free to
 * run other code (IMU reads, servo, logging) between calls.
 *
 * Typical usage:
 *   while (1) {
 *       if (MS5607_Poll()) {
 *           ts = MS5607_GetSampleTick();   // accurate measurement timestamp
 *           alt = MS5607GetAltitudeM();    // getters valid until next Poll()==1
 *           ...
 *       }
 *       // other work here — no HAL_Delay() needed
 *   }
 *
 * Do NOT mix with MS5607Update() in the same application.
 * ─────────────────────────────────────────────────────────────────────────── */

/**
 * @brief  Advance the non-blocking baro state machine.
 *         Call every main-loop iteration (or as fast as possible).
 *         Internally: IDLE → WAIT_D1 → WAIT_D2 → (kick next D1) → return 1.
 *         When returning 1, the next D1 conversion has already been issued so
 *         the baro conversion overlaps the caller's processing time.
 * @retval 1  New pressure+temperature data available. Read MS5607Get*() now.
 *         0  Conversion still in progress — call again next iteration.
 */
uint8_t  MS5607_Poll(void);

/**
 * @brief  HAL_GetTick() value captured when the D1 pressure conversion command
 *         was issued for the most recently completed sample.
 *         This is the true measurement timestamp — use it instead of calling
 *         HAL_GetTick() after the conversion math finishes (~40 ms later).
 * @note   Valid only after MS5607_Poll() has returned 1 at least once.
 */
uint32_t MS5607_GetSampleTick(void);

#ifdef __cplusplus
}
#endif

#endif /* _MS5607SPI_H_ */
