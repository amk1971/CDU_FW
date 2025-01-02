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
#include "stdlib.h"
#include "string.h"
//#include ".h"
//typedef uint8_t unsigned char


typedef enum
{
	failure = 0,
	success,
	reactTimeout,
	homeButtonPress,
	updatedFrequency,
	oldFrequency
}returnStatus;

typedef enum
{
	Idle = 0,
	SwapKey,
	page0,
	page1,
	RightSoftKey,
	StandByEdit
} SCREENState;

typedef enum
{
	Left1 = 0,
	Left2,
	Left3,
	Left4,
	Right1,
	Right2,
	Right3,
	Right4,
	Center1,
	Center2,
	Center3,
	Center4,
	Center5
}lcdCmdParam_id;


typedef struct FREQUENCIES{
	double freq;
	volatile int MHz, KHz;
	int position;
	uint8_t tileSize;
	char name[10];
}Freq_t;

typedef struct NAVPARAMS {
	Freq_t Active, Standby, P1, P2, P3, P4, P5, P6, P7, P8;
	volatile bool page;
} ScreenParams;

#define reactDuration 30000			// 30s of reaction duration in editing frequencies
#define blinkDuration 500			// 1/2 s blkink time

#define MIN_FREQUENCY	108.00
#define MAX_FREQUENCY	117.95

#define MAXBUFLEN	1024			// UART receive buffer size. Must be a multiple of 2
#define BUFLENMASK	MAXBUFLEN-1
// UART data struct.
// Contains buffers and pointers for non-blocking access to UART
typedef struct
{
	uint8_t RXbuffer[MAXBUFLEN];	// receive buffer
	uint8_t TXbuffer[MAXBUFLEN];	// transmit buffer
	uint16_t RXindex;				// pointer to last received char in RXbuffer
	uint16_t RXTail;				// pointer to last fetched char in RXbuffer
	uint16_t TXindex;				// pointer to last transmitted char in TXbuffer
	uint16_t LoadIndex;				// pointer to last written char to TXbuffer
	uint16_t TXbuflen;				// TX buffer size
	uint8_t TXcomplete;				// completed TX flag
} SerialStruct;


void swapFreq(double *param1, double *param2);
uint8_t checkDot(char* arr);
returnStatus editFreq(Freq_t freq, char *lblText, lcdCmdParam_id pos, char * str, size_t strSize);
uint8_t checkFreqLimit(double editedfreq, double upLimit, double lowLimit);


#endif /* INC_NEXTIONLCD_H_ */
