/*
 * ADFScreen.h
 *
 *  Created on: Jan 2, 2025
 *      Author: Dell
 */

#ifndef INC_ADFSCREEN_H_
#define INC_ADFSCREEN_H_

#include "switches.h"
#include "main.h"


typedef enum {
	aActiveFreq = 0,
	aStandbyFreq,
}AdfParamNumber;

returnStatus DispADFscreen(ScreenParams * Params);
uint16_t AdfScreenStateMachine(ScreenParams * Params, softKey_t softkey);


#endif /* INC_ADFSCREEN_H_ */
