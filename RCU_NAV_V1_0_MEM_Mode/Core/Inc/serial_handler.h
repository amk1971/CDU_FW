/*
 * serial_handler.h
 *
 *  Created on: Nov 29, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_SERIAL_HANDLER_H_
#define INC_SERIAL_HANDLER_H_

#include <stdbool.h>

#include "FreeRTOS.h"
#include "data_handler.h"
#include "queue.h"
#include "usart.h"
#include  "constant_defines.h"

#define CDU_UART UART5
#define NAV_RECEIVER_UART UART4

#define CDU_COM huart5
#define NAV_RECEIVER_COM huart4



//#define RCU_ID 'A'  // A for NAV
typedef enum
{
    S_FREQ,
    A_FREQ,
    VOLUME,
	Health,
    INVALID
} Message_ID;

typedef struct
{
    uint8_t mhz;
    uint16_t khz;
    Message_ID ID;
    B_Status lock_s;
    bool freq_flag;
    B_Status BFO;
    B_Status NAV;
    uint8_t volume;

} Identifier;

extern xQueueHandle xCDUCOMRxQueue;
extern xQueueHandle xCDUCOMTxQueue;

extern xQueueHandle xNAVRecRxQueue;
extern xQueueHandle xNAVRecTxQueue;

void serial_CDU_com_thread(void *pvParameters);
void serial_NAV_Receiver_com_thread(void *pvParameters);

#endif /* INC_SERIAL_HANDLER_H_ */
