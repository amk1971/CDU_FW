/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dac.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// LCD driver
#include "glcd.h"
#include "advanceglcd.h"
#include "font.h"

// Flash manager
#include "RCU_flash_mgr.h"

// Serial port protocol
#include "NMEA0183.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//formatted I/O objects
SerialStruct USB_UART;
SerialStruct PORT1, PORT2, PORT3;
char text[128], resp[2];

// serial message parameter list
CommParam_t CommParams;
// Global system state
RCUsystemstate_t SysState;		// global System State instance
xceiverstate_t XceiverState1;	// Transceiver at PORT1 State instance
xceiverstate_t XceiverState2;	// Transceiver at PORT2 State instance
xceiverstate_t XceiverState3;	// Transceiver at PORT3 State instance
LCDtext_t LCDtext;
// Default State parameters
const RCUsystemstate_t DefaultSystemState =  {
	.signature = OCCUPIED_SECTOR_CODE,
	.state = RCU_RESET,
	.Afreq = 118000,
	.Sfreq = 118000,
	.A_VOR_LOC_freq = 108000,
	.S_VOR_LOC_freq = 108000,
	.OBS = 0,
	.vol = 0,
	.squelch = 0,
	.sidetone = 0,
	.gain = 0,
	.txmode = 0,
	.left_button = OFF,
	.right_button = OFF,
	.L1_button = OFF,
	.L2_button = OFF,
	.L3_button = OFF,
	.R1_button = OFF,
	.R2_button = OFF,
	.R3_button = OFF,
	.left_inner_knob = STOP,
	.left_outer_knob = STOP,
	.right_inner_knob = STOP,
	.right_outer_knob = STOP,
	.GP1_button = OFF,
	.GP2_button = OFF,
	.GP3_button = OFF,
	.GP4_button = OFF,
	.port1power = POWER_ON,
	.port2power = POWER_OFF,
	.port3power = POWER_OFF,
	};

const xceiverstate_t DefaultXceiverState =  {

	.Afreq = 118000,
	.Sfreq = 118000,
	.status = -1,
	.sqlstatus = -1,
	.ver[0] = 0,
	.vol = 0,
	.squelch = 0,
	.sidetone = 1,
	.gain = 1,
	.ready = 0,
	.errorcode = -1,
};

// RTOS queue for display texts
QueueHandle_t LCD_queue; // RTOS queue to stream messages to the display
QueueHandle_t PORT_queue; // RTOS queue to stream messages to serial ports
QueueHandle_t PORT1_RX_queue;
QueueHandle_t PORT2_RX_queue;
QueueHandle_t PORT3_RX_queue;
// SysState backup scheduler
TimerHandle_t BackupTimer;	// timer to schedule SysState backup to Flash
uint8_t xcursor = 1;		// blinking cursor position (during (add/edit frequency)


// CANbus test
CAN_TxHeaderTypeDef pHeader; //declare a specific header for message transmittions
CAN_RxHeaderTypeDef pRxHeader; //declare header for message reception
uint32_t TxMailbox;
uint8_t a,r; //declare byte to be transmitted //declare a receive byte
CAN_FilterTypeDef sFilterConfig; //declare CAN filter structure


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//RTOS tasks
void MAIN_Task(void *pvParameters);      	// RTOS task to manage general RCU operation
void DISPLAY_Task(void *pvParameters);       // RTOS task to control LCD low level I/O
void ENCODER_Task(void *pvParameters);        // Read rotary encoders
void BUTTON_Task(void *pvParameters);         // Read panel buttons, de-bounce, etc.
void PORTS_Task(void *pvParameters);         // Manage serial ports
void DEBUG_Task(void *pvParameters);       // debug messages
void UART_TEST_TASK(void *pvParameters);	//UART TEST MESSAGES

void printf_USB(const char* format, ...);	// printf for USB port
void printf_PORT1(const char* format, ...); 	// printf for SubD-15 Port 1
void printf_PORT2(const char* format, ...); 	// printf for SubD-15 Port 2
void printf_PORT3(const char* format, ...); 	// printf for SubD-15 Port 3

void Reset_Knobs_States(void);	// return all knobs button to default modes
void Backup_Manager(void);		// manage SysState backups
void Parse_NMEA_Message(xceiverstate_t *x,char *msg);
void glcd_puts2(uint8_t x,uint8_t y,char *s);
void glcd_clear_texts(LCDtext_t *x);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_CAN_Init();
  MX_DAC_Init();
  MX_ADC1_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */

	//HAL_ADCEx_Calibration_Start(&hadc1);		// Self-calibrate analog inputs
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
	// Default RCU state
	SysState = DefaultSystemState;
	XceiverState1 = DefaultXceiverState;
	XceiverState2 = DefaultXceiverState;
	XceiverState3 = DefaultXceiverState;

	// reset UART RX buffers
	USB_UART.RXbuffer[0] = 0;
	USB_UART.RXindex = 0;
	PORT1.RXbuffer[0] = 0;
	PORT1.RXindex = 0;
	PORT2.RXbuffer[0] = 0;
	PORT2.RXindex = 0;
	PORT3.RXbuffer[0] = 0;
	PORT3.RXindex = 0;
	memset((void*)&LCDtext,0,sizeof(LCDtext));	// erase LCd strings

	// Enable UART RX interrupts
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);
//	RELAY1_OFF;
//	RELAY2_OFF;
//	RELAY3_OFF;

	// Queues
	LCD_queue = xQueueCreate(LCD_QUEUE_LEN, sizeof(LCDtext));
	PORT_queue = xQueueCreate(6, sizeof(CommParams));
	PORT1_RX_queue = xQueueCreate(3, MAX_NMEA_MSG_LEN);
	PORT2_RX_queue = xQueueCreate(3, MAX_NMEA_MSG_LEN);
	PORT3_RX_queue = xQueueCreate(3, MAX_NMEA_MSG_LEN);

	// SysState backup timer
	BackupTimer = xTimerCreate("Backup", BACKUP_RATE, pdTRUE, 0,(void*) Backup_Manager);

	// Start RTOS tasks
//	xTaskCreate(MAIN_Task, "Main", configMINIMAL_STACK_SIZE+300, NULL, tskIDLE_PRIORITY + 2, NULL);
//	xTaskCreate(DISPLAY_Task, "Display", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);
//	xTaskCreate(ENCODER_Task, "Encoders", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 4, NULL);
//	xTaskCreate(BUTTON_Task, "Buttons", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 2, NULL);
//	xTaskCreate(PORTS_Task, "Ports", configMINIMAL_STACK_SIZE + 1000, NULL, tskIDLE_PRIORITY + 3, NULL);
//	xTaskCreate(DEBUG_Task, "Debug", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(UART_TEST_TASK, "UART", configMINIMAL_STACK_SIZE + 1000, NULL, tskIDLE_PRIORITY + 1, NULL);


	// CANbus test
	CAN_SHUTDOWN_DISABLED;
	CAN_STANDBY_DISABLED;

	pHeader.DLC=1; //give message size of 1 byte
	pHeader.IDE=CAN_ID_STD; //set identifier to standard
	pHeader.RTR=CAN_RTR_DATA; //set data type to remote transmission request?
	pHeader.StdId=0x244; //define a standard identifier, used for message identification by filters (switch this for the other microcontroller)

	//filter one (stack light blink)
	sFilterConfig.FilterFIFOAssignment=CAN_FILTER_FIFO0; //set fifo assignment
	sFilterConfig.FilterIdHigh=0x245<<5; //the ID that the filter looks for (switch this for the other microcontroller)
	sFilterConfig.FilterIdLow=0;
	sFilterConfig.FilterMaskIdHigh=0;
	sFilterConfig.FilterMaskIdLow=0;
	sFilterConfig.FilterScale=CAN_FILTERSCALE_32BIT; //set filter scale
	sFilterConfig.FilterActivation=ENABLE;

	HAL_CAN_ConfigFilter(&hcan, &sFilterConfig); //configure CAN filter


	HAL_CAN_Start(&hcan); //start CAN






	// Start RTOS Scheduler
	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//===================================================================
