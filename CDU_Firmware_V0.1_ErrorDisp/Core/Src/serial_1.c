/*
 * serial_1.c
 *
 *  Created on: Oct 8, 2024
 *      Author: Hamza Javed
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "serial_1.h"
#include "main.h"
#include "debug_console.h"
#include "usart.h"
#include "data_handler.h"
#include "tacan_map.h"

/* ------------------------------ MACRO DEGIN -------------------------------------*/
#define RX_BUFFER_SIZE 26
#define TX_BUFFER_SIZE 26

/* ------------------------------ MACRO END -------------------------------------*/


/* ------------------------------ Variables START -------------------------------------*/
uint8_t rxBuffer[RX_BUFFER_SIZE];  // Buffer to store received data // +1 for NULL
uint8_t rxByte = 0;
uint8_t rxIndex = 0;
uint8_t txBuffer[TX_BUFFER_SIZE];
uint8_t check[8] = "Start\r\n";
/* ------------------------------ Variables END -------------------------------------*/


/* ------------------------------ Function Declarations START -------------------------------------*/
void UART_StartReceive(void);
void UART_StartTransmittion(void);

/* ------------------------------ Function Declarations END -------------------------------------*/

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
void serial_1553_thread(void *pvParameters)
{
	Identifier ident;
	uint8_t size = sizeof(Identifier);
	memset(&ident, 0, size);
	memset(rxBuffer, 0, RX_BUFFER_SIZE);
//	HAL_UART_Transmit(&huart4, check, strlen((char *)check), HAL_MAX_DELAY);
	UART_StartReceive();
	for(;;)
	{
	    // Handle received data (RX)
	    if (xuartRXQueue != NULL) {
	        uint8_t receivedData[RX_BUFFER_SIZE];
	        if (xQueueReceive(xuartRXQueue, &receivedData, (TickType_t)5) == pdTRUE)
	        {
	            if (xSemaphoreTake(xFlashMutex, (TickType_t)10) == pdTRUE)
	            {
	                decode_receive_data(receivedData);
	                memset(receivedData, 0, RX_BUFFER_SIZE);
	                xSemaphoreGive(xFlashMutex);
	            }
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
        if(xuartTXQueue != NULL)
        {
            if(xQueueReceive(xuartTXQueue, &ident, (TickType_t)10) == pdTRUE)
            {
            	memset(txBuffer, 0, TX_BUFFER_SIZE);
            	encode_message(txBuffer, ident.class, ident.msg_id, ident.mhz , ident.khz,  ident.freq_flag); // Encode data for transmission
//            	txBuffer[TX_BUFFER_SIZE - 1] = '\0';
            	if(strlen((char *)txBuffer) < TX_BUFFER_SIZE)
            	{
            		HAL_UART_Transmit_IT(&huart4, txBuffer, strlen((char *)txBuffer));
            	}
                // Send data over UART here (HAL_UART_Transmit or another method)
            }
        }
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

/*
 * Function: UART_StartReceive
 * Arguments: None
 *
 * Description:
 * This function initiates the reception of data over UART using interrupt mode.
 * It sets up the UART handle to receive data into the provided buffer (`rxBuffer`)
 * with the specified buffer size (`RX_BUFFER_SIZE`). The function leverages the
 * HAL_UART_Receive_IT function to enable interrupt-based data reception.
 */
void UART_StartReceive(void)
{
    HAL_UART_Receive_IT(&huart4, &rxByte, 1);
}

/*
 * Function: HAL_UART_RxCpltCallback
 * Arguments: UART_HandleTypeDef *huart
 *
 * Description:
 * This function is the UART receive complete callback function. It is called
 * by the HAL library when a UART receive operation completes. The function
 * signals the RX task to process the received data by giving the RX semaphore
 * from an ISR context. If a higher priority task is woken by the semaphore,
 * it yields the processor to that task. Optionally, it can restart UART
 * reception for new data.
 *
 * - huart: UART handle pointer for the UART instance that triggered the callback.
 */
/*void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART4)
    {
        // Store the received byte into the buffer
        rxBuffer[rxIndex++] = rxByte;

        // Check for "\r\n" in the last two received characters
        if (rxIndex >= 2 && rxBuffer[rxIndex - 2] == '\r' && rxBuffer[rxIndex - 1] == '\n') {
            // Termination detected, release the semaphore and process the message
            xSemaphoreGiveFromISR(uartRXSemaphoreHandle, NULL);

            // Optionally, reset the index for the next message
            rxIndex = 0;
        }
        else if (rxIndex >= RX_BUFFER_SIZE)
        {
            // Reset index if the buffer is full without finding \r\n
            rxIndex = 0;
        }
        else
        {
        	UART_StartReceive();
        }
    }
}*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART4)
    {
        // Store the received byte into the buffer
        rxBuffer[rxIndex++] = rxByte;

        // Check for "\r\n" in the last two received characters
        if (rxIndex >= 2 && rxBuffer[rxIndex - 2] == '\r' && rxBuffer[rxIndex - 1] == '\n') {
            // Null-terminate the message
            rxBuffer[rxIndex] = '\0';

            // Enqueue the complete message
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (xQueueSendFromISR(xuartRXQueue, rxBuffer, &xHigherPriorityTaskWoken) != pdPASS)
            {
                // Handle queue full error
            }

            // Reset index for the next message
            rxIndex = 0;

            // Yield if a higher-priority task was woken
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else if (rxIndex > RX_BUFFER_SIZE)
        {
            // Reset index if the buffer is full without finding \r\n
            rxIndex = 0;
        }
		UART_StartReceive();
    }
}

/*
 * Function: UART_StartTransmittion
 * Arguments: None
 *
 * Description:
 * This function initiates the transmission of data over UART using interrupt mode.
 * It sets up the UART handle to transmit data from the provided buffer (`txBuffer`)
 * with the size of the buffer being transmitted. The function leverages the
 * HAL_UART_Transmit_IT function to enable interrupt-based data transmission.
 */
void UART_StartTransmittion(void)
{
	HAL_UART_Transmit_IT(&huart4, txBuffer, sizeof(txBuffer));
}

/*
 * Function: HAL_UART_TxCpltCallback
 * Arguments: UART_HandleTypeDef *huart
 *
 * Description:
 * This function is the UART transmit complete callback function. It is called
 * by the HAL library when a UART transmit operation completes. The function can
 * be used to notify a task or perform other actions upon transmission completion.
 *
 * - huart: UART handle pointer for the UART instance that triggered the callback.
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART4) {
        // Optionally, you can notify a task or take other actions
//    	debug_print("send data to 1553\r\n");
    }
}

/*
 * Function: split_frequency
 * Arguments: float frequency, int *mhz, int *khz
 *
 * Description:
 * This function splits a given frequency into its integer (MHz) and fractional (kHz) parts.
 * The integer part represents the MHz, and the fractional part (multiplied by 100) represents
 * the kHz. The results are stored in the provided pointers for MHz and kHz.
 *
 * - frequency: The input frequency to be split.
 * - mhz: Pointer to an integer where the MHz part will be stored.
 * - khz: Pointer to an integer where the kHz part will be stored.
 */
void split_frequency(float frequency, uint8_t *mhz, uint16_t *khz)
{
    *mhz = (uint8_t)frequency;              // Integer part (MHz)
    *khz = (uint16_t)(((frequency - *mhz) + 0.001) * 1000);  // Fractional part (kHz)
}

/*
 * Function: send_to_uart_queue
 * Arguments: uint8_t class_id, uint8_t msg_id, float frequency
 *
 * Description:
 * This function constructs an Identifier structure with the given class ID, message ID,
 * and frequency. It splits the frequency into MHz and kHz components if the frequency
 * is greater than 0. The function then sends the Identifier structure to the UART TX queue.
 * If sending to the queue fails, an error message is printed if debugging is enabled.
 *
 * - class_id: The class ID for the identifier.
 * - msg_id: The message ID for the identifier.
 * - frequency: The frequency to be split into MHz and kHz.
 */
void send_to_uart_queue_freq(uint8_t class_id, uint8_t msg_id, float frequency)
{
    Identifier identifier;
    // Split the frequency into MHz and kHz
    if(frequency > 0.0)
    {
    	split_frequency(frequency, &identifier.mhz, &identifier.khz);
    	identifier.freq_flag = 'Y';
    }
    else
    {
    	identifier.freq_flag = 'N';
    }

    identifier.class = (Class_Id)class_id;
    identifier.msg_id = (Message_Id)msg_id;

    // Send the identifier structure to the queue
    if(xuartTXQueue != NULL)
    {
        // Successfully added to the queue
    	if(xQueueSend(xuartTXQueue, &identifier, (TickType_t)10) != pdPASS)
    	{
#if DEBUG_CONSOLE
    		debug_print("----Error: Failed to send Freq to queue. xuartTXQueue \r\n");
#endif
    	}
    }
}

void send_to_uart_queue_channel(uint8_t class_id, uint8_t msg_id, const char *key)
{
    Identifier identifier;
    float frequency = 0.0;
    // Split the frequency into MHz and kHz
    if(key != NULL)
    {
    	frequency = find_value(key);
    	if(frequency > 0)
    	{
			split_frequency(frequency, &identifier.mhz, &identifier.khz);
			identifier.freq_flag = 'Y';
    	}
    	else
    	{
    		return;
    	}
    }
    else
    {
    	identifier.freq_flag = 'N';
    }

    identifier.class = (Class_Id)class_id;
    identifier.msg_id = (Message_Id)msg_id;

    // Send the identifier structure to the queue
    if(xuartTXQueue != NULL)
    {
        // Successfully added to the queue
    	if(xQueueSend(xuartTXQueue, &identifier, (TickType_t)10) != pdPASS)
    	{
#if DEBUG_CONSOLE
    		debug_print("----Error: Failed to send Channel to queue. xuartTXQueue \r\n");
#endif
    	}
    }
}

