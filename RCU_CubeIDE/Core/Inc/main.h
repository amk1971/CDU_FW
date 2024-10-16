/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    REMOTE CONTROL UNIT - RCU
    Main.h

    MCU:        STMicroelectronics STM32F103VCT6
    Flash:      BoyaMicro BY25Q16BSTIG(T)
    Display:    Crystalfontz CFAG12864I-STI-TN

    GPD, 10/2024
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
// UART data struct.
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


extern SerialStruct USB_UART, GP_UART, PORT1, PORT2, PORT3;
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
	RCU_GAIN_SETUP,
	RCU_VOL_SET,
	RCU_SQL_SET,
	RCU_FREQ_SET,
	RCU_LOAD_FREQ,
	RCU_SELECT_FREQ,
	RCU_EDIT_FREQ,
	RCU_ADD_EDIT,
	RCU_DELETE_FREQ,
	RCU_TRANSMIT
} rcustate;


// General RCU system state variables
	typedef struct {
	uint8_t signature;	// const value to identify a empty/used sector
	rcustate state;
	float	Afreq;		// Active frequency
	float	Sfreq;		// Standby frequency
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
	powermode port1power;	// Serial PORT 1 power supply state (1: ON, 0: OFF)
	powermode port2power;	// Serial PORT 2 power supply state (1: ON, 0: OFF)
	powermode port3power;	// Serial PORT 3 power supply state (1: ON, 0: OFF)

} RCUsystemstate_t;

extern RCUsystemstate_t SysState;	// global System State instance
extern QueueHandle_t PORT_queue;	// RTOS queue to stream messages to serial ports
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
#define GP_SW4_Pin GPIO_PIN_5
#define GP_SW4_GPIO_Port GPIOE
#define GP_SW5_Pin GPIO_PIN_6
#define GP_SW5_GPIO_Port GPIOE
#define GP_SW6_Pin GPIO_PIN_13
#define GP_SW6_GPIO_Port GPIOC
#define LEFT_SW_Pin GPIO_PIN_0
#define LEFT_SW_GPIO_Port GPIOC
#define LEFT_A1_Pin GPIO_PIN_1
#define LEFT_A1_GPIO_Port GPIOC
#define LEFT_B1_Pin GPIO_PIN_2
#define LEFT_B1_GPIO_Port GPIOC
#define LEFT_B2_Pin GPIO_PIN_3
#define LEFT_B2_GPIO_Port GPIOC
#define LEFT_A2_Pin GPIO_PIN_0
#define LEFT_A2_GPIO_Port GPIOA
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
#define GP_SW1_Pin GPIO_PIN_11
#define GP_SW1_GPIO_Port GPIOE
#define GP_SW2_Pin GPIO_PIN_12
#define GP_SW2_GPIO_Port GPIOE
#define GP_SW3_Pin GPIO_PIN_13
#define GP_SW3_GPIO_Port GPIOE
#define RIGHT_SW_Pin GPIO_PIN_14
#define RIGHT_SW_GPIO_Port GPIOE
#define RIGHT_A1_Pin GPIO_PIN_15
#define RIGHT_A1_GPIO_Port GPIOE
#define RIGHT_B1_Pin GPIO_PIN_12
#define RIGHT_B1_GPIO_Port GPIOB
#define RIGHT_B2_Pin GPIO_PIN_13
#define RIGHT_B2_GPIO_Port GPIOB
#define RIGHT_A2_Pin GPIO_PIN_14
#define RIGHT_A2_GPIO_Port GPIOB
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
#define KEY_COL7_Pin GPIO_PIN_11
#define KEY_COL7_GPIO_Port GPIOA
#define KEY_COL6_Pin GPIO_PIN_12
#define KEY_COL6_GPIO_Port GPIOA
#define KEY_COL8_Pin GPIO_PIN_15
#define KEY_COL8_GPIO_Port GPIOA
#define KEY_COL9_Pin GPIO_PIN_0
#define KEY_COL9_GPIO_Port GPIOD
#define MCU_DSR1_Pin GPIO_PIN_1
#define MCU_DSR1_GPIO_Port GPIOD
#define RELAY1_Pin GPIO_PIN_3
#define RELAY1_GPIO_Port GPIOD
#define MCU_DTR1_Pin GPIO_PIN_4
#define MCU_DTR1_GPIO_Port GPIOD
#define RELAY2_Pin GPIO_PIN_7
#define RELAY2_GPIO_Port GPIOD
#define MCU_DSR2_Pin GPIO_PIN_6
#define MCU_DSR2_GPIO_Port GPIOB
#define MCU_DTR2_Pin GPIO_PIN_7
#define MCU_DTR2_GPIO_Port GPIOB
#define FLASH_CS_Pin GPIO_PIN_9
#define FLASH_CS_GPIO_Port GPIOB
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
#define	GP_SW5_IS_LOW	(HAL_GPIO_ReadPin(GP_SW5_GPIO_Port, GP_SW5_Pin) == GPIO_PIN_RESET)
#define	GP_SW6_IS_LOW	(HAL_GPIO_ReadPin(GP_SW6_GPIO_Port, GP_SW6_Pin) == GPIO_PIN_RESET)
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

