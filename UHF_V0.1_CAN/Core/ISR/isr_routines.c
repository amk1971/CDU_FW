#include "isr_routines.h"
#include "global_vars.h"
#include <string.h>

#include "..\SWITCHES_Encoder\rotary_encoder.h"
#include "main.h"
#include "serial_handler.h"
#include "usart.h"

extern s_HF_Parameters HF_parameters;
// Buffers for UART data
volatile uint8_t rxByte_CDU = 0;
// volatile uint8_t rxIndex_CDU = 0;
volatile uint8_t rxByte_HF_REC = 0;
// volatile uint8_t rxIndex_HF_REC = 0;


// Byte variables for received data

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
	if (htim->Instance == TIM6)
	{
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */
	if (htim->Instance == TIM3)  // Check if TIM2 triggered
	{
		g_vars.cursor_blink = !g_vars.cursor_blink;  // Toggle cursor blink flag

	    // Remove cursor from previous position before updating
	    if (g_vars.previous_cursor_location != END_CURSOR_LOCATION)
	    {
	        LCD_PRINT_CURSOR(false, g_vars.previous_cursor_location);
	    }

		if (g_vars.cursor_blink == ON)
		{
			LCD_PRINT_CURSOR(true,g_vars.current_cursor_location);
		}
		else
		{
			LCD_PRINT_CURSOR(false,g_vars.current_cursor_location);
		}

	g_vars.previous_cursor_location=g_vars.current_cursor_location;
	}
	/* USER CODE END Callback 1 */
}


/**
  * @brief  This function handles EXTI interrupt request.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_14) {  // Check if EXTI line 14 triggered the interrupt
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14)) {  // Check if it was from PC14
            // Handle interrupt from PC14
        	GPIO_EXTI_Callback_Left(GPIO_Pin);
        } else
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)) {  // Check if it was from PB14
            // Handle interrupt from PB14
        	HAL_GPIO_EXTI_Callback(GPIO_Pin);
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
        //EXTI->PR = EXTI_PR_PR14;  // Clear the interrupt pending flag
    } else
  /* EXTI line interrupt detected */
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00u)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
    HAL_GPIO_EXTI_Callback(GPIO_Pin);
  }
}


