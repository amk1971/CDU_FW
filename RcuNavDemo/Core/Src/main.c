
#include "main.h"
#include "cmsis_os.h"

#include "rcuCduCom.h"

#include "spif.h"
#include "glcd.h"
#include "advanceglcd.h"
#include "font.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
SPIF_HandleTypeDef spif;
float freq = 0.000;
volatile int KHz = 0;
volatile int MHz = 108;
float Standby = 108.000;
volatile int SK = 0;
volatile int SM = 108;
volatile int vol = 0;
bool edit = false;
bool on = false;
int eepDisplay[16]; // updated with toDisplay()
volatile int dispCount = 0; // incremented after every scroll and reset when something is selected
bool pause = true; // when this is up ISRs are paused CHANGE TO TRUE
int count = 0;
bool bscroll = false;
bool add = false;
bool del = false;
volatile int cursor = 0;
volatile int obs = 0;
volatile int charcount = 0;
volatile int delc = 0;
bool temp = false;
uint8_t read[1];
uint8_t write[1];

unsigned char rxbuff[1];
char rxmsg[25];
int rxcount = 0;
int faultcounter0 = 0;
int faultcounter1 = 0;
bool rxfree = false;

uint8_t response = 0;

extern unsigned char rxbuffcdu[1];
extern char rxmsgcdu[25];
extern int rxcountcdu;
extern bool rxfreecdu;

extern uint8_t responseCDU;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_UART5_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void const * argument);

int _write(int file, char *ptr, int len)
 {
	 int DataIdx;
	 for (DataIdx = 0; DataIdx < len; DataIdx++)
	 {
		 ITM_SendChar(*ptr++);
	 }
	 return len;
 }

void toDisplay() {						//reads sectors from the SPI flash memory and updates the eepDisplay
	for (int i = 0; i < 16; ++i) {
		eepDisplay[i] = -1;
	}
	count = 0;
	for (int i = 0; i < 16; i++) {
		SPIF_ReadSector(&spif, i, read, 1, 0);
		if (read[0] != 255) {
			eepDisplay[count] = i;
			count++;
		}
	}
}

void toAdd(const char* dN, int dM, int dK) {		//adds a new entry to the first available empty sector in the SPI flash memory
  for (int i = 0; i < 16; i ++) {
	SPIF_ReadSector(&spif, i, read, 1, 0);
    if (read[0] == 255) {
      for (int j = 0; j < 7; j++) {
    	write[0] = dN[j];
    	SPIF_WriteSector(&spif, i, write, 1, j);
      }
	  write[0] = dM;
	  SPIF_WriteSector(&spif, i, write, 1, 7);
	  write[0] = dK/25;
	  SPIF_WriteSector(&spif, i, write, 1, 8);
      break;
    }
  }
}

void toDelete(int index) {		//erases a specific sector in the SPI flash memory, effectively marking it as empty.
	SPIF_EraseSector(&spif, index);
}

void storestate() { // called when turned off			stores the current state of the system in a specific sector of the SPI flash memory.
  SPIF_EraseSector(&spif, 20);
  write[0] = MHz;
  SPIF_WriteSector(&spif, 20, write, 1, 0);
  write[0] = KHz/25;
  SPIF_WriteSector(&spif, 20, write, 1, 1);
  write[0] = SM;
  SPIF_WriteSector(&spif, 20, write, 1, 2);
  write[0] = SK/25;
  SPIF_WriteSector(&spif, 20, write, 1, 3);
  write[0] = vol;
  SPIF_WriteSector(&spif, 20, write, 1, 4);
  write[0] = obs;
}

void loadstate() { // called on startup					loads the system state from a specific sector in the SPI flash memory on startup.
  SPIF_ReadSector(&spif, 20, read, 1, 0);
  MHz = read[0];
  SPIF_ReadSector(&spif, 20, read, 1, 1);
  KHz = read[0] * 25;
  SPIF_ReadSector(&spif, 20, read, 1, 2);
  SM = read[0];
  SPIF_ReadSector(&spif, 20, read, 1, 3);
  SK = read[0] * 25;
  SPIF_ReadSector(&spif, 20, read, 1, 4);	//1 is not bytes to be read
  vol = read[0];
  SPIF_ReadSector(&spif, 20, read, 1, 5);	//5 is Offset: The starting point within the sector to begin reading from.
  obs = read[0];
  freq = MHz + (.001 * KHz);
  Standby = SM + (.001 * SK);
}
uint32_t millis() {								//returns the number of ticks since the RTOS started, effectively providing a millisecond timer.
  return xTaskGetTickCount();
};
// TODO end
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
osThreadId Task2handler;
void task2_init(void const * argument);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART4) {
        // Process data from UART4
        rxmsg[rxcount] = rxbuff[0];
        rxcount++;
        response = HAL_UART_Receive_IT(&huart4, rxbuff, 1); // Restart UART4 reception
    } else if (huart->Instance == UART5) {
        // Process data from UART5
        rxmsgcdu[rxcountcdu] = rxbuffcdu[0];
        rxcountcdu++;
        responseCDU = HAL_UART_Receive_IT(&huart5, rxbuffcdu, 1); // Restart UART5 reception
    }
}

osThreadId Task3handler;

