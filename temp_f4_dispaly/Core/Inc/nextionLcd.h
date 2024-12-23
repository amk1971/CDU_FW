/*
 * nextionLcd.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#ifndef INC_NEXTIONLCD_H_
#define INC_NEXTIONLCD_H_

#include <stdio.h>
#include "common.h"

#define BLACKFONT     0
#define WHITEFONT     65535
#define REDFONT	      63488

#define BLACKBG       0
#define TRANSPARENTBG 5693 //THIS COLOR WILL BE USE TO BE SAME AS THE BACKGROUND, TBD

typedef enum
{
	lcdDisp_home = 0,
	lcdDisp_nav,
	lcdDisp_com,
	lcdDisp_tacan,
	lcdDisp_adf

}lcdCmdDisp_id;

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
	Center5,
}lcdCmdParam_id;

returnStatus InitializeLCD(void);
returnStatus DispTACANscreen(void);
returnStatus DispNAVscreen(NavParams *);
returnStatus DispHomeScreen(void);
//returnStatus ChangePage(lcdCmdPage_id Page_ID);
returnStatus UpdateParamLCD(lcdCmdParam_id Param_ID, char * Parm_Vlaue);
returnStatus ReadParamLCD(lcdCmdParam_id Param_ID, void * Param_Value);

returnStatus configBgcolorLCD(lcdCmdParam_id Param_ID, int Param_Value);
returnStatus configfontcolorLCD(lcdCmdParam_id Param_ID, int Param_Value);

#endif /* INC_NEXTIONLCD_H_ */