//			UART TEST TASK
//===================================================================
uint8_t compute_checksum(char *data, int length) {
    uint8_t checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum += data[i];  // Sum ASCII values
    }
    return checksum & 0xFF;   // Keep lower 8 bits
}
#define UART_BUFFER_SIZE 8  // Expecting 6 bytes
#define CHECKSUM_LENGTH 3   // Length of the data part "101"
uint8_t uart_rx_buffer[UART_BUFFER_SIZE];  // Buffer to store received data
void UART_TEST_TASK(void *pvParameters)
{
	  int mhz = 256;
	  int khz = 475;
	  int vol = 45;
	  char o = '1';
	  char s = '0';
	  char TT = '0';
	  char D = '1';
	  // Convert integers to strings
	     char mhz_str[4], khz_str[4], vol_str[3];
	     sprintf(mhz_str, "%d", mhz);
	     sprintf(khz_str, "%d", khz);
	     sprintf(vol_str, "%d", vol);
	     // Concatenate everything into a buffer
	     char data[20];  // Ensure buffer is large enough
	     sprintf(data, "%s%s%s%c%c%c%c", mhz_str, khz_str, vol_str, o, s, TT, D);
	     // Compute checksum
	     uint8_t checksum = compute_checksum(data, strlen(data));
	     // Store checksum as two uppercase HEX characters
	     char checksum_hex[3];
	     sprintf(checksum_hex, "%02X", checksum);  // Convert to 2-digit HEX


	  char str[20];  // Ensure buffer is large enough

	  // Correct sprintf usage
	  sprintf(str, "$%d%d%d%c%c%c%c%s\r\n", mhz, khz, vol, o, s, TT, D,checksum_hex);
	for(;;)
	{
		//	   HAL_UART_Transmit(&huart4, str, 17, 1000); // to transmit data
			    if (HAL_UART_Receive(&huart4, uart_rx_buffer, UART_BUFFER_SIZE, 2000) == HAL_OK) {
		//	        // Data received successfully
			    	char data_rx[CHECKSUM_LENGTH + 1];  // Buffer to store extracted data (3 chars)
			    	    strncpy(data_rx, (char*)&uart_rx_buffer[1], CHECKSUM_LENGTH);
			    	    data_rx[CHECKSUM_LENGTH] = '\0';  // Null-terminate the string

			    	    // Extract checksum part "CC" from the received buffer
			    	    char checksum_str[3];
			    	    snprintf(checksum_str, 3, "%c%c", uart_rx_buffer[4], uart_rx_buffer[5]);

			    	    // Convert checksum "CC" from hex to integer
		//	    	    uint8_t received_checksum;
		//	    	    sscanf(checksum_str, "%2hhx", &received_checksum);
			    	    uint8_t received_checksum = (uint8_t)strtol(checksum_str, NULL, 16);
			    	    // Calculate checksum of data "101"
			    	    uint8_t calculated_checksum = compute_checksum(data, CHECKSUM_LENGTH);
			        // Process the received data (e.g., print or echo back)
			        HAL_UART_Transmit(&huart4, uart_rx_buffer, UART_BUFFER_SIZE, 2000);  // Echo back received data
			    }
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

//===================================================================
//			MAIN TASK
//===================================================================
void MAIN_Task(void *pvParameters) {

	uint16_t timeout = 0;
	//char toPrint[2*LCD_TEXT_LEN];
	LCDtext_t lcd;
	recordtype_t memdata;
	uint16_t freqindex = 0;
	uint8_t valuechanged = 0;
	uint8_t statechanged = 1;
	uint8_t charasciicode = 'A';

	static uint8_t deleteoption = 1;

	// Init Flash (must be called inside a RTOS task)
	spif_Init_Flash();
	spif_Reset_Flash();	// soft reset Flash
	vTaskDelay(1);

	xTimerStart(BackupTimer,10);	// schedule backups every 'BACKUP_RATE' ticks

	for (;;) {

		// Main RCU State Machine
		switch (SysState.state) {

			case RCU_RESET:
			if (statechanged) {
				// just landed here
				// Read first sector and search for signature byte
				spif_ReadRecord(&memdata, 0);
				// flash not formatted: format stored frequencies
				if((memdata.signature != EMPTY_SECTOR_CODE) && (memdata.signature != OCCUPIED_SECTOR_CODE)) {
#ifdef ERASE_FLASH_AT_START
					for (freqindex = 0; freqindex < SPIF_MAX_FREQS; freqindex++) {
						spif_DeleteRecord(freqindex);
						}
#endif
				}

				// SysState sector is formatted: try to load stored SysState from flash
				if(spif_SysState_in_Flash(SYSSTATE_SECTOR)) {
					spif_Load_SysState(&SysState, SYSSTATE_SECTOR);
					SysState.state = RCU_RESET;
					}
				// SysState not available: write to Flash and load default values
				else {
					spif_Store_SysState(&DefaultSystemState, SYSSTATE_SECTOR);
					SysState = DefaultSystemState;
				}
				statechanged = 0;
			}

			if(SysState.left_button == BOTH) {
				SysState.state = RCU_SETUP;
				statechanged = 1;
			}

			else if(SysState.left_button == MOMENTARY) {
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}

			Reset_Knobs_States();

			break;

		case RCU_OFF:
			if (statechanged) {
				// just landed here
				xTimerStop(BackupTimer, 10);		// stop backup timer
				Backup_Manager();		// backup SysState if changed
				statechanged = 0;
			}

			if (SysState.left_button == MOMENTARY) {
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}
			Reset_Knobs_States();

			break;

		case RCU_NORMAL:
			if (statechanged) {
				// just landed here
				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				if (SysState.txmode == 1) lcd.left1[0] = 'T';	// indicate Transmit mode

				SendMsg2Port(1, SET_ACTIVE_FREQ); 	// Send message to Serial Port
				SendMsg2Port(1, SET_STANDBY_FREQ);  	// Send message to Serial Port

				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				xTimerStart(BackupTimer, 10);
				statechanged = 0;
			}

			// switch off RCU
			if(SysState.left_button == PRESS3SEC) {
				SysState.state = RCU_OFF;
				statechanged = 1;
			}

			// jump to manage stored frequencies
			if(SysState.left_button == PRESS2SEC) {
				SysState.state = RCU_SELECT_FREQ;
				statechanged = 1;
			}

			// change Standby frequency
				valuechanged = 0;
			if (SysState.right_inner_knob == PLUS) {
				if (SysState.Sfreq < MAX_FREQ) SysState.Sfreq += 25;
				valuechanged = 1;
			}
			if (SysState.right_inner_knob == MINUS) {
				if (SysState.Sfreq > MIN_FREQ) SysState.Sfreq -= 25;
				valuechanged = 1;
			}

			if (SysState.right_outer_knob == PLUS) {
				if (SysState.Sfreq < (MAX_FREQ - 1000)) SysState.Sfreq += 1000;
				valuechanged = 1;
			}
			if (SysState.right_outer_knob == MINUS) {
				if (SysState.Sfreq > (MIN_FREQ + 1000)) SysState.Sfreq -= 1000;
				valuechanged = 1;
			}

			// Swap Active & Standby frequencies
			if(SysState.right_button == MOMENTARY) {
				SysState.Afreq = SysState.Sfreq;
				valuechanged = 1;
				SendMsg2Port(1, SET_ACTIVE_FREQ);	// Send message to Serial Port
				SendMsg2Port(1, SET_ACTIVE_VOR_LOC_FREQ);
				SendMsg2Port(1, SET_STANDBY_VOR_LOC_FREQ);
				SendMsg2Port(1, SET_OBS);
			}

			if (valuechanged) {

				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode
				SendMsg2Port(1, SET_STANDBY_FREQ);    	// Send message to Serial Port
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				valuechanged = 0;
			}

			// jump to volume set
			if(SysState.left_inner_knob != STOP) {
				SysState.state = RCU_VOL_SET;
				statechanged = 1;
			}

			// jump to squelch set
			if(SysState.left_outer_knob != STOP) {
				SysState.state = RCU_SQL_SET;
				statechanged = 1;
			}


			// ERASE ALL FLASH
			if(SysState.L1_button == PRESS3SEC){


				LCD_BKL_OFF;
				SysState = DefaultSystemState;
				spif_Store_SysState(&SysState, SYSSTATE_SECTOR);

				for (freqindex = 0; freqindex < SPIF_MAX_FREQS; freqindex++) {
					spif_DeleteRecord(freqindex);
					}
				HAL_NVIC_SystemReset();
			}

			Reset_Knobs_States();

			break;

		case RCU_SETUP:

			if (statechanged) {
					// just landed here
				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"MicG %d",(int)SysState.gain);
				sprintf(lcd.right2,"Side %d",(int)SysState.sidetone);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				statechanged = 0;
			}

			// Mic gain adjustment
			valuechanged = 0;
			if(SysState.left_inner_knob == PLUS) {
				if (SysState.gain < MAX_GAIN) SysState.gain++;
				valuechanged = 1;
			}
			if (SysState.left_inner_knob ==	MINUS) {
				if (SysState.gain > MIN_GAIN) SysState.gain--;
				valuechanged = 1;
			}

			if (valuechanged) {
				SysState.left_inner_knob = STOP;
				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"MicG %d",(int)SysState.gain);
				sprintf(lcd.right2,"Side %d",(int)SysState.sidetone);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);		// push message to queue
			}

			// Sidetone adjustment
			valuechanged = 0;
			if (SysState.left_outer_knob == PLUS) {
				if (SysState.sidetone < MAX_SIDETONE) SysState.sidetone++;
				valuechanged = 1;
			}
			if (SysState.left_outer_knob ==	MINUS) {
				if (SysState.sidetone > MIN_SIDETONE) SysState.sidetone--;
				valuechanged = 1;
			}

			if (valuechanged) {
				SysState.left_outer_knob = STOP;
				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"MicG %d",(int)SysState.gain);
				sprintf(lcd.right2,"Side %d",(int)SysState.sidetone);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode

				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
			}

			if (SysState.left_button == BOTH) {

				SendMsg2Port(1, SET_GAIN_SIDETONE);      	// Send message to Serial Port
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}

			Reset_Knobs_States();

			break;

		case RCU_VOL_SET:
			if (statechanged) {
				// just landed here
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"Vol %d",SysState.vol);
				sprintf(lcd.right2,"Sqlch %d",SysState.squelch);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				timeout = 0;
				statechanged = 0;
			}

			valuechanged = 0;

			// Set volume
			if(SysState.left_inner_knob == PLUS) {
				if (SysState.vol < MAX_VOL) SysState.vol++;
				valuechanged = 1;
			}
			if (SysState.left_inner_knob ==	MINUS) {
				if (SysState.vol > MIN_VOL) SysState.vol--;
				valuechanged = 1;
			}
			if (valuechanged) {

				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"Vol %d",SysState.vol);
				sprintf(lcd.right2,"Sqlch %d",SysState.squelch);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode

				SendMsg2Port(1, SET_VOL_SQL);        	// Send message to Serial Port
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				valuechanged = 0;
				timeout = 0; 	// reset timeout
			}

			if (SysState.left_outer_knob != STOP) {
				SysState.state = RCU_SQL_SET;
				statechanged = 1;
			}
			// return to NORMAL state if timeout
			if((timeout++) > VOL_SQL_SETUP_TIMEOUT) {
				SendMsg2Port(1, SET_VOL_SQL);         	// Send message to Serial Port
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}

			if (SysState.left_button == PRESS3SEC) {
				SysState.state = RCU_OFF;
				statechanged = 1;
			}

			Reset_Knobs_States();
			break;

		case RCU_SQL_SET:
			if (statechanged) {
				// just landed here
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"Vol %d",SysState.vol);
				sprintf(lcd.right2,"Sqlch %d",SysState.squelch);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode

				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				timeout = 0;
				statechanged = 0;
			}
			valuechanged = 0;

			// Set squelch
			if(SysState.left_outer_knob == PLUS) {
				if (SysState.squelch < MAX_SQL) SysState.squelch++;
				valuechanged = 1;
			}
			if (SysState.left_outer_knob ==	MINUS) {
				if (SysState.squelch > MIN_SQL) SysState.squelch--;
				valuechanged = 1;
			}
			if (valuechanged) {
				sprintf(lcd.left1,"A%03d.%03d",(int)(SysState.Afreq/1000),(int)(SysState.Afreq%1000));
				sprintf(lcd.right1,"S%03d.%03d",(int)(SysState.Sfreq/1000),(int)(SysState.Sfreq%1000));
				sprintf(lcd.left2,"Vol %d",SysState.vol);
				sprintf(lcd.right2,"Sqlch %d",SysState.squelch);
				if (SysState.txmode == 1) lcd.left1[0] = 'T'; 	// indicate Transmit mode

				SendMsg2Port(1, SET_VOL_SQL);         	// Send message to Serial Port
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				valuechanged = 0;
				timeout = 0;  	// reset timeout
			}

			if (SysState.left_inner_knob != STOP) {
				SysState.state = RCU_VOL_SET;
				statechanged = 1;
			}

			// return to NORMAL state if timeout
			if((timeout++) > VOL_SQL_SETUP_TIMEOUT) {
				SendMsg2Port(1, SET_VOL_SQL);         	// Send message to Serial Port
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}

			if (SysState.left_button == PRESS3SEC) {
				SysState.state = RCU_OFF;
				statechanged = 1;
			}

			Reset_Knobs_States();

			break;

		case RCU_SELECT_FREQ:
			if (statechanged) {
				// just landed here
				// search for stored frequencies
			for(freqindex = 0 ; freqindex < SPIF_MAX_FREQS ; freqindex++) {
					if (!spif_Record_Is_Empty(freqindex)) break;

				}
				// Flash empty. Return to NORMAL mode
				if(freqindex == SPIF_MAX_FREQS) {
					glcd_clear_texts(&lcd);
					sprintf((char*)lcd.left1, " NO MEM ");
					freqindex = 0;
				}
				// at least one freq stored in memory
				else {
					spif_ReadRecord(&memdata, freqindex); 	// read label and freq from Flash

					glcd_clear_texts(&lcd);
					sprintf(lcd.left1,"%s",memdata.Name);
					sprintf(lcd.right1," %03d.%03d",(int)(memdata.freq/1000),(int)(memdata.freq%1000));
				}
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);		// push message to queue
				statechanged = 0;
				timeout = 0;
			}

			// Browse stored frequency
			valuechanged = 0;

			if (SysState.left_inner_knob == PLUS) {
				if (freqindex < SPIF_MAX_FREQS) freqindex++;
				valuechanged = 1;
			}
			if (SysState.left_inner_knob ==	MINUS) {
				if (freqindex > 0) freqindex--;
				valuechanged = 1;
			}
			if (valuechanged) {
				spif_ReadRecord(&memdata, freqindex);  	// read label and freq from flash

				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"%s",memdata.Name);
				sprintf(lcd.right1," %03d.%03d",(int)(memdata.freq/1000),(int)(memdata.freq%1000));
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				valuechanged = 0;
				timeout = 0;
			}

			// jump to add/edit frequency
			if(SysState.left_button == BOTH) {
				SysState.state = RCU_ADD_EDIT;
				statechanged = 1;
			}

			// jump to delete frequency
				if(SysState.right_button == MOMENTARY) {
				SysState.state = RCU_DELETE_FREQ;
				statechanged = 1;
			}

			// Load selected frequency an jump to normal mode
			if(SysState.left_button == MOMENTARY) {

				SysState.Sfreq = memdata.freq;
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}

			// 5sec timeout, return to NORMAL mode
			if((timeout++) > FREQ_SEL_TIMEOUT) {
				SysState.state = RCU_NORMAL;
				statechanged = 1;
			}

			Reset_Knobs_States();
			break;

		case RCU_ADD_EDIT:
			if (statechanged) {
				// just landed here
				xcursor = 0;
				// load current freq from memory:
				// patch with fake data if empty
				glcd_clear_texts(&lcd);
				if(spif_Record_Is_Empty(freqindex)) {

					sprintf((char*)lcd.left1, " EMPTY ");
					memdata.freq = MIN_FREQ;
				}
				// load actual data from flash if available
				else {
					spif_ReadRecord(&memdata, freqindex);	// show label and frequency stored in 'freqindex' sector

					sprintf(lcd.left1,"%s",memdata.Name);
					sprintf(lcd.right1," %03d.%03d",(int)(memdata.freq/1000),(int)(memdata.freq%1000));
				}

				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);		// push message to queue
				statechanged = 0;
			}

			// Move cursor
			if(SysState.left_outer_knob == PLUS) {
				if (xcursor < (RECORD_NAME_LEN - 1)) xcursor++;
			}
			if (SysState.left_outer_knob ==	MINUS) {
				if (xcursor > 0) xcursor--;
			}

			// select characters
			valuechanged = 0;
			if (SysState.left_inner_knob == PLUS) {
				if (charasciicode < 'z') charasciicode++; 	// "z" = ASCII 122
				valuechanged = 1;
			}
			if (SysState.left_inner_knob ==	MINUS) {
				if (charasciicode > ' ') charasciicode--; 	// Space = ASCII 32
				valuechanged = 1;
			}

			// set frequency
			if(SysState.right_inner_knob == PLUS) {
				if (memdata.freq < MAX_FREQ) memdata.freq += 25;
				valuechanged = 1;
			}
			if (SysState.right_inner_knob == MINUS) {
				if (memdata.freq > MIN_FREQ) memdata.freq -= 25;
				valuechanged = 1;
			}

			if (SysState.right_outer_knob == PLUS) {
				if (memdata.freq < (MAX_FREQ - 1000)) memdata.freq += 1000;
				valuechanged = 1;
			}
			if (SysState.right_outer_knob == MINUS) {
				if (memdata.freq > (MIN_FREQ + 1000)) memdata.freq -= 1000;
				valuechanged = 1;
			}

			// Label or frequency changed: Update display
			if(valuechanged) {

				memdata.Name[xcursor] = charasciicode; 	// update char
				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"%s",memdata.Name);
				sprintf(lcd.right1," %03d.%03d",(int)(memdata.freq/1000),(int)(memdata.freq%1000));
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);		// push message to queue
				valuechanged = 0;
			}

			// Write freq to memory and exit
			if(SysState.left_button == MOMENTARY) {

				memdata.signature = OCCUPIED_SECTOR_CODE;
				spif_WriteRecord(&memdata, freqindex);

				SysState.state = RCU_SELECT_FREQ;
				statechanged = 1;
			}

			Reset_Knobs_States();
			break;

		case RCU_DELETE_FREQ:
			if (statechanged) {
					// just landed here
				deleteoption = 1;
				spif_ReadRecord(&memdata, freqindex);
				glcd_clear_texts(&lcd);
				sprintf(lcd.left1,"%s",memdata.Name);
				sprintf(lcd.right1,"Delete Y");

				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// push message to queue
				statechanged = 0;
			}

			// select Y/N option
			valuechanged = 0;
			if(SysState.left_inner_knob != STOP){	// knob was moved?
				deleteoption ^= 0x01;	// switch Y/N
				valuechanged = 1;
				}

			if (valuechanged) {

				glcd_clear_texts(&lcd);
				if (deleteoption == 1) {
					sprintf(lcd.left1,"%s",memdata.Name);
					sprintf(lcd.right1,"Delete Y");
				}
				else {
					sprintf(lcd.left1,"%s",memdata.Name);
					sprintf(lcd.right1,"Delete N");
				}
				xQueueSendToBack(LCD_queue, (void *)&lcd, 10);	// update display
				valuechanged = 0;
			}

			// Confirm delete
			if(SysState.right_button == MOMENTARY) {
				if (deleteoption == 1) {
					spif_DeleteRecord(freqindex);	// delete flash
				}

				SysState.state = RCU_SELECT_FREQ;
				statechanged = 1;
			}

			Reset_Knobs_States();
			break;

		}
		vTaskDelay(10);
	}
}

