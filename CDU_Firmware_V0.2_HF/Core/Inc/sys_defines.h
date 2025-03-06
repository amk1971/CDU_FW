/*
 * sys_defines.h
 *
 *  Created on: Mar 6, 2025
 *      Author: LENOVO
 */

#ifndef INC_SYS_DEFINES_H_
#define INC_SYS_DEFINES_H_

#include "main.h"

#define CAN_MESSAGE_LENGTH	8

typedef enum {
	NAV = 0,
	ADF,
	TACAN,
	HF,
	VHF,
	UHF,
	class_ID_Count
}Class_Id;

typedef enum {
	S_FREQ,
	A_FREQ,
	VOLUME,
	Health,					//Health Check message
	MODE,
	V_T_S,
	RESET_STATUS,
	COM_ERROR,
    INVALID
}Message_Id;

typedef struct {
	Class_Id eID;			//Equipment ID
	Message_Id mID;			//message ID

} Comm_bus_Frame;

typedef union
{
    struct {
        uint8_t Request_Reply : 1;
        uint8_t power_status  : 1;
        uint8_t Squelch       : 1;
        uint8_t TestTone      : 1;
        uint8_t Receiver_Mode : 1;
        uint8_t Volume;
        uint16_t Checksum;         // 2 bytes
        float   Frequency;         // 4 bytes
    } fields;
    Comm_bus_Frame data;			// data in Comm bus frame
    uint64_t RX_TX_FRAME;  // Full 8 bytes
} FRAME;


#endif /* INC_SYS_DEFINES_H_ */