#define	KEY_ROW0_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW0_GPIO_Port, KEY_ROW0_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW1_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW1_GPIO_Port, KEY_ROW1_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW2_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW2_GPIO_Port, KEY_ROW2_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW3_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW3_GPIO_Port, KEY_ROW3_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW4_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW4_GPIO_Port, KEY_ROW4_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW5_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW5_GPIO_Port, KEY_ROW5_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW6_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW6_GPIO_Port, KEY_ROW6_Pin) == GPIO_PIN_RESET)
#define	KEY_ROW7_IS_LOW	(HAL_GPIO_ReadPin(KEY_ROW7_GPIO_Port, KEY_ROW7_Pin) == GPIO_PIN_RESET)

#define	KEY_COL0_HIGH	HAL_GPIO_WritePin(KEY_COL0_GPIO_Port, KEY_COL0_Pin, GPIO_PIN_SET)
#define	KEY_COL1_HIGH	HAL_GPIO_WritePin(KEY_COL1_GPIO_Port, KEY_COL1_Pin, GPIO_PIN_SET)
#define	KEY_COL2_HIGH	HAL_GPIO_WritePin(KEY_COL2_GPIO_Port, KEY_COL2_Pin, GPIO_PIN_SET)
#define	KEY_COL3_HIGH	HAL_GPIO_WritePin(KEY_COL3_GPIO_Port, KEY_COL3_Pin, GPIO_PIN_SET)
#define	KEY_COL4_HIGH	HAL_GPIO_WritePin(KEY_COL4_GPIO_Port, KEY_COL4_Pin, GPIO_PIN_SET)
#define	KEY_COL5_HIGH	HAL_GPIO_WritePin(KEY_COL5_GPIO_Port, KEY_COL5_Pin, GPIO_PIN_SET)
#define	KEY_COL6_HIGH	HAL_GPIO_WritePin(KEY_COL6_GPIO_Port, KEY_COL6_Pin, GPIO_PIN_SET)
#define	KEY_COL7_HIGH	HAL_GPIO_WritePin(KEY_COL7_GPIO_Port, KEY_COL7_Pin, GPIO_PIN_SET)
#define	KEY_COL8_HIGH	HAL_GPIO_WritePin(KEY_COL8_GPIO_Port, KEY_COL8_Pin, GPIO_PIN_SET)
#define	KEY_COL9_HIGH	HAL_GPIO_WritePin(KEY_COL9_GPIO_Port, KEY_COL9_Pin, GPIO_PIN_SET)
#define	KEY_COL0_LOW	HAL_GPIO_WritePin(KEY_COL0_GPIO_Port, KEY_COL0_Pin, GPIO_PIN_RESET)
#define	KEY_COL1_LOW	HAL_GPIO_WritePin(KEY_COL1_GPIO_Port, KEY_COL1_Pin, GPIO_PIN_RESET)
#define	KEY_COL2_LOW	HAL_GPIO_WritePin(KEY_COL2_GPIO_Port, KEY_COL2_Pin, GPIO_PIN_RESET)
#define	KEY_COL3_LOW	HAL_GPIO_WritePin(KEY_COL3_GPIO_Port, KEY_COL3_Pin, GPIO_PIN_RESET)
#define	KEY_COL4_LOW	HAL_GPIO_WritePin(KEY_COL4_GPIO_Port, KEY_COL4_Pin, GPIO_PIN_RESET)
#define	KEY_COL5_LOW	HAL_GPIO_WritePin(KEY_COL5_GPIO_Port, KEY_COL5_Pin, GPIO_PIN_RESET)
#define	KEY_COL6_LOW	HAL_GPIO_WritePin(KEY_COL6_GPIO_Port, KEY_COL6_Pin, GPIO_PIN_RESET)
#define	KEY_COL7_LOW	HAL_GPIO_WritePin(KEY_COL7_GPIO_Port, KEY_COL7_Pin, GPIO_PIN_RESET)
#define	KEY_COL8_LOW	HAL_GPIO_WritePin(KEY_COL8_GPIO_Port, KEY_COL8_Pin, GPIO_PIN_RESET)
#define	KEY_COL9_LOW	HAL_GPIO_WritePin(KEY_COL9_GPIO_Port, KEY_COL9_Pin, GPIO_PIN_RESET)


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
#define MAX_FREQ			136.0
#define MIN_FREQ			118.0
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
