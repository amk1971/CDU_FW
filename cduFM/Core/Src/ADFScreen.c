/*
 * ADFScreen.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Dell
 */

#include "ADFScreen.h"

extern ScreenParams NavScreenParams;
extern keyPad_t keyPad;
extern lcdCmdDisp_id currentScreen, NextScreen;

SCREENState AdfScreenState = Idle;

returnStatus ChangedAdfParam(AdfParamNumber PNum, void * PVal);

returnStatus DispADFscreen(ScreenParams * Params)
{
//	static NavParams oldParams;
	char Text[50];

	sprintf(Text, "S %0.3f", Params->Standby.freq);
	UpdateParamLCD(Left1, Text);
//	ChangeNavParam(StandbyFreq, (void *) &Params->Standby);
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "ADF");
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

returnStatus ChangedAdfParam(AdfParamNumber PNum, void * PVal){
	char Text[50];

	if(PNum == aActiveFreq){
		sprintf(Text, "A %0.3f", *(double *)PVal);
		UpdateParamLCD(Center2, Text);
	}
	if(PNum == aStandbyFreq){
		sprintf(Text, "S %0.3f", *(double *)PVal);
		UpdateParamLCD(Left1, Text);
	}
	return success;
}

uint16_t AdfScreenStateMachine(ScreenParams * Params, softKey_t softkey){


	static lcdCmdParam_id Position;
//	static double * Label;
	static Freq_t * ptrFreq;
	static char Format[20];

	returnStatus retStatus;

	volatile uint16_t retKey;
	static char lblText[20];
//	bool decimal_added;
	volatile char keyVal;
	uint32_t PresetTimer = HAL_GetTick();

	retKey = get_ScanKode_from_buffer();
	keyVal = decode_keycode(retKey);
	if((retKey & 0x00FF) == 0)  keyVal = 0; // Do not process release code

	switch (AdfScreenState){
	case Idle:

		if(softkey == L1)
		{
			AdfScreenState = StandByEdit;
			configBgcolorLCD(Left1, BLACKBG);
			configfontcolorLCD(Left1, WHITEFONT);
		}

		if (keyVal == 's') // SWAP key
		{
			AdfScreenState = SwapKey;
		}

		if (keyVal == 'n') // NEXT button
		{
			AdfScreenState = page1;
		}

		if (keyVal == 'p') // using PREV button instead of BACK button(which is asked)
		{
			AdfScreenState = page0;

		} else if(softkey == R1) {
			Position = Right1;
			AdfScreenState = RightSoftKey;

			if(!(Params->page)){
				ptrFreq = &Params->P1;
				strncpy(Format, "P1 %0.3f", 10);
			}else {
				ptrFreq = &Params->P5;
				strncpy(Format, "P5 %0.3f", 10);

			}
			PresetTimer = HAL_GetTick();
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, ptrFreq->freq);
			UpdateParamLCD(Center5, lblText);
		}

		else if(softkey == R2)
		{
			Position = Right2;
			AdfScreenState = RightSoftKey;

			if(!(Params->page)){
				ptrFreq = &Params->P2;
				strncpy(Format, "P2 %0.3f", 10);
			}else {
				ptrFreq = &Params->P6;
				strncpy(Format, "P6 %0.3f", 10);

			}
			PresetTimer = HAL_GetTick();
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, ptrFreq->freq);
			UpdateParamLCD(Center5, lblText);
		}

		else if(softkey == R3)
		{
			Position = Right3;
			AdfScreenState = RightSoftKey;

			if(!(Params->page)){
				ptrFreq = &Params->P3;
				strncpy(Format, "P3 %0.3f", 10);
			}else {
				ptrFreq = &Params->P7;
				strncpy(Format, "P7 %0.3f", 10);

			}
			PresetTimer = HAL_GetTick();
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, ptrFreq->freq);
			UpdateParamLCD(Center5, lblText);
		}

		else if(softkey == R4)
		{
			Position = Right4;
			AdfScreenState = RightSoftKey;

			if(!(Params->page)){
				ptrFreq = &Params->P4;
				strncpy(Format, "P4 %0.3f", 10);
			}else {
				ptrFreq = &Params->P8;
				strncpy(Format, "P8 %0.3f", 10);

			}
			PresetTimer = HAL_GetTick();
			configBgcolorLCD(Position, BLACKBG);
			configfontcolorLCD(Position, WHITEFONT);
			sprintf(lblText, Format, ptrFreq->freq);
			UpdateParamLCD(Center5, lblText);
		}

		break;

	case StandByEdit:

		ptrFreq = &Params->Standby;
		strncpy(Format, "S %0.3f", 10);
		sprintf(lblText, Format, ptrFreq->freq);

		char result[20];
		retStatus = editFreq(* ptrFreq, lblText, Left1, result, sizeof(result));
		if(retStatus == homeButtonPress)
		{
			return 'h'; // returning Home Button press
		}
		else if(retStatus == updatedFrequency)
		{
			ptrFreq->freq = atof(&result[ptrFreq->tileSize]); // remove s_ first
		}
		else if(retStatus == oldFrequency)
		{
			ptrFreq->freq = atof(&lblText[ptrFreq->tileSize]); // remove s_ first
		}

