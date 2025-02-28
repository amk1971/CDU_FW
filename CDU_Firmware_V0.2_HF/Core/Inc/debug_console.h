/*
 * debug_console.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_DEBUG_CONSOLE_H_
#define INC_DEBUG_CONSOLE_H_
#include "FreeRTOS.h"
#include "queue.h"
#include "stdarg.h"

#define DEBUG_ENABLE	1
#define DEBUG_DISABLE	0

#define DEBUG_CONSOLE   DEBUG_DISABLE

#if DEBUG_CONSOLE


extern uint8_t Byte;

extern xQueueHandle xDebugQueue;
extern xQueueHandle xDebugRQueue;


extern void debug_console_thread(void * pvParameter);

#endif /* DEBUG Console Enable*/

extern void debug_print(const char *format, ...);

#endif /* INC_DEBUG_CONSOLE_H_ */