//===================================================================
//			LCD DISPLAY TASK
//===================================================================
void DISPLAY_Task(void *pvParameters) {

	uint8_t delaycont = 0, cursorstate = 0;
//	char str0[2*LCD_TEXT_LEN+1];
//	char str1[LCD_TEXT_LEN+1];
	LCDtext_t lcd;
	LCD_RESET_LOW;
	LCD_BKL_ON;
	vTaskDelay(10);
	LCD_RESET_HIGH;
	vTaskDelay(100);
	glcd_on();
	vTaskDelay(100);
	glcd_clear_all();
	vTaskDelay(100);

	for (;;) {
		// OFF state: clear screen
		if (SysState.state == RCU_OFF) {
			glcd_clear_all();
		}
		else {
			// Update display if new text was written
			if(xQueueReceive(LCD_queue,(void*)&lcd, 0) == pdTRUE) {

				//glcd_clear_all();
				glcd_clearline(2);
				glcd_puts2(2,2,lcd.left1);
				glcd_puts2(66,2,lcd.right1);
				glcd_clearline(5);
				glcd_puts2(2,5,lcd.left2);
				glcd_puts2(66,5,lcd.right2);
			}

			if (SysState.state == RCU_ADD_EDIT) {
				if ((delaycont++) > CURSOR_BLINK_RATE) {
					delaycont = 0;
					//show cursor
					if (cursorstate) {
						glcd_puts("-", xcursor*8, 3); 	// xcursor is updated in MAIN task
						cursorstate = 0;
					}
					// clear cursor
					else {
						glcd_clearline(3);
						cursorstate = 1;
					}
				}
			}
			else {
				glcd_clearline(3);
				delaycont = 0;
			}
		}
		vTaskDelay(20);
	}
}

