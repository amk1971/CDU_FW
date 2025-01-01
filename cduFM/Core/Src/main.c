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

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

SerialStruct BuffUART2;
SerialStruct BuffUART3;

ScreenParams NavScreenParams;
ScreenParams AdfScreenParams;

extern lcdCmdDisp_id currentScreen, NextScreen;
extern softKey_t softkey;

extern MkeyStatus_t MkeyStatus;
extern keyPad_t keyPad;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t UART_ReceivString(SerialStruct * BuffUART, char *str, int NumChar) {

	uint8_t n = 0;

	n = (BuffUART->RXindex - BuffUART->RXTail) & (BUFLENMASK);

	if((n < NumChar) || (NumChar <= 0)){
		return 0;
	}

	n = 0;

	while (NumChar--)
	{
		str[n++] = BuffUART->RXbuffer[BuffUART->RXTail++];
	}
	return NumChar;
}


void UART_SendString(UART_HandleTypeDef *huart, SerialStruct * BuffUART, const char *str, int NumChar) {

	uint8_t n = 0;

	BuffUART->TXbuflen = NumChar;//= strlen(str);
	while (BuffUART->TXbuflen > (MAXBUFLEN - BuffUART->LoadIndex));

	while (BuffUART->TXbuflen--)
	{
		BuffUART->TXbuffer[BuffUART->LoadIndex++] = str[n++];
	}

	// Start transmission if this is first byte in buffer
	if(BuffUART->TXindex == 0)
	{
		if (huart->Instance == USART2)
		{
			huart2.Instance->TDR = BuffUART->TXbuffer[BuffUART->TXindex++];
			__HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
		}
		else if (huart->Instance == USART3)
		{
			huart3.Instance->TDR = BuffUART->TXbuffer[BuffUART->TXindex++];
			__HAL_UART_ENABLE_IT(&huart3, UART_IT_TXE);
		}
	}

    //uint16_t len = strlen(str);
    //for (uint16_t i = 0; i < len; i++) {
    //    uint16_t BuffUART.next_head = (tx_head + 1) % UART_BUFFER_SIZE;
    //    while (next_head == tx_tail); // Wait if buffer is full
    //    tx_buffer[tx_head] = str[i];
    //    tx_head = next_head;
    //}
    //__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE); // Enable TXE interrupt
}


void MhzKhz2freq(int MHz, int KHz, double* freq){
	if (freq != NULL) {
        *freq = MHz + (KHz / 1000.0);
    }
}


void freq2MhzKhz(double num, volatile int *MHz, volatile int *KHz) {
	*MHz = (int)num; // Integer part as MHz
	*KHz = (int)((num - *MHz) * 1000 + 0.5); // Fractional part as kHz
}

void checksum(const char* str, char* result) {	//calculates a checksum from a given string and stores the result in a character array
    unsigned int sum = 0;
    const char* ptr = str;

    if (*ptr != '\0') {
        ptr += 6;			//starting after the first 6 characters (possibly skipping a header like e.g: "$PATCN")
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

void Sender2rcu(const char * str, int mode) { //TODO				construct and send a message (likely over UART)
    char crlf[] = {'\r','\n', 0};
    uint8_t str2[25]; // Array to hold the constructed string in ASCII

    memset(str2, 0, sizeof(str2)); // Initialize the array with zeros

    if (mode == 0) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), "$PATCN27%c%cN", m, k);
        char end2[3];
        checksum((char*)str2, end2);
        concatTwoChars((char*)str2, end2);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 1) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), "$PATCN28%c%cN", m, k);
        char end3[3];
        checksum((char*)str2, end3);
        concatTwoChars((char*)str2, end3);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 2) {
        char v = str[0];
        snprintf((char*)str2, sizeof(str2), "$PATCN73%c", v);
        char end4[3];
        checksum((char*)str2, end4);
        concatTwoChars((char*)str2, end4);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 3) {
//        snprintf((char*)str2, sizeof(str2), "$PATCN34%03d", obs);
//        char end5[3];
//        checksum((char*)str2, end5);
//        concatTwoChars((char*)str2, end5);
        concatTwoChars((char*)str2, crlf);
    }

    UART_SendString(&huart3, &BuffUART3, str2, strlen((char*)str2));
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  softKey_t softKey;
  uint8_t key;
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  volatile double x = atof("190.01");

  char str3[10];

  strncpy(str3, "190.200", 10);

  x = atof(str3);

  if(x<MIN_FREQUENCY) {
	  x = 1;
  }
  //strtod()
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */
  KeyS_init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  //----------------------------------------------------------------------------------------------------------

  NavScreenParams.Active.freq = 118.0;
  freq2MhzKhz(NavScreenParams.Active.freq, &NavScreenParams.Active.MHz, &NavScreenParams.Active.KHz);
