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
	float Active, Standby;
	bool page;
} NavParams;


#endif /* INC_NEXTIONLCD_H_ */