//===================================================================
//			ROTARY ENCODERS TASK
//===================================================================
void ENCODER_Task(void *pvParameters) {

	// A1, B1: center knob
	// A2, B2: outside knob
	// clockwise turn: Ax shifts before Bx
	// counterclockwise turn: Bx shifts before Ax
	// register bit order: |x|x|x|x|exA|exB|A|B

	// default values
	SysState.right_inner_knob = STOP;
	SysState.right_outer_knob = STOP;
	SysState.left_inner_knob = STOP;
	SysState.left_outer_knob = STOP;


	uint8_t left_center_knob = 0, left_outside_knob = 0;
	uint8_t right_center_knob = 0, right_outside_knob = 0;


	for (;;) {
		right_center_knob <<= 2;	// shift left old values
		right_center_knob |= 0xF3;	// default values: A=1, B=1
		right_outside_knob <<= 2;  	// shift left old values
		right_outside_knob |= 0xF3;  	// default values: A=1, B=1
		left_center_knob <<= 2;  	// shift left old values
		left_center_knob |= 0xF3;  	// default values: A=1, B=1
		left_outside_knob <<= 2;  	// shift left old values
		left_outside_knob |= 0xF3;  	// default values: A=1, B=1

		// update new values (leave bits 0-3 only)
		if (RIGHT_A1_IS_LOW) right_center_knob &=~0x02;
		if (RIGHT_B1_IS_LOW) right_center_knob &= ~0x01;
		if (RIGHT_A2_IS_LOW) right_outside_knob &= ~0x02;
		if (RIGHT_B2_IS_LOW) right_outside_knob &= ~0x01;
		if (LEFT_A1_IS_LOW)	 left_center_knob &= ~0x02;
		if (LEFT_B1_IS_LOW)	 left_center_knob &= ~0x01;
		if (LEFT_A2_IS_LOW)	 left_outside_knob &= ~0x02;
		if (LEFT_B2_IS_LOW)	 left_outside_knob &= ~0x01;

		// decode rotations
		// default values
//		left_knob_center_counter = 0;
//		right_knob_center_counter = 0;
//		left_knob_outside_counter = 0;
//		right_knob_outside_counter = 0;

		// +1: clockwise, -1: counterclockwise, 0: not moved
		if((right_center_knob == 0b11111110) || (right_center_knob == 0b11110001)) {
			//right_knob_center_counter = -1;
			SysState.right_inner_knob = MINUS;
		}

		if ((right_center_knob == 0b11111101) || (right_center_knob == 0b11110010)) {
			//right_knob_center_counter = 1;
			SysState.right_inner_knob = PLUS;
		}

		if ((right_outside_knob == 0b11111110) || (right_outside_knob == 0b11110001)) {
			//right_knob_outside_counter = -1;
			SysState.right_outer_knob = MINUS;
		}

		if ((right_outside_knob == 0b11111101) || (right_outside_knob == 0b11110010)) {
			//right_knob_outside_counter = 1;
			SysState.right_outer_knob = PLUS;
		}

		if ((left_center_knob == 0b11111110) || (left_center_knob == 0b11110001)) {
			//left_knob_center_counter = -1;
			SysState.left_inner_knob = MINUS;
		}
		if ((left_center_knob == 0b11111101) || (left_center_knob == 0b11110010)) {
			//left_knob_center_counter = 1;
			SysState.left_inner_knob = PLUS;
		}
		if ((left_outside_knob == 0b11111110) || (left_outside_knob == 0b11110001)) {
			//left_knob_outside_counter = -1;
			SysState.left_outer_knob = MINUS;
		}
		if ((left_outside_knob == 0b11111101) || (left_outside_knob == 0b11110010)) {
			//left_knob_outside_counter = 1;
			SysState.left_outer_knob = PLUS;
		}

		vTaskDelay(1);
	}
}

