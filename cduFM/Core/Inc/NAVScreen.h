/*
 * NAVScreen.h
 *
 *  Created on: Jan 2, 2025
 *      Author: Dell
 */

#ifndef INC_NAVSCREEN_H_
#define INC_NAVSCREEN_H_

#include "switches.h"
#include "main.h"


typedef enum {
	nActiveFreq = 0,
	nStandbyFreq,
}NavParamNumber;

returnStatus DispNAVscreen(ScreenParams * Params);
uint16_t NavScreenStateMachine(ScreenParams * Params, softKey_t softkey);

#endif /* INC_NAVSCREEN_H_ */
