/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
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
#include "stm32f1xx_it.h"
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

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
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
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
//void SVC_Handler(void)
//{
//  /* USER CODE BEGIN SVCall_IRQn 0 */
////
//  /* USER CODE END SVCall_IRQn 0 */
//  /* USER CODE BEGIN SVCall_IRQn 1 */
////
//  /* USER CODE END SVCall_IRQn 1 */
//}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
//void PendSV_Handler(void)
//{
//  /* USER CODE BEGIN PendSV_IRQn 0 */
////
//  /* USER CODE END PendSV_IRQn 0 */
//  /* USER CODE BEGIN PendSV_IRQn 1 */
////
//  /* USER CODE END PendSV_IRQn 1 */
//}

/**
  * @brief This function handles System tick timer.
  */
//void SysTick_Handler(void)
//{
//  /* USER CODE BEGIN SysTick_IRQn 0 */
////
//  /* USER CODE END SysTick_IRQn 0 */
//  HAL_IncTick();
//  /* USER CODE BEGIN SysTick_IRQn 1 */
////
//  /* USER CODE END SysTick_IRQn 1 */
//}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
	{
		USB_UART.RXbuffer[USB_UART.RXindex++] = USART1->DR;
		USB_UART.RXindex &= BUFLENMASK;
		USB_UART.RXbuffer[USB_UART.RXindex] = 0;        	// truncate string
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE) != RESET)
	{
		if (USB_UART.TXindex == USB_UART.LoadIndex)		// all buffer chars sent
			{
				__HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
				__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);

			}
		else		// pending chars in buffer
			{
				huart1.Instance->DR = (uint8_t)USB_UART.TXbuffer[USB_UART.TXindex++];
			}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TC) != RESET)
	{
		USB_UART.LoadIndex = 0;
		USB_UART.TXindex = 0;
		__HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);
	}

	return;		// by-pass default handler
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
	{
		PORT3.RXbuffer[PORT3.RXindex] = USART2->DR;
		// if end of message received ("\n"),
		// send message to the queue and reset the circular buffer pointer
		if(PORT3.RXbuffer[PORT3.RXindex] == 0x0A){

			PORT3.RXbuffer[PORT3.RXindex] = 0;	// end of string
			xQueueSendToBackFromISR(PORT3_RX_queue, (void*)PORT3.RXbuffer,NULL); // copy message to queue
			PORT3.RXindex = 0;	// reset pointer
		}
		// otherwise keep writing the circular buffer
		else{
			PORT3.RXindex++;
			PORT3.RXindex &= BUFLENMASK;
			PORT3.RXbuffer[PORT3.RXindex] = 0;  	// truncate string
		}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TXE) != RESET)
	{
		if (PORT3.TXindex == PORT3.LoadIndex)		// all buffer chars sent
			{
				__HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
				__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);

			}
		else		// pending chars in buffer
			{
				huart2.Instance->DR = (uint8_t)PORT3.TXbuffer[PORT3.TXindex++];
			}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TC) != RESET)
	{
		PORT3.LoadIndex = 0;
		PORT3.TXindex = 0;
		__HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);
	}

	return;		// by-pass default handler
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */
	if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE) != RESET)
	{
		PORT1.RXbuffer[PORT1.RXindex] = UART4->DR;
		// if end of message received ("\n"),
		// send message to the queue and reset the circular buffer pointer
		if(PORT1.RXbuffer[PORT1.RXindex] == 0x0A){

			PORT1.RXbuffer[PORT1.RXindex] = 0;	// end of string
			xQueueSendToBackFromISR(PORT1_RX_queue, (void*)PORT1.RXbuffer,NULL); // copy message to queue
			PORT1.RXindex = 0;	// reset pointer
		}
		// otherwise keep writing the circular buffer
		else{
			PORT1.RXindex++;
			PORT1.RXindex &= BUFLENMASK;
			PORT1.RXbuffer[PORT1.RXindex] = 0;  	// truncate string
		}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart4, UART_IT_TXE) != RESET)
	{
		if (PORT1.TXindex == PORT1.LoadIndex)		// all buffer chars sent
			{
				__HAL_UART_DISABLE_IT(&huart4, UART_IT_TXE);
				__HAL_UART_ENABLE_IT(&huart4, UART_IT_TC);

			}
		else		// pending chars in buffer
			{
				huart4.Instance->DR = (uint8_t)PORT1.TXbuffer[PORT1.TXindex++];
			}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart4, UART_IT_TC) != RESET)
	{
		PORT1.LoadIndex = 0;
		PORT1.TXindex = 0;
		__HAL_UART_DISABLE_IT(&huart4, UART_IT_TC);
	}

	return;		// by-pass default handler
  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
}

/**
  * @brief This function handles UART5 global interrupt.
  */
void UART5_IRQHandler(void)
{
  /* USER CODE BEGIN UART5_IRQn 0 */
	if (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE) != RESET)
	{
		PORT2.RXbuffer[PORT2.RXindex] = UART5->DR;
		// if end of message received ("\n"),
		// send message to the queue and reset the circular buffer pointer
		if(PORT2.RXbuffer[PORT2.RXindex] == 0x0A){

			PORT2.RXbuffer[PORT2.RXindex] = 0;	// end of string
			xQueueSendToBackFromISR(PORT2_RX_queue, (void*)PORT2.RXbuffer,NULL); // copy message to queue
			PORT2.RXindex = 0;	// reset pointer
		}
		// otherwise keep writing the circular buffer
		else{
			PORT2.RXindex++;
			PORT2.RXindex &= BUFLENMASK;
			PORT2.RXbuffer[PORT2.RXindex] = 0;  	// truncate string
		}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart5, UART_IT_TXE) != RESET)
	{
		if (PORT2.TXindex == PORT2.LoadIndex)		// all buffer chars sent
			{
				__HAL_UART_DISABLE_IT(&huart5, UART_IT_TXE);
				__HAL_UART_ENABLE_IT(&huart5, UART_IT_TC);

			}
		else		// pending chars in buffer
			{
				huart5.Instance->DR = (uint8_t)PORT2.TXbuffer[PORT2.TXindex++];
			}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart5, UART_IT_TC) != RESET)
	{
		PORT2.LoadIndex = 0;
		PORT2.TXindex = 0;
		__HAL_UART_DISABLE_IT(&huart5, UART_IT_TC);
	}

	return;		// by-pass default handler
  /* USER CODE END UART5_IRQn 0 */
  HAL_UART_IRQHandler(&huart5);
  /* USER CODE BEGIN UART5_IRQn 1 */

  /* USER CODE END UART5_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
