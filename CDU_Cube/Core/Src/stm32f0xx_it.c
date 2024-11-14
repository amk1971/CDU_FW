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

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
/* USER CODE BEGIN EV */
extern CAN_TxHeaderTypeDef pHeader;
extern CAN_RxHeaderTypeDef pRxHeader;
extern uint32_t TxMailbox;
extern uint8_t r;
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
//void SVC_Handler(void)
//{
//  /* USER CODE BEGIN SVC_IRQn 0 */
////
//  /* USER CODE END SVC_IRQn 0 */
//  /* USER CODE BEGIN SVC_IRQn 1 */
////
//  /* USER CODE END SVC_IRQn 1 */
//}

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
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	// USB/BOOTLOADER Serial port
	if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
	{
		if (SysState.DisplayBypass) {
			huart2.Instance->TDR = USART1->RDR;
			return;
		}
		USB_UART.RXbuffer[USB_UART.RXindex++] = USART1->RDR;
		USB_UART.RXindex &= BUFLENMASK;
		USB_UART.RXbuffer[USB_UART.RXindex] = 0;         	// truncate string
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
				huart1.Instance->TDR = (uint8_t)USB_UART.TXbuffer[USB_UART.TXindex++];
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
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

	// DISPLAY serial port
	if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
	{
		if (SysState.DisplayBypass) {
			huart1.Instance->TDR = USART2->RDR;
			return;
		}


		DISPLAY.RXbuffer[DISPLAY.RXindex++] = USART2->RDR;
		DISPLAY.RXindex &= BUFLENMASK;
		DISPLAY.RXbuffer[DISPLAY.RXindex] = 0;           	// truncate string
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TXE) != RESET)
	{
		if (DISPLAY.TXindex == DISPLAY.LoadIndex)		// all buffer chars sent
			{
				__HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
				__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);

			}
		else		// pending chars in buffer
			{
				huart2.Instance->TDR = (uint8_t)DISPLAY.TXbuffer[DISPLAY.TXindex++];
			}
	}

	if (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TC) != RESET)
	{
		DISPLAY.LoadIndex = 0;
		DISPLAY.TXindex = 0;
		__HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);
	}

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

	//	UART4:	PORT1
		if(__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE) != RESET)
		{
			PORT1.RXbuffer[PORT1.RXindex] = USART4->RDR;
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
			}      	// truncate string
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
					huart4.Instance->TDR = (uint8_t)PORT1.TXbuffer[PORT1.TXindex++];
				}
		}

		if (__HAL_UART_GET_IT_SOURCE(&huart4, UART_IT_TC) != RESET)
		{
			PORT1.LoadIndex = 0;
			PORT1.TXindex = 0;
			__HAL_UART_DISABLE_IT(&huart4, UART_IT_TC);
		}


	//	UART6:	PORT2
			if(__HAL_UART_GET_FLAG(&huart6, UART_FLAG_RXNE) != RESET)
		{
				PORT2.RXbuffer[PORT2.RXindex] = USART6->RDR;
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

		if (__HAL_UART_GET_IT_SOURCE(&huart6, UART_IT_TXE) != RESET)
		{
			if (PORT2.TXindex == PORT2.LoadIndex)		// all buffer chars sent
				{
					__HAL_UART_DISABLE_IT(&huart6, UART_IT_TXE);
					__HAL_UART_ENABLE_IT(&huart6, UART_IT_TC);

				}
			else		// pending chars in buffer
				{
					huart6.Instance->TDR = (uint8_t)PORT2.TXbuffer[PORT2.TXindex++];
				}
		}

		if (__HAL_UART_GET_IT_SOURCE(&huart6, UART_IT_TC) != RESET)
		{
			PORT2.LoadIndex = 0;
			PORT2.TXindex = 0;
			__HAL_UART_DISABLE_IT(&huart6, UART_IT_TC);
		}

	//	UART5:	PORT3

			if(__HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE) != RESET)
		{
				PORT3.RXbuffer[PORT3.RXindex] = USART5->RDR;
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

		if (__HAL_UART_GET_IT_SOURCE(&huart5, UART_IT_TXE) != RESET)
		{
			if (PORT3.TXindex == PORT3.LoadIndex)		// all buffer chars sent
				{
					__HAL_UART_DISABLE_IT(&huart5, UART_IT_TXE);
					__HAL_UART_ENABLE_IT(&huart5, UART_IT_TC);

				}
			else		// pending chars in buffer
				{
					huart5.Instance->TDR = (uint8_t)PORT3.TXbuffer[PORT3.TXindex++];
				}
		}

		if (__HAL_UART_GET_IT_SOURCE(&huart5, UART_IT_TC) != RESET)
		{
			PORT3.LoadIndex = 0;
			PORT3.TXindex = 0;
			__HAL_UART_DISABLE_IT(&huart5, UART_IT_TC);
		}

	//	UART3:	PORT4
			if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
		{
				PORT4.RXbuffer[PORT4.RXindex] = USART3->RDR;
				// if end of message received ("\n"),
				// send message to the queue and reset the circular buffer pointer
				if(PORT4.RXbuffer[PORT4.RXindex] == 0x0A){

					PORT4.RXbuffer[PORT4.RXindex] = 0;	// end of string
					xQueueSendToBackFromISR(PORT4_RX_queue, (void*)PORT4.RXbuffer,NULL); // copy message to queue
					PORT4.RXindex = 0;	// reset pointer
				}
				// otherwise keep writing the circular buffer
				else{
					PORT4.RXindex++;
					PORT4.RXindex &= BUFLENMASK;
					PORT4.RXbuffer[PORT4.RXindex] = 0;  	// truncate string
				}
		}

		if (__HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_TXE) != RESET)
		{
			if (PORT4.TXindex == PORT4.LoadIndex)		// all buffer chars sent
				{
					__HAL_UART_DISABLE_IT(&huart3, UART_IT_TXE);
					__HAL_UART_ENABLE_IT(&huart3, UART_IT_TC);

				}
			else		// pending chars in buffer
				{
					huart3.Instance->TDR = (uint8_t)PORT4.TXbuffer[PORT4.TXindex++];
				}
		}

		if (__HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_TC) != RESET)
		{
			PORT4.LoadIndex = 0;
			PORT4.TXindex = 0;
			__HAL_UART_DISABLE_IT(&huart3, UART_IT_TC);
		}

		return;		// by-pass default handlers

  /* USER CODE END USART3_8_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  HAL_UART_IRQHandler(&huart4);
  HAL_UART_IRQHandler(&huart5);
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART3_8_IRQn 1 */

  /* USER CODE END USART3_8_IRQn 1 */
}

/**
  * @brief This function handles HDMI-CEC and CAN global interrupts / HDMI-CEC wake-up interrupt through EXTI line 27.
  */
void CEC_CAN_IRQHandler(void)
{
  /* USER CODE BEGIN CEC_CAN_IRQn 0 */

  /* USER CODE END CEC_CAN_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan);
  /* USER CODE BEGIN CEC_CAN_IRQn 1 */
  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &pRxHeader, &r);
  /* USER CODE END CEC_CAN_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
