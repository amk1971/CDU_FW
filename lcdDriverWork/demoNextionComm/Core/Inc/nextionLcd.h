/*
 * nextionLcd.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#ifndef INC_NEXTIONLCD_H_
#define INC_NEXTIONLCD_H_

#include "common.h"

typedef enum
{
	lcdPage_main = 0,
	lcdPage_nav,
	lcdPage_com,
	lcdPage_tacan,
	lcdPage_adf
}lcdCmdPage_id;

typedef enum
{
	param_number = 0,
	param_float,
	param_txt
}lcdCmdParam_id;

returnStatus InitializeLCD(void);
returnStatus ChangePage(lcdCmdPage_id Page_ID);
returnStatus UpdateParamLCD(lcdCmdParam_id Param_ID, void * Parm_Vlaue);
returnStatus ReadParamLCD(lcdCmdParam_id Param_ID, void * Param_Value);

#endif /* INC_NEXTIONLCD_H_ */
