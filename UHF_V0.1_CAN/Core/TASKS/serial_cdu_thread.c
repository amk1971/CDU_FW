/*
 * serial_cdu_thread.c
 *
 *  Created on: Jan 27, 2025
 *      Author: Admin
 */

#include <stdint.h>
#include <string.h>

#include "..\ISR\isr_routines.h"
#include "data_handler.h"
#include "decode_HF.h"
#include "serial_handler.h"

extern CAN_HandleTypeDef hcan;

void serial_CDU_com_thread(void *pvParameters)
{
    Identifier ident;
    uint8_t receive_buff[RX_BUFFER_SIZE];
    static uint8_t txBuffer_CDU[TX_BUFFER_SIZE];
    memset(receive_buff, 0, RX_BUFFER_SIZE);
    memset(txBuffer_CDU, 0, TX_BUFFER_SIZE);
    //UART_StartReceive_CDU();
    HAL_CAN_Start(&hcan);
    for (;;)
    {
        if (xQueueReceive(xCDUCOMRxQueue, receive_buff, (TickType_t)10) == pdTRUE)
        {
#if DEBUG_CONSOLE
            debug_print("CDU_COM Queue_Recv :: %s\n", receive_buff);
#endif
            // decode and queue the received data

            decode_receive_data(receive_buff);
            memset(receive_buff, 0, RX_BUFFER_SIZE);
            UART_StartReceive_CDU();
        }

        if (xQueueReceive(xCDUCOMTxQueue, &ident, (TickType_t)10) == pdTRUE)
        {
            memset(txBuffer_CDU, 0, TX_BUFFER_SIZE);
            encode_message(txBuffer_CDU, ident.ID, ident.mhz, ident.khz, ident.freq_flag,
                           ident.volume);  // Encode data for transmission

#if DEBUG_CONSOLE
            debug_print("CDU_COM Tx_BUFF :: %s\n", txBuffer_CDU);
#endif
            if (strlen((char *)txBuffer_CDU) < TX_BUFFER_SIZE)
            {
                UART_StartTransmittion_CDU(txBuffer_CDU);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
