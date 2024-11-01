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
softKey_t softkey;

MkeyStatus_t MkeyStatus;

struct CYCLICBUFFER {
	uint16_t buff[32];
	char Head, Tail;
}ScanKode;

// Unused keys as: '!', '"', '#', '$', '%'

char decode_str[] = {'e', 'g', 'C', 'D', 'E', 'F', 'e', 'H', 'I', 'b',
					 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
					 'A', 'B', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3',
					 '4', '5', '6', '7', '8', '9', 'r', 'G', 's', 'p',
					 'n', 'd', 't', 'h', 'c', 'o', '_', '.', '/', '^',
					 'v', '+', '-', '!', '"', '#', '$', '%'
			};

const char* interpret_char(char c){
	switch(c){
	case 'e': return "ENT";
	case 'b': return "BACK";
	case 's': return "SWAP";
	case 'p': return "PREV";
	case 'n': return "NEXT";
	case 'd': return "DIM";
	case 't': return "BRT";
	case 'h': return "HOME";
	case 'c': return "CLR";
	case 'o': return "OK";
	case '_': return "SPACE";
	default: return c;
	}
}

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

MkeyStatus_t keyPad_Scan(void)
{
//	uint16_t key = 0;

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

		//HAL_Delay(5);

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
					keyPad.key |= 1<<c;
					keyPad.key |= r<<8;
//					MkeyStatus = Press;
				}

				while(!(HAL_GPIO_ReadPin(columnPort, columnPin)))
				{
					HAL_Delay(1);
				}

				return Press;
			}
		}
	}

	return nPress;
}

char decode_keycode(void)
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

	for(i = 1, j = 1 ; j <= 8 ; i <<= 1, j++)
		{
			if (((keyPad.key & 0xFF00) >> 8) == i)
			{
				row = j;
				break;
			}
		}

	index = col | (row << 3);

	char tkey = decode_str[index];

	tkey = interpret_char(tkey);

	return tkey;

}

void keyPad_Scan4SysTick(void)
{
	static char keys[8];
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

		if(keys[r] != c){
			ScanKode.buff[ScanKode.Head++] = c;
			ScanKode.Head &= 31;
		}

		keys[r] = c;

		//keyPad.key = c | (r<<8);
	}
}

uint16_t get_ScanKode_from_buffer(void ){
	uint16_t Kode;
	if(ScanKode.Head != ScanKode.Tail){
		Kode = ScanKode.buff[ScanKode.Tail++];
		ScanKode.Tail &= 31;
		return Kode;
	}
	return 0;
}

softKey_t soft_keysTest(void)
{
	softkey = idle;
	static char  pval = 0, val = 0;

	if(!(HAL_GPIO_ReadPin(Right_SW1_GPIO_Port, Right_SW1_Pin)))
	{
		// i am here
		val = 1;
		softkey = R1;
	}

	if(!(HAL_GPIO_ReadPin(Right_SW2_GPIO_Port, Right_SW2_Pin)))
	{
		// i am here
		val = 2;
		softkey = R2;
	}
	if(!(HAL_GPIO_ReadPin(Right_SW3_GPIO_Port, Right_SW3_Pin)))
	{
		// i am here
		val = 3;
		softkey = R3;
	}

	if(!(HAL_GPIO_ReadPin(Right_SW4_GPIO_Port, Right_SW4_Pin)))
	{
		// i am here
		val = 4;
		softkey = R4;
	}

	if(!(HAL_GPIO_ReadPin(Left_SW1_GPIO_Port, Left_SW1_Pin)))
	{
		// i am here
		val = 5;
		softkey = L1;
	}

	if(!(HAL_GPIO_ReadPin(Left_SW2_GPIO_Port, Left_SW2_Pin)))
	{
		// i am here
		val = 6;
		softkey = L2;
	}
	if(!(HAL_GPIO_ReadPin(Left_SW3_GPIO_Port, Left_SW3_Pin)))
	{
		// i am here
		val = 7;
		softkey = L3;
	}

	if(!(HAL_GPIO_ReadPin(Left_SW4_GPIO_Port, Left_SW4_Pin)))
	{
		// i am here
		val = 8;
		softkey = L4;
	}

	if (pval != val) {
		pval = val;
	}

	return softkey;

}

void Matrix_keypad_Basic_test(void)
{
	uint16_t keyRead[10];
	int i = 0;

	KeyS_init();

	for(;;)
	{
		MkeyStatus = keyPad_Scan();
		if (MkeyStatus == Press)
		{
			keyRead[i] = keyPad.key;
			i++;
			if(i >= 9)
				i=0;
		}
		else if (MkeyStatus == nPress)	// to be checked later
		{

		}
	}
}


void test_sw(void)
{
	soft_keysTest();
}
