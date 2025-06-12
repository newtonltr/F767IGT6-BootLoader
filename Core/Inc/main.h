/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tx_api.h"
#include "nx_api.h"
#include <math.h>
#include <string.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DI5_Pin GPIO_PIN_3
#define DI5_GPIO_Port GPIOE
#define DI5_EXTI_IRQn EXTI3_IRQn
#define SPI4_NSS_Pin GPIO_PIN_4
#define SPI4_NSS_GPIO_Port GPIOE
#define DI6_Pin GPIO_PIN_8
#define DI6_GPIO_Port GPIOI
#define DI6_EXTI_IRQn EXTI9_5_IRQn
#define DI7_Pin GPIO_PIN_13
#define DI7_GPIO_Port GPIOC
#define DI7_EXTI_IRQn EXTI15_10_IRQn
#define DI8_Pin GPIO_PIN_9
#define DI8_GPIO_Port GPIOI
#define DI8_EXTI_IRQn EXTI9_5_IRQn
#define EMERGENCY_Pin GPIO_PIN_10
#define EMERGENCY_GPIO_Port GPIOI
#define DRVPWR_Pin GPIO_PIN_11
#define DRVPWR_GPIO_Port GPIOI
#define PHY_RST_Pin GPIO_PIN_2
#define PHY_RST_GPIO_Port GPIOH
#define HW_VER4_Pin GPIO_PIN_7
#define HW_VER4_GPIO_Port GPIOH
#define HW_VER3_Pin GPIO_PIN_8
#define HW_VER3_GPIO_Port GPIOH
#define HW_VER2_Pin GPIO_PIN_9
#define HW_VER2_GPIO_Port GPIOH
#define HW_VER1_Pin GPIO_PIN_10
#define HW_VER1_GPIO_Port GPIOH
#define HW_VER0_Pin GPIO_PIN_11
#define HW_VER0_GPIO_Port GPIOH
#define LED_G0_Pin GPIO_PIN_6
#define LED_G0_GPIO_Port GPIOG
#define LED_G1_Pin GPIO_PIN_7
#define LED_G1_GPIO_Port GPIOG
#define DO1_Pin GPIO_PIN_0
#define DO1_GPIO_Port GPIOI
#define DO2_Pin GPIO_PIN_1
#define DO2_GPIO_Port GPIOI
#define DO3_Pin GPIO_PIN_2
#define DO3_GPIO_Port GPIOI
#define DO4_Pin GPIO_PIN_3
#define DO4_GPIO_Port GPIOI
#define DO5_Pin GPIO_PIN_9
#define DO5_GPIO_Port GPIOG
#define DO6_Pin GPIO_PIN_10
#define DO6_GPIO_Port GPIOG
#define DO7_Pin GPIO_PIN_11
#define DO7_GPIO_Port GPIOG
#define DO8_Pin GPIO_PIN_12
#define DO8_GPIO_Port GPIOG
#define DO9_Pin GPIO_PIN_13
#define DO9_GPIO_Port GPIOG
#define DO10_Pin GPIO_PIN_14
#define DO10_GPIO_Port GPIOG
#define DI1_Pin GPIO_PIN_4
#define DI1_GPIO_Port GPIOI
#define DI1_EXTI_IRQn EXTI4_IRQn
#define DI2_Pin GPIO_PIN_5
#define DI2_GPIO_Port GPIOI
#define DI2_EXTI_IRQn EXTI9_5_IRQn
#define DI3_Pin GPIO_PIN_6
#define DI3_GPIO_Port GPIOI
#define DI3_EXTI_IRQn EXTI9_5_IRQn
#define DI4_Pin GPIO_PIN_7
#define DI4_GPIO_Port GPIOI
#define DI4_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */
void sleep_s(uint32_t s);
void sleep_ms(uint32_t ms);
void sleep_us(uint32_t us);

extern TX_SEMAPHORE  sem_socket_connected;

#ifdef __GNUC__
    #define PACKED __attribute__((packed))
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
    #define PACKED __packed
#else
    #define PACKED
#endif
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
