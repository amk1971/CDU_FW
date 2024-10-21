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
	for(;;)
	{
		if(!(HAL_GPIO_ReadPin(Right_SW1_GPIO_Port, Right_SW1_Pin)))
		{
			// i am here

		}

		if(!(HAL_GPIO_ReadPin(Right_SW2_GPIO_Port, Right_SW2_Pin)))
		{
			// i am here

		}
		if(!(HAL_GPIO_ReadPin(Right_SW3_GPIO_Port, Right_SW3_Pin)))
		{
			// i am here

		}

		if(!(HAL_GPIO_ReadPin(Right_SW4_GPIO_Port, Right_SW4_Pin)))
		{
			// i am here

		}

		if(!(HAL_GPIO_ReadPin(Left_SW1_GPIO_Port, Left_SW1_Pin)))
		{
			// i am here

		}

		if(!(HAL_GPIO_ReadPin(Left_SW2_GPIO_Port, Left_SW2_Pin)))
		{
			// i am here

		}
		if(!(HAL_GPIO_ReadPin(Left_SW3_GPIO_Port, Left_SW3_Pin)))
		{
			// i am here

		}

		if(!(HAL_GPIO_ReadPin(Left_SW4_GPIO_Port, Left_SW4_Pin)))
		{
			// i am here

		}

	}


}
