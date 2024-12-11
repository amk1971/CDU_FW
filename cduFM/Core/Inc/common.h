/*
 * nextionLcd.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <stdbool.h>
#include "stdint.h"
//typedef uint8_t unsigned char


typedef enum
{
	failure = 0,
	success,
}returnStatus;

typedef struct NAVPARAMS {
	volatile double Active, Standby, P1, P2, P3, P4, P5, P6, P7, P8;
	volatile bool page;
	volatile int MHz, KHz, mhz, khz; // caps for active, small for standby
} NavParams;


#define MAXBUFLEN	2048			// UART receive buffer size. Must be a multiple of 2
#define BUFLENMASK	MAXBUFLEN-1
// UART data struct.
// Contains buffers and pointers for non-blocking access to UART
typedef struct
{
	uint8_t RXbuffer[MAXBUFLEN];	// receive buffer
	uint8_t TXbuffer[MAXBUFLEN];	// transmit buffer
	uint16_t RXindex;				// pointer to last received char in RXbuffer
	uint16_t TXindex;				// pointer to last transmitted char in TXbuffer
	uint16_t LoadIndex;				// pointer to last written char to TXbuffer
	uint16_t TXbuflen;				// TX buffer size
	uint8_t TXcomplete;				// completed TX flag
} SerialStruct;



#endif /* INC_NEXTIONLCD_H_ */
