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
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define MAXBUFLEN	128			// UART receive buffer size. Must be a multiple of 2
#define BUFLENMASK	MAXBUFLEN-1
// UART data struct
// Contains buffers and pointers for non-blocking access to UART
typedef struct
{
	uint8_t RXbuffer[MAXBUFLEN];	// receive buffer
	uint8_t TXbuffer[MAXBUFLEN];	// transmit buffer
	uint16_t RXindex;				// pointer to last received char in RXbuffer
	uint16_t TXindex;				// pointer to last transmitted char in TXbuffer
	uint16_t LoadIndex;				// pointer to last written char to TXbuffer
	uint16_t TXbuflen;				// TX buffer size
	uint8_t TXcomplete;				// completed TX flag
} SerialStruct;


extern SerialStruct USB_UART, PORT1, PORT2, PORT3;
extern SPI_HandleTypeDef hspi1;

typedef enum { OFF, MOMENTARY, PRESS2SEC, PRESS3SEC, BOTH } buttonmode;
typedef enum  { STOP, PLUS, MINUS } knobmode;
typedef enum  { POWER_ON, POWER_OFF } powermode;

// RCU state machine codes
typedef enum {
	RCU_RESET,
	RCU_OFF,
	RCU_NORMAL,
	RCU_SETUP,
	RCU_SIDETONE_SETUP,
	//RCU_GAIN_SETUP,
	RCU_VOL_SET,
	RCU_SQL_SET,
	//RCU_FREQ_SET,
	//RCU_LOAD_FREQ,
	RCU_SELECT_FREQ,
	//RCU_EDIT_FREQ,
	RCU_ADD_EDIT,
	RCU_DELETE_FREQ,
	//RCU_TRANSMIT
} rcustate;


// General RCU system state variables
	typedef struct {
	uint8_t signature;	// const value to identify a empty/used sector
	rcustate state;
	uint32_t	Afreq;		// Active frequency
	uint32_t	Sfreq;		// Standby frequency
	uint32_t	A_VOR_LOC_freq;		// Active VOR/LOC frequency
	uint32_t	S_VOR_LOC_freq;		// Standby VOR/LOC frequency
	uint16_t OBS;				// Omni-Bearing Select
	uint8_t vol;		// volume level
	uint8_t squelch;	// squelch level
	uint8_t sidetone;	// sidetone level
	uint8_t gain;		// mic gain level
	uint8_t txmode;		// transmission mode (1: TX, 0: RX)
	buttonmode left_button;		// left knob button state
	buttonmode right_button;		// right knob button state
	buttonmode L1_button;		// Left panel button 1 state
	buttonmode L2_button;		// Left panel button 2 state
	buttonmode L3_button;		// Left panel button 3 state
	buttonmode R1_button;		// Right panel button 3 state
	buttonmode R2_button;		// Right panel button 3 state
	buttonmode R3_button;		// Right panel button 3 state
	knobmode left_inner_knob;	// Left center knob state
	knobmode left_outer_knob;	// Left outer knob state
	knobmode right_inner_knob;	// Right center knob state
	knobmode right_outer_knob;	// Right outer knob state
	buttonmode GP1_button;		// General urpose switch 1 state
	buttonmode GP2_button;		// General urpose switch 2 state
	buttonmode GP3_button;		// General urpose switch 3 state
	buttonmode GP4_button;		// General urpose switch 4 state
	powermode port1power;	// Serial PORT 1 power supply state (1: ON, 0: OFF)
	powermode port2power;	// Serial PORT 2 power supply state (1: ON, 0: OFF)
	powermode port3power;	// Serial PORT 3 power supply state (1: ON, 0: OFF)

} RCUsystemstate_t;

typedef struct{
	uint32_t	Afreq;		// Active frequency
	uint32_t	Sfreq;		// Standby frequency
	uint8_t status;		// transceiver status
	uint8_t sqlstatus;	// Squelch test setting
	char	ver[5];		// VHF COM receiver software version
	uint8_t vol;		// volume level
	uint8_t squelch;	// squelch level
	uint8_t sidetone;	// sidetone level
	uint8_t gain;		// mic gain level
	uint8_t	ready;		// reset status
	uint8_t errorcode;	// error type
} xceiverstate_t;

typedef struct{
	char left1[9];
	char right1[9];
	char left2[9];
	char right2[9];
} LCDtext_t;

