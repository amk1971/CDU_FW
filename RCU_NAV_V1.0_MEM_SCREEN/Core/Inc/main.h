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
#define RELAY3_Pin GPIO_PIN_2
#define RELAY3_GPIO_Port GPIOE
#define MCU_DSR3_Pin GPIO_PIN_3
#define MCU_DSR3_GPIO_Port GPIOE
#define MCU_DTR3_Pin GPIO_PIN_4
#define MCU_DTR3_GPIO_Port GPIOE
#define SW4_Pin GPIO_PIN_5
#define SW4_GPIO_Port GPIOE
#define SW5_Pin GPIO_PIN_6
#define SW5_GPIO_Port GPIOE
#define SW6_Pin GPIO_PIN_13
#define SW6_GPIO_Port GPIOC
#define LEFT_SW_Pin GPIO_PIN_0
#define LEFT_SW_GPIO_Port GPIOC
#define LEFT_A1_Pin GPIO_PIN_1
#define LEFT_A1_GPIO_Port GPIOC
#define LEFT_A1_EXTI_IRQn EXTI1_IRQn
#define LEFT_B1_Pin GPIO_PIN_2
#define LEFT_B1_GPIO_Port GPIOC
#define LEFT_B1_EXTI_IRQn EXTI2_IRQn
#define LEFT_B2_Pin GPIO_PIN_3
#define LEFT_B2_GPIO_Port GPIOC
#define LEFT_B2_EXTI_IRQn EXTI3_IRQn
#define LEFT_A2_Pin GPIO_PIN_0
#define LEFT_A2_GPIO_Port GPIOA
#define LEFT_A2_EXTI_IRQn EXTI0_IRQn
#define LCD_BKL_Pin GPIO_PIN_1
#define LCD_BKL_GPIO_Port GPIOA
#define LCD_RESET_Pin GPIO_PIN_2
#define LCD_RESET_GPIO_Port GPIOA
#define LCD_CS2_Pin GPIO_PIN_3
#define LCD_CS2_GPIO_Port GPIOA
#define LCD_CS1_Pin GPIO_PIN_4
#define LCD_CS1_GPIO_Port GPIOA
#define LCD_DB7_Pin GPIO_PIN_5
#define LCD_DB7_GPIO_Port GPIOA
#define LCD_DB6_Pin GPIO_PIN_6
#define LCD_DB6_GPIO_Port GPIOA
#define LCD_DB5_Pin GPIO_PIN_7
#define LCD_DB5_GPIO_Port GPIOA
#define LCD_DB4_Pin GPIO_PIN_4
#define LCD_DB4_GPIO_Port GPIOC
#define LCD_DB2_Pin GPIO_PIN_5
#define LCD_DB2_GPIO_Port GPIOC
#define LCD_DB3_Pin GPIO_PIN_0
#define LCD_DB3_GPIO_Port GPIOB
#define LCD_DB0_Pin GPIO_PIN_1
#define LCD_DB0_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define LCD_DB1_Pin GPIO_PIN_7
#define LCD_DB1_GPIO_Port GPIOE
#define LCD_RW_Pin GPIO_PIN_8
#define LCD_RW_GPIO_Port GPIOE
#define LCD_E_Pin GPIO_PIN_9
#define LCD_E_GPIO_Port GPIOE
#define LCD_DI_Pin GPIO_PIN_10
#define LCD_DI_GPIO_Port GPIOE
#define SW1_Pin GPIO_PIN_11
#define SW1_GPIO_Port GPIOE
#define SW2_Pin GPIO_PIN_12
#define SW2_GPIO_Port GPIOE
#define SW3_Pin GPIO_PIN_13
#define SW3_GPIO_Port GPIOE
#define RIGHT_SW_Pin GPIO_PIN_14
#define RIGHT_SW_GPIO_Port GPIOE
#define RIGHT_A1_Pin GPIO_PIN_15
#define RIGHT_A1_GPIO_Port GPIOE
#define RIGHT_A1_EXTI_IRQn EXTI15_10_IRQn
#define TFT_TX_Pin GPIO_PIN_10
#define TFT_TX_GPIO_Port GPIOB
#define TFT_RX_Pin GPIO_PIN_11
#define TFT_RX_GPIO_Port GPIOB
#define RIGHT_B1_Pin GPIO_PIN_12
#define RIGHT_B1_GPIO_Port GPIOB
#define RIGHT_B1_EXTI_IRQn EXTI15_10_IRQn
#define RIGHT_B2_Pin GPIO_PIN_13
#define RIGHT_B2_GPIO_Port GPIOB
#define RIGHT_B2_EXTI_IRQn EXTI15_10_IRQn
#define RIGHT_A2_Pin GPIO_PIN_14
#define RIGHT_A2_GPIO_Port GPIOB
#define RIGHT_A2_EXTI_IRQn EXTI15_10_IRQn
#define KEY_ROW1_Pin GPIO_PIN_15
#define KEY_ROW1_GPIO_Port GPIOB
#define KEY_ROW0_Pin GPIO_PIN_8
#define KEY_ROW0_GPIO_Port GPIOD
#define KEY_ROW3_Pin GPIO_PIN_9
#define KEY_ROW3_GPIO_Port GPIOD
#define KEY_ROW2_Pin GPIO_PIN_10
#define KEY_ROW2_GPIO_Port GPIOD
#define KEY_ROW5_Pin GPIO_PIN_11
#define KEY_ROW5_GPIO_Port GPIOD
#define KEY_ROW4_Pin GPIO_PIN_12
#define KEY_ROW4_GPIO_Port GPIOD
#define KEY_ROW7_Pin GPIO_PIN_13
#define KEY_ROW7_GPIO_Port GPIOD
#define KEY_ROW6_Pin GPIO_PIN_14
#define KEY_ROW6_GPIO_Port GPIOD
#define KEY_COL1_Pin GPIO_PIN_15
#define KEY_COL1_GPIO_Port GPIOD
#define KEY_COL0_Pin GPIO_PIN_6
#define KEY_COL0_GPIO_Port GPIOC
#define KEY_COL3_Pin GPIO_PIN_7
#define KEY_COL3_GPIO_Port GPIOC
#define KEY_COL2_Pin GPIO_PIN_8
#define KEY_COL2_GPIO_Port GPIOC
#define KEY_COL5_Pin GPIO_PIN_9
#define KEY_COL5_GPIO_Port GPIOC
#define KEY_COL4_Pin GPIO_PIN_8
#define KEY_COL4_GPIO_Port GPIOA
#define BOOTLD_TX_Pin GPIO_PIN_9
#define BOOTLD_TX_GPIO_Port GPIOA
#define BOOTLD_RX_Pin GPIO_PIN_10
#define BOOTLD_RX_GPIO_Port GPIOA
#define KEY_COL7_Pin GPIO_PIN_11
#define KEY_COL7_GPIO_Port GPIOA
#define KEY_COL6_Pin GPIO_PIN_12
#define KEY_COL6_GPIO_Port GPIOA
#define UART4_TX_Pin GPIO_PIN_10
#define UART4_TX_GPIO_Port GPIOC
#define UART4_RX_Pin GPIO_PIN_11
#define UART4_RX_GPIO_Port GPIOC
#define UART5_TX_Pin GPIO_PIN_12
#define UART5_TX_GPIO_Port GPIOC
#define KEY_COL9_Pin GPIO_PIN_0
#define KEY_COL9_GPIO_Port GPIOD
#define MCU_DSR1_Pin GPIO_PIN_1
#define MCU_DSR1_GPIO_Port GPIOD
#define UART5_RX_Pin GPIO_PIN_2
#define UART5_RX_GPIO_Port GPIOD
#define RELAY1_Pin GPIO_PIN_3
#define RELAY1_GPIO_Port GPIOD
#define MCU_DTR1_Pin GPIO_PIN_4
#define MCU_DTR1_GPIO_Port GPIOD
#define USART2_TX_Pin GPIO_PIN_5
#define USART2_TX_GPIO_Port GPIOD
#define USART2_RX_Pin GPIO_PIN_6
#define USART2_RX_GPIO_Port GPIOD
#define RELAY2_Pin GPIO_PIN_7
#define RELAY2_GPIO_Port GPIOD
#define FLASH_SCLK_Pin GPIO_PIN_3
#define FLASH_SCLK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_4
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_5
#define FLASH_MOSI_GPIO_Port GPIOB
#define MCU_DSR2_Pin GPIO_PIN_6
#define MCU_DSR2_GPIO_Port GPIOB
#define MCU_DTR2_Pin GPIO_PIN_7
#define MCU_DTR2_GPIO_Port GPIOB
#define FLASH_CS_Pin GPIO_PIN_9
#define FLASH_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