//		Params->Standby.freq = atof(&result[2]); // remove s_ first
		AdfScreenState = idle;
		char txt[15];
		snprintf(txt, sizeof(txt), "S %0.3f", Params->Standby.freq);
		UpdateParamLCD(Left1, txt);
		configBgcolorLCD(Left1, TRANSPARENTBG);
		configfontcolorLCD(Left1, BLACKFONT);

		break;

	case SwapKey:
		swapFreq(&Params->Active.freq,&Params->Standby.freq);
		ChangedAdfParam(aStandbyFreq, (void *) &Params->Standby.freq);
		ChangedAdfParam(aActiveFreq, (void *) &Params->Active.freq);
		AdfScreenState = Idle;

		break;

	case page0:

		Params->page = false;
		DispADFscreen(Params);
		AdfScreenState = Idle;

		break;

	case page1:

		Params->page = true;	// means page1
		DispADFscreen(Params);
		AdfScreenState = Idle;

		break;

	case RightSoftKey:

		if((HAL_GetTick() - PresetTimer) > 15000) {
			AdfScreenState = idle;
			configBgcolorLCD(Position, TRANSPARENTBG);
			configfontcolorLCD(Position, BLACKFONT);
			UpdateParamLCD(Center5, "");
		}

		if(softkey == L4)
		{
			UpdateParamLCD(Center4, "PROG");

			configBgcolorLCD(Center5, BLACKBG);
			configfontcolorLCD(Center5, WHITEFONT);

			char result[20];
			retStatus = editFreq(* ptrFreq, lblText, Center5, result, sizeof(result));
//			char* result = editFreq(* ptrFreq, lblText, Center5, );
			if(retStatus == homeButtonPress)
			{
				return 'h'; // returning Home Button press
			}
			else if(retStatus == updatedFrequency)
			{
				ptrFreq->freq = atof(&result[ptrFreq->tileSize]); // remove s_ first
			}
			else if(retStatus == oldFrequency)
			{
				ptrFreq->freq = atof(&lblText[ptrFreq->tileSize]); // remove s_ first
			}

//			* Label = atof(&result[3]);
//			Params->P1.freq = * Label;
			configBgcolorLCD(Position, TRANSPARENTBG);
			configfontcolorLCD(Position, BLACKFONT);

			configBgcolorLCD(Center5, TRANSPARENTBG);
			configfontcolorLCD(Center5, BLACKFONT);

			AdfScreenState = idle;
			UpdateParamLCD(Center4, "");
			UpdateParamLCD(Center5, "");
		}

		if(keyVal == 'o') // OK Button
		{
			AdfScreenState = idle;

			Params->Standby.freq = ptrFreq->freq;
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
