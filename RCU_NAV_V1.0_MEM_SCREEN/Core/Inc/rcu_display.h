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

#endif /* INC_RCU_DISPLAY_H_ */
