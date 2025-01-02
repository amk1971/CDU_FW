/*
 * nextionLcd.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#ifndef INC_NEXTIONLCD_H_
#define INC_NEXTIONLCD_H_

//#include <stdio.h>
#include "main.h"


#define BLACKFONT     0
#define WHITEFONT     65535
#define REDFONT	      63488

#define BLACKBG       0
#define TRANSPARENTBG 5693 //THIS COLOR WILL BE USE TO BE SAME AS THE BACKGROUND, TBD

typedef enum
{
	lcdDisp_home = 0,
	lcdWaitForSW,
	lcdDisp_nav,
	lcdDisp_com,
	lcdDisp_tacan,
	lcdDisp_adf

}lcdCmdDisp_id;



returnStatus InitializeLCD(void);
returnStatus DispTACANscreen(void);
returnStatus DispHomeScreen(void);

returnStatus UpdateParamLCD(lcdCmdParam_id Param_ID, char * Parm_Vlaue);
returnStatus configBgcolorLCD(lcdCmdParam_id Param_ID, int Param_Value);
returnStatus configfontcolorLCD(lcdCmdParam_id Param_ID, int Param_Value);

#endif /* INC_NEXTIONLCD_H_ */
