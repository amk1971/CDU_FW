#include <main.h>
#include "isr_routines.h"

#include <string.h>

#include "rotary_encoder.h"
#include "serial_handler.h"
#include "usart.h"

extern s_NAV_RCU_Parameters nav_parameters;
// Buffers for UART data
volatile uint8_t rxByte_CDU = 0;
// volatile uint8_t rxIndex_CDU = 0;
volatile uint8_t rxByte_NAV_REC = 0;
// volatile uint8_t rxIndex_NAV_REC = 0;

// Byte variables for received data

// Indices for buffer management

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == LEFT_A2_Pin)
    {  // LEFT 2
    }
    else if (GPIO_Pin == LEFT_B2_Pin)
    {  // LEFT 2
    }
    else if (GPIO_Pin == LEFT_A1_Pin)
    {
        if (!nav_parameters.mem_page)
        {
            read_encoder_volume();
        }
        else
        {
            scroll_freqs_memory();
        }
    }
    else if (GPIO_Pin == LEFT_B1_Pin)
    {  // LEFT 1
        if (!nav_parameters.mem_page)
        {
            read_encoder_volume();
        }
        else
        {
            scroll_freqs_memory();
        }
    }
    else if (GPIO_Pin == RIGHT_A1_Pin)
    {  // RIGHT 1

        if (!nav_parameters.mem_page)
        {
            read_encoder_standby_khz();
        }
        else
        {
            change_saved_freq_khz();
        }
    }
    else if (GPIO_Pin == RIGHT_B1_Pin)
    {  // RIGHT 1
        if (!nav_parameters.mem_page)
        {
            read_encoder_standby_khz();
        }
        else
        {
            change_saved_freq_khz();
        }
    }
    else if (GPIO_Pin == RIGHT_A2_Pin)
    {  // RIGHT 2

        if (!nav_parameters.mem_page)
        {
            read_encoder_standby_mhz();
        }
        else
        {
            change_saved_freq_mhz();
        }
    }
    else if (GPIO_Pin == RIGHT_B2_Pin)
    {  // RIGHT 2
        if (!nav_parameters.mem_page)
        {
            read_encoder_standby_mhz();
        }
        else
        {
            change_saved_freq_mhz();
        }
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
        if (rxIndex_CDU_ISR >= 2 && rxBuffer_CDU_ISR[rxIndex_CDU_ISR - 2] == '\r' &&
            rxBuffer_CDU_ISR[rxIndex_CDU_ISR - 1] == '\n')
        {
            // Send the buffer from the ISR to the task via the queue
            if (xCDUCOMRxQueue != NULL)
            {
                xQueueSendFromISR(xCDUCOMRxQueue, rxBuffer_CDU_ISR, &xHigherPriorityTaskWoken);
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

    if (huart->Instance == NAV_RECEIVER_UART)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // Use a local buffer for storing data in ISR
        static uint8_t rxBuffer_NAV_REC_ISR[RX_BUFFER_SIZE];
        static uint8_t rxIndex_NAV_REC_ISR = 0;

        // Store the received byte into the local buffer
        rxBuffer_NAV_REC_ISR[rxIndex_NAV_REC_ISR++] = rxByte_NAV_REC;

        // Check for "\r\n" in the last two received characters
        if (rxIndex_NAV_REC_ISR >= 2 && rxBuffer_NAV_REC_ISR[rxIndex_NAV_REC_ISR - 2] == '\r' &&
            rxBuffer_NAV_REC_ISR[rxIndex_NAV_REC_ISR - 1] == '\n')
        {
            // Send the buffer to the queue for processing in the task
            if (xNAVRecRxQueue != NULL)
            {
                xQueueSendFromISR(xNAVRecRxQueue, rxBuffer_NAV_REC_ISR, &xHigherPriorityTaskWoken);
                memset(rxBuffer_NAV_REC_ISR, 0, RX_BUFFER_SIZE);
            }

            // Reset the index for the next message
            rxIndex_NAV_REC_ISR = 0;
        }
        else if (rxIndex_NAV_REC_ISR >= RX_BUFFER_SIZE)
        {
            // Reset index if the buffer is full without finding "\r\n"
            memset(rxBuffer_NAV_REC_ISR, 0, RX_BUFFER_SIZE);
            rxIndex_NAV_REC_ISR = 0;
        }

        // Yield if a higher-priority task was woken
        if (xHigherPriorityTaskWoken == pdTRUE)
        {
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        // Re-enable UART reception for the next byte
        UART_StartReceive_NAV_Receiver();
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
void UART_StartReceive_CDU(void) { HAL_UART_Receive_IT(&CDU_COM, (uint8_t *)&rxByte_CDU, 1); }

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
 * Function: UART_StartReceive_NAV_Receiver
 * Arguments: None
 *
 * Description:
 * This function initiates the reception of data over UART using interrupt mode.
 * It sets up the UART handle to receive data into the provided buffer (`rxBuffer`)
 * with the specified buffer size (`RX_BUFFER_SIZE`). The function leverages the
 * HAL_UART_Receive_IT function to enable interrupt-based data reception.
 */
void UART_StartReceive_NAV_Receiver(void) { HAL_UART_Receive_IT(&NAV_RECEIVER_COM, (uint8_t *)&rxByte_NAV_REC, 1); }

void UART_StartTransmittion_CDU(uint8_t *cdu_tx_buffer)
{
    HAL_UART_Transmit_IT(&CDU_COM, cdu_tx_buffer, strlen((char *)cdu_tx_buffer));
}