extern RCUsystemstate_t SysState;	// global System State instance
extern xceiverstate_t XceiverState1;	// Transceiver at PORT1 State instance
extern xceiverstate_t XceiverState2;	// Transceiver at PORT2 State instance
extern xceiverstate_t XceiverState3;	// Transceiver at PORT3 State instance
extern QueueHandle_t PORT_queue;	// RTOS queue to stream messages to serial ports
extern SerialStruct PORT1, PORT2, PORT3;
extern QueueHandle_t PORT1_RX_queue;
extern QueueHandle_t PORT2_RX_queue;
extern QueueHandle_t PORT3_RX_queue;
extern LCDtext_t LCDtext;
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
#define GP_OUT2_Pin GPIO_PIN_2
#define GP_OUT2_GPIO_Port GPIOE
#define RELAY3_Pin GPIO_PIN_3
#define RELAY3_GPIO_Port GPIOE
#define GP_OUT1_Pin GPIO_PIN_4
#define GP_OUT1_GPIO_Port GPIOE
#define SW4_Pin GPIO_PIN_5
#define SW4_GPIO_Port GPIOE
#define SW5_Pin GPIO_PIN_6
#define SW5_GPIO_Port GPIOE
#define SW6_Pin GPIO_PIN_13
#define SW6_GPIO_Port GPIOC
#define LEFT_A1_Pin GPIO_PIN_14
#define LEFT_A1_GPIO_Port GPIOC
#define LEFT_SW_Pin GPIO_PIN_15
#define LEFT_SW_GPIO_Port GPIOC
#define LEFT_B1_Pin GPIO_PIN_0
#define LEFT_B1_GPIO_Port GPIOC
#define LEFT_B2_Pin GPIO_PIN_1
#define LEFT_B2_GPIO_Port GPIOC
#define LEFT_A2_Pin GPIO_PIN_2
#define LEFT_A2_GPIO_Port GPIOC
#define ANALOG_IN2_Pin GPIO_PIN_3
#define ANALOG_IN2_GPIO_Port GPIOC
#define ANALOG_IN1_Pin GPIO_PIN_0
#define ANALOG_IN1_GPIO_Port GPIOA
#define GP_SW1_Pin GPIO_PIN_2
#define GP_SW1_GPIO_Port GPIOA
#define GP_SW2_Pin GPIO_PIN_3
#define GP_SW2_GPIO_Port GPIOA
#define GP_SW3_Pin GPIO_PIN_6
#define GP_SW3_GPIO_Port GPIOA
#define GP_SW4_Pin GPIO_PIN_7
#define GP_SW4_GPIO_Port GPIOA
#define LCD_BKL_Pin GPIO_PIN_4
#define LCD_BKL_GPIO_Port GPIOC
#define LCD_RESET_Pin GPIO_PIN_5
#define LCD_RESET_GPIO_Port GPIOC
#define LCD_CS2_Pin GPIO_PIN_0
#define LCD_CS2_GPIO_Port GPIOB
#define LCD_CS1_Pin GPIO_PIN_1
#define LCD_CS1_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define LCD_DB7_Pin GPIO_PIN_7
#define LCD_DB7_GPIO_Port GPIOE
#define LCD_DB6_Pin GPIO_PIN_8
#define LCD_DB6_GPIO_Port GPIOE
#define LCD_DB5_Pin GPIO_PIN_9
#define LCD_DB5_GPIO_Port GPIOE
#define LCD_DB4_Pin GPIO_PIN_10
#define LCD_DB4_GPIO_Port GPIOE
#define LCD_DB2_Pin GPIO_PIN_11
#define LCD_DB2_GPIO_Port GPIOE
#define LCD_DB3_Pin GPIO_PIN_12
#define LCD_DB3_GPIO_Port GPIOE
#define LCD_DB0_Pin GPIO_PIN_13
#define LCD_DB0_GPIO_Port GPIOE
#define LCD_DB1_Pin GPIO_PIN_14
#define LCD_DB1_GPIO_Port GPIOE
#define LCD_RW_Pin GPIO_PIN_15
#define LCD_RW_GPIO_Port GPIOE
#define LCD_E_Pin GPIO_PIN_10
#define LCD_E_GPIO_Port GPIOB
#define LCD_DI_Pin GPIO_PIN_11
#define LCD_DI_GPIO_Port GPIOB
#define CAN_SDOWN_Pin GPIO_PIN_12
#define CAN_SDOWN_GPIO_Port GPIOB
#define CAN_STDBY_Pin GPIO_PIN_13
#define CAN_STDBY_GPIO_Port GPIOB
#define RIGHT_A1_Pin GPIO_PIN_14
#define RIGHT_A1_GPIO_Port GPIOB
#define RIGHT_B1_Pin GPIO_PIN_15
#define RIGHT_B1_GPIO_Port GPIOB
#define RIGHT_SW_Pin GPIO_PIN_8
#define RIGHT_SW_GPIO_Port GPIOD
#define RIGHT_B2_Pin GPIO_PIN_9
#define RIGHT_B2_GPIO_Port GPIOD
#define SW1_Pin GPIO_PIN_10
#define SW1_GPIO_Port GPIOD
#define SW2_Pin GPIO_PIN_11
#define SW2_GPIO_Port GPIOD
#define SW3_Pin GPIO_PIN_12
#define SW3_GPIO_Port GPIOD
#define RIGHT_A2_Pin GPIO_PIN_13
#define RIGHT_A2_GPIO_Port GPIOD
#define RELAY2_Pin GPIO_PIN_8
#define RELAY2_GPIO_Port GPIOA
#define BOOTLD_TX_Pin GPIO_PIN_9
#define BOOTLD_TX_GPIO_Port GPIOA
#define BOOTLD_RX_Pin GPIO_PIN_10
#define BOOTLD_RX_GPIO_Port GPIOA
#define RELAY1_Pin GPIO_PIN_15
#define RELAY1_GPIO_Port GPIOA
#define MCU_DTR1_Pin GPIO_PIN_0
#define MCU_DTR1_GPIO_Port GPIOD
#define MCU_DSR1_Pin GPIO_PIN_1
#define MCU_DSR1_GPIO_Port GPIOD
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_4
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_5
#define FLASH_MOSI_GPIO_Port GPIOB
#define MCU_DSR2_Pin GPIO_PIN_7
#define MCU_DSR2_GPIO_Port GPIOB
#define MCU_DTR2_Pin GPIO_PIN_8
#define MCU_DTR2_GPIO_Port GPIOB
#define FLASH_CS_Pin GPIO_PIN_9
#define FLASH_CS_GPIO_Port GPIOB
#define MCU_DSR3_Pin GPIO_PIN_0
#define MCU_DSR3_GPIO_Port GPIOE
#define MCU_DTR3_Pin GPIO_PIN_1
#define MCU_DTR3_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
#define	RELAY1_OFF	HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, GPIO_PIN_SET)
#define	RELAY2_OFF	HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, GPIO_PIN_SET)
#define	RELAY3_OFF	HAL_GPIO_WritePin(RELAY3_GPIO_Port, RELAY3_Pin, GPIO_PIN_SET)
#define	RELAY1_ON	HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, GPIO_PIN_RESET)
#define	RELAY2_ON	HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, GPIO_PIN_RESET)
#define	RELAY3_ON	HAL_GPIO_WritePin(RELAY3_GPIO_Port, RELAY3_Pin, GPIO_PIN_RESET)