int main(void)
{


  HAL_Init();


  SystemClock_Config();


  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  SPIF_Init(&spif, &hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin);
  toDisplay();
  GLCD_INIT();		//setting up the display controller, setting display parameters (contrast, brightness), and preparing the display for use.
  loadstate();

  responseCDU = HAL_UART_Receive_IT(&huart5, rxbuffcdu, 1); // Start UART5 in interrupt mode
  response = HAL_UART_Receive_IT(&huart4, rxbuff, 1); // Start UART4 in interrupt mode

  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

//  osThreadDef(Task2, task2_init, osPriorityNormal, 0, 128);	//128 is stack size (in bytes) requirements for the thread function.
//  Task2handler = osThreadCreate(osThread(Task2), NULL);

  osThreadDef(Task3, task3_init, osPriorityNormal, 0, 128);	//128 is stack size (in bytes) requirements for the thread function.
  Task3handler = osThreadCreate(osThread(Task3), NULL);

  osKernelStart();

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
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
  if (HAL_LIN_Init(&huart5, UART_LINBREAKDETECTLENGTH_10B) != HAL_OK)
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
  huart3.Init.BaudRate = 9600;
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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
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
                             GP_SW2_Pin GP_SW3_Pin RIGHT_SW_Pin */
    GPIO_InitStruct.Pin = MCU_DSR3_Pin|GP_SW4_Pin|GP_SW5_Pin|GP_SW1_Pin
                            |GP_SW2_Pin|GP_SW3_Pin|RIGHT_SW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pins : GP_SW6_Pin LEFT_SW_Pin */
    GPIO_InitStruct.Pin = GP_SW6_Pin|LEFT_SW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : PC14 PC15 */
    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : LEFT_A1_Pin LEFT_B1_Pin LEFT_B2_Pin */
    GPIO_InitStruct.Pin = LEFT_A1_Pin|LEFT_B1_Pin|LEFT_B2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : LEFT_A2_Pin */
    GPIO_InitStruct.Pin = LEFT_A2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
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

    /*Configure GPIO pins : BOOT1_Pin KEY_ROW1_Pin */
    GPIO_InitStruct.Pin = BOOT1_Pin|KEY_ROW1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : RIGHT_A1_Pin */
    GPIO_InitStruct.Pin = RIGHT_A1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RIGHT_A1_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : RIGHT_B1_Pin RIGHT_B2_Pin RIGHT_A2_Pin */
    GPIO_InitStruct.Pin = RIGHT_B1_Pin|RIGHT_B2_Pin|RIGHT_A2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : KEY_ROW0_Pin KEY_ROW3_Pin KEY_ROW2_Pin KEY_ROW5_Pin
                             KEY_ROW4_Pin KEY_ROW7_Pin KEY_ROW6_Pin MCU_DSR1_Pin */
    GPIO_InitStruct.Pin = KEY_ROW0_Pin|KEY_ROW3_Pin|KEY_ROW2_Pin|KEY_ROW5_Pin
                            |KEY_ROW4_Pin|KEY_ROW7_Pin|KEY_ROW6_Pin|MCU_DSR1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*Configure GPIO pins : KEY_COL1_Pin KEY_COL9_Pin RELAY1_Pin MCU_DTR1_Pin
                             RELAY2_Pin */
    GPIO_InitStruct.Pin = KEY_COL1_Pin|KEY_COL9_Pin|RELAY1_Pin|MCU_DTR1_Pin
                            |RELAY2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*Configure GPIO pin : PB6 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : PB8 */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PE0 PE1 */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

//    /*Configures the port and pin on which the EVENTOUT Cortex signal will be connected */
//    HAL_GPIOEx_ConfigEventout(AFIO_EVENTOUT_PORT_B, AFIO_EVENTOUT_PIN_6);
//
//    /*Enables the Event Output */
//    HAL_GPIOEx_EnableEventout();

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

void read_encoder() { // MHz right outer knob HERE				//Handles the right outer knob.Updates SM value.
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  if (!pause) {
    static uint8_t old_AB = 3;  // Lookup table index
    static int8_t encval = 0;   // Encoder value
    static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; //{0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

    old_AB <<=2;  // Remember previous state

    if (HAL_GPIO_ReadPin(RIGHT_A2_GPIO_Port, RIGHT_A2_Pin)) old_AB |= 0x02; // Add current state of pin A
    if (HAL_GPIO_ReadPin(RIGHT_B2_GPIO_Port, RIGHT_B2_Pin)) old_AB |= 0x01; // Add current state of pin B

    encval += enc_states[( old_AB & 0x0f )];

    // Update counter if encoder has rotated a full indent, that is at least 4 steps
    if( encval > 0 ) {        // Four steps forward
      int changevalue = 1;
      if (SM < 117) {
        SM = SM + changevalue;              // Update counter
      }
      encval = 0;
    }
    else if( encval < 0 ) {        // Four steps backward
      int changevalue = -1;
      if (SM > 108) {
        SM = SM + changevalue;              // Update counter
      }
      encval = 0;
    }
  }
}

void read_encoder2() { // KHz right inner knob					//Handles the right inner knob, Updates SK value.
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  if (!pause) {
    static uint8_t old_GH = 3;  // Lookup table index
    static int8_t encval = 0;   // Encoder value
    static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

    old_GH <<=2;  // Remember previous state

    if (HAL_GPIO_ReadPin(RIGHT_A1_GPIO_Port, RIGHT_A1_Pin)) old_GH |= 0x02; // Add current state of pin G
    if (HAL_GPIO_ReadPin(RIGHT_B1_GPIO_Port, RIGHT_B1_Pin)) old_GH |= 0x01; // Add current state of pin H

    encval += enc_states[( old_GH & 0x0f )];

    // Update counter if encoder has rotated a full indent, that is at least 4 steps
    if( encval > 0 ) {        // Four steps forward
      int changevalue = 50;
      if (SK < 950) {
        SK = SK + changevalue;              // Update counter
      }
      encval = 0;
    }
    else if( encval < 0 ) {        // Four steps backward
      int changevalue = -50;
      if (SK > 0) {
        SK = SK + changevalue;              // Update counter
      }
      encval = 0;
    }
  }
}

void read_encoder3() { // volume left inner knob				//Handles the left inner knob for volume control, Updates vol value.
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  if (!pause) {
    static uint8_t old_CD = 3;  // Lookup table index
    static int8_t encval = 0;   // Encoder value
    static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

    old_CD <<=2;  // Remember previous state

    if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin)) old_CD |= 0x02; // Add current state of pin C
    if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin)) old_CD |= 0x01; // Add current state of pin D

    encval += enc_states[( old_CD & 0x0f )];

    // Update counter if encoder has rotated a full indent, that is at least 4 steps
    if( encval > 1 ) {        // Four steps forward
      int changevalue = 1;
      if (vol < 20) {
        vol = vol + changevalue;              // Update counter
      }
      encval = 0;
    }
    else if( encval < -1 ) {        // Four steps backward
      int changevalue = -1;
      if (vol > 0) {
        vol = vol + changevalue;              // Update counter
      }
      encval = 0;
    }
  }
}

