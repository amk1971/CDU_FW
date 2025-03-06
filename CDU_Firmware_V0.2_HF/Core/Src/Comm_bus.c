/*
 * TFT_LCD.c
 *
 *  Created on: Mar 6, 2025
 *      Author: Abdul Malik Khan
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"


#include "data_handler.h"
#include "data_validation.h"
#include "debug_console.h"
#include "Comm_bus.h"
#include "keyboard.h"
#include "main.h"
#include "can.h"



extern CAN_HandleTypeDef hcan;
extern xQueueHandle xcanRXQueue;

/*
 * Function: serial_1553_thread
 * Arguments: void *pvParameters
 *
 * Description:
 * This function handles the communication thread for a serial 1553 interface.
 * It continuously receives and transmits data over UART, using a semaphore to
 * manage RX data and a queue to manage TX data. The function includes a delay
 * to allow other tasks to run and ensures proper handling of message encoding
 * and transmission over UART4.
 *
 * - pvParameters: Pointer to any parameters passed to the thread function.
 */
void Comm_bus_thread(void *pvParameters)
{
	uint32_t Request_Time;
	FRAME f;
	Identifier ident;
	uint8_t size = sizeof(Identifier);
	memset(&ident, 0, size);

	  CAN_Filter_Config();

	  if(HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING)!=HAL_OK)
	  {
		  Error_Handler();
	  }

	  if(HAL_CAN_Start(&hcan)!=HAL_OK)
	  {
		  Error_Handler();
	  }

	  Request_Time = HAL_GetTick();
	for(;;)
	{
		if((HAL_GetTick() - Request_Time) > 2000){
			f.data.eID = UHF;
			f.data.mID = Health;
			CAN_SEND_FRAME(&f);
		}
	    // Handle received data (RX)
	    if (xcanRXQueue != NULL) {
	        uint8_t receivedData[10];
	        if (xQueueReceive(xcanRXQueue, &receivedData, (TickType_t)5) == pdTRUE)
	        {
	        	cdu_parameters.UHF_message_counter = 0;		//Message Received
	        }
	    }
//		if(uartRXSemaphoreHandle != NULL)
//		{
//	        // Handle receive data (RX)
//			if(xSemaphoreTake(uartRXSemaphoreHandle, (TickType_t ) 10) == pdTRUE)
//			{
//				if(xSemaphoreTake(xFlashMutex, (TickType_t) 10) == pdTRUE)
//				{
//					decode_receive_data(rxBuffer);
//					memset(rxBuffer, 0, RX_BUFFER_SIZE);
//					UART_StartReceive();
//					xSemaphoreGive(xFlashMutex);
//				}
//			}
//		}
        // Handle transmitted data (TX)
 //       if(xuartTXQueue != NULL)
        {
//            if(xQueueReceive(xuartTXQueue, &ident, (TickType_t)10) == pdTRUE)
            {
//            	memset(txBuffer, 0, TX_BUFFER_SIZE);
//            	encode_message(txBuffer, ident.class, ident.msg_id, ident.mhz , ident.khz, ident.vol, ident.mode, ident.freq_flag); // Encode data for transmission
//            	txBuffer[TX_BUFFER_SIZE - 1] = '\0';
//            	if(strlen((char *)txBuffer) < TX_BUFFER_SIZE)
            	{
//            		HAL_UART_Transmit_IT(&huart4, txBuffer, strlen((char *)txBuffer));
            	}
                // Send data over UART here (HAL_UART_Transmit or another method)
            }
        }
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
