/*
 * nextionLcd.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#ifndef INC_NEXTIONLCD_H_
#define INC_NEXTIONLCD_H_

typedef enum
{
	lcdPage_main = "0",
	lcdPage_nav = "1",
	lcdPage_com = "2",
	lcdPage_tacan = "3",
	lcdPage_adf = "4",
}lcdCmdPage_id;

typedef enum
{
	param_number = "n0",
	param_float = "x0",
	param_txt = "t0",
}lcdCmdParam_id;

typedef enum
{
	failure = 0,
	success,
}returnStatus;

int InitializeLCD(void);
int ChangePage(int Page_ID);
int UpdateParamLCD(int Param_ID, void * Parm_Vlaue);
int ReadParamLCD( int Param_ID, void * Param_Value);

#endif /* INC_NEXTIONLCD_H_ */