#define	MCU_DTR1_HIGH	HAL_GPIO_WritePin(MCU_DTR1_GPIO_Port, MCU_DTR1_Pin, GPIO_PIN_SET)
#define	MCU_DTR2_HIGH	HAL_GPIO_WritePin(MCU_DTR2_GPIO_Port, MCU_DTR2_Pin, GPIO_PIN_SET)
#define	MCU_DTR3_HIGH	HAL_GPIO_WritePin(MCU_DTR3_GPIO_Port, MCU_DTR3_Pin, GPIO_PIN_SET)
#define	MCU_DTR1_LOW	HAL_GPIO_WritePin(MCU_DTR1_GPIO_Port, MCU_DTR1_Pin, GPIO_PIN_RESET)
#define	MCU_DTR2_LOW	HAL_GPIO_WritePin(MCU_DTR2_GPIO_Port, MCU_DTR2_Pin, GPIO_PIN_RESET)
#define	MCU_DTR3_LOW	HAL_GPIO_WritePin(MCU_DTR3_GPIO_Port, MCU_DTR3_Pin, GPIO_PIN_RESET)

#define	LCD_BKL_ON	HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_SET)
#define	LCD_BKL_OFF	HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_RESET)
#define	LCD_RESET_HIGH	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET)
#define	LCD_CS1_HIGH	HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET)
#define	LCD_CS2_HIGH	HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET)
#define	LCD_DB0_HIGH	HAL_GPIO_WritePin(LCD_DB0_GPIO_Port, LCD_DB0_Pin, GPIO_PIN_SET)
#define	LCD_DB1_HIGH	HAL_GPIO_WritePin(LCD_DB1_GPIO_Port, LCD_DB1_Pin, GPIO_PIN_SET)
#define	LCD_DB2_HIGH	HAL_GPIO_WritePin(LCD_DB2_GPIO_Port, LCD_DB2_Pin, GPIO_PIN_SET)
#define	LCD_DB3_HIGH	HAL_GPIO_WritePin(LCD_DB3_GPIO_Port, LCD_DB3_Pin, GPIO_PIN_SET)
#define	LCD_DB4_HIGH	HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_SET)
#define	LCD_DB5_HIGH	HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_SET)
#define	LCD_DB6_HIGH	HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_SET)
#define	LCD_DB7_HIGH	HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET)
#define	LCD_RW_HIGH	HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_SET)
#define	LCD_E_HIGH	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET)
#define	LCD_DI_HIGH	HAL_GPIO_WritePin(LCD_DI_GPIO_Port, LCD_DI_Pin, GPIO_PIN_SET)
#define	LCD_RESET_LOW	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET)
#define	LCD_CS1_LOW	HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET)
#define	LCD_CS2_LOW	HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET)
#define	LCD_DB0_LOW	HAL_GPIO_WritePin(LCD_DB0_GPIO_Port, LCD_DB0_Pin, GPIO_PIN_RESET)
#define	LCD_DB1_LOW	HAL_GPIO_WritePin(LCD_DB1_GPIO_Port, LCD_DB1_Pin, GPIO_PIN_RESET)
#define	LCD_DB2_LOW	HAL_GPIO_WritePin(LCD_DB2_GPIO_Port, LCD_DB2_Pin, GPIO_PIN_RESET)
#define	LCD_DB3_LOW	HAL_GPIO_WritePin(LCD_DB3_GPIO_Port, LCD_DB3_Pin, GPIO_PIN_RESET)
#define	LCD_DB4_LOW	HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_RESET)
#define	LCD_DB5_LOW	HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_RESET)
#define	LCD_DB6_LOW	HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_RESET)
#define	LCD_DB7_LOW	HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_RESET)
#define	LCD_RW_LOW	HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET)
#define	LCD_E_LOW	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET)
#define	LCD_DI_LOW	HAL_GPIO_WritePin(LCD_DI_GPIO_Port, LCD_DI_Pin, GPIO_PIN_RESET)


