   /*
 * nextionLcd.c
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#include "nextionLcd.h"
#include "stdbool.h"
#include <stdlib.h>
#include "stm32f0xx_hal.h"
#include <stdio.h>
#include "string.h"

#define LCDUart huart2
extern SerialStruct BuffUART2;

extern UART_HandleTypeDef LCDUart;

returnStatus DispTACANscreen(void);
returnStatus DispADFscreen(void);

lcdCmdDisp_id currentScreen, NextScreen;

uint16_t TIMEOUT = 1000;
uint8_t endCmd[3] = {0xff, 0xff, 0xff};
//uint8_t pageResponse[2] = {0, 0, 0};

returnStatus InitializeLCD(void)
{
	//initialization of lcd, like setting values to default or some like displaying main page etc
	if(DispHomeScreen())
	{
		currentScreen = lcdDisp_home;
		return success;
	}
	else
	{
		return failure;
	}
}

returnStatus DispTACANscreen(void)
{
	UpdateParamLCD(Left1, "S 22Y");
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "TACAN");
	UpdateParamLCD(Center2, "A 83X");
	UpdateParamLCD(Center3, "");
	UpdateParamLCD(Center4, "PROG");
	UpdateParamLCD(Center5, "P1 19X");
	UpdateParamLCD(Right1, "P 1");
	UpdateParamLCD(Right2, "P 2");
	UpdateParamLCD(Right3, "P 3");
	UpdateParamLCD(Right4, "P 4");

	return success;
}


returnStatus DispADFscreen(void)
{
	UpdateParamLCD(Left1, "S 195.00");
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "ADF");
	UpdateParamLCD(Center2, "A 210.00");
	UpdateParamLCD(Center3, "");
	UpdateParamLCD(Center4, "");
	UpdateParamLCD(Center5, "");
	UpdateParamLCD(Right1, "P 1");
	UpdateParamLCD(Right2, "P 2");
	UpdateParamLCD(Right3, "P 3");
	UpdateParamLCD(Right4, "P 4");

	return success;
}

returnStatus DispHomeScreen(void)
{
	UpdateParamLCD(Left1, "NAV");
	HAL_Delay(50);
	configBgcolorLCD(Left1, TRANSPARENTBG);
	configfontcolorLCD(Left1, BLACKFONT);

	UpdateParamLCD(Left2, "ADF");
	configBgcolorLCD(Left2, TRANSPARENTBG);
	configfontcolorLCD(Left2, BLACKFONT);

	UpdateParamLCD(Left3, "TACAN");
	configBgcolorLCD(Left3, TRANSPARENTBG);
	configfontcolorLCD(Left3, BLACKFONT);

	UpdateParamLCD(Left4, "");
	configBgcolorLCD(Left4, TRANSPARENTBG);
	configfontcolorLCD(Left4, BLACKFONT);

	UpdateParamLCD(Center1, "HOME");
	configBgcolorLCD(Center1, TRANSPARENTBG);
	configfontcolorLCD(Center1, BLACKFONT);

	UpdateParamLCD(Center2, "");
	configBgcolorLCD(Center2, TRANSPARENTBG);
	configfontcolorLCD(Center2, BLACKFONT);

	UpdateParamLCD(Center3, "");
	configBgcolorLCD(Center3, TRANSPARENTBG);
	configfontcolorLCD(Center3, BLACKFONT);

	UpdateParamLCD(Center4, "");
	configBgcolorLCD(Center4, TRANSPARENTBG);
	configfontcolorLCD(Center4, BLACKFONT);

	UpdateParamLCD(Center5, "");
	configBgcolorLCD(Center5, TRANSPARENTBG);
	configfontcolorLCD(Center5, BLACKFONT);

	UpdateParamLCD(Right1, "HF");
	configBgcolorLCD(Right1, TRANSPARENTBG);
	configfontcolorLCD(Right1, REDFONT);

	UpdateParamLCD(Right2, "VHF");
	configBgcolorLCD(Right2, TRANSPARENTBG);
	configfontcolorLCD(Right2, REDFONT);

	UpdateParamLCD(Right3, "UHF");
	configBgcolorLCD(Right3, TRANSPARENTBG);
	configfontcolorLCD(Right3, REDFONT);

	UpdateParamLCD(Right4, "");
	configBgcolorLCD(Right4, TRANSPARENTBG);
	configfontcolorLCD(Right4, BLACKFONT);

	return success;
}

returnStatus UpdateParamLCD(lcdCmdParam_id Param_ID, char * Param_Value)
{
	uint8_t * txBuffer = malloc(30 * sizeof(char));  // Command buffer
	int len = 0;
	char param_ID[10];

	switch (Param_ID)
	{
		case Left1:
		{
			strncpy(txBuffer, "t0.txt=\"", 9);
			strncat(txBuffer, Param_Value, 5);
			strncat(txBuffer, "\"", 2);
			//len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);


			break;
		}
		case Left2:
			{
				strncpy(param_ID, "t1", 3);
				len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

				break;
			}
		case Left3:
		{
			strncpy(param_ID, "t2", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

			break;
		}
		case Left4:
		{
			strncpy(param_ID, "t3", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

			break;
		}
		case Right1:
		{
			strncpy(param_ID, "t4", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			break;
		}
		case Right2:
		{
			strncpy(param_ID, "t5", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

			break;
		}
		case Right3:
		{
			strncpy(param_ID, "t6", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);


			break;
		}
		case Right4:
		{
			strncpy(param_ID, "t7", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

			break;
		}
		case Center1:
		{
			strncpy(param_ID, "t8", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

			break;
		}
		case Center2:
		{
			strncpy(param_ID, "t9", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

			break;
		}
		case Center3:
		{
			strncpy(param_ID, "t10", 4);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			break;
		}
		case Center4:
			{
				strncpy(param_ID, "t11", 4);
				len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);

				break;
			}
		case Center5:
			{
				strncpy(param_ID, "t12", 4);
				len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
				break;
			}

		default:
			break;
	}
	UART_SendString(&huart2, &BuffUART2, txBuffer, strlen(txBuffer));

	//if (HAL_UART_Transmit(&LCDUart, txBuffer, strlen(txBuffer), TIMEOUT) != HAL_OK)
	//{
	//	return failure;
	//}

	UART_SendString(&huart2, &BuffUART2, (uint8_t *)endCmd, 3);
	// Send termination bytes (0xFF 0xFF 0xFF)
	//if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
	//{
	//	return failure;
	//}

	return success;
}

returnStatus configfontcolorLCD(lcdCmdParam_id Param_ID, int Param_Value)
{
	uint8_t * txBuffer = malloc(30 * sizeof(char));  // Command buffer
	int len = 0;
	char param_ID[10];

	switch (Param_ID)
	{
		case Left1:
		{
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

		}

		case Left2:
        {
            strncpy(param_ID, "t1", 3);
            len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

            break;
        }

		case Left3:
		{
			strncpy(param_ID, "t2", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);
			break;
		}

		case Left4:
		{
			strncpy(param_ID, "t3", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);
			break;
		}

		case Right1:
		{
			strncpy(param_ID, "t4", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

			break;
		}

		case Right2:
		{
			strncpy(param_ID, "t5", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

			break;
		}

		case Right3:
		{
			strncpy(param_ID, "t6", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

			break;
		}

		case Right4:
		{
			strncpy(param_ID, "t7", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);
			break;
		}

		case Center1:
		{
			strncpy(param_ID, "t8", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);
			break;
		}

		case Center2:
		{
			strncpy(param_ID, "t9", 3);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

			break;
		}

		case Center3:
		{
			strncpy(param_ID, "t10", 4);
			len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);
			break;
		}

		case Center4:
        {
            strncpy(param_ID, "t11", 4);
            len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

            break;
        }

		case Center5:
        {
            strncpy(param_ID, "t12", 4);
            len = sprintf((char *)txBuffer, "%s.pco=%d", param_ID, Param_Value);

            break;
        }

		default:
			break;
	}
	UART_SendString(&huart2, &BuffUART2, txBuffer, strlen(txBuffer));

	//if (HAL_UART_Transmit(&LCDUart, txBuffer, strlen(txBuffer), TIMEOUT) != HAL_OK)
	//{
	//	return failure;
	//}

	UART_SendString(&huart2, &BuffUART2, (uint8_t *)endCmd, 3);
	// Send termination bytes (0xFF 0xFF 0xFF)
	//if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
	//{
	//	return failure;
	//}


	return success;
}

returnStatus configBgcolorLCD(lcdCmdParam_id Param_ID, int Param_Value)
{
	uint8_t * txBuffer = malloc(30 * sizeof(char));  // Command buffer
	int len = 0;
	char param_ID[10];

	switch (Param_ID)
	{
		case Left1:
		{
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);

			break;
		}

		case Left2:
        {
            strncpy(param_ID, "t1", 3);
            len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


            break;
        }

		case Left3:
		{
			strncpy(param_ID, "t2", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);

			break;
		}

		case Left4:
		{
			strncpy(param_ID, "t3", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);

			break;
		}

		case Right1:
		{
			strncpy(param_ID, "t4", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Right2:
		{
			strncpy(param_ID, "t5", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Right3:
		{
			strncpy(param_ID, "t6", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Right4:
		{
			strncpy(param_ID, "t7", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Center1:
		{
			strncpy(param_ID, "t8", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Center2:
		{
			strncpy(param_ID, "t9", 3);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Center3:
		{
			strncpy(param_ID, "t10", 4);
			len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


			break;
		}

		case Center4:
        {
            strncpy(param_ID, "t11", 4);
            len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


            break;
        }

		case Center5:
        {
            strncpy(param_ID, "t12", 4);
            len = sprintf((char *)txBuffer, "%s.bco=%d", param_ID, Param_Value);


            break;
        }

		default:
			break;
	}
	UART_SendString(&huart2, &BuffUART2, txBuffer, strlen(txBuffer));

	//if (HAL_UART_Transmit(&LCDUart, txBuffer, strlen(txBuffer), TIMEOUT) != HAL_OK)
	//{
	//	return failure;
	//}

	UART_SendString(&huart2, &BuffUART2, (uint8_t *)endCmd, 3);
	// Send termination bytes (0xFF 0xFF 0xFF)
	//if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
	//{
	//	return failure;
	//}
	return success;
}

//returnStatus ReadParamLCD(lcdCmdParam_id Param_ID, void * Param_Value)
//{
//    char txBuffer[30], param_ID[10];
//	switch (Param_ID)
//	{
//		case param_number:
//			strncpy(param_ID, "n0", 3);
//			break;
//		case param_float:
//			strncpy(param_ID, "x0", 3);
//			break;
//		default:
////    			strncpy(param_ID, "0", 2);
//			break;
//	}
//
//    // Command to request the value of the parameter from the Nextion display
//    int len = sprintf((char *)txBuffer, "get %s.val", (void *) param_ID); // Request value of Param_ID
//    HAL_UART_Transmit(&huart3, (uint8_t *)txBuffer, len, TIMEOUT);      // Send request to Nextion
//    HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);  // Send termination bytes
//
//    // Wait and receive the response from Nextion (the response format depends on your Nextion configuration)
//    uint8_t rxBuffer[8];  // Assuming a response fits in 8 bytes
//    if (HAL_UART_Receive(&huart3, rxBuffer, 8, TIMEOUT) != HAL_OK)  // Read response
//    {
//        return failure;  // Error in receiving data
//    }
//
//    *Param_Value = (rxBuffer[4] << 24) |
//                   (rxBuffer[3] << 16) |
//                   (rxBuffer[2] << 8)  |
//                   (rxBuffer[1]);
//
//    return success;  // Success
//}
