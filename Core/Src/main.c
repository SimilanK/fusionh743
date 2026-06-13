/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include "MS5607SPI.h"
#include "lsm6dso32.h"
#include "Kalman2State.h"
#include "ao_flight.h"
#include "airbrake_app.h"
#include "mpl3115a2.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint32_t timestamp;
    int32_t  pressure_pa;
    double   temperature_c;
    double   altitude_m;
    float    servo_deg;
    float    accel_x_mss;
    float    accel_y_mss;
    float    accel_z_mss;
    float    gyro_x_rads;
    float    gyro_y_rads;
    float    gyro_z_rads;
    float    imu_temp_c;
    /* Kalman filter outputs — logged alongside raw sensors for validation */
    float    kf_altitude_m;   /* fused altitude, AGL [m]    */
    float    kf_velocity_ms;  /* fused vertical velocity [m/s], positive = up */
} BundleData_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// commanded degree value.  Centre should always stay at 1520 µs.
// Start with the values below and increase/decrease by ~50 µs steps.       */
#define SERVO_MIN_US   560   /* pulse width (µs) that produces physical  0° */
#define SERVO_MAX_US  2460   /* pulse width (µs) that produces physical 180° (estimated: symmetric around 1520 µs center) */

/* 100 Hz fixed output rate.  MPL_OSR_1 conversion time is ~6 ms, so a
 * 10 ms gate always finds a fresh sample with ~4 ms to spare.            */
#define SAMPLE_PERIOD_MS   10u

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SD_HandleTypeDef hsd1;

SPI_HandleTypeDef hspi4;
DMA_HandleTypeDef hdma_spi4_rx;
DMA_HandleTypeDef hdma_spi4_tx;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile BundleData_t liveTelemetry = {0};
char uartTxBuffer[200];   /* 200 bytes: original 12 cols (~85 chars) + 2 KF cols + margin */

/* 32-byte aligned: SDMMC IDMA bypasses D-Cache and reads directly from SRAM.
   FatFs's internal sector buffer (fs->win) lives inside SDFatFs — aligning it
   ensures SCB_CleanDCache covers the correct cache lines before IDMA reads. */
__attribute__((aligned(32))) FATFS SDFatFs;
__attribute__((aligned(32))) FIL   LogFile;
__attribute__((aligned(32))) char  sdWriteBuffer[200];
uint8_t sdReady = 0;

/* Kalman filter instance — passed by pointer to ao_flight_update() every sample.
 * ao_ground_height and ao_ground_accel are now owned by ao_flight.c.           */
Kalman2State kf;

//i2c baro (I2C1)
MPL3115A2_t baro;
//i2c imu (I2C2)
LSM6DSO32_t imu;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_SPI4_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint32_t servoDegToUs(float deg)
{
    return (uint32_t)(SERVO_MIN_US + (deg / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US));
}

