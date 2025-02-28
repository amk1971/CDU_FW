/*
 * serial_1.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_SERIAL_1_H_
#define INC_SERIAL_1_H_

#include "semphr.h"


//extern SemaphoreHandle_t uartRXSemaphoreHandle;
extern xQueueHandle xuartTXQueue;
extern xQueueHandle xuartRXQueue;


void serial_1553_thread(void *pvParameters);
extern void send_to_uart_queue_freq(uint8_t class_id, uint8_t msg_id, float frequency);
extern void send_to_uart_queue_channel(uint8_t class_id, uint8_t msg_id, const char *key);

#endif /* INC_SERIAL_1_H_ */
