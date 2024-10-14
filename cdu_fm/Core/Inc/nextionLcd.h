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
	nav_param_afreq = 0,
	nav_param_sfreq,
	nav_param_vol,
	nav_param_obs,
	com_param_afreq,
	com_param_sfreq,
	com_param_vol,
	com_param_sql,
	com_param_mic,
	com_param_stone,
}lcdCmdParam_id;

returnStatus InitializeLCD(void);
returnStatus ChangePage(lcdCmdPage_id Page_ID);
returnStatus UpdateParamLCD(lcdCmdParam_id Param_ID, void * Parm_Vlaue);
returnStatus ReadParamLCD(lcdCmdParam_id Param_ID, void * Param_Value);

#endif /* INC_NEXTIONLCD_H_ */
