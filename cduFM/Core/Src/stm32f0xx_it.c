/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern SerialStruct BuffUART2;
extern SerialStruct BuffUART3;
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
	keyPad_Scan4SysTick();
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles RCC and CRS global interrupts.
  */
void RCC_CRS_IRQHandler(void)
{
  /* USER CODE BEGIN RCC_CRS_IRQn 0 */

  /* USER CODE END RCC_CRS_IRQn 0 */
  /* USER CODE BEGIN RCC_CRS_IRQn 1 */

  /* USER CODE END RCC_CRS_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
volatile uint32_t Source = huart2.Instance->ISR;
	if (__HAL_UART_GET_IT(&huart2, UART_IT_RXNE))
	{
		BuffUART2.RXbuffer[BuffUART2.RXindex++] = huart2.Instance->RDR & 0x0FF;// USART2->DR;
		BuffUART2.RXindex &= BUFLENMASK;
		BuffUART2.RXbuffer[BuffUART2.RXindex] = 0;         	// truncate string

//		__HAL_UART_CLEAR_IT(&huart2, UART_IT_RXNE);
	}

	if (huart2.Instance->ISR & (1<<(UART_IT_ORE>>8 & 0x1F)))  //handling overrun error, if
	{
		//__HAL_UART_CLEAR_IT(&huart2, UART_IT_ORE);
		huart2.Instance->ICR = 8;
	}


	if (__HAL_UART_GET_IT(&huart2, UART_IT_TXE) )
	{
		if (BuffUART2.TXindex == BuffUART2.LoadIndex)		// all buffer chars sent
			{
				__HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
				//__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);
				BuffUART2.LoadIndex = 0;
				BuffUART2.TXindex = 0;
			}
		else		// pending chars in buffe0xffr
			{
				huart2.Instance->TDR = (uint8_t)BuffUART2.TXbuffer[BuffUART2.TXindex++];
			}
//		__HAL_UART_CLEAR_IT(&huart2, UART_IT_TXE);
	}

//	if (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TC) != RESET)
//	{
//		BuffUART2.LoadIndex = 0;
//		BuffUART2.TXindex = 0;
//		__HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);
//		__HAL_UART_CLEAR_IT(&huart2, 0xffffffff);
//	}

	return;		// by-pass default handler


  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles USART3 to USART8 global interrupts / USART3 wake-up interrupt through EXTI line 28.
  */
void USART3_8_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_8_IRQn 0 */
	volatile uint32_t Source = huart3.Instance->ISR;

	// RXNE (Receive Not Empty) interrupt handling
	if (__HAL_UART_GET_IT(&huart3, UART_IT_RXNE))
	{
		BuffUART3.RXbuffer[BuffUART3.RXindex++] = huart3.Instance->RDR & 0x0FF; // Read received data
		BuffUART3.RXindex &= BUFLENMASK; // Mask to avoid buffer overflow
		BuffUART3.RXbuffer[BuffUART3.RXindex] = 0; // Null-terminate string
	}

	// ORE (Overrun Error) interrupt handling
	if (huart3.Instance->ISR & (1 << (UART_IT_ORE >> 8 & 0x1F)))
	{
		huart3.Instance->ICR = 8; // Clear Overrun Error flag
	}

	// TXE (Transmit Data Register Empty) interrupt handling
	if (__HAL_UART_GET_IT(&huart3, UART_IT_TXE))
	{
		if (BuffUART3.TXindex == BuffUART3.LoadIndex) // All buffer chars sent
		  {
			__HAL_UART_DISABLE_IT(&huart3, UART_IT_TXE); // Disable TXE interrupt
			BuffUART3.LoadIndex = 0;
			BuffUART3.TXindex = 0;
		  }
		else // Pending chars in buffer
		  {
			huart3.Instance->TDR = (uint8_t)BuffUART3.TXbuffer[BuffUART3.TXindex++];
		  }
	}

	return;		// by-pass default handler

  /* USER CODE END USART3_8_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);(&huart3);
  /* USER CODE BEGIN USART3_8_IRQn 1 */

  /* USER CODE END USART3_8_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
