#pragma once
/*! \file NMEA0183.h					
\brief NMEA 0183 parser objects and encode/decode functions					
*/ 					
					
#ifndef NMEA0183_H					
#define NMEA0183_H					
					
#include <stdint.h>					
#include "stm32f1xx_hal.h"					
					
///////////////////////////////////////////////					
//				DEFINITIONS	
///////////////////////////////////////////////					
// Data Output Requests
#define NMEA_RESET_STATUS				0x20
#define NMEA_COMM_XCEIVER_STATUS		0x35
#define NMEA_COMM_SW_VERSION			0x36
#define NMEA_COMM_VOL_SQUELCH			0x71
#define NMEA_COMM_GAIN_SIDETONE			0x72

#define MAX_NMEA_MSG_LEN				25

///////////////////////////////////////////////					
//				GLOBALS	
///////////////////////////////////////////////		
typedef enum {
	REQ_RESETSTATUS,
	REQ_XCEIVERSTATUS,
	REQ_SWVERSION,
	REQ_VOL_SQL,
	REQ_GAIN_SIDETONE,
	SET_ACTIVE_FREQ,
	SET_STANDBY_FREQ,
	SET_VOL_SQL,
	SET_GAIN_SIDETONE,
	SET_STANDBY_VOR_LOC_FREQ,
	SET_ACTIVE_VOR_LOC_FREQ,
	SET_OBS,

} commmsgtype;

typedef struct {
	uint8_t portnum;
	commmsgtype msgtype;
} CommParam_t;

extern uint8_t nmea_message[MAX_NMEA_MSG_LEN];
extern CommParam_t CommParams;

///////////////////////////////////////////////
//				PROTOTYPES
///////////////////////////////////////////////
void SendMsg2Port(uint8_t portnum, commmsgtype msgtype);

#endif // END OF FILE					
