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
#include "common.h"
#include "nextionLcd.h"
#include "switches.h"

#define CLIENTCODE

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
#define Left_SW2_Pin GPIO_PIN_2
#define Left_SW2_GPIO_Port GPIOE
#define Left_SW1_Pin GPIO_PIN_3
#define Left_SW1_GPIO_Port GPIOE
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
#define Right_SW4_Pin GPIO_PIN_1
#define Right_SW4_GPIO_Port GPIOD
#define Right_SW3_Pin GPIO_PIN_2
#define Right_SW3_GPIO_Port GPIOD
#define Right_SW2_Pin GPIO_PIN_3
#define Right_SW2_GPIO_Port GPIOD
#define Right_SW1_Pin GPIO_PIN_4
#define Right_SW1_GPIO_Port GPIOD
#define Left_SW4_Pin GPIO_PIN_0
#define Left_SW4_GPIO_Port GPIOE
#define Left_SW3_Pin GPIO_PIN_1
#define Left_SW3_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