//===================================================================
//			PANEL BUTTONS TASK
//===================================================================
void BUTTON_Task(void *pvParameters) {

	uint16_t left_knob_counter = 0;
	uint16_t right_knob_counter = 0;
	uint16_t L1_counter = 0;
	uint16_t L2_counter = 0;
	uint16_t L3_counter = 0;
	uint16_t R1_counter = 0;
	uint16_t R2_counter = 0;
	uint16_t R3_counter = 0;

	uint16_t GP1_counter = 0;
	uint16_t GP2_counter = 0;
	uint16_t GP3_counter = 0;
	uint16_t GP4_counter = 0;

	TickType_t xLastWakeTime;

	xLastWakeTime = xTaskGetTickCount();
	for (;;) {

		// detect button release and calculate press times
		// detect both knobs pressed
		if((!LEFT_SW_IS_LOW) && (!RIGHT_SW_IS_LOW)) {
			if ((left_knob_counter > BTN_SHORT_PRESS) && (right_knob_counter > BTN_SHORT_PRESS)) {
				SysState.left_button = BOTH;
				SysState.right_button = BOTH;
				left_knob_counter = 0;
				right_knob_counter = 0;
			}
		}

		// detect single button press
		// Left knob switch released
		if(!LEFT_SW_IS_LOW) {

			if (left_knob_counter > BTN_3SEC_PRESS) SysState.left_button = PRESS3SEC;
			else if (left_knob_counter > BTN_2SEC_PRESS) SysState.left_button = PRESS2SEC;
			else if (left_knob_counter > BTN_SHORT_PRESS) SysState.left_button = MOMENTARY;
			//else SysState.left_button = OFF;
			left_knob_counter = 0;
		}
		else left_knob_counter++;

		// Right knob switch released
		if (!RIGHT_SW_IS_LOW) {

			if (right_knob_counter > BTN_3SEC_PRESS) SysState.right_button = PRESS3SEC;
			else if (right_knob_counter > BTN_2SEC_PRESS) SysState.right_button = PRESS2SEC;
			else if (right_knob_counter > BTN_SHORT_PRESS) SysState.right_button = MOMENTARY;
			//else SysState.right_button = OFF;

			right_knob_counter = 0;
		}
		else right_knob_counter++;

		// Left panel switch 1 released
		if (!SW4_IS_LOW) {

			if (L1_counter > BTN_3SEC_PRESS) SysState.L1_button = PRESS3SEC;
			else if (L1_counter > BTN_2SEC_PRESS) SysState.L1_button = PRESS2SEC;
			else if (L1_counter > BTN_SHORT_PRESS) SysState.L1_button = MOMENTARY;
			//else SysState.L1_button = OFF;

			L1_counter = 0;
		}
		else L1_counter++;

		// Left panel switch 2 released
		if (!SW5_IS_LOW) {

			if (L2_counter > BTN_3SEC_PRESS) SysState.L2_button = PRESS3SEC;
			else if (L2_counter > BTN_2SEC_PRESS) SysState.L2_button = PRESS2SEC;
			else if (L2_counter > BTN_SHORT_PRESS) SysState.L2_button = MOMENTARY;
			//else SysState.L2_button = OFF;

			L2_counter = 0;
		}
		else L2_counter++;

		// Left panel switch 3 released
		if (!SW6_IS_LOW) {

			if (L3_counter > BTN_3SEC_PRESS) SysState.L3_button = PRESS3SEC;
			else if (L3_counter > BTN_2SEC_PRESS) SysState.L3_button = PRESS2SEC;
			else if (L3_counter > BTN_SHORT_PRESS) SysState.L3_button = MOMENTARY;
			//else SysState.L3_button = OFF;

			L3_counter = 0;
		}
		else L3_counter++;

		// Right panel switch 1 released
		if (!SW1_IS_LOW) {

			if (R1_counter > BTN_3SEC_PRESS) SysState.R1_button = PRESS3SEC;
			else if (R1_counter > BTN_2SEC_PRESS) SysState.R1_button = PRESS2SEC;
			else if (R1_counter > BTN_SHORT_PRESS) SysState.R1_button = MOMENTARY;
			//else SysState.R1_button = OFF;

			R1_counter = 0;
		}
		else R1_counter++;

		// Right panel switch 2 released
		if (!SW2_IS_LOW) {

			if (R2_counter > BTN_3SEC_PRESS) SysState.R2_button = PRESS3SEC;
			else if (R2_counter > BTN_2SEC_PRESS) SysState.R2_button = PRESS2SEC;
			else if (R2_counter > BTN_SHORT_PRESS) SysState.R2_button = MOMENTARY;
			//else SysState.R2_button = OFF;

			R2_counter = 0;
		}
		else R2_counter++;

		// Right panel switch 3 released
		if (!SW3_IS_LOW) {

			if (R3_counter > BTN_3SEC_PRESS) SysState.R3_button = PRESS3SEC;
			else if (R3_counter > BTN_2SEC_PRESS) SysState.R3_button = PRESS2SEC;
			else if (R3_counter > BTN_SHORT_PRESS) SysState.R3_button = MOMENTARY;
			//else SysState.R3_button = OFF;

			R3_counter = 0;
		}
		else R3_counter++;


		// General Purpose Switch 1 released
		if (!GP_SW1_IS_LOW) {

			if (GP1_counter > BTN_3SEC_PRESS) SysState.GP1_button = PRESS3SEC;
			else if (GP1_counter > BTN_2SEC_PRESS) SysState.GP1_button = PRESS2SEC;
			else if (GP1_counter > BTN_SHORT_PRESS) SysState.GP1_button = MOMENTARY;
			//else SysState.GP1_button = OFF;

			GP1_counter = 0;
		}
		else GP1_counter++;


		// General Purpose Switch 2 released
		if (!GP_SW2_IS_LOW) {

			if (GP2_counter > BTN_3SEC_PRESS) SysState.GP2_button = PRESS3SEC;
			else if (GP2_counter > BTN_2SEC_PRESS) SysState.GP2_button = PRESS2SEC;
			else if (GP2_counter > BTN_SHORT_PRESS) SysState.GP2_button = MOMENTARY;
			//else SysState.GP2_button = OFF;

			GP2_counter = 0;
		}
		else GP2_counter++;


		// General Purpose Switch 3 released
		if (!GP_SW3_IS_LOW) {

			if (GP3_counter > BTN_3SEC_PRESS) SysState.GP3_button = PRESS3SEC;
			else if (GP3_counter > BTN_2SEC_PRESS) SysState.GP3_button = PRESS2SEC;
			else if (GP3_counter > BTN_SHORT_PRESS) SysState.GP3_button = MOMENTARY;
			//else SysState.GP3_button = OFF;

			GP3_counter = 0;
		}
		else GP3_counter++;


		// General Purpose Switch 4 released
		if (!GP_SW4_IS_LOW) {

			if (GP4_counter > BTN_3SEC_PRESS) SysState.GP4_button = PRESS3SEC;
			else if (GP4_counter > BTN_2SEC_PRESS) SysState.GP4_button = PRESS2SEC;
			else if (GP4_counter > BTN_SHORT_PRESS) SysState.GP4_button = MOMENTARY;
			//else SysState.GP4_button = OFF;

			GP4_counter = 0;
		}
		else GP4_counter++;

		vTaskDelayUntil(&xLastWakeTime, 10); 	// task rate = 10ms
	}
}