void read_encoder4() { // squelch left outer knob					// Handles the left outer knob for squelch control. Updates obs value
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  if (!pause) {
    static uint8_t old_IJ = 3;  // Lookup table index
    static int8_t encval = 0;   // Encoder value
    static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

    old_IJ <<=2;  // Remember previous state

    if (HAL_GPIO_ReadPin(LEFT_A2_GPIO_Port, LEFT_A2_Pin)) old_IJ |= 0x02; // Add current state of pin I
    if (HAL_GPIO_ReadPin(LEFT_B2_GPIO_Port, LEFT_B2_Pin)) old_IJ |= 0x01; // Add current state of pin J

    encval += enc_states[( old_IJ & 0x0f )];

    // Update counter if encoder has rotated a full indent, that is at least 4 steps
    if( encval > 1 ) {        // Four steps forward
      int changevalue = 1;
      if (obs < 359) {
        obs = obs + changevalue;              // Update counter
      }
      encval = 0;
    }
    else if( encval < -1 ) {        // Four steps backward
      int changevalue = -1;
      if (obs > 0) {
        obs = obs + changevalue;              // Update counter
      }
      encval = 0;
    }
  }
}

void scroll() { // scroll left inner knob								//Handles scrolling using the left inner knob. Updates dispCount value
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_CD = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value
  static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

  old_CD <<=2;  // Remember previous state

  if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin)) old_CD |= 0x02; // Add current state of pin C
  if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin)) old_CD |= 0x01; // Add current state of pin D

  encval += enc_states[( old_CD & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 1 ) {        // Four steps forward
    int changevalue = 1;
    if (dispCount < count-1) {
      dispCount = dispCount + changevalue;              // Update counter
    }
    encval = 0;
  }
  else if( encval < -1 ) {        // Four steps backward
    int changevalue = -1;
    if (dispCount > 0) {
      dispCount = dispCount + changevalue;              // Update counter
    }
    encval = 0;
  }
}

void scroll2() { // scroll left inner knob
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_CD = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value
  static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

  old_CD <<=2;  // Remember previous state

  if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin)) old_CD |= 0x02; // Add current state of pin C
  if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin)) old_CD |= 0x01; // Add current state of pin D

  encval += enc_states[( old_CD & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 1 ) {        // Four steps forward
    int changevalue = 1;
    if (dispCount < count) {
      dispCount = dispCount + changevalue;              // Update counter
    }
    encval = 0;
  }
  else if( encval < -1 ) {        // Four steps backward
    int changevalue = -1;
    if (dispCount > 0) {
      dispCount = dispCount + changevalue;              // Update counter
    }
    encval = 0;
  }
}

void cursormove() { // micgain left inner knob								//Handles cursor movement using the left inner knob. Updates cursor value.
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_CD = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value
  static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

  old_CD <<=2;  // Remember previous state

  if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin)) old_CD |= 0x02; // Add current state of pin C
  if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin)) old_CD |= 0x01; // Add current state of pin D

  encval += enc_states[( old_CD & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 1 ) {        // Four steps forward
    int changevalue = 1;
    if (cursor < 8) {
      cursor = cursor + changevalue;              // Update counter
    }
    encval = 0;
  }
  else if( encval < -1 ) {        // Four steps backward
    int changevalue = -1;
    if (cursor > 0) {
      cursor = cursor + changevalue;              // Update counter
    }
    encval = 0;
  }
}

void delmove() { // micgain left inner knob									// Handles deletion using the left inner knob. Updates delc value.
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_CD = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value
  static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

  old_CD <<=2;  // Remember previous state

  if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin)) old_CD |= 0x02; // Add current state of pin C
  if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin)) old_CD |= 0x01; // Add current state of pin D

  encval += enc_states[( old_CD & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 1 ) {        // Four steps forward
    int changevalue = 1;
    if (delc < 1) {
      delc = delc + changevalue;              // Update counter
    }
    encval = 0;
  }
  else if( encval < -1 ) {        // Four steps backward
    int changevalue = -1;
    if (delc > 0) {
      delc = delc + changevalue;              // Update counter
    }
    encval = 0;
  }
}

