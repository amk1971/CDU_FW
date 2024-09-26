/*
 * rcuCduCom.c
 *
 *  Created on: Sep 23, 2024
 *      Author: Dell
 */

#include <stdbool.h>
#include "rcuCduCom.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart5;

uint8_t responseCDU = 0;

extern float freq;
extern volatile int KHz;
extern volatile int MHz ;
extern float Standby ;
extern volatile int SK ;
extern volatile int SM ;
extern volatile int vol;
extern volatile int obs;

unsigned char rxbuffcdu[1];
char rxmsgcdu[25];
int rxcountcdu = 0;
bool rxfreecdu = false;

void Sender2cdu(const char * str, int mode) { //TODO				construct and send a message (likely over UART)
    char crlf[] = {'\r','\n', 0};
    uint8_t str2[25]; // Array to hold the constructed string in ASCII

    memset(str2, 0, sizeof(str2)); // Initialize the array with zeros

    if (mode == 0) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), "$PATNV27%c%cN", m, k);
        char end2[3];
        checksum((char*)str2, end2);
        concatTwoChars((char*)str2, end2);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 1) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), "$PATNV28%c%cN", m, k);
        char end3[3];
        checksum((char*)str2, end3);
        concatTwoChars((char*)str2, end3);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 2) {
        char v = str[0];
        snprintf((char*)str2, sizeof(str2), "$PATNV73%c", v);
        char end4[3];
        checksum((char*)str2, end4);
        concatTwoChars((char*)str2, end4);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 3) {
        snprintf((char*)str2, sizeof(str2), "$PATNV34%03d", obs);
        char end5[3];
        checksum((char*)str2, end5);
        concatTwoChars((char*)str2, end5);
        concatTwoChars((char*)str2, crlf);
    }

    HAL_UART_Transmit(&huart5, str2, strlen((char*)str2), 500);
    HAL_Delay(100);
}

void task3_init(void const * argument)
{ // data synchronization between cdu and rcu


//	unsigned long time = 0;

	for(;;)
	{
		  //continuously receive data through UART=====================================
//		  response = HAL_UART_Receive(&huart5, rxbuffcdu, 1, 500);
//		  HAL_UART

		  //printf("%u\n",rxbuffcdu);
		//HAL_Delay(100);
		  if(responseCDU==HAL_OK) //if transfer is successful
		  {
			  responseCDU = 2;
//			  rxmsgcdu[rxcountcdu] = rxbuffcdu[0];
//			  rxcountcdu++;
			  if (rxcountcdu > 24) {
				  rxcountcdu = 0;
			  }
//			  if (rxbuffcdu[0] == '$')
//			  {
//				 time = millis();
//			  }

			  if (rxbuffcdu[0] == '\n')
			  {
//				  unsigned int duration = millis() - time;
//				  print();
				  rxmsgcdu[rxcountcdu] = 0;
				  rxcountcdu = 0;
				  rxfreecdu = true;
			  }
		  }
//		  else
//		  {
//			  // no message in 1 seconds
//			  __HAL_UART_FLUSH_DRREGISTER(&huart5);  // Clear the UART Data Register
//			  rxcountcdu = 0;
//		  }
		  if(rxfreecdu)
		  {
			  rxfreecdu = false;
			  if(rxmsgcdu[6] == '2' && rxmsgcdu[7] == '7')
			  {
				  float rxfreq;
				  rxfreq = (rxmsgcdu[8] + 48) + ((rxmsgcdu[9] - 48) * .025); // active frequency
				  if (rxfreq != freq)
				  {
					  freq = rxfreq;
					  MHz = rxmsgcdu[8] + 48;
					  KHz = (rxmsgcdu[9] - 48) * .025;
				  }
				  else
				  {

				  }
			  }
			  else if(rxmsgcdu[6] == '2' && rxmsgcdu[7] == '8')// standby frequency
			  {
				  float rxfreq;
				  rxfreq = (rxmsgcdu[8] + 48) + ((rxmsgcdu[9] - 48) * .025);
				  if (rxfreq != Standby)
				  {
					  Standby = rxfreq;
					  SM = rxmsgcdu[8] + 48;
					  SK = (rxmsgcdu[9] - 48) * .025;
				  }
				  else
				  {

				  }

			  }
			  else if(rxmsgcdu[6] == '3' && rxmsgcdu[7] == '4')// obs value
			  {
				  int i, val = 0;
				  for (i = 8; i <= 10; i++) {
				          val = val * 10 + (rxmsgcdu[i] - '0');
				      }
				  if (val != obs)
				  {
					  obs = val;
				  }
				  else
				  {
					  //do nothing
				  }


			  }
			  else if(rxmsgcdu[6] == '7' && rxmsgcdu[7] == '3')// volume level
			  {
				  int val;
				  val = rxmsgcdu[8] - '0'; // subtracting 48
				  if (val != vol)
				  {
					  vol = val;
				  }
				  else
				  {
					  //do nothing
				  }
			  }
		  }
	}
}
