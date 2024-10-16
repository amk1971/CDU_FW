/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    REMOTE CONTROL UNIT - RCU
    Main.c

    MCU:        STMicroelectronics STM32F103VCT6
    Flash:      BoyaMicro BY25Q16BSTIG(T)
    Display:    Crystalfontz CFAG12864I-STI-TN

    GPD, 10/2024
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

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
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
//formatted I/O objects
SerialStruct USB_UART;
SerialStruct GP_UART;
SerialStruct PORT1, PORT2, PORT3;
char text[128], resp[2];

// serial message parameter list
CommParam_t CommParams;
// Global system state
RCUsystemstate_t SysState;		// global System State instance

// Default State parameters
const RCUsystemstate_t DefaultSystemState =  {
	.signature = OCCUPIED_SECTOR_CODE,
	.state = RCU_RESET,
	.Afreq = 118.000,
	.Sfreq = 118.000,
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
	.port1power = POWER_ON,
	.port2power = POWER_OFF,
	.port3power = POWER_OFF,
	};

// RTOS queue for display texts
QueueHandle_t LCD_queue; // RTOS queue to stream messages to the display
QueueHandle_t PORT_queue; // RTOS queue to stream messages to serial ports
// SysState backup scheduler
TimerHandle_t BackupTimer;	// timer to schedule SysState backup to Flash
uint8_t xcursor = 1;		// blinking cursor position (during (add/edit frequency)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */

//RTOS tasks
void MAIN_Task(void *pvParameters);      	// RTOS task to manage general RCU operation
void DISPLAY_Task(void *pvParameters);       // RTOS task to control LCD low level I/O
void ENCODER_Task(void *pvParameters);        // Read rotary encoders
void BUTTON_Task(void *pvParameters);         // Read panel buttons, de-bounce, etc.
void PORTS_Task(void *pvParameters);         // Manage serial ports
void DEBUG_Task(void *pvParameters);       // debug messages

void printf_USB(const char* format, ...);	// printf for USB port
void printf_PORT1(const char* format, ...); 	// printf for SubD-15 Port 1
void printf_PORT2(const char* format, ...); 	// printf for SubD-15 Port 2
void printf_PORT3(const char* format, ...); 	// printf for SubD-15 Port 3
void printf_GP_PORT(const char* format, ...); 	// printf for gen. purpose serial port

