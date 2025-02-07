/*! \file NMEA0183.c					
\brief NMEA 0183 parser objects and encode/decode functions					
*/ 					
					
					
#include <stdio.h>
#include <stdint.h>	
#include "main.h"
#include "NMEA0183.h"			
					

///////////////////////////////////////////////					
//				FUNCTIONS	
///////////////////////////////////////////////
//-------------------------------------------------------------------								
//  SEND MESSAGE TO SERIAL PORT								
//								
/*! \fn void SendMsg2Port(uint8_t portnum,commmsgtype msgtype)								
\brief Send a random NMEA0183 message to the Tranceiver								
								
Post serial port number and message type code to queue (actual message send by other task)								
\return None								
*/								
//-------------------------------------------------------------------								
void SendMsg2Port(uint8_t portnum /* Port 1/2/3 */, commmsgtype msgtype /* message type code */)								
{								
	CommParams.portnum = portnum;
	CommParams.msgtype = msgtype;
	xQueueSendToBack(PORT_queue, (void*)&CommParams, 10);
}								