//===================================================================
//			SERIAL PORTS MANAGER TASK
//===================================================================
void PORTS_Task(void *pvParameters) {

	uint8_t i, txtlen, checksum = 0;
	uint8_t temp[14], code1, code2;
	uint32_t ftemp1, ftemp2;
	uint8_t msg[MAX_NMEA_MSG_LEN];
	CommParam_t param;
	UART_HandleTypeDef *handle;
	for (;;) {

		// Process incoming messages from PORT1
		if(xQueueReceive(PORT1_RX_queue, (void*)msg, 0) == pdTRUE) {

			Parse_NMEA_Message(&XceiverState1, (char*)msg);
		}

		// Process incoming messages from PORT2
		if(xQueueReceive(PORT2_RX_queue, (void*)msg, 0) == pdTRUE) {

			Parse_NMEA_Message(&XceiverState2, (char*)msg);
		}

		// Process incoming messages from PORT3
		if(xQueueReceive(PORT3_RX_queue, (void*)msg, 0) == pdTRUE) {

			Parse_NMEA_Message(&XceiverState3, (char*)msg);
		}

		//process outgoing messages
		// pop messages from queue
		if(xQueueReceive(PORT_queue, (void*)&param, 0) == pdTRUE) {


			// select UART instance per port number
			switch (param.portnum) {
				case 1:
					handle = &huart4;
				break;

				case 2:
					handle = &huart5;
				break;

				case 3:
					handle = &huart2;
				break;
			}

			// Pack and send message
			switch(param.msgtype) {

			case REQ_RESETSTATUS:
				sprintf((char*)temp,
					"$PMRRV2420000");
			break;

			case REQ_XCEIVERSTATUS:
				sprintf((char*)temp,
					"$PMRRV2435000");

			break;

			case REQ_SWVERSION:
				sprintf((char*)temp,
					"$PMRRV2436000");

			break;

			case REQ_VOL_SQL:
				sprintf((char*)temp,
					"$PMRRV2471000");

			break;

			case REQ_GAIN_SIDETONE:
				sprintf((char*)temp,
					"$PMRRV2472000");

			break;

			case SET_ACTIVE_FREQ:
				// split frequency into integer and decimal parts
				// ftemp1. integer parts, ftemp2: decimal part
				ftemp1 = SysState.Afreq/1000;
				ftemp2 = SysState.Afreq%1000;
				code1 = (uint8_t)(ftemp1 - 0x30);	// encode "MHz"
				code2 = (uint8_t)(ftemp2/25) + 0x30; // encode "KHz"

				sprintf((char*)temp,
					"$PMRRV42%c%c0",
					code1,
					code2);

			break;

			case SET_STANDBY_FREQ:
				// split frequency into integer and decimal parts
				// ftemp1: integer part, ftemp2: decimal part
				ftemp1 = SysState.Sfreq/1000;
				ftemp2 = SysState.Sfreq%1000;
				code1 = (uint8_t)(ftemp1 - 0x30);	// encode "MHz"
				code2 = (uint8_t)(ftemp2/25) + 0x30; // encode "KHz"

				sprintf((char*)temp,
					"$PMRRV29%c%c0",
					code1,
					code2);
			break;

			case SET_VOL_SQL:
				sprintf((char*)temp, "$PMRRV71%c%c", SysState.vol + 0x30, SysState.squelch + 0x30);
			break;

			case SET_GAIN_SIDETONE:
				sprintf((char*)temp, "$PMRRV72%c%c", SysState.gain + 0x30, SysState.sidetone + 0x30);
			break;

			case SET_STANDBY_VOR_LOC_FREQ:
				// split frequency into integer and decimal parts
				// ftemp1: integer part, ftemp2: decimal part
				ftemp1 = SysState.S_VOR_LOC_freq/1000;
				ftemp2 = SysState.S_VOR_LOC_freq%1000;
				code1 = (uint8_t)(ftemp1 - 0x30);	// encode "MHz"
				code2 = (uint8_t)(ftemp2/25) + 0x30; // encode "KHz"

				sprintf((char*)temp,
					"$PMRRV28%c%c0",
					code1,
					code2);
			break;

			case SET_ACTIVE_VOR_LOC_FREQ:
				// split frequency into integer and decimal parts
				// ftemp1. integer parts, ftemp2: decimal part
				ftemp1 = SysState.A_VOR_LOC_freq/1000;
				ftemp2 = SysState.A_VOR_LOC_freq%1000;
				code1 = (uint8_t)(ftemp1 - 0x30);	// encode "MHz"
				code2 = (uint8_t)(ftemp2/25) + 0x30; // encode "KHz"

				sprintf((char*)temp,
					"$PMRRV27%c%c0",
					code1,
					code2);
			break;

			case SET_OBS:
				// encode OBS degrees as a 3-digit integer with leading zeros
				sprintf((char*)temp,
					"$PMRRV34%03d0",
					SysState.OBS);
			break;
			}

		// calculate checksum
		txtlen = strlen((char*)temp);
		checksum = 0;
		for (i = 1; i < txtlen; i++) {

			checksum -= temp[i];
		}

		// assemble message: temp string + checksum + CR + LF
		sprintf((char*)msg, "%s%02X\r\n", temp, checksum);

		// send message to serial port
			txtlen = strlen((char*)msg);
			HAL_UART_Transmit(handle, msg, txtlen, 100);
		}

		vTaskDelay(100);
	}
}

