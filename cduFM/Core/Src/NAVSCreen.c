/*
 * NAVSCreen.c
 *
 *  Created on: Oct 30, 2024
 *      Author: LENOVO
 */

#include "nextionlcd.h"
#include "switches.h"
#include "stdlib.h"
#include "string.h"

bool flag = false;


typedef enum
{
	Idle = 0,
	SwapKey,
	page0,
	page1,
	RightSoftKey,
	StandByEdit
} NAVSCREENState;

extern NavParams NavScreenParams;
extern keyPad_t keyPad;

typedef enum {
	ActiveFreq = 0,
	StandbyFreq,
	P1, P2, P3, P4, P5, P6, P7, P8
}NavParamNumber;

NAVSCREENState NavScreenState = Idle;
int NAVScreenPage = 0;

returnStatus ChangeNavParam(NavParamNumber PNum, void * PVal);


returnStatus DispNAVscreen(NavParams * Params)
{
//	static NavParams oldParams;
	char Text[50];

	sprintf(Text, "S %f", Params->Standby);
	UpdateParamLCD(Left1, Text);
//	ChangeNavParam(StandbyFreq, (void *) &Params->Standby);
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "VOL");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "NAV");
	sprintf(Text, "A %f", Params->Active);
	UpdateParamLCD(Center2, Text);
//	ChangeNavParam(ActiveFreq, (void *) &Params->Active);
	UpdateParamLCD(Center3, "");
	UpdateParamLCD(Center4, "");
	UpdateParamLCD(Center5, "");

	if(!(Params->page))
	{
		UpdateParamLCD(Right1, "P 1");
		UpdateParamLCD(Right2, "P 2");
		UpdateParamLCD(Right3, "P 3");
		UpdateParamLCD(Right4, "P 4");
	}
	else
	{
		UpdateParamLCD(Right1, "P 5");
		UpdateParamLCD(Right2, "P 6");
		UpdateParamLCD(Right3, "P 7");
		UpdateParamLCD(Right4, "P 8");
	}

	configfontcolorLCD(Right1, BLACKFONT);
	configfontcolorLCD(Right2, BLACKFONT);
	configfontcolorLCD(Right3, BLACKFONT);
	configfontcolorLCD(Right4, BLACKFONT);

	return success;
}

returnStatus ChangeNavParam(NavParamNumber PNum, void * PVal){
	char Text[50];

	if(PNum == ActiveFreq){
		sprintf(Text, "A %f", *(double *)PVal);
		UpdateParamLCD(Center2, Text);
	}
	if(PNum == StandbyFreq){
		sprintf(Text, "S %f", *(double *)PVal);
		UpdateParamLCD(Left1, Text);
	}
}

void swapFreq(double *param1, double *param2)
{
	double temp = *param1;
	*param1 = *param2;
	*param2 = temp;
}

uint16_t NavScreenStateMachine(NavParams * Params){

	uint16_t ret;
	static char lblText[20];
	static lcdCmdParam_id Position;
	static double * Label;
	static char Format[20];
	softKey_t softkey;
	bool decimal_added = 0;

	ret = get_ScanKode_from_buffer();
	softkey = check_soft_keys();
	switch (NavScreenState){
	case Idle:

		if(softkey == L1)
		{
			NavScreenState = StandByEdit;
			configBgcolorLCD(Left1, BLACKBG);
			configfontcolorLCD(Left1, WHITEFONT);
		}

//		ret = get_ScanKode_from_buffer();
		if (ret == 0x008) // SWAP key
		{
			NavScreenState = SwapKey;
		}

		if (ret == 0x020) // NEXT button
		{
			NavScreenState = page1;
		}

		if (ret == 0x110) // using B button instead of BACK
		{
			NavScreenState = page0;
		} else if(softkey == R1) {
			Position = Right1;
			NavScreenState = RightSoftKey;

			if(!(Params->page)){
				Label = &Params->P1;
				strncpy(Format, "P1 %0.3f", 9);
			}else {
				Label = &Params->P5;
				strncpy(Format, "P5 %0.3f", 9);

			}
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, * Label);
			UpdateParamLCD(Center5, lblText);
		}

		else if(softkey == R2)
		{
			Position = Right2;
			NavScreenState = RightSoftKey;

			if(!(Params->page)){
				Label = &Params->P2;
				strncpy(Format, "P2 %0.3f", 9);
			}else {
				Label = &Params->P6;
				strncpy(Format, "P6 %0.3f", 9);

			}
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, * Label);
			UpdateParamLCD(Center5, lblText);
		}

		else if(softkey == R3)
		{
			Position = Right3;
			NavScreenState = RightSoftKey;

			if(!(Params->page)){
				Label = &Params->P3;
				strncpy(Format, "P3 %0.3f", 9);
			}else {
				Label = &Params->P7;
				strncpy(Format, "P7 %0.3f", 9);

			}
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, * Label);
			UpdateParamLCD(Center5, lblText);
		}

		else if(softkey == R4)
		{
			Position = Right4;
			NavScreenState = RightSoftKey;

			if(!(Params->page)){
				Label = &Params->P4;
				strncpy(Format, "P4 %0.3f", 9);
			}else {
				Label = &Params->P8;
				strncpy(Format, "P8 %0.3f", 9);

			}
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, * Label);
			UpdateParamLCD(Center5, lblText);
		}

		break;

	case StandByEdit:

		char text[9];