// Indices for buffer management
void GPIO_EXTI_Callback_Left(uint16_t GPIO_Pin){
 if (GPIO_Pin == LEFT_A1_Pin)
	{
		if (!HF_parameters.PROG == OFF)
		{
			read_encoder_volume();
		}
	}


}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == LEFT_A2_Pin)
	{  // LEFT 2
		if (HF_parameters.PROG == ON)
		{
			scroll_freqs_memory();
		}
	}
	else if (GPIO_Pin == LEFT_B2_Pin)
	{  // LEFT 2
		if (HF_parameters.PROG == ON)
		{
			scroll_freqs_memory();
		}
	}
	else if (GPIO_Pin == LEFT_B1_Pin)
	{  // LEFT 1
		if (HF_parameters.PROG == OFF)
		{
			read_encoder_volume();
		}
	}
	else if (GPIO_Pin == RIGHT_A1_Pin)
	{

        if ((HF_parameters.PROG == ON))// && (HF_parameters.FRQ_CH == FRQ) && (HF_parameters.Cursor == ON))
        {
        	//adjust_digit_interrupt();
        	read_encoder_channel_khz();
        }
        else
        {
        	if(HF_parameters.FRQ_CH == FRQ){
            //change_saved_channel_khz();
        		read_encoder_standby_khz();
        	} else {
        		//No Change on Inner Knob
        	}
        }
	}
	else if (GPIO_Pin == RIGHT_B1_Pin)
	{  // DONE change of FRQ and select preset on CH and non functional in GD
        //if ((!HF_parameters.PROG) && (HF_parameters.FRQ_CH == FRQ) && (HF_parameters.Cursor == ON))
        if(HF_parameters.PROG == ON){
        	//adjust_digit_interrupt();
        	read_encoder_channel_khz();
        }
        else
        {
        	if(HF_parameters.FRQ_CH == FRQ){
            //change_saved_channel_khz();
        		read_encoder_standby_khz();
        	} else {
        		//No Change on Inner Knob
        	}
        }
	}
	else if (GPIO_Pin == RIGHT_A2_Pin)
	{  // DONE change of FRQ and select preset on CH and non functional in GD
		//if ((!HF_parameters.PROG) && (HF_parameters.FRQ_CH == FRQ)){
			if (HF_parameters.PROG == ON){
			//read_encoder_standby_mhz();
				//move_cursor();
				read_encoder_channel_mhz();
			} else {
	        	if(HF_parameters.FRQ_CH == FRQ){
	            //change_saved_channel_khz();
	        		read_encoder_standby_mhz();
	        	} else {
	        		encoder_change_channel();
	        	}
			}
		//}
	}
	else if (GPIO_Pin == RIGHT_B2_Pin)
	{  // DONE change of FRQ and select preset on CH and non functional in GD
		//if ((!HF_parameters.PROG) && (HF_parameters.FRQ_CH == FRQ)){
			if (HF_parameters.PROG == ON){
				read_encoder_channel_mhz();
				//move_cursor();
			} else {
	        	if(HF_parameters.FRQ_CH == FRQ){
	            //change_saved_channel_khz();
	        		read_encoder_standby_mhz();
	        	} else {
	        		encoder_change_channel();
	        	}
			}
		//}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == CDU_UART)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		// Push the received byte into a buffer
		static uint8_t rxBuffer_CDU_ISR[RX_BUFFER_SIZE];
		static uint8_t rxIndex_CDU_ISR = 0;

		rxBuffer_CDU_ISR[rxIndex_CDU_ISR++] = rxByte_CDU;

		// Check for "\r\n" in the last two received characters
		if (rxIndex_CDU_ISR >= 2
				&& rxBuffer_CDU_ISR[rxIndex_CDU_ISR - 2] == '\r'
				&& rxBuffer_CDU_ISR[rxIndex_CDU_ISR - 1] == '\n')
		{
			// Send the buffer from the ISR to the task via the queue
			if (xCDUCOMRxQueue != NULL)
			{
				xQueueSendFromISR(xCDUCOMRxQueue, rxBuffer_CDU_ISR,
						&xHigherPriorityTaskWoken);
				memset(rxBuffer_CDU_ISR, 0, RX_BUFFER_SIZE);
			}

			// Reset the index for the next message
			rxIndex_CDU_ISR = 0;
		}
		else if (rxIndex_CDU_ISR >= RX_BUFFER_SIZE)
		{
			// Reset the index if the buffer is full without finding "\r\n"
			memset(rxBuffer_CDU_ISR, 0, RX_BUFFER_SIZE);
			rxIndex_CDU_ISR = 0;
		}

		// Yield if a higher-priority task was woken
		if (xHigherPriorityTaskWoken == pdTRUE)
		{
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
		}

		// Re-enable UART reception for the next byte
		UART_StartReceive_CDU();
	}

	if (huart->Instance == HF_RECEIVER_UART)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		// Use a local buffer for storing data in ISR
		static uint8_t rxBuffer_HF_REC_ISR[RX_BUFFER_SIZE];
		static uint8_t rxIndex_HF_REC_ISR = 0;

		// Store the received byte into the local buffer
		rxBuffer_HF_REC_ISR[rxIndex_HF_REC_ISR++] = rxByte_HF_REC;

		// Check for "\r\n" in the last two received characters
		if (rxIndex_HF_REC_ISR >= 2
				&& rxBuffer_HF_REC_ISR[rxIndex_HF_REC_ISR - 2] == '\r'
				&& rxBuffer_HF_REC_ISR[rxIndex_HF_REC_ISR - 1] == '\n')
		{
			// Send the buffer to the queue for processing in the task
			if (xHFRecRxQueue != NULL)
			{
				xQueueSendFromISR(xHFRecRxQueue, rxBuffer_HF_REC_ISR,
						&xHigherPriorityTaskWoken);
				memset(rxBuffer_HF_REC_ISR, 0, RX_BUFFER_SIZE);
			}

			// Reset the index for the next message
			rxIndex_HF_REC_ISR = 0;
		}
		else if (rxIndex_HF_REC_ISR >= RX_BUFFER_SIZE)
		{
			// Reset index if the buffer is full without finding "\r\n"
			memset(rxBuffer_HF_REC_ISR, 0, RX_BUFFER_SIZE);
			rxIndex_HF_REC_ISR = 0;
		}

		// Yield if a higher-priority task was woken
		if (xHigherPriorityTaskWoken == pdTRUE)
		{
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
		}

		// Re-enable UART reception for the next byte
		UART_StartReceive_HF_Receiver();
	}
}

/*
 * Function: UART_StartReceive_CDU
 * Arguments: None
 *
 * Description:
 * This function initiates the reception of data over UART using interrupt mode.
 * It sets up the UART handle to receive data into the provided buffer (`rxBuffer`)
 * with the specified buffer size (`RX_BUFFER_SIZE`). The function leverages the
 * HAL_UART_Receive_IT function to enable interrupt-based data reception.
 */
void UART_StartReceive_CDU(void)
{
	HAL_UART_Receive_IT(&CDU_COM, (uint8_t*) &rxByte_CDU, 1);
}

/*
 * Function: UART_StartTransmittion_CDU
 * Arguments: None
 *
 * Description:
 * This function initiates the transmission of data over UART using interrupt mode.
 * It sets up the UART handle to transmit data from the provided buffer (`txBuffer`)
 * with the size of the buffer being transmitted. The function leverages the
 * HAL_UART_Transmit_IT function to enable interrupt-based data transmission.
 */

/*
 * Function: UART_StartReceive_HF_Receiver
 * Arguments: None
 *
 * Description:
 * This function initiates the reception of data over UART using interrupt mode.
 * It sets up the UART handle to receive data into the provided buffer (`rxBuffer`)
 * with the specified buffer size (`RX_BUFFER_SIZE`). The function leverages the
 * HAL_UART_Receive_IT function to enable interrupt-based data reception.
 */
void UART_StartReceive_HF_Receiver(void)
{
	HAL_UART_Receive_IT(&HF_RECEIVER_COM, (uint8_t*) &rxByte_HF_REC, 1);
}

void UART_StartTransmittion_CDU(uint8_t *cdu_tx_buffer)
{
	HAL_UART_Transmit_IT(&CDU_COM, cdu_tx_buffer,
			strlen((char*) cdu_tx_buffer));
}