void charmove() { // sidetone left outer knob							//Handles character movement using the left outer knob. Updates charcount value.
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_IJ = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value
  static const int8_t enc_states[]  = {0,0,0,0,0,-1,0,-1,0,0,0,1,0,0,0,1}; // Lookup table

  old_IJ <<=2;  // Remember previous state

  if (HAL_GPIO_ReadPin(LEFT_A2_GPIO_Port, LEFT_A2_Pin)) old_IJ |= 0x02; // Add current state of pin I
  if (HAL_GPIO_ReadPin(LEFT_B2_GPIO_Port, LEFT_B2_Pin)) old_IJ |= 0x01; // Add current state of pin J

  encval += enc_states[( old_IJ & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 1 ) {        // Four steps forward
    int changevalue = 1;
    if (charcount < 39) {
      charcount = charcount + changevalue;              // Update counter
    }
    encval = 0;
  }
  else if( encval < -1 ) {        // Four steps backward
    int changevalue = -1;
    if (charcount > 0) {
      charcount = charcount + changevalue;              // Update counter
    }
    encval = 0;
  }
}

void leftinnerknob() {
  if (bscroll) {
    scroll();
  }
  else if (del) {
    delmove();
  }
  else if (add) {
    cursormove(); //flipped
  }
  else if (edit) {
    scroll2();
  }
  else {
    read_encoder3();
  }
}

void leftouterknob() {
  if (add) {
    charmove(); //flipped
  }
  else {
    read_encoder4();
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == LEFT_A2_Pin) { // LEFT 2 could be 8 idk
		leftouterknob();
	}
	else if (GPIO_Pin == LEFT_B2_Pin) { // LEFT 2
		leftouterknob();
	}
	else if (GPIO_Pin == LEFT_A1_Pin) { // LEFT 1
		leftinnerknob();
	}
	else if (GPIO_Pin == LEFT_B1_Pin) { // LEFT 1
		leftinnerknob();
	}
	else if (GPIO_Pin == RIGHT_A1_Pin) { // RIGHT 1
		read_encoder2();
	}
	else if (GPIO_Pin == RIGHT_B1_Pin) { // RIGHT 1
		read_encoder2();
	}
	else if (GPIO_Pin == RIGHT_A2_Pin) { // RIGHT 2
		read_encoder();
	}
	else if (GPIO_Pin == RIGHT_B2_Pin) { // RIGHT 2
		read_encoder();
	}
}

void checksum(const char* str, char* result) {				//calculates a checksum from a given string and stores the result in a character array
    unsigned int sum = 0;
    const char* ptr = str;

    if (*ptr != '\0') {
        ptr += 6;			//starting after the first 6 characters (possibly skipping a header like e.g: "$PMRRV")
    }

    while (*ptr != '\0') {//// Loop through the rest of the string
        sum += *ptr;		 // Sum up the ASCII values of the characters
        ptr++;
    }

    sum &= 0xFF;	// Keep only the least significant byte of the sum

    result[0] = ((sum >> 4) & 0xF) + 0x30;	// Convert high nibble (4 bits) to ASCII
    result[1] = (sum & 0xF) + 0x30;			// Convert low nibble (4 bits) to ASCII

    result[2] = '\0';
}

void concatTwoChars(char* base, const char* woo) {
    while (*base) { // Move to the end of the base string
        base++;
    }
    while (*woo) { // Copy the characters from toAdd
        *base = *woo;
        base++;
        woo++;
    }
    *base = '\0'; // Null terminate the resulting string
}

void LCDPrint(char* str) { //TODO Should be done needs testing, issues should not be here though
	if (str[0] == 'x') {
	  glcd_on();
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	  glcd_clear_all();
	}
	else if (str[0] == 'o') {
      glcd_off();
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	}
	else if (str[0] == 'f') { //frequency
	  char str0[8];
	  char str1[8];
	  sprintf(str0, "%0.3f", freq);
	  sprintf(str1, "%0.3f", Standby);
	  glcd_puts("A", 2, 2);
	  glcd_puts(str0, 11, 2);
	  glcd_puts(".", 34, 2);
	  glcd_puts(str0+4, 38, 2);
	  glcd_puts(" ", 62, 2);
	  glcd_puts("S ", 66, 2);
	  glcd_puts(str1, 75, 2);
	  glcd_puts(".", 98, 2);
	  glcd_puts(str1+4, 102, 2);
	  glcd_clearline(3);

	  char str2[5];
	  char str3[5];
	  sprintf(str2, "%d", vol);
	  sprintf(str3, "%03d", obs);
	  glcd_puts("Vol ", 2, 5);
	  glcd_puts(str2, 30, 5);
	  if (vol < 10) {
	  	 glcd_puts(" ", 38, 5);
	  }
	  glcd_puts(" ", 56, 5);
	  glcd_puts(" ", 62, 5);
	  glcd_puts("OBS ", 66, 5);
	  glcd_puts(str3, 94, 5);
	  glcd_clearline(6);

	}
	else if (str[0] == 'v') {
	  char str0[8];
	  char str1[8];
	  sprintf(str0, "%0.3f", freq);
	  sprintf(str1, "%0.3f", Standby);
	  glcd_puts("A", 2, 2);
	  glcd_puts(str0, 11, 2);
	  glcd_puts(".", 34, 2);
	  glcd_puts(str0+4, 38, 2);
	  glcd_puts(" ", 62, 2);
	  glcd_puts("S ", 66, 2);
	  glcd_puts(str1, 75, 2);
	  glcd_puts(".", 98, 2);
	  glcd_puts(str1+4, 102, 2);
	  glcd_clearline(3);

	  char str2[5];
	  char str3[5];
	  sprintf(str2, "%d", vol);
	  sprintf(str3, "%03d", obs);
	  glcd_puts("Vol ", 2, 5);
	  glcd_puts(str2, 30, 5);
	  if (vol < 10) {
		 glcd_puts(" ", 38, 5);
	  }
	  glcd_puts(" ", 56, 5);
	  glcd_puts(" ", 62, 5);
	  glcd_puts("OBS ", 66, 5);
	  glcd_puts(str3, 94, 5);
	  glcd_clearline(6);
	}
	else if (str[0] == 'b') {
	  char str0[8];
	  char str1[8];
	  sprintf(str0, "%0.3f", freq);
	  sprintf(str1, "%0.3f", Standby);
	  glcd_puts("A", 2, 2);
	  glcd_puts(str0, 11, 2);
	  glcd_puts(".", 34, 2);
	  glcd_puts(str0+4, 38, 2);
	  glcd_puts(" ", 62, 2);
	  glcd_puts("S ", 66, 2);
	  glcd_puts(str1, 75, 2);
	  glcd_puts(".", 98, 2);
	  glcd_puts(str1+4, 102, 2);
	  glcd_clearline(3);

	  char str2[5];
	  char str3[5];
	  sprintf(str2, "%d", vol);
	  sprintf(str3, "%03d", obs);
	  glcd_puts("Vol ", 2, 5);
	  glcd_puts(str2, 30, 5);
	  if (vol < 10) {
		 glcd_puts(" ", 38, 5);
	  }
	  glcd_puts(" ", 56, 5);
	  glcd_puts(" ", 62, 5);
	  glcd_puts("OBS ", 66, 5);
	  glcd_puts(str3, 94, 5);
	  glcd_clearline(6);
	}
	else if (str[0] == 'k'){ // scroll
	  glcd_clear_all();
	  glcd_puts(str+1, 2, 3);
	  glcd_puts("   ", 58, 3);
	  glcd_puts(str+8, 75, 3);
	  glcd_puts(".", 98, 3);
	  glcd_puts(str+12, 102, 3);
	  glcd_clearline(4);
	}
	else if (str[0] == 'n') {// no mem
	  glcd_clear_all();
	  str = "NO MEM";
	  glcd_puts(str, 66, 3);
	}
	else if (str[0] == 'e') {
	  glcd_clear_all();
	  if (str[1] == ':') {
		str = "EMPTY";
		glcd_puts(str, 66, 3);
	  }
	  else {
		glcd_puts(str+1, 2, 3);
		glcd_puts("   ", 58, 3);
		glcd_puts(str+8, 75, 3);
		glcd_puts(".", 98, 3);
		glcd_puts(str+12, 102, 3);
		glcd_clearline(4);
	  }
	}
	else if (str[0] == 'a') {
	  //a_CCCCCCCMHZKHZ
      glcd_clear_all();
	  glcd_puts(str+2, 2, 3);
	  glcd_puts("   ", 58, 3);
	  glcd_puts(str+9, 75, 3);
	  glcd_puts(".", 98, 3);
	  glcd_puts(str+13, 102, 3);
	  glcd_clearline(4);
	  int cursor = str[1] - '0';
	  if (cursor == 7) {
		glcd_puts("___", 75, 4);
	  }
	  else if (cursor == 8) {
		glcd_puts("___", 102, 4);
	  }
	  if (cursor >= 0 && cursor <= 6) {
		glcd_puts("_", 2+(cursor*8), 4);
	  }
	}
	else if (str[0] == 'd') {
	  //dbCCCCCCC
	  glcd_clear_all();
	  glcd_puts(str+2, 2, 3);
	  glcd_puts("   ", 58, 3);
	  glcd_puts("Delete ", 66, 3);
	  int yn = str[1] - '0';
	  if (yn == 1) {
		glcd_puts("Y", 118, 3);
	  }
	  else {
		glcd_puts("N", 118, 3);
	  }
	}
}

void Sender(const char * str, int mode) { //TODO				construct and send a message (likely over UART)
    char crlf[] = {'\r','\n', 0};
    uint8_t str2[25]; // Array to hold the constructed string in ASCII

    memset(str2, 0, sizeof(str2)); // Initialize the array with zeros

    if (mode == 0) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), "$PMRRV27%c%cN", m, k);
        char end2[3];
        checksum((char*)str2, end2);
        concatTwoChars((char*)str2, end2);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 1) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), "$PMRRV28%c%cN", m, k);
        char end3[3];
        checksum((char*)str2, end3);
        concatTwoChars((char*)str2, end3);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 2) {
        char v = str[0];
        snprintf((char*)str2, sizeof(str2), "$PMRRV73%c", v);
        char end4[3];
        checksum((char*)str2, end4);
        concatTwoChars((char*)str2, end4);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 3) {
        snprintf((char*)str2, sizeof(str2), "$PMRRV34%03d", obs);
        char end5[3];
        checksum((char*)str2, end5);
        concatTwoChars((char*)str2, end5);
        concatTwoChars((char*)str2, crlf);
    }

    HAL_UART_Transmit(&huart4, str2, strlen((char*)str2), 500);
    HAL_Delay(100);
}

