/*
 * NAVSCreen.c
 *
 *  Created on: Oct 30, 2024
 *      Author: LENOVO
 */

#include "common.h"

typedef enum
{
	Idle = 0,
	SwapKey,
	RightSoftKey1,
	StandByEdit
} NAVSCREENState;

typedef enum {
	ActiveFreq = 0,
	StandbyFreq,
	P1, P2, P3, P4, P5, P6, P7, P8
}NavParamNumber;

NAVSCREENState NavScreenState = Idle;
int NAVScreenPage = 0;


returnStatus DispNAVscreen(NavParams * Params)
{
	static NavParams oldParams;
	char Text[50];

	//sprintf(Text, "S %f", Params->Standby);
	//UpdateParamLCD(Left1, Text);
	ChangeNavParam(StandbyFreq, (void *) &Params->Standby);
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "VOL");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "NAV");
	//sprintf(Text, "A %f", Params->Active);
	//UpdateParamLCD(Center2, Text);
	ChangeNavParam(ActiveFreq, (void *) &Params->Active);
	UpdateParamLCD(Center3, "");
	UpdateParamLCD(Center4, "");
	UpdateParamLCD(Center5, "");
	UpdateParamLCD(Right1, "P 1");
	UpdateParamLCD(Right2, "P 2");
	UpdateParamLCD(Right3, "P 3");
	UpdateParamLCD(Right4, "P 4");

	return success;
}
returnStatus ChangeNavParam(NavParamNumber PNum, void * PVal){
	char Text[50];

	if(PNum == ActiveFreq){
		sprintf(Text, "A %f", *(float *)PVal);
		UpdateParamLCD(Center2, Text);
	}
	if(PNum == StandbyFreq){
		sprintf(Text, "S %f", *(float *)PVal);
		UpdateParamLCD(Left1, Text);
	}

}

void NavScreenStateMachine(void ){

	switch (NavScreenState){
	case Idle:
		if(soft_keysTest() == L1){
			NavScreenState = StandByEdit;
		}
		break;
	case StandByEdit:
		break;
	}

}