void Reset_Knobs_States(void);	// return all knobs button to default modes
void Backup_Manager(void);		// manage SysState backups

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
  MX_USART3_UART_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
	// Default RCU state
	SysState = DefaultSystemState;

	// reset UART RX buffers
	USB_UART.RXbuffer[0] = 0;
	USB_UART.RXindex = 0;
	PORT1.RXbuffer[0] = 0;
	PORT1.RXindex = 0;
	PORT2.RXbuffer[0] = 0;
	PORT2.RXindex = 0;
	PORT3.RXbuffer[0] = 0;
	PORT3.RXindex = 0;
	GP_UART.RXbuffer[0] = 0;
	GP_UART.RXindex = 0;

	// Enable UART RX interrupts
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
//	__HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
//	__HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);
	RELAY1_OFF;
	RELAY2_OFF;
	RELAY3_OFF;

	// Queues
	LCD_queue = xQueueCreate(LCD_QUEUE_LEN, LCD_TEXT_LEN);
	PORT_queue = xQueueCreate(6, sizeof(CommParams));

	// SysState backup timer
	BackupTimer = xTimerCreate("Backup", BACKUP_RATE, pdTRUE, 0, Backup_Manager);

	// Start RTOS tasks
	xTaskCreate(MAIN_Task, "Capture", configMINIMAL_STACK_SIZE+300, NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(DISPLAY_Task, "Main", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(ENCODER_Task, "Debug", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 4, NULL);
	xTaskCreate(BUTTON_Task, "LEDs", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(PORTS_Task, "LEDs", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(DEBUG_Task, "LEDs", configMINIMAL_STACK_SIZE + 300, NULL, tskIDLE_PRIORITY + 1, NULL);

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
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, RELAY3_Pin|MCU_DTR3_Pin|LCD_DB1_Pin|LCD_RW_Pin
                          |LCD_E_Pin|LCD_DI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_BKL_Pin|LCD_RESET_Pin|LCD_CS2_Pin|LCD_CS1_Pin
                          |LCD_DB7_Pin|LCD_DB6_Pin|LCD_DB5_Pin|KEY_COL4_Pin
                          |KEY_COL7_Pin|KEY_COL6_Pin|KEY_COL8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_DB4_Pin|LCD_DB2_Pin|KEY_COL0_Pin|KEY_COL3_Pin
                          |KEY_COL2_Pin|KEY_COL5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_DB3_Pin|LCD_DB0_Pin|MCU_DTR2_Pin|FLASH_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, KEY_COL1_Pin|KEY_COL9_Pin|RELAY1_Pin|MCU_DTR1_Pin
                          |RELAY2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RELAY3_Pin MCU_DTR3_Pin LCD_DB1_Pin LCD_RW_Pin
                           LCD_E_Pin LCD_DI_Pin */
  GPIO_InitStruct.Pin = RELAY3_Pin|MCU_DTR3_Pin|LCD_DB1_Pin|LCD_RW_Pin
                          |LCD_E_Pin|LCD_DI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : MCU_DSR3_Pin GP_SW4_Pin GP_SW5_Pin GP_SW1_Pin
                           GP_SW2_Pin GP_SW3_Pin RIGHT_SW_Pin RIGHT_A1_Pin */
  GPIO_InitStruct.Pin = MCU_DSR3_Pin|GP_SW4_Pin|GP_SW5_Pin|GP_SW1_Pin
                          |GP_SW2_Pin|GP_SW3_Pin|RIGHT_SW_Pin|RIGHT_A1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : GP_SW6_Pin LEFT_SW_Pin LEFT_A1_Pin LEFT_B1_Pin
                           LEFT_B2_Pin */
  GPIO_InitStruct.Pin = GP_SW6_Pin|LEFT_SW_Pin|LEFT_A1_Pin|LEFT_B1_Pin
                          |LEFT_B2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LEFT_A2_Pin */
  GPIO_InitStruct.Pin = LEFT_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LEFT_A2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_BKL_Pin LCD_RESET_Pin LCD_CS2_Pin LCD_CS1_Pin
                           LCD_DB7_Pin LCD_DB6_Pin LCD_DB5_Pin KEY_COL4_Pin
                           KEY_COL7_Pin KEY_COL6_Pin KEY_COL8_Pin */
  GPIO_InitStruct.Pin = LCD_BKL_Pin|LCD_RESET_Pin|LCD_CS2_Pin|LCD_CS1_Pin
                          |LCD_DB7_Pin|LCD_DB6_Pin|LCD_DB5_Pin|KEY_COL4_Pin
                          |KEY_COL7_Pin|KEY_COL6_Pin|KEY_COL8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DB4_Pin LCD_DB2_Pin KEY_COL0_Pin KEY_COL3_Pin
                           KEY_COL2_Pin KEY_COL5_Pin */
  GPIO_InitStruct.Pin = LCD_DB4_Pin|LCD_DB2_Pin|KEY_COL0_Pin|KEY_COL3_Pin
                          |KEY_COL2_Pin|KEY_COL5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DB3_Pin LCD_DB0_Pin MCU_DTR2_Pin FLASH_CS_Pin */
  GPIO_InitStruct.Pin = LCD_DB3_Pin|LCD_DB0_Pin|MCU_DTR2_Pin|FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BOOT1_Pin RIGHT_B1_Pin RIGHT_B2_Pin RIGHT_A2_Pin
                           MCU_DSR2_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin|RIGHT_B1_Pin|RIGHT_B2_Pin|RIGHT_A2_Pin
                          |MCU_DSR2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_ROW1_Pin */
  GPIO_InitStruct.Pin = KEY_ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_ROW1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY_ROW0_Pin KEY_ROW3_Pin KEY_ROW2_Pin KEY_ROW5_Pin
                           KEY_ROW4_Pin KEY_ROW7_Pin KEY_ROW6_Pin */
  GPIO_InitStruct.Pin = KEY_ROW0_Pin|KEY_ROW3_Pin|KEY_ROW2_Pin|KEY_ROW5_Pin
                          |KEY_ROW4_Pin|KEY_ROW7_Pin|KEY_ROW6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY_COL1_Pin KEY_COL9_Pin RELAY1_Pin MCU_DTR1_Pin
                           RELAY2_Pin */
  GPIO_InitStruct.Pin = KEY_COL1_Pin|KEY_COL9_Pin|RELAY1_Pin|MCU_DTR1_Pin
                          |RELAY2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : MCU_DSR1_Pin */
  GPIO_InitStruct.Pin = MCU_DSR1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MCU_DSR1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
//===================================================================
//			MAIN TASK
//===================================================================
void MAIN_Task(void *pvParameters) {

	uint16_t timeout = 0;
	char toPrint[LCD_TEXT_LEN];
	recordtype_t memdata;
	uint16_t freqindex;
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
				sprintf(toPrint,
					"A%07.3fS%07.3f",
					SysState.Afreq,
					SysState.Sfreq);

				if (SysState.txmode == 1) toPrint[0] = 'T';	// indicate Transmit mode

				SendMsg2Port(1, SET_ACTIVE_FREQ); 	// Send message to Serial Port
				SendMsg2Port(1, SET_STANDBY_FREQ);  	// Send message to Serial Port

				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
				if (SysState.Sfreq < MAX_FREQ) SysState.Sfreq += 0.025;
				valuechanged = 1;
			}
			if (SysState.right_inner_knob == MINUS) {
				if (SysState.Sfreq > MIN_FREQ) SysState.Sfreq -= 0.025;
				valuechanged = 1;
			}

			if (SysState.right_outer_knob == PLUS) {
				if (SysState.Sfreq < (MAX_FREQ - 1)) SysState.Sfreq++;
				valuechanged = 1;
			}
			if (SysState.right_outer_knob == MINUS) {
				if (SysState.Sfreq > (MIN_FREQ + 1)) SysState.Sfreq--;
				valuechanged = 1;
			}

			// Swap Active & Standby frequencies
			if(SysState.right_button == MOMENTARY) {
				SysState.Afreq = SysState.Sfreq;
				valuechanged = 1;
				SendMsg2Port(1, SET_ACTIVE_FREQ);	// Send message to Serial Port
			}

			if (valuechanged) {

				sprintf(toPrint,
					"A%07.3fS%07.3f",
					SysState.Afreq,
					SysState.Sfreq);
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode
				SendMsg2Port(1, SET_STANDBY_FREQ);    	// Send message to Serial Port
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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

			Reset_Knobs_States();

			break;

		case RCU_SETUP:

			if (statechanged) {
					// just landed here
				sprintf(toPrint,
					"A%07.3f MicG %02d",
					SysState.gain);
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
				sprintf(toPrint,
					"A%07.3f MicG %02u",
					SysState.Afreq,
					SysState.gain);
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, toPrint, 10);		// push message to queue
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
				sprintf(toPrint,
					"A%07.3f Side %02u",
					SysState.Afreq,
					SysState.sidetone);
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode

				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
				if(SysState.vol != 0) {
					sprintf(toPrint,
						"A%07.3f VOL %02d ",
						SysState.Afreq,
						SysState.vol);
				}

				else {
					sprintf(toPrint,
						"A%07.3f VOL OFF",
						SysState.Afreq,
						SysState.vol);
				}
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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

				if (SysState.vol != 0) {
					sprintf(toPrint,
						"A%07.3f VOL %02d ",
						SysState.Afreq,
						SysState.vol);
				}

				else {
					sprintf(toPrint,
						"A%07.3f VOL OFF",
						SysState.Afreq,
						SysState.vol);
				}
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode


				SendMsg2Port(1, SET_VOL_SQL);        	// Send message to Serial Port
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
				if(SysState.squelch != 0) {
					sprintf(toPrint,
						"A%07.3f SQL %02d ",
						SysState.Afreq,
						SysState.squelch);
				}

				else {
					sprintf(toPrint,
						"A%07.3f SQL OFF",
						SysState.Afreq,
						SysState.squelch);
				}
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
				if (SysState.squelch != 0) {
					sprintf(toPrint,
						"A%07.3f SQL %02d ",
						SysState.Afreq,
						SysState.squelch);
				}

				else {
					sprintf(toPrint,
						"A%07.3f SQL OFF",
						SysState.Afreq,
						SysState.squelch);
				}
				if (SysState.txmode == 1) toPrint[0] = 'T'; 	// indicate Transmit mode
				SendMsg2Port(1, SET_VOL_SQL);         	// Send message to Serial Port
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
					sprintf(toPrint, "     NO MEM       ");
					freqindex = 0;
				}
				// at least one freq stored in memory
				else {
					spif_ReadRecord(&memdata, freqindex); 	// read label and freq from Flash
					sprintf(toPrint,
						"%s %07.3f ",
						memdata.Name,
						memdata.freq);
				}
				xQueueSendToBack(LCD_queue, toPrint, 10);		// push message to queue
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
				sprintf(toPrint,
					"%s %07.3f ",
					memdata.Name,
					memdata.freq);
				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
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
				if(spif_Record_Is_Empty(freqindex)) {

					sprintf(toPrint, "EMPTY             ");
					memdata.freq = MIN_FREQ;
				}
				// load actual data from flash if available
				else {
					spif_ReadRecord(&memdata, freqindex);	// show label and frequency stored in 'freqindex' sector
					sprintf(toPrint,
						"%s %07.3f ",
						memdata.Name,
						memdata.freq);
				}


				xQueueSendToBack(LCD_queue, toPrint, 10);		// push message to queue
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
				if (memdata.freq < MAX_FREQ) memdata.freq += 0.025;
				valuechanged = 1;
			}
			if (SysState.right_inner_knob == MINUS) {
				if (memdata.freq > MIN_FREQ) memdata.freq -= 0.025;
				valuechanged = 1;
			}

			if (SysState.right_outer_knob == PLUS) {
				if (memdata.freq < (MAX_FREQ - 1)) memdata.freq++;
				valuechanged = 1;
			}
			if (SysState.right_outer_knob == MINUS) {
				if (memdata.freq > (MIN_FREQ + 1)) memdata.freq--;
				valuechanged = 1;
			}

			// Label or frequency changed: Update display
			if(valuechanged) {

				memdata.Name[xcursor] = charasciicode; 	// update char
				sprintf(toPrint,
					"%s %07.3f ",
					memdata.Name,
					memdata.freq);

				xQueueSendToBack(LCD_queue, toPrint, 10);		// push message to queue
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
				sprintf(toPrint,
					"%s Delete Y",
					memdata.Name);

				xQueueSendToBack(LCD_queue, toPrint, 10);	// push message to queue
				statechanged = 0;
			}

			// select Y/N option
			valuechanged = 0;
			if(SysState.left_inner_knob != STOP){	// knob was moved?
				deleteoption ^= 0x01;	// switch Y/N
				valuechanged = 1;
				}

			if (valuechanged) {
				if (deleteoption == 1) {
					sprintf(toPrint,
						"%s Delete Y",
						memdata.Name);
				}
				else {
					sprintf(toPrint,
						"%s Delete N",
						memdata.Name);
				}
				xQueueSendToBack(LCD_queue, toPrint, 10);	// update display
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

	uint8_t i;
	uint8_t micgain = 1;
	uint8_t delaycont = 0, cursorstate = 0;
	char str0[LCD_TEXT_LEN+1];
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
			if(xQueueReceive(LCD_queue, str0, 0) == pdTRUE) {
				str0[LCD_TEXT_LEN] = 0;		// null terminate string
				glcd_puts(str0, 0, 3);
			}

			if (SysState.state == RCU_ADD_EDIT) {
				if ((delaycont++) > CURSOR_BLINK_RATE) {
					delaycont = 0;
					//show cursor
					if (cursorstate) {
						glcd_puts("-", xcursor*8, 4); 	// xcursor is updated in MAIN task
						cursorstate = 0;
					}
					// clear cursor
					else {
						glcd_clearline(4);
						cursorstate = 1;
					}
				}
			}
			else {
				glcd_clearline(4);
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
		if (!GP_SW1_IS_LOW) {

			if (L1_counter > BTN_3SEC_PRESS) SysState.L1_button = PRESS3SEC;
			else if (L1_counter > BTN_2SEC_PRESS) SysState.L1_button = PRESS2SEC;
			else if (L1_counter > BTN_SHORT_PRESS) SysState.L1_button = MOMENTARY;
			//else SysState.L1_button = OFF;

			L1_counter = 0;
		}
		else L1_counter++;

		// Left panel switch 2 released
		if (!GP_SW2_IS_LOW) {

			if (L2_counter > BTN_3SEC_PRESS) SysState.L2_button = PRESS3SEC;
			else if (L2_counter > BTN_2SEC_PRESS) SysState.L2_button = PRESS2SEC;
			else if (L2_counter > BTN_SHORT_PRESS) SysState.L2_button = MOMENTARY;
			//else SysState.L2_button = OFF;

			L2_counter = 0;
		}
		else L2_counter++;

		// Left panel switch 3 released
		if (!GP_SW3_IS_LOW) {

			if (L3_counter > BTN_3SEC_PRESS) SysState.L3_button = PRESS3SEC;
			else if (L3_counter > BTN_2SEC_PRESS) SysState.L3_button = PRESS2SEC;
			else if (L3_counter > BTN_SHORT_PRESS) SysState.L3_button = MOMENTARY;
			//else SysState.L3_button = OFF;

			L3_counter = 0;
		}
		else L3_counter++;

		// Right panel switch 1 released
		if (!GP_SW4_IS_LOW) {

			if (R1_counter > BTN_3SEC_PRESS) SysState.R1_button = PRESS3SEC;
			else if (R1_counter > BTN_2SEC_PRESS) SysState.R1_button = PRESS2SEC;
			else if (R1_counter > BTN_SHORT_PRESS) SysState.R1_button = MOMENTARY;
			//else SysState.R1_button = OFF;

			R1_counter = 0;
		}
		else R1_counter++;

		// Right panel switch 2 released
		if (!GP_SW5_IS_LOW) {

			if (R2_counter > BTN_3SEC_PRESS) SysState.R2_button = PRESS3SEC;
			else if (R2_counter > BTN_2SEC_PRESS) SysState.R2_button = PRESS2SEC;
			else if (R2_counter > BTN_SHORT_PRESS) SysState.R2_button = MOMENTARY;
			//else SysState.R2_button = OFF;

			R2_counter = 0;
		}
		else R2_counter++;

		// Right panel switch 3 released
		if (!GP_SW6_IS_LOW) {

			if (R3_counter > BTN_3SEC_PRESS) SysState.R3_button = PRESS3SEC;
			else if (R3_counter > BTN_2SEC_PRESS) SysState.R3_button = PRESS2SEC;
			else if (R3_counter > BTN_SHORT_PRESS) SysState.R3_button = MOMENTARY;
			//else SysState.R3_button = OFF;

			R3_counter = 0;
		}
		else R3_counter++;


		vTaskDelayUntil(&xLastWakeTime, 10); 	// task rate = 10ms
	}
}

//===================================================================
//			SERIAL PORTS MANAGER TASK
//===================================================================
void PORTS_Task(void *pvParameters) {

	uint8_t i, txtlen, checksum = 0;
	uint8_t temp[13], code1, code2;
	double ftemp1, ftemp2;
	uint8_t msg[26];
	CommParam_t param;
	UART_HandleTypeDef *handle;
	for (;;) {

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
				sprintf(temp,
					"$PMRRV2420000");
			break;

			case REQ_XCEIVERSTATUS:
				sprintf(temp,
					"$PMRRV2435000");

			break;

			case REQ_SWVERSION:
				sprintf(temp,
					"$PMRRV2436000");

			break;

			case REQ_VOL_SQL:
				sprintf(temp,
					"$PMRRV2471000");

			break;

			case REQ_GAIN_SIDETONE:
				sprintf(temp,
					"$PMRRV2472000");

			break;

			case SET_ACTIVE_FREQ:
				// split frequency into integer and decimal parts
				// ftemp1. integer parts, ftemp2: decimal part
				ftemp2 = modf((double)SysState.Afreq, (double*)&ftemp1);
				code1 = (uint8_t)(ftemp1 - 48.0);	// encode "MHz"
				code2 = (uint8_t)(ftemp2 / .025) + 0x30; // encode "KHz"

				sprintf(temp,
					"$PMRRV42%c%c0",
					code1,
					code2);

			break;

			case SET_STANDBY_FREQ:
				// split frequency into integer and decimal parts
				// ftemp1: integer part, ftemp2: decimal part
				ftemp2 = modf((double)SysState.Sfreq, (double*)&ftemp1);
				code1 = (uint8_t)(ftemp1 - 48.0);	// encode "MHz"
				code2 = (uint8_t)(ftemp2 /.025) + 0x30; // encode "KHz"

				sprintf(temp,
					"$PMRRV29%c%c0",
					code1,
					code2);
			break;

			case SET_VOL_SQL:
				sprintf(temp, "$PMRRV71%c%c", SysState.vol + 0x30, SysState.squelch + 0x30);
			break;

			case SET_GAIN_SIDETONE:
				sprintf(temp, "$PMRRV72%c%c", SysState.gain + 0x30, SysState.sidetone + 0x30);
			break;
			}

		// calculate checksum
		txtlen = strlen(temp);
		checksum = 0;
		for (i = 1; i < txtlen; i++) {

			checksum -= temp[i];
		}

		// assemble message: temp string + checksum + CR + LF
		sprintf(msg, "%s%02X\r\n", temp, checksum);

		// send message to serial port
			txtlen = strlen(msg);
			HAL_UART_Transmit(handle, msg, txtlen, 100);
		}

		vTaskDelay(100);
	}
}

//===================================================================
//			DEBUG TASK
//===================================================================
void DEBUG_Task(void *pvParameters) {


	for (;;) {


		vTaskDelay(-1); // sleep forever
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
//			PRINT TO GP SERIAL PORT
//-------------------------------------------------------------------
void printf_GP_PORT(const char* format, ...)
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
	GP_UART.TXbuflen = strlen(portstring);
	while (GP_UART.TXbuflen > (MAXBUFLEN - GP_UART.LoadIndex)) ;

	while (GP_UART.TXbuflen--)
	{
		GP_UART.TXbuffer[GP_UART.LoadIndex++] = portstring[n++];
	}

	// Start transmission if this is first byte in buffer
	if(GP_UART.TXindex == 0)
	{
		huart3.Instance->DR = GP_UART.TXbuffer[GP_UART.TXindex++];
		__HAL_UART_ENABLE_IT(&huart3, UART_IT_TXE);
	}
	GP_UART.TXcomplete = 1;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
