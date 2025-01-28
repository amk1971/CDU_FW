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

#include "FreeRTOS.h"
#include "data_handler.h"
#include "debug_console.h"
#include "decode_nav.h"
#include "isr_routines.h"
#include "main.h"
#include "nav_funcs.h"
#include "task.h"
#include "usart.h"

void serial_CDU_com_thread(void *pvParameters)
{
    Identifier ident;
    uint8_t receive_buff[RX_BUFFER_SIZE];
    static uint8_t txBuffer_CDU[TX_BUFFER_SIZE];
    memset(receive_buff, 0, RX_BUFFER_SIZE);
    memset(txBuffer_CDU, 0, TX_BUFFER_SIZE);
    UART_StartReceive_CDU();
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

/* ------------------------------- NAV Receiver Communication --------------------------- */

void serial_NAV_Receiver_com_thread(void *pvParameters)
{
    s_NAV_RCU_Parameters NAV_rcu_param;
    Identifier ident;
    uint8_t receive_buff_NAV[RX_BUFFER_SIZE];
    memset(receive_buff_NAV, 0, RX_BUFFER_SIZE);
    UART_StartReceive_NAV_Receiver();
    //	HAL_UART_Transmit(&NAV_RECEIVER_COM, comstart, strlen(comstart), 1000);
    for (;;)
    {
        if (xQueueReceive(xNAVRecRxQueue, receive_buff_NAV, (TickType_t)10) == pdTRUE)
        {
#if DEBUG_CONSOLE
            debug_print("NAV_Receiver_COM Queue_Recv :: %s\n", receive_buff_NAV);
#endif

            bool decode_success = true;  // decode_message_NAV(receive_buff_NAV, &mesg);

            if ((decode_success) && (xMainNotifyQueue != NULL))
            {
                if (xQueueSend(xMainNotifyQueue, &ident, (TickType_t)10) != pdPASS)
                {
#if DEBUG_CONSOLE
                    debug_print("----Error: Failed to send Channel to queue. xMainNotifyQueue \r\n");
#endif
                }
            }
            memset(receive_buff_NAV, 0, RX_BUFFER_SIZE);
            UART_StartReceive_NAV_Receiver();  // Handle the Data received from NAV Receiver and queue the ident
                                               // structure only LOCK needs to be updated.
        }

        if (xQueueReceive(xNAVRecTxQueue, &NAV_rcu_param, (TickType_t)10) == pdTRUE)
        {
#if DEBUG_CONSOLE
            debug_print("receive to transmit\n");
#endif
//            t_value = get_23_value_to_transmit(NAV_rcu_param.active_freq, NAV_rcu_param.NAV_ant, NAV_rcu_param.bfo,
//                                               NAV_rcu_param.volume);
//            fill_tx_buf_send(t_value);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
