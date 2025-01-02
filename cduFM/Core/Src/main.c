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
#include "spi.h"
#include "spif.h"
#include "usart.h"
#include "gpio.h"

#include "NAVScreen.h"
#include "ADFScreen.h"

#define FLASHUPDATEDELAY 10000	//update flash after 10 whenever there is a change in any value.

SPIF_HandleTypeDef spif;

globalvar_t gVar;
uint8_t read[1];
uint8_t write[1];

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SerialStruct BuffUART2;
SerialStruct BuffUART3;

ScreenParams NavScreenParams;
ScreenParams AdfScreenParams;

extern lcdCmdDisp_id currentScreen, NextScreen;

softKey_t softKey;

extern MkeyStatus_t MkeyStatus;
extern keyPad_t keyPad;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

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

//void toDisplay() {						//reads sectors from the SPI flash memory and updates the eepDisplay
//	for (int i = 0; i < 16; ++i) {
//		eepDisplay[i] = -1;
//	}
//	count = 0;
//	for (int i = 0; i < 16; i++) {
//		SPIF_ReadSector(&spif, i, read, 1, 0);
//		if (read[0] != 255) {
//			eepDisplay[count] = i;
//			count++;
//		}
//	}
//}

//void toAdd(const char* dN, int dM, int dK) {		//adds a new entry to the first available empty sector in the SPI flash memory
//  for (int i = 0; i < 16; i ++) {
//	SPIF_ReadSector(&spif, i, read, 1, 0);
//    if (read[0] == 255) {
//      for (int j = 0; j < 7; j++) {
//    	write[0] = dN[j];
//    	SPIF_WriteSector(&spif, i, write, 1, j);
//      }
//	  write[0] = dM;
//	  SPIF_WriteSector(&spif, i, write, 1, 7);
//	  write[0] = dK/25;
//	  SPIF_WriteSector(&spif, i, write, 1, 8);
//      break;
//    }
//  }
//}

//void toDelete(int index) {		//erases a specific sector in the SPI flash memory, effectively marking it as empty.
//	SPIF_EraseSector(&spif, index);
//}
//
//void storestate() { // called when turned off			stores the current state of the system in a specific sector of the SPI flash memory.
//  SPIF_EraseSector(&spif, 20);
//  write[0] = MHz;
//  SPIF_WriteSector(&spif, 20, write, 1, 0);
//  write[0] = KHz/25;
//  SPIF_WriteSector(&spif, 20, write, 1, 1);
//  write[0] = SM;
//  SPIF_WriteSector(&spif, 20, write, 1, 2);
//  write[0] = SK/25;
//  SPIF_WriteSector(&spif, 20, write, 1, 3);
//  write[0] = vol;
//  SPIF_WriteSector(&spif, 20, write, 1, 4);
//  write[0] = obs;
//}

//void loadstate() { // called on startup					loads the system state from a specific sector in the SPI flash memory on startup.
//  SPIF_ReadSector(&spif, 20, read, 1, 0);
//  MHz = read[0];
//  if (MHz > 117) MHz = 117;
//  if (MHz < 108) MHz = 108;
//  SPIF_ReadSector(&spif, 20, read, 1, 1);
//  KHz = read[0] * 25;
//  if(KHz > 950) KHz = 950;
//  SPIF_ReadSector(&spif, 20, read, 1, 2);
//  SM = read[0];
//  if (SM > 117) SM = 117;
//  if (SM < 108) SM = 108;
//  SPIF_ReadSector(&spif, 20, read, 1, 3);
//  SK = read[0] * 25;
//  if(SK > 950) SK = 950;
//  SPIF_ReadSector(&spif, 20, read, 1, 4);	//1 is not bytes to be read
//  vol = read[0];
//  SPIF_ReadSector(&spif, 20, read, 1, 5);	//5 is Offset: The starting point within the sector to begin reading from.
//  obs = read[0];
//  freq = MHz + (.001 * KHz);
//  Standby = SM + (.001 * SK);
//}


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

  /* USER CODE BEGIN 1 */

  uint8_t key;
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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  SPIF_Init(&spif, &hspi1, SPI1_CS_GPIO_Port, SPI1_CS_Pin);

  KeyS_init();

//  loadstate();
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
  /* USER CODE END 2 */

  gVar.flashDirty = true;	// to store the data from RAM to Flash
  gVar.updateFlashTimer = HAL_GetTick();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  softKey = check_soft_keys();
	  switch(currentScreen)
	  {
	  case lcdDisp_home:
		  if(softKey == L1)
		  {
			  currentScreen = lcdWaitForSW;
			  NextScreen = lcdDisp_nav;
			  DispNAVscreen(&NavScreenParams);
		  }
		  if(softKey == L2)
		  {
			  currentScreen = lcdWaitForSW;
			  NextScreen = lcdDisp_adf;
			  DispADFscreen(&AdfScreenParams);
		  }
		  if(softKey == L3)
		  {
			  currentScreen = lcdWaitForSW;
			  NextScreen = lcdDisp_tacan;
			  DispTACANscreen();
		  }
		  break;

	  case lcdWaitForSW:
		  if (softKey == idle) currentScreen = NextScreen;

		  break;

	  case lcdDisp_nav:
		  //MkeyStatus = keyPad_Scan();

		  key = NavScreenStateMachine(&NavScreenParams, softKey);

		  if (NavScreenParams.Active.freq != Nav_afreq_last || NavScreenParams.Standby.freq != Nav_sfreq_last)
		  {
			  gVar.flashDirty = true;
			  gVar.updateFlashTimer = HAL_GetTick();

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
		  key = AdfScreenStateMachine(&AdfScreenParams, softKey);

		  if (AdfScreenParams.Active.freq != Adf_afreq_last || AdfScreenParams.Standby.freq != Adf_sfreq_last)
		  {
			  gVar.flashDirty = true;
			  gVar.updateFlashTimer = HAL_GetTick();


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
	  if(gVar.flashDirty)
	  {
		  if((HAL_GetTick() - gVar.updateFlashTimer) >= FLASHUPDATEDELAY)
		  {
//			  storestate();
			  gVar.flashDirty = false;
//			  gVar.updateFlashTimer = HAL_GetTick();
		  }

	  }
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
