/*
 * switches.c
 *
 *  Created on: Oct 21, 2024
 *      Author: Dell
 */

#include "switches.h"
#include "main.h"

#define DEBOUNCE_DELAY 20

keyPad_t keyPad;

uint16_t Read_Port(GPIO_TypeDef *GPIOx)
{
    return (uint16_t)(GPIOx->IDR);  // Read and return the entire port input register
}

void KeyS_init()
{
	keyPad.ColumnS = 8;
	keyPad.RowS = 8;
	keyPad.key = 0;
}

uint16_t keyPad_Scan(void)
{
	uint16_t key = 0;

	for(uint8_t r = 0; r < keyPad.RowS; r++)
	{
//			for(uint8_t i = 0; r < KeyPad.RowS; i++)
//			{
//
//			}

		// Set all rows high except the current row
		HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, (r == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, (r == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, (r == 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, (r == 3) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW5_GPIO_Port, ROW5_Pin, (r == 4) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW6_GPIO_Port, ROW6_Pin, (r == 5) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW7_GPIO_Port, ROW7_Pin, (r == 6) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(ROW8_GPIO_Port, ROW8_Pin, (r == 7) ? GPIO_PIN_RESET : GPIO_PIN_SET);

		HAL_Delay(5);

		for(int c = 0; c < keyPad.ColumnS; c++)
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
				HAL_Delay(DEBOUNCE_DELAY);

				if(!(HAL_GPIO_ReadPin(columnPort, columnPin)))
				{
					key |= 1<<c;
					key |= 1<<(r+8);
				}

				while(!(HAL_GPIO_ReadPin(columnPort, columnPin)))
				{
					HAL_Delay(1);
				}

				return key;
			}
		}
	}

	return key;
}

void decode_keycode(void)
{
	uint8_t i, j, col, row, index;

	for(i = 1, j = 1 ; j <= 8 ; i <<= 1, j++)
	{
		if ((keyPad.key & 0xFF) == i)
		{
			col = j;
			break;
		}
	}
	row = keyPad.key >> 8;

	index = col | (row << 3);

	char decode_str[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', // Uppercase letters
	        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'  // Digits
			};

	char Tkey = decode_str[index];


}

void keyPad_Scan4SisTick(void)
{

	for(uint8_t r = 0; r < keyPad.RowS; r++)
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


		uint16_t c1c4_C = ~Read_Port(GPIOC);
		uint16_t c5c6_F	= ~Read_Port(GPIOF);
		uint16_t c7c8_A = ~Read_Port(GPIOA);

		uint8_t c = ((c1c4_C & 0b00001111) << 0) |
					((c5c6_F & 0b00001100) << 2) |
					((c7c8_A & 0b00000011) << 6);

		keyPad.key = c | (r<<8);

//		if(c != 0)
//			return key;
	}
}

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
	uint16_t keyRead;

	KeyS_init();

	for(;;)
	{
		keyRead = keyPad_Scan();
	}
}


void test_sw(void)
{
	soft_keysTest();

}