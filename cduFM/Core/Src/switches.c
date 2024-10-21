/*
 * switches.c
 *
 *  Created on: Oct 21, 2024
 *      Author: Dell
 */

#include "switches.h"
#include "main.h"

void test_sw(void)
{
	static char  pval = 0, val = 0;
	for(;;)
	{
		if(!(HAL_GPIO_ReadPin(Right_SW1_GPIO_Port, Right_SW1_Pin)))
		{
			// i am here
			val = 1;
		}

		if(!(HAL_GPIO_ReadPin(Right_SW2_GPIO_Port, Right_SW2_Pin)))
		{
			// i am here
			val = 2;
		}
		if(!(HAL_GPIO_ReadPin(Right_SW3_GPIO_Port, Right_SW3_Pin)))
		{
			// i am here
			val = 3;
		}

		if(!(HAL_GPIO_ReadPin(Right_SW4_GPIO_Port, Right_SW4_Pin)))
		{
			// i am here
			val = 4;
		}

		if(!(HAL_GPIO_ReadPin(Left_SW1_GPIO_Port, Left_SW1_Pin)))
		{
			// i am here
			val = 5;
		}

		if(!(HAL_GPIO_ReadPin(Left_SW2_GPIO_Port, Left_SW2_Pin)))
		{
			// i am here
			val = 6;
		}
		if(!(HAL_GPIO_ReadPin(Left_SW3_GPIO_Port, Left_SW3_Pin)))
		{
			// i am here
			val = 7;
		}

		if(!(HAL_GPIO_ReadPin(Left_SW4_GPIO_Port, Left_SW4_Pin)))
		{
			// i am here
			val = 8;
		}

		if (pval != val) {
			pval = val;
		}

	}


}
