/*
 * switches.h
 *
 *  Created on: Oct 21, 2024
 *      Author: Dell
 */

#ifndef INC_SWITCHES_H_
#define INC_SWITCHES_H_

#include <stdint.h>



typedef struct
{
	uint8_t ColumnS;
	uint8_t RowS;
	uint16_t key;

}keyPad_t;

typedef enum
{
	Press,
	nPress,
	Hold,

}MkeyStatus_t;

typedef enum
{
	idle = 0,
	L1,
	L2,
	L3,
	L4,
	R1,
	R2,
	R3,
	R4,

}softKey_t;


MkeyStatus_t keyPad_Scan(void);

#endif /* INC_SWITCHES_H_ */
