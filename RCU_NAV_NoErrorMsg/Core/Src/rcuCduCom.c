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

#define ACTIVE_FREQ_MESSAGE  "$PATCN27%c%cN"
#define STANDBY_FREQ_MESSAGE "$PATCN28%c%cN"
#define VOLUME_MESSAGE      "$PATCN73%c"
#define OBS_MESSAGE         "$PATCN34%03d"

#define ACTIVE_FREQ_IDENTIFIER  "27"
#define STANDBY_FREQ_IDENTIFIER "28"
#define VOLUME_IDENTIFIER       "73"
#define OBS_IDENTIFIER  	    "34"

// N: NAV, C: COM, A: ADF, T: TACAN
#define RCU_DESIG				'N'


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
        snprintf((char*)str2, sizeof(str2), ACTIVE_FREQ_MESSAGE, m, k);
        char end2[3];
        checksum((char*)str2, end2);
        concatTwoChars((char*)str2, end2);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 1) {
        char m = str[0];
        char k = str[1];
        snprintf((char*)str2, sizeof(str2), STANDBY_FREQ_MESSAGE, m, k);
        char end3[3];
        checksum((char*)str2, end3);
        concatTwoChars((char*)str2, end3);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 2) {
        char v = str[0];
        snprintf((char*)str2, sizeof(str2), VOLUME_MESSAGE, v);
        char end4[3];
        checksum((char*)str2, end4);
        concatTwoChars((char*)str2, end4);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 3) {
        snprintf((char*)str2, sizeof(str2), OBS_MESSAGE, obs);
        char end5[3];
        checksum((char*)str2, end5);
        concatTwoChars((char*)str2, end5);
        concatTwoChars((char*)str2, crlf);
    }

    HAL_UART_Transmit(&huart5, str2, strlen((char*)str2), 500);
    HAL_Delay(100);
}

bool Test_Pattern (char * msg, char * pattern) {
	if(*msg != *pattern) return false;
	if(*(msg+1) != *(pattern+1)) return false;
	return true;
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
			  if(rxmsgcdu[5] == RCU_DESIG)
			  {
				  if(Test_Pattern(&rxmsgcdu[6], ACTIVE_FREQ_IDENTIFIER))
				  {
					  float rxfreq;
					  rxfreq = (rxmsgcdu[8] + 48) + ((rxmsgcdu[9] - 48) * .025); // active frequency
					  if (rxfreq != freq)
					  {
						  freq = rxfreq;
						  MHz = rxmsgcdu[8] + 48;
						  KHz = (rxmsgcdu[9] - 48) * 25;
	  //					  char Mfinal[3] = {rxmsgcdu[8],rxmsgcdu[9],0};
	  //					  Sender(Mfinal, 0); //set active
					  }
					  else
					  {

					  }
				  }
				  else if(Test_Pattern(&rxmsgcdu[6], STANDBY_FREQ_IDENTIFIER))// standby frequency, rxmsgcdu[6] == '2' && rxmsgcdu[7] == '8'
				  {
					  float rxfreq;
					  rxfreq = (rxmsgcdu[8] + 48) + ((rxmsgcdu[9] - 48) * .025);
					  if (rxfreq != Standby)
					  {
						  Standby = rxfreq;
						  SM = rxmsgcdu[8] + 48;
						  SK = (rxmsgcdu[9] - 48) * 25;
					  }
					  else
					  {

					  }

				  }
				  else if(Test_Pattern(&rxmsgcdu[6], OBS_IDENTIFIER))// obs value
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
				  else if(Test_Pattern(&rxmsgcdu[6], VOLUME_IDENTIFIER))// volume level
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
//			  else if(rxmsgcdu[4] == COM)
//			  {
//				  //hey, COM message receive here...
//			  }
		  }
	}
}
