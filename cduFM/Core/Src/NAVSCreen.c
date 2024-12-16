/*
 * NAVSCreen.c
 *
 *  Created on: Oct 30, 2024
 *      Author: LENOVO
 */

#include "main.h"

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
//int NAVScreenPage = 0;

//--------------------------------


returnStatus ChangeNavParam(NavParamNumber PNum, void * PVal);


returnStatus DispNAVscreen(NavParams * Params)
{
//	static NavParams oldParams;
	char Text[50];

	sprintf(Text, "S %0.3f", Params->Standby.freq);
	UpdateParamLCD(Left1, Text);
//	ChangeNavParam(StandbyFreq, (void *) &Params->Standby);
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "VOL");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "NAV");
	sprintf(Text, "A %0.3f", Params->Active.freq);
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
		sprintf(Text, "A %0.3f", *(double *)PVal);
		UpdateParamLCD(Center2, Text);
	}
	if(PNum == StandbyFreq){
		sprintf(Text, "S %0.3f", *(double *)PVal);
		UpdateParamLCD(Left1, Text);
	}
}

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

char* editFreq(NavFreq_t freq, const char *lblText, lcdCmdParam_id pos)
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

uint16_t NavScreenStateMachine(NavParams * Params){


	static lcdCmdParam_id Position;
	static double * Label;
	static char Format[20];
	softKey_t softkey;

	volatile uint16_t ret;
	static char lblText[20];
//	bool decimal_added;
	volatile char keyVal;

	ret = get_ScanKode_from_buffer();
	keyVal = decode_keycode(ret);
	if((ret & 0x00FF) == 0)  keyVal = 0; // Do not process release code
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
		if (keyVal == 's') // SWAP key
		{
			NavScreenState = SwapKey;
		}

		if (keyVal == 'n') // NEXT button
		{
			NavScreenState = page1;
		}

		if (keyVal == 'b') // BACK button
		{
			NavScreenState = page0;
		} else if(softkey == R1) {
			Position = Right1;
			NavScreenState = RightSoftKey;

			if(!(Params->page)){
				Label = &Params->P1.freq;
				strncpy(Format, "P1 %0.3f", 10);
			}else {
				Label = &Params->P5.freq;
				strncpy(Format, "P5 %0.3f", 10);

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
				Label = &Params->P2.freq;
				strncpy(Format, "P2 %0.3f", 10);
			}else {
				Label = &Params->P6.freq;
				strncpy(Format, "P6 %0.3f", 10);

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
				Label = &Params->P3.freq;
				strncpy(Format, "P3 %0.3f", 10);
			}else {
				Label = &Params->P7.freq;
				strncpy(Format, "P7 %0.3f", 10);

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
				Label = &Params->P4.freq;
				strncpy(Format, "P4 %0.3f", 10);
			}else {
				Label = &Params->P8.freq;
				strncpy(Format, "P8 %0.3f", 10);

			}
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, * Label);
			UpdateParamLCD(Center5, lblText);
		}

		break;

	case StandByEdit:

		Label = &Params->Standby.freq;
		strncpy(Format, "S %0.3f", 10);
		sprintf(lblText, Format, * Label);

//		char text[9];
//
////		sprintf(text, "S %f", Params->Standby);
//		snprintf(text, sizeof(text), "S %.3f", Params->Standby.freq);
//
//		while (ret != 0x004) // OK Button
//		{
//			ret = get_ScanKode_from_buffer();
//			keyVal = decode_keycode(ret);
//			if (ret != 0)
//			{
//				int len = strlen(text) ;
//
//				if (ret == 0x110) // using B button instead of BACK
//				{
//					if (len > 2) {   // Ensure there's something to delete beyond the prefix "S "
//						text[len - 1] = '\0';  // Remove last character
//						if (text[len - 2] == '.')
//						{
//							decimal_added = 0;
//						}
//						len--;
////						UpdateParamLCD( , text);
//					}
//				}
//				else if (ret == 0x408 && !decimal_added && len < 9)
//				{
//					strncat(text, ".", 1);
//					decimal_added = 1;
//					UpdateParamLCD(Left1, text);
//				}
//				else if (keyVal >= '0' && keyVal <= '9' && len < 9)
//				{
//					char keyChar = (char) keyVal;
//					strncat(text, &keyChar, 1);
//					UpdateParamLCD(Left1, text);
//				}
//			}
//		}

		char* result = editFreq(Params->Standby, lblText, Left1);

		Params->Standby.freq = atof(&result[2]); // remove s_ first
		NavScreenState = idle;
		char txt[7];
		snprintf(txt, sizeof(txt), "S %0.3f", Params->Standby.freq);
		UpdateParamLCD(Left1, txt);
		configBgcolorLCD(Left1, TRANSPARENTBG);
		configfontcolorLCD(Left1, BLACKFONT);

		break;

	case SwapKey:
		swapFreq(&Params->Active.freq,&Params->Standby.freq);
		ChangeNavParam(StandbyFreq, (void *) &Params->Standby.freq);
		ChangeNavParam(ActiveFreq, (void *) &Params->Active.freq);
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

		if(softkey == L4)
		{
			UpdateParamLCD(Center4, "PROG");

			configBgcolorLCD(Center5, BLACKBG);
			configfontcolorLCD(Center5, WHITEFONT);

			char* result = editFreq(Params->P1, lblText, Center5);

			* Label = atof(&result[3]);
			Params->P1.freq = * Label;
			configBgcolorLCD(Position, TRANSPARENTBG);
			configfontcolorLCD(Position, BLACKFONT);

			configBgcolorLCD(Center5, TRANSPARENTBG);
			configfontcolorLCD(Center5, BLACKFONT);

			NavScreenState = idle;
			UpdateParamLCD(Center4, "");
			UpdateParamLCD(Center5, "");
		}

		if(keyVal == 'o') // OK Button
		{
			NavScreenState = idle;

			Params->Standby.freq = * Label;
			sprintf(lblText, "S %0.3f", Params->Standby.freq);
			UpdateParamLCD(Left1, lblText);

			configBgcolorLCD(Position, TRANSPARENTBG);
			configfontcolorLCD(Position, BLACKFONT);

			UpdateParamLCD(Center5, "");
		}

			break;


	default:
		break;
	}



	return keyVal;

}
