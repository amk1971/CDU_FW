/*
 * nextionLcd.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <stdbool.h>

typedef enum
{
	failure = 0,
	success,
}returnStatus;

typedef struct NAVPARAMS {
	double Active, Standby, P1, P2, P3, P4, P5, P6, P7, P8;
	bool page;
} NavParams;


#endif /* INC_NEXTIONLCD_H_ */
