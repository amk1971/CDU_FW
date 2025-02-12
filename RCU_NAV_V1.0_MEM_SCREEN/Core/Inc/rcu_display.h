/*
 * rcu_display.h
 *
 *  Created on: Nov 28, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_RCU_DISPLAY_H_
#define INC_RCU_DISPLAY_H_

#include "glcd.h"
#include "stdio.h"
#include "stm32f1xx.h"

#define TF_X1		5
#define TF_X2		50
#define TF_Y1		5
#define TF_Y2		15

#define SQ_X1		5
#define SQ_X2		25
#define SQ_Y1		25
#define SQ_Y2		35

#define PROG_X1		5
#define PROG_X2		40
#define PROG_Y1		45
#define PROG_Y2		55

#define T_GD_MV_X1	55
#define T_GD_MV_X2	90
#define T_GD_MV_Y1	5
#define T_GD_MV_Y2	15

#define VOL_X1		55
#define VOL_X2		85
#define VOL_Y1		45
#define VOL_Y2		55

#define FREQ_CH_X1	100
#define FREQ_CH_X2	125
#define FREQ_CH_Y1	5
#define FREQ_CH_Y2	18

#define TST_X1		100
#define TST_X2		125
#define TST_Y1		25
#define TST_Y2		35

#define MN_BOTH_X1	100
#define MN_BOTH_X2	125
#define MN_BOTH_Y1	45
#define MN_BOTH_Y2	55

/*-----------------------------------------------------------------------------------------------
                                  **advance glcd function**
------------------------------------------------------------------------------------------------*/

void GLCD_INIT(void);
void glcd_axis(void);
void glcd_plot(float number, char step);
void GLCD_INIT(void);
void line(int x1, int x2, int y);
void LCD_power_on(void);
void LCD_power_off(void);
void LCD_Print_Home(void);
void LCD_PRINT_MEM_SCREEN(uint8_t page);

void LCD_Print_Dummy(uint8_t UPDATE_FLAGS,uint8_t sValue);
void update_following(uint8_t UPDATE_FLAGS, char* sValue);
char* convert_to_str(uint8_t value);

typedef enum
{
	TF,
	SQ,
	PROG,
	T_GD_MV,
	FREQ_CH,
	TST,
	VOL,
	MN_BOTH,
}Update_flags_data;

#endif /* INC_RCU_DISPLAY_H_ */