//===================================================================
//			DEBUG TASK
//===================================================================
void DEBUG_Task(void *pvParameters) {

	//uint16_t adcval[2];
	uint32_t dac1val = 0, dac2val = 0;
	uint8_t a;
	// start ADC1 DMA channel scan
	//HAL_ADC_Start_DMA(&hadc1,
//	(uint16_t*) adcval,
//	2);


	for (;;) {

		//adcval = (uint16_t)HAL_ADC_GetValue(&hadc1);
//		printf_USB("%u\t%u\r\n", adcval[1], adcval[2]);
//		HAL_ADC_Start_DMA(&hadc1,(uint16_t*) adcval,2);
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac1val);
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac2val);

		dac1val+=100;
		dac1val%=4096;
		dac2val += 200;
		dac2val%=4096;

		// CANbus test
		a++; //increment a
		HAL_CAN_AddTxMessage(&hcan, &pHeader, &a, &TxMailbox);  //function to add message for transmition



		vTaskDelay(200);

	}
}

//-------------------------------------------------------------------
//			PRINT TO USB
//-------------------------------------------------------------------
void printf_USB(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char USBstring[64];
	uint8_t n = 0;

	vsnprintf(USBstring, 64, format, args);

#ifdef USE_SEMIHOSTING
	printf("%s", USBstring);
	printf("x\n");
#else
	USB_UART.TXbuflen = strlen(USBstring);
	while (USB_UART.TXbuflen > (MAXBUFLEN - USB_UART.LoadIndex)) ;

	while (USB_UART.TXbuflen--)
	{
		USB_UART.TXbuffer[USB_UART.LoadIndex++] = USBstring[n++];
	}

	// Start transmission if this is first byte in buffer
	if(USB_UART.TXindex == 0)
	{
		huart1.Instance->DR = USB_UART.TXbuffer[USB_UART.TXindex++];
		__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
	}
	USB_UART.TXcomplete = 1;
#endif
}

//-------------------------------------------------------------------
//			PRINT TO PORT1
//-------------------------------------------------------------------
void printf_PORT1(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char portstring[64];
	uint8_t n = 0;

	vsnprintf(portstring, 64, format, args);

#ifdef USE_SEMIHOSTING
	printf("%s", portstring);
	printf("x\n");
#else
	PORT1.TXbuflen = strlen(portstring);
	while (PORT1.TXbuflen > (MAXBUFLEN - PORT1.LoadIndex)) ;

	while (PORT1.TXbuflen--)
	{
		PORT1.TXbuffer[PORT1.LoadIndex++] = portstring[n++];
	}

	// Start transmission if this is first byte in buffer
	if(PORT1.TXindex == 0)
	{
		huart4.Instance->DR = PORT1.TXbuffer[PORT1.TXindex++];
		__HAL_UART_ENABLE_IT(&huart4, UART_IT_TXE);
	}
	PORT1.TXcomplete = 1;
#endif
}

//-------------------------------------------------------------------
//			PRINT TO PORT2
//-------------------------------------------------------------------
void printf_PORT2(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char portstring[64];
	uint8_t n = 0;

	vsnprintf(portstring, 64, format, args);

#ifdef USE_SEMIHOSTING
	printf("%s", portstring);
	printf("x\n");
#else
	PORT2.TXbuflen = strlen(portstring);
	while (PORT2.TXbuflen > (MAXBUFLEN - PORT2.LoadIndex)) ;

	while (PORT2.TXbuflen--)
	{
		PORT2.TXbuffer[PORT2.LoadIndex++] = portstring[n++];
	}

	// Start transmission if this is first byte in buffer
	if(PORT2.TXindex == 0)
	{
		huart5.Instance->DR = PORT2.TXbuffer[PORT2.TXindex++];
		__HAL_UART_ENABLE_IT(&huart5, UART_IT_TXE);
	}
	PORT2.TXcomplete = 1;
#endif
}

