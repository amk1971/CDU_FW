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

extern UART_HandleTypeDef huart3;



uint16_t TIMEOUT = 1000;
uint8_t endCmd[3] = {0xff, 0xff, 0xff};
uint8_t pageResponse = 0;

returnStatus InitializeLCD(void)
{
	//initialization of lcd, like setting values to default or some like displaying main page etc
	if(ChangePage(lcdPage_main) == success){
		return success;
	}
	else
	{
		return failure;
	}
}

returnStatus ChangePage(lcdCmdPage_id Page_ID)
{
	char txBuffer[20], page_ID[10];
	switch (Page_ID)
	{
		case lcdPage_main: 
			strncpy(page_ID, "0", 2);
			break;
		case lcdPage_nav:
			strncpy(page_ID, "1", 2);
			break;
		default:
			strncpy(page_ID, "0", 2);
			break;
	}
	int len = sprintf((char *)txBuffer, "page %s", page_ID);
	HAL_UART_Transmit(&huart3, (uint8_t *)txBuffer, len, TIMEOUT);
	HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);

//	pageResponse = HAL_UART_Receive_IT(&huart3, rxBuffer, 1); //receiving 1 byte only of output(hex): 02 00 00 00
	HAL_UART_Receive_IT(&huart3, pageResponse, 1); //receiving 1 byte only of output(hex): 02 00 00 00

	if (pageResponse == Page_ID)
   		return success;
   	else
   		return failure;
}

//returnStatus nextion_sendstr(char *id, char *string)
//{
//	char buffer[50];
//	int len = sprintf((char *)buffer, "%s.txt=\"%s\"",id, string);
//	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
//	HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
//}
//
//void nextion_sendnum(char *obj, int16_t num)
//{
//	uint8_t *buffer = malloc(30 * sizeof(char));//30 bytes of buffer
//	int len = sprintf((char *)buffer, "%s.val=%d", obj, num);
//	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
//	HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
//	free(buffer);
//}
//
//void nextion_sendfloat(char *obj, float num, uint16_t dp)
//{
//	// covert first to integer
//	int16_t number = num*(pow(10,dp));
//	uint8_t *buffer = malloc(30 * sizeof(char));//30 bytes of buffer
//
//	int len = sprintf((char *)buffer, "%s.vvs1=%d", obj, dp);
//	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
//	HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
//
//	len = sprintf((char *)buffer, "%s.val=%d", obj, number);
//	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
//	HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
//	free(buffer);
//}
//

returnStatus UpdateNavScreen(void * Parameters)
{
	UpdateParamLCD(Center1, "    NAV      ");
	UpdateParamLCD(Right1, "P1");

}

returnStatus UpdateParamLCD(lcdCmdParam_id Param_ID, char * Param_Value)
{
	uint8_t txBuffer = malloc(30 * sizeof(char));  // Command buffer
	int len = 0;
	char param_ID[10];

	// Handle the parameter based on its type (integer, float, or string)
	switch (Param_ID) {

		// for nav.................................................................................
		case nav_param_afreq:
		{
			strncpy(param_ID, "x0", 2);
			// Cast Param_Value to float and send it (as a whole number)
			float value = *(float *)Param_Value;
			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}

			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}

			break;
		}
		case nav_param_sfreq:
		{
			strncpy(param_ID, "x1", 2);
			float value = *(float *)Param_Value;
			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}
			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}
			break;
		}
		case nav_param_vol:
		{
			strncpy(param_ID, "n0", 2);
			// Cast Param_Value to integer and send as .val
			int32_t param_value = *(int32_t *)Param_Value;
//			len = sprintf((char *)buffer, "%s.vvs1=3", param_ID);
//			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
//			HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
			len = sprintf((char *)txBuffer, "%s.val=%d", param_ID, param_value);
			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}
			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}
			break;
		}
		case nav_param_obs:
		{
			strncpy(param_ID, "n1", 2);
			// Cast Param_Value to integer and send as .val
			int32_t param_value = *(int32_t *)Param_Value;
//			len = sprintf((char *)buffer, "%s.vvs1=3", param_ID);
//			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, len, TIMEOUT);
//			HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10);
			len = sprintf((char *)txBuffer, "%s.val=%d", param_ID, param_value);
			if (HAL_UART_Transmit(&huart3, txBuffer, len, TIMEOUT) != HAL_OK)
			{
				return failure;
			}
			// Send termination bytes (0xFF 0xFF 0xFF)
			if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK)
			{
				return failure;
			}
			break;
		}

		// for com.................................................................................
		case com_param_afreq:
		{
			strncpy(param_ID, "x0", 2);
			// Cast Param_Value to float and send it (as a whole number)
			float value = *(float *)Param_Value;
			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
			break;
		}
		case com_param_sfreq:
		{
			strncpy(param_ID, "x1", 2);
			// Cast Param_Value to float and send it (as a whole number)
			float value = *(float *)Param_Value;
			int32_t param_value = value*(pow(10,3));//10^3, for converting into integer
			len = sprintf((char *)txBuffer, "%s.val=%ld", param_ID, param_value);
			break;
		}


//		case param_txt: {
//			strncpy(param_ID, "t0", 2);
//			// Cast Param_Value to string and send as .txt
//			char *string_value = (char *)Param_Value;
//			len = sprintf((char *)txBuffer, "%s.txt=\"%s\"", param_ID, string_value);
//			break;
//		}
		default:
			break;
	}

//	// Transmit the command over UART
//	if (HAL_UART_Transmit(&huart3, buffer, len, TIMEOUT) != HAL_OK) {
//		return failure;
//	}
//
//	// Send termination bytes (0xFF 0xFF 0xFF)
//	if (HAL_UART_Transmit(&huart3, (uint8_t *)endCmd, 3, TIMEOUT/10) != HAL_OK) {
//		return failure;
//	}

	return success;
}
//
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    	if (huart->Instance == USART3)
    	{
    		HAL_UART_Receive_IT(&huart3, pageResponse, 1); // Restart UART5 reception
    	}

}
