/*
 * common.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Dell
 */

#include "common.h"
#include "nextionLcd.h"
#include "switches.h"
#include "stm32f0xx_hal.h"

#define isdigit(c) (c >= '0' && c <= '9')

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


double myatof(const char *s)
{
  // This function stolen from either Rolf Neugebauer or Andrew Tolmach.
  // Probably Rolf.
  double a = 0.0;
  int e = 0;
  int c;
  while ((c = *s++) != '\0' && isdigit(c)) {
    a = a*10.0 + (c - '0');
  }
  if (c == '.') {
    while ((c = *s++) != '\0' && isdigit(c)) {
      a = a*10.0 + (c - '0');
      e = e-1;
    }
  }
  if (c == 'e' || c == 'E') {
    int sign = 1;
    int i = 0;
    c = *s++;
    if (c == '+')
      c = *s++;
    else if (c == '-') {
      c = *s++;
      sign = -1;
    }
    while (isdigit(c)) {
      i = i*10 + (c - '0');
      c = *s++;
    }
    e += i*sign;
  }
  while (e > 0) {
    a *= 10.0;
    e--;
  }
  while (e < 0) {
    a *= 0.1;
    e++;
  }
  return a;
}


char * editFreq(Freq_t freq, char *lblText, lcdCmdParam_id pos)
{

	volatile uint16_t ret;
	volatile uint8_t keyVal = 0;
	static char str[20], strBlink[20];
	strncpy(str, lblText, sizeof(str));
	bool decimal_added = checkDot(str);
	static uint32_t curTickValue;// = HAL_GetTick();
	bool blink = false;
	bool message = false;
	uint32_t reactTime = HAL_GetTick();
	uint32_t blinkTimer = reactTime;
	uint32_t Messagetimer = reactTime;
	while(keyVal != 'o') // OK Button
	{
		if(message && (HAL_GetTick() > Messagetimer )) {
			UpdateParamLCD(Center3, "");
			message = false;
		}
		if ((HAL_GetTick() - blinkTimer) >= blinkDuration) {
			strncpy(strBlink, str, 1+strlen(str));
			blinkTimer = HAL_GetTick();
			if(!blink){
				//char cursor[2] = {'|', 0};
				strncat(strBlink, "|", 2);
			} else {
				strncat(strBlink, " ", 2);
			}
			UpdateParamLCD(pos, strBlink);
			blink = !blink;
		}

		if ((HAL_GetTick() - reactTime) >= reactDuration) {
			return lblText;
		}
		ret = get_ScanKode_from_buffer();
		if ((ret & 0x00FF) == 0) {				//Do not process Release Codes
			curTickValue = HAL_GetTick() - 210;
		}
		else
		{
			reactTime = HAL_GetTick();
			keyVal = decode_keycode(ret);

			int len = strlen(str);
			if(keyVal == 'b') // BACK button
			{
				//reactTime = HAL_GetTick();
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
				//reactTime = HAL_GetTick();
				curTickValue = HAL_GetTick();

				decimal_added = 1;
				UpdateParamLCD(pos, str);
				//volatile int x = atoi(&str[freq.tileSize]);
				if(atoi(&str[freq.tileSize]) < MIN_FREQUENCY) {
					UpdateParamLCD(Center3, "< MIN");
					message = true;
					Messagetimer = HAL_GetTick()+1000;
				}
				if(atoi(&str[freq.tileSize]) > MAX_FREQUENCY) {
					UpdateParamLCD(Center3, "> MAX");
					message = true;
					Messagetimer = HAL_GetTick()+1000;
				}
				strncat(str, ".", 2);
			}
			else if ((keyVal >= '0') && (keyVal <= '9') &&
					((HAL_GetTick() - curTickValue) > 200))
			{
				if((decimal_added && (freq.tileSize + 7) < 10) || (!decimal_added && len < 6))
				{
					//reactTime = HAL_GetTick();
					char keyChar[2] = {(char) keyVal, 0};
					curTickValue = HAL_GetTick();
					strncat(str, keyChar, 1);
					UpdateParamLCD(pos, str);
				}
			}
		}
	}

	//strncpy(, &str[freq.tileSize], 10);

	//volatile double x = myatof(&str[freq.tileSize]);
	//x = strtod(lblText, 0);
	if(myatof(&str[freq.tileSize]) < MIN_FREQUENCY) {
		UpdateParamLCD(Center3, "< MIN");
		message = true;
		Messagetimer = HAL_GetTick()+1000;
	}
	if(myatof(&str[freq.tileSize]) > MAX_FREQUENCY) {
		UpdateParamLCD(Center3, "> MAX");
		message = true;
		Messagetimer = HAL_GetTick()+1000;
	}
	while (HAL_GetTick() < Messagetimer);
	UpdateParamLCD(Center3, "");
	return str;
}


