/*
 * nextionLcd.c
 *
 *  Created on: Oct 8, 2024
 *      Author: Dell
 */

#include "nextionLcd.h"
#include "stdbool.h"


uint16_t TIMEOUT = 1000;
uint8_t endCmd[3] = {0xff, 0xff, 0xff};
uint8_t pageResponse = 0;

returnStatus InitializeLCD(void)
{
	//initialization of lcd, like setting values to default or some like displaying main page etc
	ChangePage(lcdPage_main);
}

returnStatus ChangePage(lcdCmdPage_id Page_ID)
{
	char txBuffer[20], page_ID[10];
	switch (Page_ID) {
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
	

	int len = sprintf((char *)txBuffer, "page %s", pageName);
	HAL_UART_Transmit(&huart4, (uint8_t *)txBuffer, len, TIMEOUT);
	HAL_UART_Transmit(&huart4, (uint8_t *)endCmd, 3, TIMEOUT/10);

//	pageResponse = HAL_UART_Receive_IT(&huart4, rxBuffer, 1); //receiving 1 byte only of output(hex): 02 00 00 00
	HAL_UART_Receive_IT(&huart4, pageResponse, 1); //receiving 1 byte only of output(hex): 02 00 00 00

	if (pageResponse == Page_ID)
   		return success;
   	else
   		return failure;
}

returnStatus nextion_sendstr(char *id, char *string)
{
	char buffer[50];
	int len = sprintf((char *)buffer, "%s.txt=\"%s\"",id, string);
	HAL_UART_Transmit(&huart4, (uint8_t *)buffer, len, TIMEOUT);
	HAL_UART_Transmit(&huart4, (uint8_t *)endCmd, 3, TIMEOUT/10);
}

void nextion_sendnum(char *obj, int16_t num)
{
	uint8_t *buffer = malloc(30 * sizeof(char));//30 bytes of buffer
	int len = sprintf((char *)buffer, "%s.val=%d", obj, num);
	HAL_UART_Transmit(&huart4, (uint8_t *)buffer, len, TIMEOUT);
	HAL_UART_Transmit(&huart4, (uint8_t *)endCmd, 3, TIMEOUT/10);
	free(buffer);
}

void nextion_sendfloat(char *obj, float num, uint16_t dp)
{
	// covert first to integer
	int16_t number = num*(pow(10,dp));
	uint8_t *buffer = malloc(30 * sizeof(char));//30 bytes of buffer

	int len = sprintf((char *)buffer, "%s.vvs1=%d", obj, dp);
	HAL_UART_Transmit(&huart4, (uint8_t *)buffer, len, TIMEOUT);
	HAL_UART_Transmit(&huart4, (uint8_t *)endCmd, 3, TIMEOUT/10);

	len = sprintf((char *)buffer, "%s.val=%d", obj, number);
	HAL_UART_Transmit(&huart4, (uint8_t *)buffer, len, TIMEOUT);
	HAL_UART_Transmit(&huart4, (uint8_t *)endCmd, 3, TIMEOUT/10);
	free(buffer);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    	if (huart->Instance == UART4)
    	{
    		HAL_UART_Receive_IT(&huart4, pageResponse, 1); // Restart UART5 reception
    	}

}
