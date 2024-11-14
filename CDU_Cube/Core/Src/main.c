/* USER CODE BEGIN Header */
/**

CDU - CONTROL AND DISPLAY UNIT

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
// Flash manager
#include "flash_mgr.h"
// Serial port protocol
#include "NMEA0183.h"

// Keyboard decoder
#include "CDU_keyboard.h"

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
CAN_HandleTypeDef hcan;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
//SemaphoreHandle_t Semaphore;
//formatted I/O objects
SerialStruct USB_UART;
SerialStruct DISPLAY;
SerialStruct PORT1, PORT2, PORT3, PORT4;
// serial message parameter list
CommParam_t CommParams;
// Global system state
systemstate_t SysState; 		// global System State instance
xceiverstate_t XceiverState1;	// Transceiver at PORT1 State instance
xceiverstate_t XceiverState2;	// Transceiver at PORT2 State instance
xceiverstate_t XceiverState3;	// Transceiver at PORT3 State instance
xceiverstate_t XceiverState4;	// Transceiver at PORT4 State instance
// Default State parameters
const systemstate_t DefaultSystemState = {
	.signature = OCCUPIED_SECTOR_CODE,
	.state = CDU_RESET,
	.Afreq = 118.000,
	.Sfreq = 118.000,
	.vol = 0,
	.squelch = 0,
	.sidetone = 0,
	.gain = 0,
	.txmode = 0,
	.L1_button = OFF,
	.L2_button = OFF,
	.L3_button = OFF,
	.L4_button = OFF,
	.R1_button = OFF,
	.R2_button = OFF,
	.R3_button = OFF,
	.R4_button = OFF,
	.GP1_button = OFF,
	.GP2_button = OFF,
	.GP3_button = OFF,
	.GP4_button = OFF,
	.port1power = POWER_ON,
	.port2power = POWER_OFF,
	.port3power = POWER_OFF,
	.port4power = POWER_OFF,
	.DisplayBypass = 0
};

QueueHandle_t DISPLAY_queue;  // RTOS queue to stream messages to the display
QueueHandle_t PORT_queue;   // RTOS queue to stream messages to serial ports
QueueHandle_t PORT1_RX_queue;
QueueHandle_t PORT2_RX_queue;
QueueHandle_t PORT3_RX_queue;
QueueHandle_t PORT4_RX_queue;

TimerHandle_t BackupTimer; 	// timer to schedule SysState backup to Flash

// CAN Objects
CAN_TxHeaderTypeDef pHeader; //declare a specific header for message transmissions
CAN_RxHeaderTypeDef pRxHeader; //declare header for message reception
uint32_t TxMailbox;
uint8_t a,r; //declare byte to be transmitted //declare a receive byte
CAN_FilterTypeDef sFilterConfig; //declare CAN filter structure


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART4_UART_Init(void);
static void MX_USART5_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */
// RTOS Tasks
void MAIN_Task(void *pvParameters);         // Main state machine processing
void KEYBOARD_Task(void *pvParameters);     	// Task to read buttons
void PORTS_Task(void *pvParameters);       // Task to manage Serial Ports 1-4
void DISPLAY_Task(void *pvParameters);        // manage display messages
void DEBUG_Task(void *pvParameters);         // Misc debug tasks
void printf_USB(const char* format, ...); 	// printf for USB port
void printf_DISPLAY(const char* format, ...); 	// printf for display commands
void Reset_Button_States(void); 	// return all buttons to default modes
void Backup_Manager(void); 		// manage SysState backups
void Reset_Switch_States(void);
void Parse_NMEA_Message(xceiverstate_t *x,char *msg);

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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART4_UART_Init();
  MX_USART5_UART_Init();
  MX_USART6_UART_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
	// Default CDU state
	SysState = DefaultSystemState;

	// reset UART RX buffers
	DISPLAY.RXbuffer[0] = 0;
	DISPLAY.RXindex = 0;
	PORT1.RXbuffer[0] = 0;
	PORT1.RXindex = 0;
	PORT2.RXbuffer[0] = 0;
	PORT2.RXindex = 0;
	PORT3.RXbuffer[0] = 0;
	PORT3.RXindex = 0;
	PORT4.RXbuffer[0] = 0;
	PORT4.RXindex = 0;
	DISPLAY.RXbuffer[0] = 0;
	DISPLAY.RXindex = 0;

	// CAN bus  declarations
	pHeader.DLC=1; //give message size of 1 byte
	pHeader.IDE=CAN_ID_STD; //set identifier to standard
	pHeader.RTR=CAN_RTR_DATA; //set data type to remote transmission request
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
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING); //enable interrupts

	// Enable UART RX interrupts
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE);
	RELAY1_OFF;
	RELAY2_OFF;
	RELAY3_OFF;
	RELAY4_OFF;

	CAN_SHUTDOWN_DISABLED;
	CAN_STANDBY_DISABLED;

	// Queues
	DISPLAY_queue = xQueueCreate(3, DISPLAY_TEXT_LEN);
	PORT_queue = xQueueCreate(6, sizeof(CommParams));

	PORT1_RX_queue = xQueueCreate(3, NMEAMSGLEN);
	PORT2_RX_queue = xQueueCreate(3, NMEAMSGLEN);
	PORT3_RX_queue = xQueueCreate(3, NMEAMSGLEN);
	PORT4_RX_queue = xQueueCreate(3, NMEAMSGLEN);

	// SysState backup timer
	BackupTimer = xTimerCreate("Backup", BACKUP_RATE, pdTRUE, 0,(void*) Backup_Manager);

	xTaskCreate(MAIN_Task, "Main", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(KEYBOARD_Task, "Kbd", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(PORTS_Task, "Ports", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(DISPLAY_Task, "Display", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(DEBUG_Task, "Debug", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 3, NULL);

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

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
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART4_UART_Init(void)
{

  /* USER CODE BEGIN USART4_Init 0 */

  /* USER CODE END USART4_Init 0 */

  /* USER CODE BEGIN USART4_Init 1 */

  /* USER CODE END USART4_Init 1 */
  huart4.Instance = USART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART4_Init 2 */

  /* USER CODE END USART4_Init 2 */

}

/**
  * @brief USART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART5_UART_Init(void)
{

  /* USER CODE BEGIN USART5_Init 0 */

  /* USER CODE END USART5_Init 0 */

  /* USER CODE BEGIN USART5_Init 1 */

  /* USER CODE END USART5_Init 1 */
  huart5.Instance = USART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART5_Init 2 */

  /* USER CODE END USART5_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CAN_SHTDOWN_Pin|CAN_STDBY_Pin|ROW1_Pin|MCU_DTR3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ROW2_Pin|ROW3_Pin|ROW4_Pin|MCU_DTR2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, ROW5_Pin|ROW6_Pin|ROW7_Pin|ROW8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RELAY1_Pin|RELAY3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MCU_DTR1_GPIO_Port, MCU_DTR1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, MCU_DTR4_Pin|RELAY4_Pin|RELAY2_Pin|FLASH_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LEFT_SW2_Pin LEFT_SW1_Pin MCU_DSR3_Pin LEFT_SW4_Pin
                           LEFT_SW3_Pin */
  GPIO_InitStruct.Pin = LEFT_SW2_Pin|LEFT_SW1_Pin|MCU_DSR3_Pin|LEFT_SW4_Pin
                          |LEFT_SW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CAN_SHTDOWN_Pin CAN_STDBY_Pin ROW1_Pin MCU_DTR3_Pin */
  GPIO_InitStruct.Pin = CAN_SHTDOWN_Pin|CAN_STDBY_Pin|ROW1_Pin|MCU_DTR3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW2_Pin ROW3_Pin ROW4_Pin MCU_DTR2_Pin */
  GPIO_InitStruct.Pin = ROW2_Pin|ROW3_Pin|ROW4_Pin|MCU_DTR2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW5_Pin ROW6_Pin ROW7_Pin ROW8_Pin */
  GPIO_InitStruct.Pin = ROW5_Pin|ROW6_Pin|ROW7_Pin|ROW8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : COL1_Pin COL2_Pin COL3_Pin COL4_Pin
                           MCU_DSR2_Pin */
  GPIO_InitStruct.Pin = COL1_Pin|COL2_Pin|COL3_Pin|COL4_Pin
                          |MCU_DSR2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : COL5_Pin COL6_Pin */
  GPIO_InitStruct.Pin = COL5_Pin|COL6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : COL7_Pin COL8_Pin */
  GPIO_InitStruct.Pin = COL7_Pin|COL8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : RELAY1_Pin RELAY3_Pin */
  GPIO_InitStruct.Pin = RELAY1_Pin|RELAY3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : MCU_DTR1_Pin */
  GPIO_InitStruct.Pin = MCU_DTR1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MCU_DTR1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MCU_DSR1_Pin GP_SW2_Pin GP_SW1_Pin */
  GPIO_InitStruct.Pin = MCU_DSR1_Pin|GP_SW2_Pin|GP_SW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB12 PB13 PB14
                           PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PE12 PE14 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_12|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : MCU_DTR4_Pin RELAY4_Pin RELAY2_Pin FLASH_CS_Pin */
  GPIO_InitStruct.Pin = MCU_DTR4_Pin|RELAY4_Pin|RELAY2_Pin|FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : MCU_DSR4_Pin RIGHT_SW1_Pin RIGHT_SW2_Pin RIGHT_SW3_Pin
                           RIGHT_SW4_Pin GP_SW4_Pin GP_SW3_Pin */
  GPIO_InitStruct.Pin = MCU_DSR4_Pin|RIGHT_SW1_Pin|RIGHT_SW2_Pin|RIGHT_SW3_Pin
                          |RIGHT_SW4_Pin|GP_SW4_Pin|GP_SW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15
                           PD0 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 PC8 PC9
                           PC10 PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PF6 PF11 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//===================================================================
//			KEYBOARD TASK
//===================================================================
void KEYBOARD_Task(void *pvParameters) {

	/*
	 * 	SUMMARY:
	 * 	Each button has an associated counter and a mode register
	 * 	by default, all button have their counters = 0 and OFF state
	 * 	pressed buttons increment the counter
	 * 	when button are released, the counter is compared with 3 limits
	 * 	to determine a momentary or longer press
	 *  this task only assigns momentary, short press or long press modes
	 *  button mode is set back to OFF by the Main task after processing the associated actions
	 * */

	uint8_t i, key;
	// reset button counters
	uint16_t L1_counter = 0;
	uint16_t L2_counter = 0;
	uint16_t L3_counter = 0;
	uint16_t L4_counter = 0;
	uint16_t R1_counter = 0;
	uint16_t R2_counter = 0;
	uint16_t R3_counter = 0;
	uint16_t R4_counter = 0;
	uint16_t GP1_counter = 0;
	uint16_t GP2_counter = 0;
	uint16_t GP3_counter = 0;
	uint16_t GP4_counter = 0;

	// reset keyboard key counters
	for(i = 0 ;i < 59 ; i++){
		key_counter[i] = 0;
	}


	TickType_t xLastWakeTime = xLastWakeTime = xTaskGetTickCount();

	for (;;) {

	// Scan main keyboard
	for(i = 0;i <= 8;i++){

		key = KBD_scan_row(i);
		if (key!= 0) break;
		}
	//any keys pressed?
//		if (key != 0) {
			// check which key was pressed
			for (i = 0; i <59; i++) {
				// process keys on release
				if (key != keymap[i].code) {

					if (key_counter[i] > BTN_3SEC_PRESS) key_mode[i] = PRESS3SEC;
					else if (key_counter[i] > BTN_2SEC_PRESS) key_mode[i] = PRESS2SEC;
					else if (key_counter[i] > BTN_SHORT_PRESS) {
						key_mode[i] = MOMENTARY;
					}
					key_counter[i] = 0;


				}
				else {
					key_counter[i]++;	// increment counter while the key is pressed
					break;		// process single keys pressed only
				}
			}
//		}

	// Process panel buttons
		// Left panel switch 1 released
	if(!LEFT_SW1_CLOSED) {

		if (L1_counter > BTN_3SEC_PRESS) SysState.L1_button = PRESS3SEC;
		else if (L1_counter > BTN_2SEC_PRESS) SysState.L1_button = PRESS2SEC;
		else if (L1_counter > BTN_SHORT_PRESS) SysState.L1_button = MOMENTARY;
		//else SysState.L1_button = OFF;

		L1_counter = 0;
	}
	else L1_counter++;

	// Left panel switch 2 released
	if(!LEFT_SW2_CLOSED) {

			if (L2_counter > BTN_3SEC_PRESS) SysState.L2_button = PRESS3SEC;
			else if (L2_counter > BTN_2SEC_PRESS) SysState.L2_button = PRESS2SEC;
			else if (L2_counter > BTN_SHORT_PRESS) SysState.L2_button = MOMENTARY;
			//else SysState.L2_button = OFF;

			L2_counter = 0;
		}
		else L2_counter++;

		// Left panel switch 3 released
		if(!LEFT_SW3_CLOSED) {

			if (L3_counter > BTN_3SEC_PRESS) SysState.L3_button = PRESS3SEC;
			else if (L3_counter > BTN_2SEC_PRESS) SysState.L3_button = PRESS2SEC;
			else if (L3_counter > BTN_SHORT_PRESS) SysState.L3_button = MOMENTARY;
			//else SysState.L3_button = OFF;

			L3_counter = 0;
		}
		else L3_counter++;

		// Left panel switch 4 released
		if(!LEFT_SW4_CLOSED) {

			if (L4_counter > BTN_3SEC_PRESS) SysState.L4_button = PRESS3SEC;
			else if (L4_counter > BTN_2SEC_PRESS) SysState.L4_button = PRESS2SEC;
			else if (L4_counter > BTN_SHORT_PRESS) SysState.L4_button = MOMENTARY;
			//else SysState.L4_button = OFF;

			L4_counter = 0;
		}
		else L4_counter++;



		// Right panel switch 1 released
	if(!RIGHT_SW1_CLOSED) {

			if (R1_counter > BTN_3SEC_PRESS) SysState.R1_button = PRESS3SEC;
			else if (R1_counter > BTN_2SEC_PRESS) SysState.R1_button = PRESS2SEC;
			else if (R1_counter > BTN_SHORT_PRESS) SysState.R1_button = MOMENTARY;
			//else SysState.R1_button = OFF;

			R1_counter = 0;
		}
		else R1_counter++;

		// Right panel switch 2 released
		if(!RIGHT_SW2_CLOSED) {

			if (R2_counter > BTN_3SEC_PRESS) SysState.R2_button = PRESS3SEC;
			else if (R2_counter > BTN_2SEC_PRESS) SysState.R2_button = PRESS2SEC;
			else if (R2_counter > BTN_SHORT_PRESS) SysState.R2_button = MOMENTARY;
			//else SysState.R2_button = OFF;

			R2_counter = 0;
		}
		else R2_counter++;

		// Right panel switch 3 released
		if(!RIGHT_SW3_CLOSED) {

			if (R3_counter > BTN_3SEC_PRESS) SysState.R3_button = PRESS3SEC;
			else if (R3_counter > BTN_2SEC_PRESS) SysState.R3_button = PRESS2SEC;
			else if (R3_counter > BTN_SHORT_PRESS) SysState.R3_button = MOMENTARY;
			//else SysState.R3_button = OFF;

			R3_counter = 0;
		}
		else R3_counter++;

		// Right panel switch 4 released
		if(!RIGHT_SW4_CLOSED) {

			if (R4_counter > BTN_3SEC_PRESS) SysState.R4_button = PRESS3SEC;
			else if (R4_counter > BTN_2SEC_PRESS) SysState.R4_button = PRESS2SEC;
			else if (R4_counter > BTN_SHORT_PRESS) SysState.R4_button = MOMENTARY;
			//else SysState.R4_button = OFF;

			R4_counter = 0;
		}
		else R4_counter++;

		// Process General Purpose switches GP1-GP4

		// General purpose  switch 1 released
	if(!GP_SW1_CLOSED) {

			if (GP1_counter > BTN_3SEC_PRESS) SysState.GP1_button = PRESS3SEC;
			else if (GP1_counter > BTN_2SEC_PRESS) SysState.GP1_button = PRESS2SEC;
			else if (GP1_counter > BTN_SHORT_PRESS) SysState.GP1_button = MOMENTARY;
			//else SysState.GP1_button = OFF;

			GP1_counter = 0;
		}
		else GP1_counter++;

		// General purpose  switch 2 released
		if(!GP_SW2_CLOSED) {

			if (GP2_counter > BTN_3SEC_PRESS) SysState.GP2_button = PRESS3SEC;
			else if (GP2_counter > BTN_2SEC_PRESS) SysState.GP2_button = PRESS2SEC;
			else if (GP2_counter > BTN_SHORT_PRESS) SysState.GP2_button = MOMENTARY;
			//else SysState.GP2_button = OFF;

			GP2_counter = 0;
		}
		else GP2_counter++;

		// General purpose  switch 3 released
		if(!GP_SW3_CLOSED) {

			if (GP3_counter > BTN_3SEC_PRESS) SysState.GP3_button = PRESS3SEC;
			else if (GP3_counter > BTN_2SEC_PRESS) SysState.GP3_button = PRESS2SEC;
			else if (GP3_counter > BTN_SHORT_PRESS) SysState.GP3_button = MOMENTARY;
			//else SysState.GP3_button = OFF;

			GP3_counter = 0;
		}
		else GP3_counter++;

		// General purpose  switch 4 released
		if(!GP_SW4_CLOSED) {

			if (GP4_counter > BTN_3SEC_PRESS) SysState.GP4_button = PRESS3SEC;
			else if (GP4_counter > BTN_2SEC_PRESS) SysState.GP4_button = PRESS2SEC;
			else if (GP4_counter > BTN_SHORT_PRESS) SysState.GP4_button = MOMENTARY;
			//else SysState.GP4_button = OFF;

			GP4_counter = 0;
		}
		else GP4_counter++;

		vTaskDelayUntil(&xLastWakeTime, MS(10));
	}

}

//===================================================================
//			PORTS TASK
//===================================================================
void PORTS_Task(void *pvParameters) {

	uint8_t msg[NMEAMSGLEN];

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

		// Process incoming messages from PORT4
		if(xQueueReceive(PORT4_RX_queue, (void*)msg, 0) == pdTRUE) {

			Parse_NMEA_Message(&XceiverState4, (char*)msg);
		}

		//process outgoing messages
		// TODO

		vTaskDelay(MS(100));
	}

}

//===================================================================
//			DISPLAY TASK
//===================================================================
void DISPLAY_Task(void *pvParameters) {

	uint8_t i, backlight = 100;

	printf_DISPLAY("t0.txt=\"CDU h/w test\"");

	for (;;) {



		for(i = 0 ;i < 59 ; i++){
			if(key_mode[i] == MOMENTARY){


				switch(i){
				case 6:		// dim
					if(backlight>10) backlight -=10;
					printf_DISPLAY("dim=%d",backlight);
				break;

				case 7:		// brt
					if(backlight<100) backlight +=10;
					printf_DISPLAY("dim=%d",backlight);
				break;

				case 58:		// CLR
					printf_DISPLAY("slt0.txt=\"\"");
				break;

				case 55:		// ENT
					printf_DISPLAY("slt0.txt+=\"\\r\"");
				break;

				case 0:		// BCK
					printf_DISPLAY("slt0.txt-=1");
				break;

				default:
					printf_DISPLAY("slt0.txt+=\"%s\"",keymap[i].label);
				}
			}
		}


		Reset_Switch_States();
		vTaskDelay(MS(10));
	}

}

//===================================================================
//			MAIN TASK
//===================================================================
void MAIN_Task(void *pvParameters) {

	TickType_t xLastWakeTime;
	// Init Flash (must be called inside a RTOS task)
	spif_Init_Flash();
	spif_Reset_Flash(); 	// soft reset Flash
	vTaskDelay(MS(1));
	DISPLAY.RXbuffer[0] = 0;
	PORT2.RXbuffer[0] = 0;
	PORT3.RXbuffer[0] = 0;
	PORT4.RXbuffer[0] = 0;

	xLastWakeTime = xTaskGetTickCount();

	for (;;) {


		if (SysState.L1_button == MOMENTARY) RELAY1_ON;
		if (SysState.L2_button == MOMENTARY) RELAY1_OFF;
		if (SysState.L3_button == MOMENTARY) RELAY2_ON;
		if (SysState.L4_button == MOMENTARY) RELAY2_OFF;

		if (SysState.R1_button == MOMENTARY) RELAY3_ON;
		if (SysState.R2_button == MOMENTARY) RELAY3_OFF;
		if (SysState.R3_button == MOMENTARY) RELAY4_ON;
		if (SysState.R4_button == MOMENTARY) RELAY4_OFF;

		if (SysState.GP1_button == MOMENTARY) printf_USB("GP1 ");
		if (SysState.GP2_button == MOMENTARY) printf_USB("GP2 ");
		if (SysState.GP3_button == MOMENTARY) printf_USB("GP3 ");
		if (SysState.GP4_button == MOMENTARY) printf_USB("GP4 ");

		Reset_Switch_States();
		vTaskDelayUntil(&xLastWakeTime, MS(100));
	}

}

//===================================================================
//			DEBUG TASK
//===================================================================
void DEBUG_Task(void *pvParameters) {

	uint8_t a = 0;
	for (;;) {


		HAL_CAN_AddTxMessage(&hcan, &pHeader, &a, &TxMailbox);
		a++;
		vTaskDelay(MS(1000));
	}

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
	systemstate_t oldSysState, newSysState;

	//  Backup SysState
	// check SysState copy in flash.
	spif_Load_SysState(&oldSysState, SYSSTATE_SECTOR);
	// Compare with current version.
	newSysState = SysState;
	newSysState.state = CDU_RESET; 	// compare structs in Reset mode
	if(memcmp(&newSysState, &oldSysState, (size_t)sizeof(SysState)) != 0) {
		//  Overwrite old version in flash if changed
		spif_Store_SysState(&newSysState, SYSSTATE_SECTOR);
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

	USB_UART.TXbuflen = strlen(USBstring);
	while (USB_UART.TXbuflen > (MAXBUFLEN - USB_UART.LoadIndex)) ;

	while (USB_UART.TXbuflen--)
	{
		USB_UART.TXbuffer[USB_UART.LoadIndex++] = USBstring[n++];
	}

	// Start transmission if this is first byte in buffer
	if(USB_UART.TXindex == 0)
	{
		huart1.Instance->TDR = USB_UART.TXbuffer[USB_UART.TXindex++];
		__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
	}
	USB_UART.TXcomplete = 1;
}

//-------------------------------------------------------------------
//			PRINT TO DISPLAY
//-------------------------------------------------------------------
void printf_DISPLAY(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char TFTstring[64];
	uint8_t n = 0;

	vsnprintf(TFTstring, 64, format, args);

	DISPLAY.TXbuflen = strlen(TFTstring);
	while (DISPLAY.TXbuflen > (MAXBUFLEN - DISPLAY.LoadIndex)) ;

	while (DISPLAY.TXbuflen--)
	{
		DISPLAY.TXbuffer[DISPLAY.LoadIndex++] = TFTstring[n++];
	}
	// add Nextion ending characters
	DISPLAY.TXbuffer[DISPLAY.LoadIndex++] = 0xFF;
	DISPLAY.TXbuffer[DISPLAY.LoadIndex++] = 0xFF;
	DISPLAY.TXbuffer[DISPLAY.LoadIndex++] = 0xFF;

	// Start transmission if this is first byte in buffer
	if(DISPLAY.TXindex == 0)
	{
		huart2.Instance->TDR = DISPLAY.TXbuffer[DISPLAY.TXindex++];
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
	}
	DISPLAY.TXcomplete = 1;
}


//-------------------------------------------------------------------
//  RESET SWITCH STATES
//
/*! \fn void Reset_Knobs_States(void)
\brief Reset switch SysState registers to default values

Set all switches to OFF mode
\return None
*/
//-------------------------------------------------------------------
void Reset_Switch_States(void)
{
	uint8_t i;

	// reset panel keys states
	SysState.GP1_button = OFF;
	SysState.GP2_button = OFF;
	SysState.GP3_button = OFF;
	SysState.GP4_button = OFF;
	SysState.L1_button = OFF;
	SysState.L2_button = OFF;
	SysState.L3_button = OFF;
	SysState.L4_button = OFF;
	SysState.R1_button = OFF;
	SysState.R2_button = OFF;
	SysState.R3_button = OFF;
	SysState.R4_button = OFF;

	// reset main keyboard states
	for(i = 0 ;i < 59 ; i++){
		key_mode[i] = OFF;
	}
}

void vApplicationIdleHook( void ){

	uint32_t pinvalue;

	if(SysState.DisplayBypass){
	// copy USB RX pin to Display TX pin
	pinvalue = BOOTLOAD_RX_GPIO_Port->IDR & BOOTLOAD_RX_Pin;
	pinvalue >>= 8;
	TFT_TX_GPIO_Port->ODR &= ~TFT_TX_Pin;	// pin low
	TFT_TX_GPIO_Port->ODR |= pinvalue;		// copy value

	// copy Display RX pin to USB TX pin
	pinvalue = TFT_RX_GPIO_Port->IDR & TFT_RX_Pin;
	pinvalue <<= 6;
	BOOTLOAD_TX_GPIO_Port->ODR &= ~BOOTLOAD_TX_Pin;	// pin low
	BOOTLOAD_TX_GPIO_Port->ODR |= pinvalue;		// copy value

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
		x->Afreq = (float)msg[8] + 48.0;
		x->Afreq += ((float)msg[9] - 48.0) * .025;
		// read Standby freq
		x->Sfreq = (float)msg[10] + 48.0;
		x->Sfreq += ((float)msg[11] - 48.0) * .025;

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
