/*
 * ADFScreen.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Dell
 */

#include "main.h"

extern ScreenParams NavScreenParams;
extern keyPad_t keyPad;

typedef enum {
	ActiveFreq = 0,
	StandbyFreq,
	P1, P2, P3, P4, P5, P6, P7, P8
}AdfParamNumber;

SCREENState AdfScreenState = Idle;

returnStatus ChangedAdfParam(AdfParamNumber PNum, void * PVal);

returnStatus DispAdfscreen(ScreenParams * Params)
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

	if(PNum == ActiveFreq){
		sprintf(Text, "A %0.3f", *(double *)PVal);
		UpdateParamLCD(Center2, Text);
	}
	if(PNum == StandbyFreq){
		sprintf(Text, "S %0.3f", *(double *)PVal);
		UpdateParamLCD(Left1, Text);
	}
}

uint16_t AdfScreenStateMachine(ScreenParams * Params){


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

		if (keyVal == 'b') // BACK button
		{
			AdfScreenState = page0;
		} else if(softkey == R1) {
			Position = Right1;
			AdfScreenState = RightSoftKey;

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
			AdfScreenState = RightSoftKey;

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
			AdfScreenState = RightSoftKey;

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
			AdfScreenState = RightSoftKey;

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

		char* result = editFreq(Params->Standby, lblText, Left1);

		Params->Standby.freq = atof(&result[2]); // remove s_ first
		AdfScreenState = idle;
		char txt[7];
		snprintf(txt, sizeof(txt), "S %0.3f", Params->Standby.freq);
		UpdateParamLCD(Left1, txt);
		configBgcolorLCD(Left1, TRANSPARENTBG);
		configfontcolorLCD(Left1, BLACKFONT);

		break;

	case SwapKey:
		swapFreq(&Params->Active.freq,&Params->Standby.freq);
		ChangedAdfParam(StandbyFreq, (void *) &Params->Standby.freq);
		ChangedAdfParam(ActiveFreq, (void *) &Params->Active.freq);
		AdfScreenState = Idle;

		break;

	case page0:

		Params->page = false;
		DispNAVscreen(Params);
		AdfScreenState = Idle;

		break;

	case page1:

		Params->page = true;	// means page1
		DispNAVscreen(Params);
		AdfScreenState = Idle;

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

			AdfScreenState = idle;
			UpdateParamLCD(Center4, "");
			UpdateParamLCD(Center5, "");
		}

		if(keyVal == 'o') // OK Button
		{
			AdfScreenState = idle;

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
