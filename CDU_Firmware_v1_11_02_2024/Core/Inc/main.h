/* USER CODE BEGIN Header */
/**

CONTROL AND DISPLAY UNIT

MCU: STM32F091VCT6
Datasheet: file:///C:/Users/GPD40/Downloads/stm32f091vc-4.pdf

Flash memory: BY25Q16BSTIG(T)
Datasheet: https://www.lcsc.com/datasheet/lcsc_datasheet_1912111437_BOYAMICRO-BY25Q16BSTIG-T_C382744.pdf

Display: Nextion NX4832F035
Datasheet: https://nextion.tech/datasheets/NX4832F035

CAN Controller: Texas TCAN334DR
Datasheet: https://www.ti.com/lit/ds/symlink/tcan330.pdf

GPD, 10/2024

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
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "list.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define MAXBUFLEN	128			// UART receive buffer size. Must be a multiple of 2
#define BUFLENMASK	MAXBUFLEN-1
	// UART data struct.
	// Contains buffers and pointers for non-blocking access to UART
	typedef struct
	{
		uint8_t RXbuffer[MAXBUFLEN]; 	// receive buffer
		uint8_t TXbuffer[MAXBUFLEN]; 	// transmit buffer
		uint16_t RXindex; 				// pointer to last received char in RXbuffer
		uint16_t TXindex; 				// pointer to last transmitted char in TXbuffer
		uint16_t LoadIndex; 				// pointer to last written char to TXbuffer
		uint16_t TXbuflen; 				// TX buffer size
		uint8_t TXcomplete; 				// completed TX flag
	} SerialStruct;


	extern SerialStruct USB_UART, DISPLAY, PORT1, PORT2, PORT3, PORT4;
	extern SPI_HandleTypeDef hspi1;

	typedef enum { OFF, MOMENTARY, PRESS2SEC, PRESS3SEC, BOTH } buttonmode;
	typedef enum  { STOP, PLUS, MINUS } knobmode;
	typedef enum  { POWER_ON, POWER_OFF } powermode;

	// CDU state machine codes
typedef enum {
		CDU_RESET,
		CDU_OFF,
		CDU_NORMAL,
		CDU_SETUP,
		CDU_SIDETONE_SETUP,
		CDU_GAIN_SETUP,
		CDU_VOL_SET,
		CDU_SQL_SET,
		CDU_FREQ_SET,
		CDU_LOAD_FREQ,
		CDU_SELECT_FREQ,
		CDU_EDIT_FREQ,
		CDU_ADD_EDIT,
		CDU_DELETE_FREQ,
		CDU_TRANSMIT
	} cdustate;

	// General CDU system state variables
			typedef struct {
		uint8_t signature; 	// const value to identify a empty/used sector
		cdustate state;
		float	Afreq; 		// Active frequency
		float	Sfreq; 		// Standby frequency
		uint8_t vol; 		// volume level
		uint8_t squelch; 	// squelch level
		uint8_t sidetone; 	// sidetone level
		uint8_t gain; 		// mic gain level
		uint8_t txmode; 		// transmission mode (1: TX, 0: RX)
		buttonmode L1_button; 		// Left panel button 1 (top) state
		buttonmode L2_button; 		// Left panel button 2 state
		buttonmode L3_button; 		// Left panel button 3 state
		buttonmode L4_button;  		// Left panel button 4 state
		buttonmode R1_button; 		// Right panel button 1 (top) state
		buttonmode R2_button; 		// Right panel button 2 state
		buttonmode R3_button; 		// Right panel button 3 state
		buttonmode R4_button;  		// Right panel button 4 state
		buttonmode GP1_button;  	// General Purpose button 1 state
		buttonmode GP2_button;  	// General Purpose button 2 state
		buttonmode GP3_button;  	// General Purpose button 3 state
		buttonmode GP4_button;   	// General Purpose button 4 state
		powermode port1power; 	// Serial PORT 1 power supply state (1: ON, 0: OFF)
		powermode port2power; 	// Serial PORT 2 power supply state (1: ON, 0: OFF)
		powermode port3power; 	// Serial PORT 3 power supply state (1: ON, 0: OFF)
		powermode port4power;  	// Serial PORT 4 power supply state (1: ON, 0: OFF)
		uint8_t DisplayBypass;  // 1: bypass mode (Display programming), 0: normal

	} systemstate_t;

	typedef struct{
		float	Afreq;		// Active frequency
		float	Sfreq;		// Standby frequency
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

	extern systemstate_t SysState; 	// global System State instance
	extern xceiverstate_t XceiverState1;	// Transceiver at PORT1 State instance
	extern xceiverstate_t XceiverState2;	// Transceiver at PORT2 State instance
	extern xceiverstate_t XceiverState3;	// Transceiver at PORT3 State instance
	extern xceiverstate_t XceiverState4;	// Transceiver at PORT4 State instance
	extern QueueHandle_t PORT_queue; 	// RTOS queue to stream messages to serial ports
	extern QueueHandle_t PORT1_RX_queue;
	extern QueueHandle_t PORT2_RX_queue;
	extern QueueHandle_t PORT3_RX_queue;
	extern QueueHandle_t PORT4_RX_queue;
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

// Macros for GPIO access

// Serial port control
#define	DTR1_HIGH	HAL_GPIO_WritePin(MCU_DTR1_GPIO_Port, MCU_DTR1_Pin, GPIO_PIN_SET)
#define	DTR1_LOW	HAL_GPIO_WritePin(MCU_DTR1_GPIO_Port, MCU_DTR1_Pin, GPIO_PIN_RESET)
#define	DTR2_HIGH	HAL_GPIO_WritePin(MCU_DTR2_GPIO_Port, MCU_DTR2_Pin, GPIO_PIN_SET)
#define	DTR2_LOW	HAL_GPIO_WritePin(MCU_DTR2_GPIO_Port, MCU_DTR2_Pin, GPIO_PIN_RESET)
#define	DTR3_HIGH	HAL_GPIO_WritePin(MCU_DTR3_GPIO_Port, MCU_DTR3_Pin, GPIO_PIN_SET)
#define	DTR3_LOW	HAL_GPIO_WritePin(MCU_DTR3_GPIO_Port, MCU_DTR3_Pin, GPIO_PIN_RESET)
#define	DTR4_HIGH	HAL_GPIO_WritePin(MCU_DTR4_GPIO_Port, MCU_DTR4_Pin, GPIO_PIN_SET)
#define	DTR4_LOW	HAL_GPIO_WritePin(MCU_DTR4_GPIO_Port, MCU_DTR4_Pin, GPIO_PIN_RESET)
#define	DSR1_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR1_GPIO_Port, MCU_DSR1_Pin) == GPIO_PIN_RESET)
#define	DSR2_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR2_GPIO_Port, MCU_DSR2_Pin) == GPIO_PIN_RESET)
#define	DSR3_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR3_GPIO_Port, MCU_DSR3_Pin) == GPIO_PIN_RESET)
#define	DSR4_IS_LOW	(HAL_GPIO_ReadPin(MCU_DSR3_GPIO_Port, MCU_DSR3_Pin) == GPIO_PIN_RESET)

// Remote power Relay control
#define	RELAY1_ON	HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, GPIO_PIN_SET)
#define	RELAY1_OFF	HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, GPIO_PIN_RESET)
#define	RELAY2_ON	HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, GPIO_PIN_SET)
#define	RELAY2_OFF	HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, GPIO_PIN_RESET)
#define	RELAY3_ON	HAL_GPIO_WritePin(RELAY3_GPIO_Port, RELAY3_Pin, GPIO_PIN_SET)
#define	RELAY3_OFF	HAL_GPIO_WritePin(RELAY3_GPIO_Port, RELAY3_Pin, GPIO_PIN_RESET)
#define	RELAY4_ON	HAL_GPIO_WritePin(RELAY4_GPIO_Port, RELAY4_Pin, GPIO_PIN_SET)
#define	RELAY4_OFF	HAL_GPIO_WritePin(RELAY4_GPIO_Port, RELAY4_Pin, GPIO_PIN_RESET)

// Side keyboard control
#define	RIGHT_SW1_CLOSED	(HAL_GPIO_ReadPin(RIGHT_SW1_GPIO_Port, RIGHT_SW1_Pin) == GPIO_PIN_RESET)
#define	RIGHT_SW2_CLOSED	(HAL_GPIO_ReadPin(RIGHT_SW2_GPIO_Port, RIGHT_SW2_Pin) == GPIO_PIN_RESET)
#define	RIGHT_SW3_CLOSED	(HAL_GPIO_ReadPin(RIGHT_SW3_GPIO_Port, RIGHT_SW3_Pin) == GPIO_PIN_RESET)
#define	RIGHT_SW4_CLOSED	(HAL_GPIO_ReadPin(RIGHT_SW4_GPIO_Port, RIGHT_SW4_Pin) == GPIO_PIN_RESET)
#define	LEFT_SW1_CLOSED	    (HAL_GPIO_ReadPin(LEFT_SW1_GPIO_Port, LEFT_SW1_Pin) == GPIO_PIN_RESET)
#define	LEFT_SW2_CLOSED	    (HAL_GPIO_ReadPin(LEFT_SW2_GPIO_Port, LEFT_SW2_Pin) == GPIO_PIN_RESET)
#define	LEFT_SW3_CLOSED	    (HAL_GPIO_ReadPin(LEFT_SW3_GPIO_Port, LEFT_SW3_Pin) == GPIO_PIN_RESET)
#define	LEFT_SW4_CLOSED	    (HAL_GPIO_ReadPin(LEFT_SW4_GPIO_Port, LEFT_SW4_Pin) == GPIO_PIN_RESET)

// General purpose switches states
#define	GP_SW1_CLOSED	(HAL_GPIO_ReadPin(GP_SW1_GPIO_Port, GP_SW1_Pin) == GPIO_PIN_RESET)
#define	GP_SW2_CLOSED	(HAL_GPIO_ReadPin(GP_SW2_GPIO_Port, GP_SW2_Pin) == GPIO_PIN_RESET)
#define	GP_SW3_CLOSED	(HAL_GPIO_ReadPin(GP_SW3_GPIO_Port, GP_SW3_Pin) == GPIO_PIN_RESET)
#define	GP_SW4_CLOSED	(HAL_GPIO_ReadPin(GP_SW4_GPIO_Port, GP_SW4_Pin) == GPIO_PIN_RESET)

// CAN controller shutdown control
#define	CAN_SHUTDOWN_ENABLED	HAL_GPIO_WritePin(CAN_SHTDOWN_GPIO_Port, CAN_SHTDOWN_Pin, GPIO_PIN_SET)
#define	CAN_SHUTDOWN_DISABLED	HAL_GPIO_WritePin(CAN_SHTDOWN_GPIO_Port, CAN_SHTDOWN_Pin, GPIO_PIN_RESET)

// CAN controller stand-by control
#define	CAN_STANDBY_ENABLED	HAL_GPIO_WritePin(CAN_STDBY_GPIO_Port, CAN_STDBY_Pin, GPIO_PIN_SET)
#define	CAN_STANDBY_DISABLED	HAL_GPIO_WritePin(CAN_STDBY_GPIO_Port, CAN_STDBY_Pin, GPIO_PIN_RESET)

// Keyboard matrix scan
#define	ROW1_HIGH	HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET)
#define	ROW2_HIGH	HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET)
#define	ROW3_HIGH	HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET)
#define	ROW4_HIGH	HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET)
#define	ROW5_HIGH	HAL_GPIO_WritePin(ROW5_GPIO_Port, ROW5_Pin, GPIO_PIN_SET)
#define	ROW6_HIGH	HAL_GPIO_WritePin(ROW6_GPIO_Port, ROW6_Pin, GPIO_PIN_SET)
#define	ROW7_HIGH	HAL_GPIO_WritePin(ROW7_GPIO_Port, ROW7_Pin, GPIO_PIN_SET)
#define	ROW8_HIGH	HAL_GPIO_WritePin(ROW8_GPIO_Port, ROW8_Pin, GPIO_PIN_SET)
#define	ROW1_LOW	HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_RESET)
#define	ROW2_LOW	HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_RESET)
#define	ROW3_LOW	HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_RESET)
#define	ROW4_LOW	HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_RESET)
#define	ROW5_LOW	HAL_GPIO_WritePin(ROW5_GPIO_Port, ROW5_Pin, GPIO_PIN_RESET)
#define	ROW6_LOW	HAL_GPIO_WritePin(ROW6_GPIO_Port, ROW6_Pin, GPIO_PIN_RESET)
#define	ROW7_LOW	HAL_GPIO_WritePin(ROW7_GPIO_Port, ROW7_Pin, GPIO_PIN_RESET)
#define	ROW8_LOW	HAL_GPIO_WritePin(ROW8_GPIO_Port, ROW8_Pin, GPIO_PIN_RESET)
#define	COL1_IS_LOW	(HAL_GPIO_ReadPin(COL1_GPIO_Port, COL1_Pin) == GPIO_PIN_RESET)
#define	COL2_IS_LOW	(HAL_GPIO_ReadPin(COL2_GPIO_Port, COL2_Pin) == GPIO_PIN_RESET)
#define	COL3_IS_LOW	(HAL_GPIO_ReadPin(COL3_GPIO_Port, COL3_Pin) == GPIO_PIN_RESET)
#define	COL4_IS_LOW	(HAL_GPIO_ReadPin(COL4_GPIO_Port, COL4_Pin) == GPIO_PIN_RESET)
#define	COL5_IS_LOW	(HAL_GPIO_ReadPin(COL5_GPIO_Port, COL5_Pin) == GPIO_PIN_RESET)
#define	COL6_IS_LOW	(HAL_GPIO_ReadPin(COL6_GPIO_Port, COL6_Pin) == GPIO_PIN_RESET)
#define	COL7_IS_LOW	(HAL_GPIO_ReadPin(COL7_GPIO_Port, COL7_Pin) == GPIO_PIN_RESET)
#define	COL8_IS_LOW	(HAL_GPIO_ReadPin(COL8_GPIO_Port, COL8_Pin) == GPIO_PIN_RESET)

#define	FLASH_CS_HIGH	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)
#define	FLASH_CS_LOW	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)

// Button press timings  (RTOS task rate units, unit = 10ms)
#define BTN_SHORT_PRESS			10
#define BTN_2SEC_PRESS			200
#define BTN_3SEC_PRESS			300

// LCD queue
#define DISPLAY_QUEUE_LEN		3		// buffer size
#define DISPLAY_TEXT_LEN		16		// max text length in chars

// General config switches
#define ERASE_FLASH_AT_START	// uncomment to format the flash at first time boot

// Macro for millisecond calculation
#define MS(X)			(configTICK_RATE_HZ*X)/1000

// Misc timings
#define BACKUP_RATE			10000	// millisecs

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
