/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f0xx_hal.h"

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
#define LEFT_SW2_Pin GPIO_PIN_2
#define LEFT_SW2_GPIO_Port GPIOE
#define LEFT_SW2_EXTI_IRQn EXTI2_3_IRQn
#define LEFT_SW1_Pin GPIO_PIN_3
#define LEFT_SW1_GPIO_Port GPIOE
#define LEFT_SW1_EXTI_IRQn EXTI2_3_IRQn
#define ROW1_Pin GPIO_PIN_6
#define ROW1_GPIO_Port GPIOE
#define ROW2_Pin GPIO_PIN_13
#define ROW2_GPIO_Port GPIOC
#define ROW3_Pin GPIO_PIN_14
#define ROW3_GPIO_Port GPIOC
#define ROW4_Pin GPIO_PIN_15
#define ROW4_GPIO_Port GPIOC
#define ROW5_Pin GPIO_PIN_9
#define ROW5_GPIO_Port GPIOF
#define ROW6_Pin GPIO_PIN_10
#define ROW6_GPIO_Port GPIOF
#define ROW7_Pin GPIO_PIN_0
#define ROW7_GPIO_Port GPIOF
#define ROW8_Pin GPIO_PIN_1
#define ROW8_GPIO_Port GPIOF
#define COL1_Pin GPIO_PIN_0
#define COL1_GPIO_Port GPIOC
#define COL2_Pin GPIO_PIN_1
#define COL2_GPIO_Port GPIOC
#define COL3_Pin GPIO_PIN_2
#define COL3_GPIO_Port GPIOC
#define COL4_Pin GPIO_PIN_3
#define COL4_GPIO_Port GPIOC
#define COL5_Pin GPIO_PIN_2
#define COL5_GPIO_Port GPIOF
#define COL6_Pin GPIO_PIN_3
#define COL6_GPIO_Port GPIOF
#define COL7_Pin GPIO_PIN_0
#define COL7_GPIO_Port GPIOA
#define COL8_Pin GPIO_PIN_1
#define COL8_GPIO_Port GPIOA
#define TFT_TX_Pin GPIO_PIN_2
#define TFT_TX_GPIO_Port GPIOA
#define TFT_RX_Pin GPIO_PIN_3
#define TFT_RX_GPIO_Port GPIOA
#define RELAY1_Pin GPIO_PIN_6
#define RELAY1_GPIO_Port GPIOA
#define RELAY3_Pin GPIO_PIN_7
#define RELAY3_GPIO_Port GPIOA
#define MCU_DTR1_Pin GPIO_PIN_0
#define MCU_DTR1_GPIO_Port GPIOB
#define MCU_DSR1_Pin GPIO_PIN_1
#define MCU_DSR1_GPIO_Port GPIOB
#define MCU_TX1_Pin GPIO_PIN_8
#define MCU_TX1_GPIO_Port GPIOE
#define MCU_RX1_Pin GPIO_PIN_9
#define MCU_RX1_GPIO_Port GPIOE
#define RELAY4_Pin GPIO_PIN_10
#define RELAY4_GPIO_Port GPIOD
#define RELAY2_Pin GPIO_PIN_11
#define RELAY2_GPIO_Port GPIOD
#define BOOTLD_TXD_Pin GPIO_PIN_9
#define BOOTLD_TXD_GPIO_Port GPIOA
#define BOOTLD_RXD_Pin GPIO_PIN_10
#define BOOTLD_RXD_GPIO_Port GPIOA
#define RIGHT_SW1_Pin GPIO_PIN_1
#define RIGHT_SW1_GPIO_Port GPIOD
#define RIGHT_SW2_Pin GPIO_PIN_2
#define RIGHT_SW2_GPIO_Port GPIOD
#define RIGHT_SW3_Pin GPIO_PIN_3
#define RIGHT_SW3_GPIO_Port GPIOD
#define RIGHT_SW4_Pin GPIO_PIN_4
#define RIGHT_SW4_GPIO_Port GPIOD
#define GP_SW4_Pin GPIO_PIN_5
#define GP_SW4_GPIO_Port GPIOD
#define GP_SW3_Pin GPIO_PIN_6
#define GP_SW3_GPIO_Port GPIOD
#define FLASH_CS_Pin GPIO_PIN_7
#define FLASH_CS_GPIO_Port GPIOD
#define FLASH_SCLK_Pin GPIO_PIN_3
#define FLASH_SCLK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_4
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_5
#define FLASH_MOSI_GPIO_Port GPIOB
#define GP_SW2_Pin GPIO_PIN_6
#define GP_SW2_GPIO_Port GPIOB
#define GP_SW1_Pin GPIO_PIN_7
#define GP_SW1_GPIO_Port GPIOB
#define LEFT_SW4_Pin GPIO_PIN_0
#define LEFT_SW4_GPIO_Port GPIOE
#define LEFT_SW4_EXTI_IRQn EXTI0_1_IRQn
#define LEFT_SW3_Pin GPIO_PIN_1
#define LEFT_SW3_GPIO_Port GPIOE
#define LEFT_SW3_EXTI_IRQn EXTI0_1_IRQn

/* USER CODE BEGIN Private defines */
#define ENABLE 	1
#define DISABLE 0
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