//  NavScreenParams.Active.MHz = 118;
//  NavScreenParams.Active.KHz = 0;
  NavScreenParams.Standby.freq = 108.4;
  freq2MhzKhz(NavScreenParams.Standby.freq, &NavScreenParams.Standby.MHz, &NavScreenParams.Standby.KHz);
  NavScreenParams.Standby.tileSize = 2;
//  NavScreenParams.Standby.MHz = 108;
//  NavScreenParams.Standby.KHz = 4;
  NavScreenParams.page = false; // means page0
  NavScreenParams.P1.freq = 123.45;
  NavScreenParams.P1.tileSize = 3;
  NavScreenParams.P3.freq = 354.85;
  NavScreenParams.P6.freq = 534.15;
  NavScreenParams.P8.freq = 878.98;

  static double Nav_afreq_last = 118.0;
  static double Nav_sfreq_last = 108.4;

  //----------------------------------------------------------------------------------------------------------

  AdfScreenParams.Active.freq = 108.0;
  freq2MhzKhz(AdfScreenParams.Active.freq, &AdfScreenParams.Active.MHz, &AdfScreenParams.Active.KHz);
//  AdfScreenParams.Active.MHz = 118;
//  AdfScreenParams.Active.KHz = 0;
  AdfScreenParams.Standby.freq = 118.4;
  freq2MhzKhz(AdfScreenParams.Standby.freq, &AdfScreenParams.Standby.MHz, &AdfScreenParams.Standby.KHz);
  AdfScreenParams.Standby.tileSize = 2;
//  AdfScreenParams.Standby.MHz = 108;
//  AdfScreenParams.Standby.KHz = 4;
  AdfScreenParams.page = false; // means page0
  AdfScreenParams.P1.freq = 125.45;
  AdfScreenParams.P1.tileSize = 3;
  AdfScreenParams.P3.freq = 334.85;
  AdfScreenParams.P6.freq = 536.15;
  AdfScreenParams.P8.freq = 877.98;

  static double Adf_afreq_last = 108.0;
  static double Adf_sfreq_last = 118.4;

  //----------------------------------------------------------------------------------------------------------

  InitializeLCD();
  //Matrix_keypad_Basic_test();
