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
#include "constant_defines.h"
#include "data_handler.h"
#include "queue.h"
#include "usart.h"


#define CDU_UART UART5
#define HF_RECEIVER_UART UART4

#define CDU_COM huart5
#define HF_RECEIVER_COM huart4

extern xQueueHandle xCDUCOMRxQueue;
extern xQueueHandle xCDUCOMTxQueue;

extern xQueueHandle xHFRecRxQueue;
extern xQueueHandle xHFRecTxQueue;

void serial_CDU_com_thread(void *pvParameters);
void serial_HF_Receiver_com_thread(void *pvParameters);

#endif /* INC_SERIAL_HANDLER_H_ */