#define	FLASH_CS_HIGH	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)
#define	FLASH_CS_LOW	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)

// Poll pins states
#define	MCU_DSR1_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR1_GPIO_Port, MCU_DSR1_Pin) == GPIO_PIN_RESET)
#define	MCU_DSR2_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR2_GPIO_Port, MCU_DSR2_Pin) == GPIO_PIN_RESET)
#define	MCU_DSR3_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR3_GPIO_Port, MCU_DSR3_Pin) == GPIO_PIN_RESET)

#define	GP_SW1_IS_LOW	(HAL_GPIO_ReadPin(GP_SW1_GPIO_Port, GP_SW1_Pin) == GPIO_PIN_RESET)
#define	GP_SW2_IS_LOW	(HAL_GPIO_ReadPin(GP_SW2_GPIO_Port, GP_SW2_Pin) == GPIO_PIN_RESET)
#define	GP_SW3_IS_LOW	(HAL_GPIO_ReadPin(GP_SW3_GPIO_Port, GP_SW3_Pin) == GPIO_PIN_RESET)
#define	GP_SW4_IS_LOW	(HAL_GPIO_ReadPin(GP_SW4_GPIO_Port, GP_SW4_Pin) == GPIO_PIN_RESET)
#define	LEFT_SW_IS_LOW	(HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == GPIO_PIN_RESET)
#define	LEFT_A1_IS_LOW	(HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin) == GPIO_PIN_RESET)
#define	LEFT_A2_IS_LOW	(HAL_GPIO_ReadPin(LEFT_A2_GPIO_Port, LEFT_A2_Pin) == GPIO_PIN_RESET)
#define	LEFT_B1_IS_LOW	(HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin) == GPIO_PIN_RESET)
#define	LEFT_B2_IS_LOW	(HAL_GPIO_ReadPin(LEFT_B2_GPIO_Port, LEFT_B2_Pin) == GPIO_PIN_RESET)
#define	RIGHT_SW_IS_LOW	(HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == GPIO_PIN_RESET)
#define	RIGHT_A1_IS_LOW	(HAL_GPIO_ReadPin(RIGHT_A1_GPIO_Port, RIGHT_A1_Pin) == GPIO_PIN_RESET)
#define	RIGHT_A2_IS_LOW	(HAL_GPIO_ReadPin(RIGHT_A2_GPIO_Port, RIGHT_A2_Pin) == GPIO_PIN_RESET)
#define	RIGHT_B1_IS_LOW	(HAL_GPIO_ReadPin(RIGHT_B1_GPIO_Port, RIGHT_B1_Pin) == GPIO_PIN_RESET)
#define	RIGHT_B2_IS_LOW	(HAL_GPIO_ReadPin(RIGHT_B2_GPIO_Port, RIGHT_B2_Pin) == GPIO_PIN_RESET)

