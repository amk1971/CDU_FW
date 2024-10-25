/*
 * switches.c
 *
 *  Created on: Oct 21, 2024
 *      Author: Dell
 */

#include "switches.h"
#include "main.h"

void soft_keysTest(void)
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

void Matrix_keypad_test(void)
{
	uint8_t colArray[8] = {0};
	uint8_t pressedKey[8][8] = {0};
	for(;;)
	{
		for(int r = 0; r < 8; r++)
		{
			// Set all rows high except the current row
			HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, (r == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, (r == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, (r == 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, (r == 3) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW5_GPIO_Port, ROW5_Pin, (r == 4) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW6_GPIO_Port, ROW6_Pin, (r == 5) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW7_GPIO_Port, ROW7_Pin, (r == 6) ? GPIO_PIN_RESET : GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW8_GPIO_Port, ROW8_Pin, (r == 7) ? GPIO_PIN_RESET : GPIO_PIN_SET);

			for(int c = 0; c < 8; c++)
			{
				GPIO_TypeDef* columnPort;
				uint16_t columnPin;

				switch(c)
				{
					case 0: columnPort = COL1_GPIO_Port; columnPin = COL1_Pin; break;
					case 1: columnPort = COL2_GPIO_Port; columnPin = COL2_Pin; break;
					case 2: columnPort = COL3_GPIO_Port; columnPin = COL3_Pin; break;
					case 3: columnPort = COL4_GPIO_Port; columnPin = COL4_Pin; break;
					case 4: columnPort = COL5_GPIO_Port; columnPin = COL5_Pin; break;
					case 5: columnPort = COL6_GPIO_Port; columnPin = COL6_Pin; break;
					case 6: columnPort = COL7_GPIO_Port; columnPin = COL7_Pin; break;
					case 7: columnPort = COL8_GPIO_Port; columnPin = COL8_Pin; break;
					default: break;
				}

				if(!(HAL_GPIO_ReadPin(columnPort, columnPin)))
				{
					colArray[r] |= (1 << c);  // Mark the column bit in the colArray

					 pressedKey[r][c] = 1;
				}
				else
				{
					colArray[r] &= ~(1 << c); // Clear the bit if no button press

					 pressedKey[r][c] = 0;
				}
			}
		}
	}
}


void test_sw(void)
{
	soft_keysTest();

}