//		sprintf(text, "S %f", Params->Standby);
		snprintf(text, sizeof(text), "S %.3f", Params->Standby); // or "%.1f" if single decimal precision

		while (ret != 0x004) // OK Button
		{
			ret = get_ScanKode_from_buffer();
			char keyVal = decode_keycode(ret);
			if (ret != 0)
			{
				int len = strlen(text) ;

				if (ret == 0x110) // using B button instead of BACK
				{
					if (len > 2) {   // Ensure there's something to delete beyond the prefix "S "
						text[len - 1] = '\0';  // Remove last character
						if (text[len - 2] == '.')
						{
							decimal_added = 0;
						}
						len--;
						UpdateParamLCD(Left1, text);
					}
				}
				else if (ret == 0x408 && !decimal_added && len < 9)
				{
					strncat(text, ".", 1);
					decimal_added = 1;
					UpdateParamLCD(Left1, text);
				}
				else if (keyVal >= '0' && keyVal <= '9' && len < 9)
				{
					char keyChar = (char) keyVal;
					strncat(text, &keyChar, 1);
					UpdateParamLCD(Left1, text);
				}
			}
		}
//		volatile char * removeit = &text[2];
		Params->Standby = atof(&text[2]); // remove s_ first
		NavScreenState = idle;
		char txt[7];
		snprintf(txt, sizeof(txt), "S %f", Params->Standby);
		UpdateParamLCD(Left1, txt);
		configBgcolorLCD(Left1, TRANSPARENTBG);
		configfontcolorLCD(Left1, BLACKFONT);

		break;

	case SwapKey:
		swapFreq(&Params->Active,&Params->Standby);
		ChangeNavParam(StandbyFreq, (void *) &Params->Standby);
		ChangeNavParam(ActiveFreq, (void *) &Params->Active);
		NavScreenState = Idle;

		break;

	case page0:

		Params->page = false;
		DispNAVscreen(Params);
		NavScreenState = Idle;

		break;

	case page1:

		Params->page = true;	// means page1
		DispNAVscreen(Params);
		NavScreenState = Idle;

		break;

	case RightSoftKey:


//		ret = get_ScanKode_from_buffer();
		if(softkey == L4)
		{
			UpdateParamLCD(Center4, "PROG");

			configBgcolorLCD(Center5, BLACKBG);
			configfontcolorLCD(Center5, WHITEFONT);

			char str[20];
			//snprintf(str, sizeof(str), Format, * Label);
			strncpy(str, lblText, sizeof(str));
			static uint32_t curTickValue;// = HAL_GetTick();
			while(ret != 0x004) // OK Button
			{
				ret = get_ScanKode_from_buffer();
				if (ret == 0) {
					curTickValue = HAL_GetTick() - 100;
				} else {
					char keyVal = decode_keycode(ret);

					int len = strlen(str);
					if((ret == 0x110) || (ret == 001)) // using B button instead of BACK
					{
						if (len > 2)
						{
							str[len-1]='\0';
							len--;
						}
						UpdateParamLCD(Center5, str);
					}
					else if ((ret == 0x408) && (!decimal_added) && (len < 9) &&
							((HAL_GetTick() - curTickValue) > 100))
					{
						curTickValue = HAL_GetTick();
						strncat(text, ".", 1);
						decimal_added = 1;
						UpdateParamLCD(Center5, text);
					}
					else if ((keyVal >= '0') && (keyVal <= '9') && (len < 9) &&
							((HAL_GetTick() - curTickValue) > 100))
					{
						char keyChar[2] = {(char) keyVal, 0};
						curTickValue = HAL_GetTick();
						strncat(str, keyChar, 1);
						UpdateParamLCD(Center5, str);
					}
				}

			}
			* Label = atof(&str[3]);
			configBgcolorLCD(Position, TRANSPARENTBG);
			configfontcolorLCD(Position, BLACKFONT);

			configBgcolorLCD(Center5, TRANSPARENTBG);
			configfontcolorLCD(Center5, BLACKFONT);

			NavScreenState = idle;
			UpdateParamLCD(Center4, "");
			UpdateParamLCD(Center5, "");
		}

		if(ret == 0x110) // OK Button
		{
			NavScreenState = idle;

			Params->Standby = * Label;
			sprintf(lblText, "S %f", Params->Standby);
			UpdateParamLCD(Left1, lblText);

			configBgcolorLCD(Position, TRANSPARENTBG);
			configfontcolorLCD(Position, BLACKFONT);

			UpdateParamLCD(Center5, "");
		}

			break;


	default:
		break;
	}



	return ret;

}