//  KeyS_init();
  while (1)
  {
	  softKey = check_soft_keys();
	  switch(currentScreen)
	  {
	  case lcdDisp_home:
		  if(softkey == L1)
		  {
			  currentScreen = lcdWaitForSW;
			  NextScreen = lcdDisp_nav;
			  DispNAVscreen(&NavScreenParams);
		  }
		  if(softkey == L2)
		  {
			  currentScreen = lcdWaitForSW;
			  NextScreen = lcdDisp_adf;
			  DispADFscreen(&AdfScreenParams);
		  }
		  if(softkey == L3)
		  {
			  currentScreen = lcdWaitForSW;
			  NextScreen = lcdDisp_tacan;
			  DispTACANscreen();
		  }
		  break;

	  case lcdWaitForSW:
		  if (softkey == idle) currentScreen = NextScreen;

		  break;

	  case lcdDisp_nav:
		  //MkeyStatus = keyPad_Scan();

		  key = NavScreenStateMachine(&NavScreenParams, softKey);

		  if (NavScreenParams.Active.freq != Nav_afreq_last || NavScreenParams.Standby.freq != Nav_sfreq_last)
		  {

			  freq2MhzKhz(NavScreenParams.Active.freq, &NavScreenParams.Active.MHz, &NavScreenParams.Active.KHz);
			  freq2MhzKhz(NavScreenParams.Standby.freq, &NavScreenParams.Standby.MHz, &NavScreenParams.Standby.KHz);

			  int MA = NavScreenParams.Active.MHz - 48;
			  int KA = (NavScreenParams.Active.KHz/25) + 48;
			  char m = (char)MA;
			  char k = (char)KA;
			  char Mfinal[3] = {m,k,0};
			  Sender2rcu(Mfinal,0); //sending also to rcu for synchronization

			  MA = NavScreenParams.Standby.MHz - 48;
			  KA = (NavScreenParams.Standby.KHz/25) + 48;
			  m = (char)MA;
			  k = (char)KA;
			  char Sfinal[3] = {m,k,0};

			  Sender2rcu(Sfinal,1); //sending also to rcu for synchronization

			  Nav_sfreq_last = NavScreenParams.Standby.freq;
			  Nav_afreq_last = NavScreenParams.Active.freq;

		  }

		  if (key == 'h')	// HOME button
		  {
			  currentScreen = lcdDisp_home;
			  DispHomeScreen();
		  }
		  else if(key == 0)
		  {
			  // do nothing here
		  }
		  break;

	  case lcdDisp_adf:
//		  MkeyStatus = keyPad_Scan();
		  key = AdfScreenStateMachine(&AdfScreenParams);

		  if (AdfScreenParams.Active.freq != Adf_afreq_last || AdfScreenParams.Standby.freq != Adf_sfreq_last)
		  {

			  freq2MhzKhz(AdfScreenParams.Active.freq, &AdfScreenParams.Active.MHz, &AdfScreenParams.Active.KHz);
			  freq2MhzKhz(AdfScreenParams.Standby.freq, &AdfScreenParams.Standby.MHz, &AdfScreenParams.Standby.KHz);

			  int MA = AdfScreenParams.Active.MHz - 48;
			  int KA = (AdfScreenParams.Active.KHz/25) + 48;
			  char m = (char)MA;
			  char k = (char)KA;
			  char Mfinal[3] = {m,k,0};
			  Sender2rcu(Mfinal,0); //sending also to rcu for synchronization

			  MA = AdfScreenParams.Standby.MHz - 48;
			  KA = (AdfScreenParams.Standby.KHz/25) + 48;
			  m = (char)MA;
			  k = (char)KA;
			  char Sfinal[3] = {m,k,0};

			  Sender2rcu(Sfinal,1); //sending also to rcu for synchronization

			  Adf_sfreq_last = AdfScreenParams.Standby.freq;
			  Adf_afreq_last = AdfScreenParams.Active.freq;

		  }

		  if (key == 'h')	// HOME button
		  {
			  currentScreen = lcdDisp_home;
			  DispHomeScreen();
		  }
		  else if(key == 0)
		  {
			  // do nothing here
		  }
		  break;

	  case lcdDisp_tacan:
		  MkeyStatus = keyPad_Scan();
		  if (keyPad.key == 0x101)	// to be checked later
		  {
			  currentScreen = lcdDisp_home;
			  DispHomeScreen();
		  }
		  else if(keyPad.key == 0)
		  {
			  // do nothing here
		  }
		  break;
	  default:
		  break;
	  }

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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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

  //huart2.Instance->CR1 |= USART_CR1_FIFOEN;
  /* USER CODE BEGIN USART2_Init 2 */
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
  //__HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);

  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
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
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_TXE);

  HAL_NVIC_SetPriority(USART3_8_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART3_8_IRQn);
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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ROW2_Pin|ROW3_Pin|ROW4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, ROW5_Pin|ROW6_Pin|ROW7_Pin|ROW8_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : Left_SW2_Pin Left_SW1_Pin Left_SW4_Pin Left_SW3_Pin */
  GPIO_InitStruct.Pin = Left_SW2_Pin|Left_SW1_Pin|Left_SW4_Pin|Left_SW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : ROW1_Pin */
  GPIO_InitStruct.Pin = ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ROW1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW2_Pin ROW3_Pin ROW4_Pin */
  GPIO_InitStruct.Pin = ROW2_Pin|ROW3_Pin|ROW4_Pin;
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

  /*Configure GPIO pins : COL1_Pin COL2_Pin COL3_Pin COL4_Pin */
  GPIO_InitStruct.Pin = COL1_Pin|COL2_Pin|COL3_Pin|COL4_Pin;
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

  /*Configure GPIO pins : Right_SW4_Pin Right_SW3_Pin Right_SW2_Pin Right_SW1_Pin */
  GPIO_InitStruct.Pin = Right_SW4_Pin|Right_SW3_Pin|Right_SW2_Pin|Right_SW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