#define	SW1_IS_LOW	(HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET)
#define	SW2_IS_LOW	(HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET)
#define	SW3_IS_LOW	(HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET)
#define	SW4_IS_LOW	(HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET)
#define	SW5_IS_LOW	(HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET)
#define	SW6_IS_LOW	(HAL_GPIO_ReadPin(SW6_GPIO_Port, SW6_Pin) == GPIO_PIN_RESET)

#define	GP_OUT1_HIGH	HAL_GPIO_WritePin(GP_OUT1_GPIO_Port, GP_OUT1_Pin, GPIO_PIN_SET)
#define	GP_OUT1_LOW		HAL_GPIO_WritePin(GP_OUT1_GPIO_Port, GP_OUT1_Pin, GPIO_PIN_RESET)
#define	GP_OUT2_HIGH	HAL_GPIO_WritePin(GP_OUT2_GPIO_Port, GP_OUT2_Pin, GPIO_PIN_SET)
#define	GP_OUT2_LOW		HAL_GPIO_WritePin(GP_OUT2_GPIO_Port, GP_OUT2_Pin, GPIO_PIN_RESET)

#define CAN_SDOWN_Pin GPIO_PIN_12
#define CAN_SDOWN_GPIO_Port GPIOB
#define CAN_STDBY_Pin GPIO_PIN_13
#define CAN_STDBY_GPIO_Port GPIOB

// CAN controller shutdown control
#define	CAN_SHUTDOWN_ENABLED	HAL_GPIO_WritePin(CAN_SDOWN_GPIO_Port, CAN_SDOWN_Pin, GPIO_PIN_SET)
#define	CAN_SHUTDOWN_DISABLED	HAL_GPIO_WritePin(CAN_SDOWN_GPIO_Port, CAN_SDOWN_Pin, GPIO_PIN_RESET)
// CAN controller stand-by control
#define	CAN_STANDBY_ENABLED		HAL_GPIO_WritePin(CAN_STDBY_GPIO_Port, CAN_STDBY_Pin, GPIO_PIN_SET)
#define	CAN_STANDBY_DISABLED	HAL_GPIO_WritePin(CAN_STDBY_GPIO_Port, CAN_STDBY_Pin, GPIO_PIN_RESET)

// Button press timings  (RTOS task rate units, unit = 10ms)
#define BTN_SHORT_PRESS			10
#define BTN_2SEC_PRESS			200
#define BTN_3SEC_PRESS			300

// LCD queue
#define LCD_QUEUE_LEN		3		// buffer size
#define LCD_TEXT_LEN		16		// LCD width in chars

// Setup ranges
#define MIN_GAIN		0
#define MAX_GAIN		30
#define MIN_SIDETONE		0
#define MAX_SIDETONE		20
#define MAX_FREQ			136000
#define MIN_FREQ			118000
#define MIN_VOL				0
#define MAX_VOL				20
#define MIN_SQL				0
#define MAX_SQL				20


// Misc timings
#define MAIN_TASK_RATE		10
#define CURSOR_BLINK_RATE	15
#define VOL_SQL_SETUP_TIMEOUT	2000/MAIN_TASK_RATE
#define FREQ_SEL_TIMEOUT		5000/MAIN_TASK_RATE
#define BACKUP_RATE			10000

// General config switches
#define ERASE_FLASH_AT_START	// uncomment to format the flash at first time boot

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
