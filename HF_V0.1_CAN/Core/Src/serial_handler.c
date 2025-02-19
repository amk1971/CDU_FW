/*
 * serial_handler.c
 *
 *  Created on: Nov 29, 2024
 *      Author: Hamza Javed
 */

/* ------------------------------ INCLUDES ------------------------------ */
#include "serial_handler.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "..\ISR\isr_routines.h"
#include "..\HF\encode_HF.h"
#include "..\HF\HF_funcs.h"
#include "FreeRTOS.h"
#include "data_handler.h"
#include "debug_console.h"
#include "decode_HF.h"
#include "main.h"
#include "task.h"
#include "usart.h"

/* ------------------------------- HF Receiver Communication --------------------------- */

void serial_HF_Receiver_com_thread(void *pvParameters)
{
    s_HF_Parameters HF_rcu_param;
    Identifier ident;
    uint8_t receive_buff_HF[RX_BUFFER_SIZE];
    char tx_buffer[TX_BUFFER_SIZE_INTERFACE];
    memset(receive_buff_HF, 0, RX_BUFFER_SIZE);
    UART_StartReceive_HF_Receiver();
    //	HAL_UART_Transmit(&HF_RECEIVER_COM, comstart, strlen(comstart), 1000);
    for (;;)
    {

        if (xQueueReceive(xHFRecRxQueue, receive_buff_HF, (TickType_t)10) == pdTRUE)
        {
#if DEBUG_CONSOLE
            debug_print("HF_Receiver_COM Queue_Recv :: %s\n", receive_buff_HF);
#endif

            bool decode_success = true;  // decode_message_HF(receive_buff_HF, &mesg);

            if ((decode_success) && (xMainNotifyQueue != NULL))
            {
                if (xQueueSend(xMainNotifyQueue, &ident, (TickType_t)10) != pdPASS)
                {
#if DEBUG_CONSOLE
                    debug_print("----Error: Failed to send Channel to queue. xMainNotifyQueue \r\n");
#endif
                }
            }
            memset(receive_buff_HF, 0, RX_BUFFER_SIZE);
            UART_StartReceive_HF_Receiver();  // Handle the Data received from HF Receiver and queue the ident
                                               // structure only LOCK needs to be updated.
        }

        if (xQueueReceive(xHFRecTxQueue, &HF_rcu_param, (TickType_t)10) == pdTRUE)
        {
#if DEBUG_CONSOLE
            debug_print("receive to transmit\n");
#endif
            encode_message_for_interface(&HF_rcu_param, tx_buffer);
            HAL_UART_Transmit_IT(&HF_RECEIVER_COM, (uint8_t*)&tx_buffer, strlen(tx_buffer));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
