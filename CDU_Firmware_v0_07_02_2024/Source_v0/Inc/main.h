/* USER CODE BEGIN Header */
/**

    CDU
    Control and Display Unit
    
    Main Header
    
    MCU: STM32F091VCT6
    
    GPD, 07/2024


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
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
	
/* FreeRTOS */
//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "timers.h"
//#include "semphr.h"
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
#define LEFT_SW1_Pin GPIO_PIN_3
#define LEFT_SW1_GPIO_Port GPIOE
#define CAN_SHTDOWN_Pin GPIO_PIN_4
#define CAN_SHTDOWN_GPIO_Port GPIOE
#define CAN_STDBY_Pin GPIO_PIN_5
#define CAN_STDBY_GPIO_Port GPIOE
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
#define MCU_TX2_Pin GPIO_PIN_4
#define MCU_TX2_GPIO_Port GPIOA
#define MCU_RX2_Pin GPIO_PIN_5
#define MCU_RX2_GPIO_Port GPIOA
#define RELAY1_Pin GPIO_PIN_6
#define RELAY1_GPIO_Port GPIOA
#define RELAY3_Pin GPIO_PIN_7
#define RELAY3_GPIO_Port GPIOA
#define MCU_DTR2_Pin GPIO_PIN_4
#define MCU_DTR2_GPIO_Port GPIOC
#define MCU_DSR2_Pin GPIO_PIN_5
#define MCU_DSR2_GPIO_Port GPIOC
#define MCU_DTR1_Pin GPIO_PIN_0
#define MCU_DTR1_GPIO_Port GPIOB
#define MCU_DSR1_Pin GPIO_PIN_1
#define MCU_DSR1_GPIO_Port GPIOB
#define MCU_TX1_Pin GPIO_PIN_8
#define MCU_TX1_GPIO_Port GPIOE
#define MCU_RX1_Pin GPIO_PIN_9
#define MCU_RX1_GPIO_Port GPIOE
#define MCU_TX3_Pin GPIO_PIN_10
#define MCU_TX3_GPIO_Port GPIOE
#define MCU_RX3_Pin GPIO_PIN_11
#define MCU_RX3_GPIO_Port GPIOE
#define MCU_DTR3_Pin GPIO_PIN_13
#define MCU_DTR3_GPIO_Port GPIOE
#define MCU_DSR3_Pin GPIO_PIN_15
#define MCU_DSR3_GPIO_Port GPIOE
#define MCU_TX4_Pin GPIO_PIN_10
#define MCU_TX4_GPIO_Port GPIOB
#define MCU_RX4_Pin GPIO_PIN_11
#define MCU_RX4_GPIO_Port GPIOB
#define MCU_DTR4_Pin GPIO_PIN_8
#define MCU_DTR4_GPIO_Port GPIOD
#define MCU_DSR4_Pin GPIO_PIN_9
#define MCU_DSR4_GPIO_Port GPIOD
#define RELAY4_Pin GPIO_PIN_10
#define RELAY4_GPIO_Port GPIOD
#define RELAY2_Pin GPIO_PIN_11
#define RELAY2_GPIO_Port GPIOD
#define BOOTLOAD_TX_Pin GPIO_PIN_9
#define BOOTLOAD_TX_GPIO_Port GPIOA
#define BOOTLOAD_RX_Pin GPIO_PIN_10
#define BOOTLOAD_RX_GPIO_Port GPIOA
#define RIGHT_SW4_Pin GPIO_PIN_1
#define RIGHT_SW4_GPIO_Port GPIOD
#define RIGHT_SW3_Pin GPIO_PIN_2
#define RIGHT_SW3_GPIO_Port GPIOD
#define RIGHT_SW2_Pin GPIO_PIN_3
#define RIGHT_SW2_GPIO_Port GPIOD
#define RIGHT_SW1_Pin GPIO_PIN_4
#define RIGHT_SW1_GPIO_Port GPIOD
#define GP_SW4_Pin GPIO_PIN_5
#define GP_SW4_GPIO_Port GPIOD
#define GP_SW3_Pin GPIO_PIN_6
#define GP_SW3_GPIO_Port GPIOD
#define FLASH_CS_Pin GPIO_PIN_7
#define FLASH_CS_GPIO_Port GPIOD
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
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
#define LEFT_SW3_Pin GPIO_PIN_1
#define LEFT_SW3_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