static void servoSetAngle(float deg)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, servoDegToUs(deg));
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_SPI4_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  //servo init
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  //baroinit — OSR_1: ~6 ms conversion, polled every 10 ms (100 Hz)
  if (MPL3115A2_Init(&baro, &hi2c1, MPL_MODE_BAROMETER, MPL_OSR_1) != MPL_OK) {
//      Error_Handler();
  }

  //imu init — LSM6DSO32 on I2C2 (SA0/SDO → VDD → address 0x6B)
  //           ±32g accel, ±2000 dps gyro, 104 Hz ODR
  if (LSM6DSO32_Init(&imu, &hi2c2, LSM6DSO32_ADDR_6B,
                     LSM6_ODR_104HZ, LSM6_FS_XL_32G, LSM6_FS_G_2000DPS) != LSM6_OK) {
      Error_Handler();
  }

  //UART header — two KF columns added at the end (backward compatible: old parsers ignore them)
  const char *uartHeader = "timestamp_ms,pressure_pa,temperature_c,altitude_m,servo_deg,"
                            "ax_mss,ay_mss,az_mss,gx_rads,gy_rads,gz_rads,imu_temp_c,"
                            "kf_altitude_m,kf_velocity_ms\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t *)uartHeader, (uint16_t)strlen(uartHeader), HAL_MAX_DELAY);

  //sd init — 500ms lets SDMMC card finish power-up before f_mount
  {
       FRESULT fres;
       char sdDbg[48];
       int  dbgLen;

       fres = f_mount(&SDFatFs, "", 1);
       dbgLen = snprintf(sdDbg, sizeof(sdDbg), "SD mount: %d\r\n", (int)fres);
       HAL_UART_Transmit(&huart2, (uint8_t *)sdDbg, (uint16_t)dbgLen, HAL_MAX_DELAY);

       if (fres == FR_OK) {
           fres = f_open(&LogFile, "FLIGHT01.CSV", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
           dbgLen = snprintf(sdDbg, sizeof(sdDbg), "SD open: %d\r\n", (int)fres);
           HAL_UART_Transmit(&huart2, (uint8_t *)sdDbg, (uint16_t)dbgLen, HAL_MAX_DELAY);

           if (fres == FR_OK) {
               f_lseek(&LogFile, f_size(&LogFile));
               if (f_size(&LogFile) == 0) {
                   const char *hdr = "timestamp_ms,pressure_pa,temperature_c,altitude_m,servo_deg,"
                                      "ax_mss,ay_mss,az_mss,gx_rads,gy_rads,gz_rads,imu_temp_c,"
                                      "kf_altitude_m,kf_velocity_ms\r\n";
                   UINT bw;
                   f_write(&LogFile, hdr, strlen(hdr), &bw);
                   f_sync(&LogFile);
               }
               sdReady = 1;
               HAL_UART_Transmit(&huart2, (uint8_t *)"SD ready\r\n", 10, HAL_MAX_DELAY);
           }
       }
   }

  /* Initialise the flight state machine.
   * Calibration now happens non-blockingly inside ao_flight_update() during the
   * AO_FLIGHT_STARTUP state — no blocking loop here.                           */
  ao_flight_init();
  HAL_UART_Transmit(&huart2, (uint8_t *)"State: STARTUP — collecting pad samples...\r\n", 44, HAL_MAX_DELAY);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      static uint32_t next_tick    = 0;   /* fixed-rate gate deadline         */
      static uint32_t prev_ts      = 0;   /* previous sample tick for dt      */
      static float    servoPos     = 0.0f;
      static float    servoDir     = 1.0f;
      static uint32_t servoTick    = 0;
      static uint8_t  syncCount    = 0;

      /* ── Fixed 100 Hz gate ───────────────────────────────────────────────── */
      uint32_t now = HAL_GetTick();
      if (now < next_tick) continue;
      next_tick = now + SAMPLE_PERIOD_MS;

      /* Timestamp locked to gate open — uniform 10 ms spacing in the log. */
      liveTelemetry.timestamp = now;

      /* ── Barometer (MPL3115A2, I2C) ──────────────────────────────────────── */
      {
          uint8_t baro_rdy = 0;
          float pa, tc;
          MPL3115A2_DataReady(&baro, &baro_rdy);
          if (baro_rdy && MPL3115A2_ReadPressure(&baro, &pa, &tc) == MPL_OK) {
              liveTelemetry.pressure_pa   = (int32_t)pa;
              liveTelemetry.temperature_c = tc;
              liveTelemetry.altitude_m    = MPL3115A2_AltitudeFromPressure(pa);
          }
          /* Not ready: previous values repeat. At OSR_1 (~6 ms) with a 10 ms
           * gate there is ~4 ms margin; should not occur at steady state.    */
      }

      /* ── IMU (LSM6DSO32, I2C2) ──────────────────────────────────────────── */
      if (LSM6DSO32_Update(&imu) == LSM6_OK) {
          liveTelemetry.accel_x_mss = imu.accel_x_mss;
          liveTelemetry.accel_y_mss = imu.accel_y_mss;
          liveTelemetry.accel_z_mss = imu.accel_z_mss;
          liveTelemetry.gyro_x_rads = imu.gyro_x_rads;
          liveTelemetry.gyro_y_rads = imu.gyro_y_rads;
          liveTelemetry.gyro_z_rads = imu.gyro_z_rads;
          liveTelemetry.imu_temp_c  = imu.temp_c;
      }

      /* ── Flight state machine + Kalman filter ───────────────────────────── */
      {
          ao_flight_state_t prev_state = ao_flight_state;

          float dt = (prev_ts > 0) ? (float)(now - prev_ts) / 1000.0f : 0.0f;
          prev_ts = now;

          ao_flight_update(&kf,
                           liveTelemetry.accel_z_mss,
                           dt,
                           (float)liveTelemetry.altitude_m);

          if (ao_flight_state == AO_FLIGHT_STARTUP) {
              liveTelemetry.kf_altitude_m  = 0.0f;
              liveTelemetry.kf_velocity_ms = 0.0f;
          } else {
              liveTelemetry.kf_altitude_m  = Kalman2State_Altitude(&kf);
              liveTelemetry.kf_velocity_ms = Kalman2State_Velocity(&kf);
          }

          {
              static uint32_t prev_rejects = 0;
              if (ao_cal_reject_count != prev_rejects) {
                  prev_rejects = ao_cal_reject_count;
                  char w[64];
                  int wn = snprintf(w, sizeof(w),
                                    "WARN: cal rejected (#%lu) — retrying\r\n",
                                    (unsigned long)ao_cal_reject_count);
                  HAL_UART_Transmit(&huart2, (uint8_t *)w, (uint16_t)wn, HAL_MAX_DELAY);
              }
          }

          if (prev_state != ao_flight_state) {
              switch (ao_flight_state) {
              case AO_FLIGHT_PAD: {
                  char msg[80];
                  int n = snprintf(msg, sizeof(msg),
                                   "State: PAD  h0=%.2fm  g=%.4fm/s2\r\n",
                                   (double)ao_ground_height,
                                   (double)ao_ground_accel);
                  HAL_UART_Transmit(&huart2, (uint8_t *)msg, (uint16_t)n, HAL_MAX_DELAY);
                  break;
              }
              case AO_FLIGHT_BOOST:
                  HAL_UART_Transmit(&huart2, (uint8_t *)"State: BOOST\r\n", 14, HAL_MAX_DELAY);
                  break;
              default: break;
              }
          }
      }

      /* ── UART — every sample (10 Hz, within 115200 baud budget) ─────────── */
      {
          int len = snprintf(uartTxBuffer, sizeof(uartTxBuffer),
                             "%lu,%ld,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.4f,%.4f,%.4f,%.2f,%.3f,%.4f\r\n",
                             liveTelemetry.timestamp,
                             liveTelemetry.pressure_pa,
                             liveTelemetry.temperature_c,
                             liveTelemetry.altitude_m,
                             (double)liveTelemetry.servo_deg,
                             (double)liveTelemetry.accel_x_mss,
                             (double)liveTelemetry.accel_y_mss,
                             (double)liveTelemetry.accel_z_mss,
                             (double)liveTelemetry.gyro_x_rads,
                             (double)liveTelemetry.gyro_y_rads,
                             (double)liveTelemetry.gyro_z_rads,
                             (double)liveTelemetry.imu_temp_c,
                             (double)liveTelemetry.kf_altitude_m,
                             (double)liveTelemetry.kf_velocity_ms);
          if (len > 0) {
              HAL_UART_Transmit(&huart2, (uint8_t *)uartTxBuffer, (uint16_t)len, HAL_MAX_DELAY);
          }
      }

      /* ── SD — every sample; sync every 10 rows ───────────────────────────── */
      if (sdReady) {
          UINT bw;
          int sdLen = snprintf(sdWriteBuffer, sizeof(sdWriteBuffer),
                               "%lu,%ld,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.4f,%.4f,%.4f,%.2f,%.3f,%.4f\r\n",
                               liveTelemetry.timestamp,
                               liveTelemetry.pressure_pa,
                               liveTelemetry.temperature_c,
                               liveTelemetry.altitude_m,
                               (double)liveTelemetry.servo_deg,
                               (double)liveTelemetry.accel_x_mss,
                               (double)liveTelemetry.accel_y_mss,
                               (double)liveTelemetry.accel_z_mss,
                               (double)liveTelemetry.gyro_x_rads,
                               (double)liveTelemetry.gyro_y_rads,
                               (double)liveTelemetry.gyro_z_rads,
                               (double)liveTelemetry.imu_temp_c,
                               (double)liveTelemetry.kf_altitude_m,
                               (double)liveTelemetry.kf_velocity_ms);
          if (sdLen > 0) {
              f_write(&LogFile, sdWriteBuffer, (UINT)sdLen, &bw);
              if (++syncCount >= 10) {
                  f_sync(&LogFile);
                  syncCount = 0;
              }
          }
      }

      /* ── Servo sweep — independent 150 ms cadence ───────────────────────── */
      if (now - servoTick >= 150) {
          servoTick = now;
          const float servoMin = 27.0f, servoMax = 140.0f;
          const float step     = (servoMax - servoMin) / 40.0f;
          servoPos += servoDir * step;
          if (servoPos >= servoMax) { servoPos = servoMax; servoDir = -1.0f; }
          if (servoPos <= servoMin) { servoPos = servoMin; servoDir =  1.0f; }
          liveTelemetry.servo_deg = servoPos;
          servoSetAngle(servoPos);
      }
  }





  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x30B70F24;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00B03FDB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv = 4;
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi4.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 0x0;
  hspi4.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi4.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 239;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 3029;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1520;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : SD_DETECT_Pin */
  GPIO_InitStruct.Pin = SD_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SD_DETECT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
