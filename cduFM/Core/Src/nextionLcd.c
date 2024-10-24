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

extern UART_HandleTypeDef LCDUart;

//lcdCmdDisp_id screen = lcdDisp_home;

screen = lcdDisp_home;

uint16_t TIMEOUT = 1000;
uint8_t endCmd[3] = {0xff, 0xff, 0xff};
//uint8_t pageResponse[2] = {0, 0, 0};

returnStatus InitializeLCD(void)
{
	//initialization of lcd, like setting values to default or some like displaying main page etc
	if(DispHomeScreen()){
		return success;
	}
	else
	{
		return failure;
	}
}

//returnStatus ChangePage(lcdCmdPage_id Page_ID)
//{
//	char txBuffer[20], page_ID[10];
//	switch (Page_ID)
//	{
//		case lcdPage_main:
//			strncpy(page_ID, "0", 2);
//			break;
//		case lcdPage_nav:
//			strncpy(page_ID, "1", 2);
//			break;
//		default:
//			strncpy(page_ID, "0", 2);
//			break;
//	}
//	int len = sprintf((char *)txBuffer, "page %s", page_ID);
//	HAL_UART_Transmit(&LCDUart, (uint8_t *)txBuffer, len, TIMEOUT);
//	HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10);
//
////	pageResponse = HAL_UART_Receive_IT(&huart3, rxBuffer, 1); //receiving 1 byte only of output(hex): 02 00 00 00
//	HAL_UART_Receive_IT(&LCDUart, pageResponse, 1); //receiving 1 byte only of output(hex): 02 00 00 00
//
//	if (pageResponse == Page_ID)
//   		return success;
//   	else
//   		return failure;
//}

returnStatus DispNavScreen(void)
{
	UpdateParamLCD(Left1, "S 110.10");
	UpdateParamLCD(Left2, "");
	UpdateParamLCD(Left3, "VOL");
	UpdateParamLCD(Left4, "PROG");
	UpdateParamLCD(Center1, "NAV");
	UpdateParamLCD(Center2, "A 118.00");
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
	UpdateParamLCD(Left2, "ADF");
	UpdateParamLCD(Left3, "TACAN");
	//UpdateParamLCD(Left4, "M/B");
	UpdateParamLCD(Center1, "HOME");
	UpdateParamLCD(Center2, "");
	UpdateParamLCD(Center3, "");
	UpdateParamLCD(Center4, "");
	UpdateParamLCD(Center5, "");
	UpdateParamLCD(Right1, "HF");
	UpdateParamLCD(Right2, "VHF");
	UpdateParamLCD(Right3, "UHF");
	UpdateParamLCD(Right4, "");

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
			if (HAL_UART_Transmit(&LCDUart, txBuffer, strlen(txBuffer), TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Left2:
			{
				strncpy(param_ID, "t1", 3);
				len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
				if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
				{
					return failure;
				}

				// Send termination bytes (0xFF 0xFF 0xFF)
				if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
				{
					return failure;
				}

				break;
			}
		case Left3:
		{
			strncpy(param_ID, "t2", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Left4:
		{
			strncpy(param_ID, "t3", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Right1:
		{
			strncpy(param_ID, "t4", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Right2:
		{
			strncpy(param_ID, "t5", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Right3:
		{
			strncpy(param_ID, "t6", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}
			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Right4:
		{
			strncpy(param_ID, "t7", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Center1:
		{
			strncpy(param_ID, "t8", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Center2:
		{
			strncpy(param_ID, "t9", 3);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Center3:
		{
			strncpy(param_ID, "t10", 4);
			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
			if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case Center4:
			{
				strncpy(param_ID, "t11", 4);
				len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
				if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
				{
					return failure;
				}

				// Send termination bytes (0xFF 0xFF 0xFF)
				if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
				{
					return failure;
				}

				break;
			}
		case Center5:
			{
				strncpy(param_ID, "t12", 4);
				len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, Param_Value);
				if (HAL_UART_Transmit(&LCDUart, txBuffer, len, TIMEOUT) != HAL_OK)
				{
					return failure;
				}

				// Send termination bytes (0xFF 0xFF 0xFF)
				if (HAL_UART_Transmit(&LCDUart, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
				{
					return failure;
				}

				break;
			}

//		// for nav.................................................................................
//		case nav_param_afreq:
//		{
//			strncpy(param_ID, "x0", 2);
//			// Cast Param_Value to float and send it (as a whole number)
//			float value = *(float *)Param_Value;
//			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
//			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
//			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
//			{
//				return failure;
//			}
//
//			// Send termination bytes (0xFF 0xFF 0xFF)
//			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
//			{
//				return failure;
//			}
//
//			break;
//		}
//		case nav_param_sfreq:
//		{
//			strncpy(param_ID, "x1", 2);
//			float value = *(float *)Param_Value;
//			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
//			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
//			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
//			{
//				return failure;
//			}
//			// Send termination bytes (0xFF 0xFF 0xFF)
//			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
//			{
//				return failure;
//			}
//			break;
//		}
//		case nav_param_vol:
//		{
//			strncpy(param_ID, "n0", 2);
//			// Cast Param_Value to integer and send as .val
//			int32_t param_value = *(int32_t *)Param_Value;
////			len = sprintf((char *)buffer, "%s.vvs1=3", param_ID);
////			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
////			HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
//			len = sprintf((char *)txBuffer, "%s.val=%d", param_ID, param_value);
//			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
//			{
//				return failure;
//			}
//			// Send termination bytes (0xFF 0xFF 0xFF)
//			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
//			{
//				return failure;
//			}
//			break;
//		}
//		case nav_param_obs:
//		{
//			strncpy(param_ID, "n1", 2);
//			// Cast Param_Value to integer and send as .val
//			int32_t param_value = *(int32_t *)Param_Value;
////			len = sprintf((char *)buffer, "%s.vvs1=3", param_ID);
////			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
////			HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
//			len = sprintf((char *)txBuffer, "%s.val=%d", param_ID, param_value);
//			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
//			{
//				return failure;
//			}
//			// Send termination bytes (0xFF 0xFF 0xFF)
//			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
//			{
//				return failure;
//			}
//			break;
//		}
//
//		// for com.................................................................................
//		case com_param_afreq:
//		{
//			strncpy(param_ID, "x0", 2);
//			// Cast Param_Value to float and send it (as a whole number)
//			float value = *(float *)Param_Value;
//			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
//			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
//			break;
//		}
//		case com_param_sfreq:
//		{
//			strncpy(param_ID, "x1", 2);
//			// Cast Param_Value to float and send it (as a whole number)
//			float value = *(float *)Param_Value;
//			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
//			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
//			break;
//		}
//
//
////		case param_txt: {
////			strncpy(param_ID, "t0", 2);
////			// Cast Param_Value to string and send as .txt
////			char *string_value = (char *)Param_Value;
////			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, string_value);
////			break;
////		}

		default:
			break;
	}

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
