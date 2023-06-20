/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define DEBUG_UART_TX_Pin GPIO_PIN_2
#define DEBUG_UART_TX_GPIO_Port GPIOA
#define DEBUG_UART_RX_Pin GPIO_PIN_3
#define DEBUG_UART_RX_GPIO_Port GPIOA
#define LED_DATA_Pin GPIO_PIN_0
#define LED_DATA_GPIO_Port GPIOB
#define LED_NW_Pin GPIO_PIN_1
#define LED_NW_GPIO_Port GPIOB
#define LED_PWR_Pin GPIO_PIN_2
#define LED_PWR_GPIO_Port GPIOB
#define MAIN_UART_TX_Pin GPIO_PIN_10
#define MAIN_UART_TX_GPIO_Port GPIOB
#define MAIN_UART_RX_Pin GPIO_PIN_11
#define MAIN_UART_RX_GPIO_Port GPIOB
#define READY_3V3_Pin GPIO_PIN_15
#define READY_3V3_GPIO_Port GPIOB
#define BACKUP_3V3_Pin GPIO_PIN_8
#define BACKUP_3V3_GPIO_Port GPIOA
#define RS232_TX_Pin GPIO_PIN_9
#define RS232_TX_GPIO_Port GPIOA
#define RS232_RX_Pin GPIO_PIN_10
#define RS232_RX_GPIO_Port GPIOA
#define WDT_FEED_3V3_Pin GPIO_PIN_4
#define WDT_FEED_3V3_GPIO_Port GPIOB
#define WDT_RESET_Pin GPIO_PIN_5
#define WDT_RESET_GPIO_Port GPIOB
#define HARD_RESET_Pin GPIO_PIN_9
#define HARD_RESET_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
