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

#define ACTIVE_FREQ_MESSAGE      "$PATCV42%c%cN"
#define STANDBY_FREQ_MESSAGE     "$PATCV29%c%cN"
#define VOLUME_SQUELCH_MESSAGE   "$PATCV71%c%c"
#define MICGAIN_SIDETONE_MESSAGE "$PATCV72%c%c"

#define ACTIVE_FREQ_IDENTFIER 		"42"
#define STANDBY_FREQ_IDENTFIER 		"29"
#define VOLUME_SQUELCH_IDENTIFER    "71"
#define MICGAIN_SIDETONE_IDENTIFIER "72"

// N: NAV, C: COM, A: ADF, T: TACAN
#define RCU_DESIG				 'C'


uint8_t responseCDU = 0;

extern float freq;
extern volatile int MHz;
extern volatile int KHz ;
extern float Standby ;
extern volatile int SK ;
extern volatile int SM ;
extern volatile int vol;
extern volatile int squelch;
extern volatile int micgain;
extern volatile int sidetone;

unsigned char rxbuffcdu[1];
char rxmsgcdu[25];
int rxcountcdu = 0;
bool rxfreecdu = false;

void Sender2cdu(const char * str, int mode) { //TODO
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
        char s = str[1];
        snprintf((char*)str2, sizeof(str2), VOLUME_SQUELCH_MESSAGE, v, s);
        char end4[3];
        checksum((char*)str2, end4);
        concatTwoChars((char*)str2, end4);
        concatTwoChars((char*)str2, crlf);
    } else if (mode == 3) {
        char m = str[0];
        char s = str[1];
        snprintf((char*)str2, sizeof(str2), MICGAIN_SIDETONE_MESSAGE, m, s);
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

int response;
//	unsigned long time = 0;

	for(;;)
	{
		  //continuously receive data through UART=====================================
		//  response = HAL_UART_Receive(&huart5, rxbuffcdu, 1, 500);
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
			  if(rxmsgcdu[4] == RCU_DESIG)
			  {
				  if(Test_Pattern(&rxmsgcdu[6], ACTIVE_FREQ_IDENTFIER))
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
				  else if(Test_Pattern(&rxmsgcdu[6], STANDBY_FREQ_IDENTFIER))// standby frequency
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
				  else if(Test_Pattern(&rxmsgcdu[6], VOLUME_SQUELCH_IDENTIFER))// vol, squelch value
				  {
					  int val = 0;

					  val = rxmsgcdu[8] - '0'; // subtracting 48
					  if (val != vol)
					  {
						  vol = val;
					  }
					  else
					  {
						  //do nothing
					  }

					  val = rxmsgcdu[9] - '0'; // subtracting 48
					  if (val != squelch)
					  {
						  squelch = val;
					  }
					  else
					  {
						  //do nothing
					  }




				  }
				  else if(Test_Pattern(&rxmsgcdu[6], MICGAIN_SIDETONE_IDENTIFIER))// micGain, sideTone level, rxmsgcdu[6] == '7' && rxmsgcdu[7] == '2'
				  {
					  int val = 0;

					  val = rxmsgcdu[8] - '0'; // subtracting 48
					  if (val != micgain)
					  {
						  micgain = val;
					  }
					  else
					  {
						  //do nothing
					  }

					  val = rxmsgcdu[9] - '0'; // subtracting 48
					  if (val != sidetone)
					  {
						  sidetone = val;
					  }
					  else
					  {
						  //do nothing
					  }
				  }

			  }

		  }
	}
}