//-------------------------------------------------------------------
//			PRINT TO PORT3
//-------------------------------------------------------------------
void printf_PORT3(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char portstring[64];
	uint8_t n = 0;

	vsnprintf(portstring, 64, format, args);

#ifdef USE_SEMIHOSTING
	printf("%s", portstring);
	printf("x\n");
#else
	PORT3.TXbuflen = strlen(portstring);
	while (PORT3.TXbuflen > (MAXBUFLEN - PORT3.LoadIndex)) ;

	while (PORT3.TXbuflen--)
	{
		PORT3.TXbuffer[PORT3.LoadIndex++] = portstring[n++];
	}

	// Start transmission if this is first byte in buffer
	if(PORT3.TXindex == 0)
	{
		huart2.Instance->DR = PORT3.TXbuffer[PORT3.TXindex++];
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
	}
	PORT3.TXcomplete = 1;
#endif
}

//-------------------------------------------------------------------
//  RESET KNOBS STATES
//
/*! \fn void Reset_Knobs_States(void)
\brief Reset knobs registers to default values

Set knobs to STOP mode and buttons to OFF mode
\return None
*/
//-------------------------------------------------------------------
void Reset_Knobs_States(void)
{
	SysState.left_button = OFF;
	SysState.right_button = OFF;
	SysState.left_inner_knob = STOP;
	SysState.left_outer_knob = STOP;
	SysState.right_inner_knob = STOP;
	SysState.right_outer_knob = STOP;
}

//-------------------------------------------------------------------
//  BACKUP MANAGER
//
/*! \fn void Backup_Manager(void)
\brief Manage scheduled data backups in flash

Callback function of RTOS timer
\return None
*/
//-------------------------------------------------------------------
void Backup_Manager(void)
{
	RCUsystemstate_t oldSysState, newSysState;

	//  Backup SysState
	// check SysState copy in flash.
	spif_Load_SysState(&oldSysState, SYSSTATE_SECTOR);
	// Compare with current version.
	newSysState = SysState;
	newSysState.state = RCU_RESET;	// compare structs in Reset mode
	if(memcmp(&newSysState, &oldSysState, (size_t)sizeof(SysState)) != 0) {
		//  Overwrite old version in flash if changed
		spif_Store_SysState(&newSysState, SYSSTATE_SECTOR);
	}
}

//-------------------------------------------------------------------
//  PARSE NMEA MESSAGE
//
/*! \fn void Parse_NMEA_Message(xceiverstate_t *x,char *msg)
\brief Parse messagefrom COM2K device

copies data form output messages to the
\return None
*/
//-------------------------------------------------------------------
void Parse_NMEA_Message(xceiverstate_t *x /* Pointer to xceiverstate_t struct for this port */, char *msg /* message text */)
{

	// Check RESET STATUS
	if(strstr(msg, "$PMRRV20")){
		x->status = 1;	// Xceiver online
	}

	// Check COMMUNICATIONS ERROR
	if(strstr(msg, "$PMRRV27")){
		x->errorcode = msg[8] - '0';	// Error code
	}

	// Check COM TRANSCEIVER STATUS
	if(strstr(msg, "$PMRRV35")){
		// read Active freq
		x->Afreq = ((uint32_t)msg[8] + 0x30) * 1000;
		x->Afreq += ((uint32_t)msg[9] - 0x30) * 25;
		// read Standby freq
		x->Sfreq = ((uint32_t)msg[10] + 0x30) * 1000;
		x->Sfreq += ((uint32_t)msg[11] - 0x30) * 25;

		// Read xceiver status
		x->status = msg[12];

		// read SQL test mode
		x->sqlstatus = msg[13];
	}

	// Check COM SOFTWARE VERSION
	if(strstr(msg, "$PMRRV30")){
		strncpy(x->ver, (char*)(msg+8),5);
		x->ver[4]=0;	// truncate string
	}

	// Check VOLUME AND SQUELCH
	if(strstr(msg, "$PMRRV71")){
		x->vol = msg[8] - 0x30;
		x->squelch = msg[9] - 0x30;
	}

	// Check MIC GAIN AND SIDETONE
	if(strstr(msg, "$PMRRV72")){
		x->gain = msg[8] - 0x30;
		x->sidetone = msg[9] - 0x30;
	}
}

//-------------------------------------------------------------------
//  PRINT STRING TO LCD
//
/*! \fn void glcd_puts2(uint8_t x,uint8_t y,char *s)
\brief Print string to LCD with fix for decimal dots

Print string at (x,y), with fix for decimal dot
\return None
*/
//-------------------------------------------------------------------
void glcd_puts2(uint8_t x /* screen row */, uint8_t y /* screen column */, char *s /* String to print */)
{
char str0[8], str1[8], *p;
uint8_t	textlen;
	// Check for dots in the string
	p = strchr(s, '.');

	// dot found -> fix string
	if(p){
		*p = 0;
		strncpy(str0, s, 8);	// string before dot
		textlen = strlen(str0)*8;
		strncpy(str1,(char*)(p+1), 8); // string after dot
		str1[3]=0;						// truncate string
		glcd_puts(str0, x, y);	// print split string with offset
		glcd_puts(".", x+ textlen - 1, y);
		glcd_puts(str1, x+textlen+3, y);
	}
	// No dots -> print normal
	else{
		// patch for Squelch string
		if(strstr(s,"Sqlch")){
			p = strchr(s, ' ');
			*p = 0;
			strncpy(str0, s, 8);	// string before space
			strncpy(str1,(char*)(p+1), 8); // string after space
			str1[3]=0;						// truncate string
			glcd_puts("Sqlch ", x, y);	// print split string with offset
			glcd_puts(str1, x+44, y);
		}

		else if(strstr(s,"Delete")){
			p = strchr(s, ' ');
			*p = 0;
			strncpy(str0, s, 8);	// string before space
			strncpy(str1,(char*)(p+1), 8); // string after space
			str1[3]=0;						// truncate string
			glcd_puts("Delete ", x, y);	// print split string with offset
			glcd_puts(str1, x+52, y);
		}

		else{
			glcd_puts(s, x, y);
		}
	}
}


//-------------------------------------------------------------------
//  CLEAR LCD TEXTS
//
/*! \fn void glcd_clear_texts(LCDtext_t *x)
\brief Clear LCD strings

set all 4 LCDtext sttings to NULL
\return None
*/
//-------------------------------------------------------------------
void glcd_clear_texts(LCDtext_t *x /* pointer to LCDtext_t struct with strings */)
{
	x->left1[0] = 0;
	x->right1[0] = 0;
	x->left2[0] = 0;
	x->right2[0] = 0;
}


void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	(void) pxTask;
	(void) pcTaskName;

	for (;;) ;
}

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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