void StartDefaultTask(void const * argument)
{
	  /* USER CODE BEGIN 5 */
		char toPrint[20];
		  /* Infinite loop */
		  for(;;)
		  {

	//Message Handling for Transceiver Status:==================================================================================================================

		  if (rxfree) {
				if (rxmsg[6] == '2' && rxmsg[7] == '8')
				{ // comm transceiver status
					float commfreq;
					commfreq = (rxmsg[8] + 48) + ((rxmsg[9] - 48) * .025);
					if (commfreq != freq)
					{
						faultcounter1 += 1;
					    int MA = MHz - 48;
					    int KA = (KHz/25) + 48;
					    char m = (char)MA;
					    char k = (char)KA;
					    char Mfinal[3] = {m,k,0};
					    Sender(Mfinal, 0); //set active				which will be something like by default........idts
						if (faultcounter1 > 2) {
							glcd_puts("Error 1", 0, 7);
						}
					}
					else
					{
						if (faultcounter1 > 2)
						{
							glcd_clearline(7);
						}
						faultcounter1 = 0;
					}
				}
				rxfree = false;
			}

	//Displaying Frequencies on the LCD:-----------------------------------------------------------------------------------------------------------------------

			static unsigned long last = 0;
		    static float freq_last = 108.000;
			static float sfreq_last = 108.000;
			static int vol_last = 0;
			static int obs_last = 0;
			freq = MHz + (.001 * KHz);
			Standby = SM + (.001 * SK);

			if (millis() - last > 2000 && temp)
			{ // if time between action > 2 send freq in NOTE: will need to add a check later as some actions cant be exited.

			  sprintf(toPrint, "%c%0.3f%0.3f", 'f', freq, Standby);		//The string "....." is stored into toPrint instead of printing on stdout
			  LCDPrint(toPrint);

			  freq_last = freq;
			  sfreq_last = Standby;
			  temp = false;
			}

	//Handling Button Presses for Editing:---------------------------------------------------------------------------------------------------------------------

			bool both = false;
			// Current time
			unsigned long currentTime = millis();

			// Read the button states(which is actually i think a kind of switch.....)
			bool currentA = HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == 0;
			bool currentB = HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 0;

			while ((millis() - currentTime) < 100 && (currentA || currentB)) {
			  if (currentA) {
				if (HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 0) {
				  both = true;
				  break;
				}
			  }
			  else {
				if (HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == 0) {
				  both = true;
				  break;
				}
			  }
			}

			// Check if both buttons were pressed within the 50 ms window
			if (both) {//==========================================================================================================================

				  if (on) { // Enter edit mode TODO
					toDisplay();
					edit = true;
					pause = true;
					HAL_Delay(500);
					bool both = false;

					char str[8];
					if (dispCount == count) {
					  str[0] = ':'; //EMPTY
					}
					else {

		//Reading from External Memory (SPIF) and Displaying Data ===============

					  for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
						str[i] = read[0];
					  }
					  str[7] = 0;
					}
					SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
					int val1 = read[0];
					SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
					int val2 = read[0];
					float store = val1 + (.025 * val2);

					sprintf(toPrint, "%c%s%0.3f", 'e', str, store);
					LCDPrint(toPrint);

					while (true) {
					  // Current time
					  unsigned long currentTime = millis();
					  static int lastDC = 0;
					  // Read the button states
					  bool currentA = HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == 0;
					  bool currentB = HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 0;

					  while ((millis() - currentTime) < 100 && (currentA || currentB)) {
						if (currentA) {
						  if (HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 0) {
							both = true;
							break;
						  }
						}
						else {
						  if (HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == 0) {
							both = true;
							break;
						  }
						}
					  }

					  if (both) {
						break;
					  }
					  else if (currentB) { //select to edit
						cursor = 0;
						if (dispCount == count && count != 16) { // EMPTY add new one
						  add = true;
						  int dM = 108;
						  int dK = 0;
						  char dN[8] = {' ' ,' ' ,' ' ,' ' ,' ' ,' ' ,' ' ,0};
						  HAL_Delay(100);

						  if (cursor < 7) { // moved to a char
							if (dN[cursor] == ' ') {
							  charcount = 0;
							}
							else if (dN[cursor] < ':') {
							  charcount = dN[cursor] - '/';
							}
							else {
							  charcount = dN[cursor] - '6';
							}
						  }
						  else if (cursor == 7) {
							charcount = dM - 108;
						  }
						  else {
							charcount = dK / 50;
						  }
		//Cursor Movement and Editing:===================================================
						  if (cursor < 7) { //
							if (charcount == 0) {
							  dN[cursor] = (char)(charcount+32);
							}
							else if (charcount < 11) {
							  dN[cursor] = (char)(charcount+47);
							}
							else {
							  dN[cursor] = (char)(charcount+54);
							}
						  }
						  else if (cursor == 7) { //mhz
							if (charcount > 9) {
							  charcount = 9;
							}
							dM = 108 + charcount;
						  }
						  else {
							if (charcount > 19) {
							  charcount = 19;
							}
							dK = 50 * charcount;
						  }

						  float pfreq = dM + (dK * .001);

						  sprintf(toPrint, "%c%d%s%0.3f",'a',cursor,dN,pfreq);
						  LCDPrint(toPrint);

						  while(HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 1) { // Leave on press TODO might need to swap highs and lows
							static int last_cursor = 0;
							static int last_cc = 0;
							if (last_cc != charcount) { // change the actual values of dN dM and dK, incrementing values
							  if (cursor < 7) { //
								if (charcount == 0) {
								  dN[cursor] = (char)(charcount+32);
								}
								else if (charcount < 11) {
								  dN[cursor] = (char)(charcount+47);
								}
								else {
								  dN[cursor] = (char)(charcount+54);
								}
							  }
							  else if (cursor == 7) { //mhz
								if (charcount > 9) {
								  charcount = 9;
								}
								dM = 108 + charcount;
							  }
							  else {
								if (charcount > 19) {
								  charcount = 19;
								}
								dK = 50 * charcount;
							  }

							  float pfreq = dM + (dK * .001);
							  sprintf(toPrint, "%c%d%s%0.3f",'a',cursor,dN,pfreq);
							  LCDPrint(toPrint);
							  last_cc = charcount;
							} ///////// case 1
							else if (last_cursor != cursor) { // change the value of charcount based on where we are
							  if (cursor < 7) { // moved to a char
								if (dN[cursor] == ' ') {
								  charcount = 0;
								}
								else if (dN[cursor] < ':') {
								  charcount = dN[cursor] - '/';
								}
								else {
								  charcount = dN[cursor] - '6';
								}
							  }
							  else if (cursor == 7) {
								charcount = dM - 108;
							  }
							  else {
								charcount = dK / 50;
							  }

							  float pfreq = dM + (dK * .001);
							  sprintf(toPrint, "%c%d%s%0.3f",'a',cursor,dN,pfreq);
							  LCDPrint(toPrint);
							  last_cc = charcount;
							  last_cursor = cursor;
							} ///////// case 2
						  }
						  //Some functionality to add dN,dM,and dK to the eeprom
						  toAdd(dN,dM,dK);
						  HAL_Delay(500);
						  add = false;
						  pause = false;
						  toDisplay();

						  char str[8];
						  if (dispCount == count) {
							str[0] = ':'; //EMPTY
						  }
						  else {
							for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				  str[i] = EEPROM.read(eepDisplay[dispCount] + i);
							  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							  str[i] = read[0];
							}
							str[7] = 0;
						  }
			//			  float store = EEPROM.read(eepDisplay[dispCount] + 7) + (.025 * EEPROM.read(eepDisplay[dispCount] + 8));
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						  int val1 = read[0];
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						  int val2 = read[0];
						  float store = val1 + (.025 * val2);

						  sprintf(toPrint, "%c%s%0.3f",'e',str,store);
						  LCDPrint(toPrint);
						}
						else { // regular edit 2
						  add = true;

						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						  int dM = read[0];
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						  int dK = read[0] * 25;
						  char dN[8] = {' ' ,' ' ,' ' ,' ' ,' ' ,' ' ,' ' ,0};
						  for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				dN[i] = EEPROM.read(eepDisplay[dispCount] + i);
							SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							dN[i] = read[0];
						  }
						  HAL_Delay(100);

						  if (cursor < 7) { // moved to a char
							if (dN[cursor] == ' ') {
							  charcount = 0;
							}
							else if (dN[cursor] < ':') {
							  charcount = dN[cursor] - '/';
							}
							else {
							  charcount = dN[cursor] - '6';
							}
						  }
						  else if (cursor == 7) {
							charcount = dM - 108;
						  }
						  else {
							charcount = dK / 50;
						  }

						  if (cursor < 7) { //
							if (charcount == 0) {
							  dN[cursor] = (char)(charcount+32);
							}
							else if (charcount < 11) {
							  dN[cursor] = (char)(charcount+47);
							}
							else {
							  dN[cursor] = (char)(charcount+54);
							}
						  }
						  else if (cursor == 7) { //mhz
							if (charcount > 9) {
							  charcount = 9;
							}
							dM = 108 + charcount;
						  }
						  else {
							if (charcount > 19) {
							  charcount = 19;
							}
							dK = 50 * charcount;
						  }


						  float pfreq = dM + (dK * .001);
						  sprintf(toPrint, "%c%d%s%0.3f",'a',cursor,dN,pfreq);
						  LCDPrint(toPrint);

						  while(HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 1) { // Leave on press
							static int last_cursor = 0;
							static int last_cc = 0;
							if (last_cc != charcount) { // change the actual values of dN dM and dK, incrementing values WORKS
							  if (cursor < 7) { //
								if (charcount == 0) {
								  dN[cursor] = (char)(charcount+32);
								}
								else if (charcount < 11) {
								  dN[cursor] = (char)(charcount+47);
								}
								else {
								  dN[cursor] = (char)(charcount+54);
								}
							  }
							  else if (cursor == 7) { //mhz
								if (charcount > 9) {
								  charcount = 9;
								}
								dM = 108 + charcount;
							  }
							  else {
								if (charcount > 19) {
								  charcount = 19;
								}
								dK = 50 * charcount;
							  }


							  float pfreq = dM + (dK * .001);
							  sprintf(toPrint,"%c%d%s%0.3f",'a',cursor,dN,pfreq);
							  LCDPrint(toPrint);
							  last_cc = charcount;
							} ///////// case 1
							else if (last_cursor != cursor) { // change the value of charcount based on where we are WORKS????
							  if (cursor < 7) { // moved to a char
								if (dN[cursor] == ' ') {
								  charcount = 0;
								}
								else if (dN[cursor] < ':') {
								  charcount = dN[cursor] - '/';
								}
								else {
								  charcount = dN[cursor] - '6';
								}
							  }
							  else if (cursor == 7) {
								charcount = dM - 108;
							  }
							  else {
								charcount = dK / 50;
							  }

							  float pfreq = dM + (dK * .001);
							  sprintf(toPrint,"%c%d%s%0.3f",'a',cursor,dN,pfreq);
							  LCDPrint(toPrint);
							  last_cc = charcount;
							  last_cursor = cursor;
							} ///////// case 2
						  }
		//Committing Changes to External Memory:==========================================

						  //update the eeprom with new values
						  SPIF_EraseSector(&spif, eepDisplay[dispCount]);
						  for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				EEPROM.update(eepDisplay[dispCount] + i, dN[i]);
							write[0] = dN[i];
							SPIF_WriteSector(&spif, eepDisplay[dispCount], write, 1, i);
						  }
			//			  EEPROM.update(eepDisplay[dispCount] + 7, dM);
			//			  EEPROM.update(eepDisplay[dispCount] + 8, dK/25);
						  write[0] = dM;
						  SPIF_WriteSector(&spif, eepDisplay[dispCount], write, 1, 7);
						  write[0] = dK/25;
						  SPIF_WriteSector(&spif, eepDisplay[dispCount], write, 1, 8);

						  HAL_Delay(500);
						  add = false;
						  pause = false;
						  toDisplay();

						  char str[8];
						  if (dispCount == count) {
							str[0] = ':'; //EMPTY
						  }
						  else {
							for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				  str[i] = EEPROM.read(eepDisplay[dispCount] + i);
							  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							  str[i] = read[0];
							}
							str[7] = 0;
						  }

						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						  int val1 = read[0];
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						  int val2 = read[0];
						  float store = val1 + (.025 * val2);

						  sprintf(toPrint, "%c%s%0.3f", 'e',str,store);
						  LCDPrint(toPrint);
						}
					  }
					  else if (currentA) { //delete
						if (dispCount != count) { // Not empty
						  delc = 0;
						  del = true;
						  pause = true;
						  char dN[8];
						  for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				dN[i] = EEPROM.read(eepDisplay[dispCount] + i);
							SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							dN[i] = read[0];
						  }

						  sprintf(toPrint, "%c%d%s",'d',delc,dN);
						  LCDPrint(toPrint);
						  HAL_Delay(500);
						  while (HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == 1) {
							static int last_delc = 0;
							if (last_delc != delc) {

							  sprintf(toPrint, "%c%d%s",'d',delc,dN);
							  LCDPrint(toPrint);
							  last_delc = delc;
							}
						  }
						  if (delc > 0) { //
							toDelete(eepDisplay[dispCount]);
						  }
						  del = false;
						  pause = false;
						  toDisplay();
						  HAL_Delay(500);
						}
						char str[8];
						if (dispCount == count) {
						  str[0] = ':'; //EMPTY
						}
						else {
						  for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				str[i] = EEPROM.read(eepDisplay[dispCount] + i);
							SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							str[i] = read[0];
						  }
						  str[7] = 0;
						}

						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						int val1 = read[0];
						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						int val2 = read[0];
						float store = val1 + (.025 * val2);

						sprintf(toPrint, "%c%s%0.3f", 'e',str,store);
						LCDPrint(toPrint);
						lastDC = dispCount;
					  }

					  if (lastDC != dispCount) {
						char str[8];
						if (dispCount == count) {
						  str[0] = ':'; //EMPTY
						}
						else {
						  for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				str[i] = EEPROM.read(eepDisplay[dispCount] + i);
							SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							str[i] = read[0];
						  }
						  str[7] = 0;
						}

						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						int val1 = read[0];
						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						int val2 = read[0];
						float store = val1 + (.025 * val2);

						sprintf(toPrint, "%c%s%0.3f", 'e',str,store);
						LCDPrint(toPrint);
						lastDC = dispCount;
					  }
					}
					edit = false;
					pause = false;
					dispCount = 0;

					sprintf(toPrint, "%c%0.3f%0.3f", 'f', freq, Standby);
					LCDPrint(toPrint);

					freq_last = freq;
					sfreq_last = Standby;
					HAL_Delay(500);
					last = millis();

				  }
			}//if (both) end here    ==========================================================================================================================

	//if(Right_pin)
			else if (HAL_GPIO_ReadPin(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin) == 0 && !pause) { // Check if button is pressed
			  float temp = freq;
			  int tk = KHz;
			  int tm = MHz;

			  freq = Standby;
			  KHz = SK;
			  MHz = SM;

			  Standby = temp;
			  SK = tk;
			  SM = tm;
			  HAL_Delay(500);
			  last = millis();
			}

	//if(left_pin)				==================================================================================================================================

			else if (HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 0) { // off/stored frequencies
				  if (!on) {
					on = true;
					pause = false;

					LCDPrint("x");
					sprintf(toPrint, "%c%0.3f%0.3f", 'f', freq, Standby);
					LCDPrint(toPrint);
					freq_last = freq;
					sfreq_last = Standby;
					HAL_Delay(500);
					// LOAD DATA, give old data from last session for display to load (can be done by just loading the data from eeprom into the global variables)
				  }
				  else
				  { // it is already on
					unsigned long time = millis();
					while (true) {
					  if (HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 1 || (millis() - time > 3000)) {
						break;
					  }
					}
					if (millis() - time > 3000) {
					  storestate();
					  on = false;
					  pause = true;
					  bscroll = false;

					  LCDPrint("o");
					  HAL_Delay(3000);
					}
					else
					{ // stored frequency mode
					  toDisplay();
					  if (eepDisplay[0] == -1) { // no mem exit
						//Serial2.print("n");
						LCDPrint("n");
						HAL_Delay(500);
						temp = true;
					  }
					  else
					  { // turn on stored frequency mode and stay until click again
						//initial print
						static int lastDC = 0;
						char str[8];
						for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//			  str[i] = EEPROM.read(eepDisplay[dispCount] + i);
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);;
						  str[i] = read[0];
						}
						str[7] = 0;

			//			float store = EEPROM.read(eepDisplay[dispCount] + 7) + (.025 * EEPROM.read(eepDisplay[dispCount] + 8));
						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						int val1 = read[0];
						SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						int val2 = read[0];
						float store = val1 + (.025 * val2);

						sprintf(toPrint, "%c%s%0.3f", 'k', str, store);
						LCDPrint(toPrint);

						pause = true;
						bscroll = true;
						temp = true;
						unsigned long timer = millis();
						bool valid = false;
						//loop + scroll setup
						while (millis() - timer < 5000)
						{
						  if (HAL_GPIO_ReadPin(LEFT_SW_GPIO_Port, LEFT_SW_Pin) == 0) {
							valid = true;
							break;
						  }
						  if (lastDC != dispCount) {
							timer = millis();
							char str[8];
							for (int i = 0; i < 7; i++) { // 9 chars, first 7 are the name, last 2 are the freq
			//				  str[i] = EEPROM.read(eepDisplay[dispCount] + i);
							  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, i);
							  str[i] = read[0];
							}
							str[7] = 0;

							SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
							int val1 = read[0];
							SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
							int val2 = read[0];
							float store = val1 + (.025 * val2);

							sprintf(toPrint, "%c%s%0.3f", 'k', str, store);
							LCDPrint(toPrint);

							lastDC = dispCount;
						  }
						}
						//post loop (pressed button)
						//change the standby to the stored that was selected and call it a day freq will update and display
						if (valid)
						{
			//			  SM = EEPROM.read(eepDisplay[dispCount] + 7);
			//			  SK = EEPROM.read(eepDisplay[dispCount] + 8) * 25;
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 7);
						  SM = read[0];
						  SPIF_ReadSector(&spif, eepDisplay[dispCount], read, 1, 8);
						  SK = read[0] * 25;
						}
						dispCount = 0;
						lastDC = 0;
						pause = false;
						bscroll = false;
						LCDPrint("f");
						HAL_Delay(500);
					  }
					}
				  }
				  last = millis();
			}

	// If count has changed print the new value to serial ==================================================================================================================

			if (vol != vol_last) { // mode v
			  int V = vol + 48;
			  char v = (char)V;
			  char VSfinal[2] = {v,0};
			  Sender(VSfinal,2); //set vol

			  Sender2cdu(VSfinal,2); //sending also to cdu for synchronization

			  sprintf(toPrint, "%c%0.3f%d", 'v', freq, vol);
			  LCDPrint(toPrint);

			  vol_last = vol;
			  last = millis();
			}
			if (obs != obs_last) { // mode s

			  sprintf(toPrint, "%c%03d", 'b', obs);
			  LCDPrint(toPrint);

			  Sender("",3);

			  Sender2cdu("",3); //sending also to cdu for synchronization

			  obs_last = obs;
			  last = millis();
			}
			if (freq != freq_last || Standby != sfreq_last) { // mode f
			  int MA = MHz - 48;
			  int KA = (KHz/25) + 48;
			  char m = (char)MA;
			  char k = (char)KA;
			  char Mfinal[3] = {m,k,0};
			  Sender(Mfinal, 0); //set active

			  Sender2cdu(Mfinal,0); //sending also to cdu for synchronization

			  MA = SM - 48;
			  KA = (SK/25) + 48;
			  m = (char)MA;
			  k = (char)KA;
			  char Sfinal[3] = {m,k,0};
			  Sender(Sfinal, 1); //set standby

			  Sender2cdu(Sfinal,1); //sending also to cdu for synchronization

			  sprintf(toPrint, "%c%0.3f%0.3f", 'f', freq, Standby);
			  LCDPrint(toPrint);

			  sfreq_last = Standby;
			  freq_last = freq;
			  last = millis();
			}
		  }

}

void task2_init(void const * argument) { // rx

	for(;;)
	{

	  //continuously receive data through UART=====================================
//	  response = HAL_UART_Receive(&huart4, rxbuff, 1, 1000);


	  if(response==HAL_OK) //if transfer is successful
	  {
		  if (faultcounter0 > 2)
		  {
			  glcd_clearline(7);
		  }
		  faultcounter0 = 0;

//		  rxmsg[rxcount] = rxbuff[0];
//		  rxcount++;
		  if (rxcount > 24) {
			  rxcount = 0;
		  }

		  if (rxbuff[0] == '\n')
		  {
			  rxmsg[rxcount] = 0;
			  rxcount = 0;
			  rxfree = true;
		  }
	  }
	  else
	  {
		  // no message in 1 seconds
//		  __HAL_UART_FLUSH_DRREGISTER(&huart4);  // Clear the UART Data Register
		  rxcount = 0;
		  faultcounter0++;
		  if (faultcounter0 > 2)
		  {
			  glcd_puts("Error 0", 0, 7);
		  }
	  }
	}
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
