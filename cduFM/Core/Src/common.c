/*
 * common.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Dell
 */

#include "common.h"

void swapFreq(double *param1, double *param2)
{
	double temp = *param1;
	*param1 = *param2;
	*param2 = temp;
}

uint8_t checkDot(char* arr){

	uint8_t i = 0;
	while(arr[i] != '\0'){
		if(arr[i] == '.')
			return 1;
		else
			i++;
	}
	return 0;
}

char* editFreq(Freq_t freq, const char *lblText, lcdCmdParam_id pos)
{

	volatile uint16_t ret;
	volatile uint8_t keyVal = 0;
	static char str[20];
	strncpy(str, lblText, sizeof(str));
	bool decimal_added = checkDot(str);
	static uint32_t curTickValue;// = HAL_GetTick();
	while(keyVal != 'o') // OK Button
	{
		ret = get_ScanKode_from_buffer();
		if ((ret & 0x00FF) == 0) {				//Do not process Release Codes
			curTickValue = HAL_GetTick() - 200;
		}
		else
		{
			keyVal = decode_keycode(ret);

			int len = strlen(str);
			if(keyVal == 'b') // BACK button
			{
				if (len > freq.tileSize)
				{
					if(str[len-1] == '.') decimal_added = 0;
					str[len-1]='\0';
					len--;
				}
				UpdateParamLCD(pos, str);
			}
			else if ((keyVal == '.') && (!decimal_added) && (len < (freq.tileSize + 7)) &&
					((HAL_GetTick() - curTickValue) > 200))
			{
				curTickValue = HAL_GetTick();
				strncat(str, ".", 1);
				decimal_added = 1;
				UpdateParamLCD(pos, str);
			}
			else if ((keyVal >= '0') && (keyVal <= '9') &&
					((HAL_GetTick() - curTickValue) > 200))
			{
				if((decimal_added && (freq.tileSize + 7) < 10) || (!decimal_added && len < 6))
				{
					char keyChar[2] = {(char) keyVal, 0};
					curTickValue = HAL_GetTick();
					strncat(str, keyChar, 1);
					UpdateParamLCD(pos, str);
				}
			}
		}
	}
	return str;
}

uint8_t checkFreqLimit(double editedfreq, double upLimit, double lowLimit)
{

}
