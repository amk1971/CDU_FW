#pragma once
/*! \file RCU_flash_mgr.h					
\brief RCU Flash storage manager and access methods					
*/ 					
					
#ifndef RCU_FLASH_MGR_H					
#define RCU_FLASH_MGR_H					
									
#include <stdint.h>					
					
///////////////////////////////////////////////					
//				DEFINITIONS	
///////////////////////////////////////////////	
#define RECORD_NAME_LEN	7				
#define RECORD_LEN	20
#define SPIF_DUMMY_BYTE 0xA5
#define SPIF_CMD_READSFDP 0x5A
#define SPIF_CMD_ID 0x90
#define SPIF_CMD_JEDECID 0x9F
#define SPIF_CMD_UNIQUEID 0x4B
#define SPIF_CMD_WRITEDISABLE 0x04
#define SPIF_CMD_READSTATUS1 0x05
#define SPIF_CMD_READSTATUS2 0x35
#define SPIF_CMD_READSTATUS3 0x15
#define SPIF_CMD_WRITESTATUSEN 0x50
#define SPIF_CMD_WRITESTATUS1 0x01
#define SPIF_CMD_WRITESTATUS2 0x31
#define SPIF_CMD_WRITESTATUS3 0x11
#define SPIF_CMD_WRITEENABLE 0x06
#define SPIF_CMD_ADDR4BYTE_EN 0xB7
#define SPIF_CMD_ADDR4BYTE_DIS 0xE9
#define SPIF_CMD_PAGEPROG3ADD 0x02
#define SPIF_CMD_PAGEPROG4ADD 0x12
#define SPIF_CMD_READDATA3ADD 0x03
#define SPIF_CMD_READDATA4ADD 0x13
#define SPIF_CMD_FASTREAD3ADD 0x0B
#define SPIF_CMD_FASTREAD4ADD 0x0C
#define SPIF_CMD_SECTORERASE3ADD 0x20
#define SPIF_CMD_SECTORERASE4ADD 0x21
#define SPIF_CMD_BLOCKERASE3ADD 0xD8
#define SPIF_CMD_BLOCKERASE4ADD 0xDC
#define SPIF_CMD_CHIPERASE1 0x60
#define SPIF_CMD_CHIPERASE2 0xC7
#define SPIF_CMD_SUSPEND 0x75
#define SPIF_CMD_RESUME 0x7A
#define SPIF_CMD_POWERDOWN 0xB9
#define SPIF_CMD_RELEASE 0xAB
#define SPIF_CMD_FRAMSERNO 0xC3

#define SPIF_CMD_RESET1 0x66
#define SPIF_CMD_RESET2 0x99



#define SPIF_STATUS1_BUSY (1 << 0)
#define SPIF_STATUS1_WEL (1 << 1)
#define SPIF_STATUS1_BP0 (1 << 2)
#define SPIF_STATUS1_BP1 (1 << 3)
#define SPIF_STATUS1_BP2 (1 << 4)
#define SPIF_STATUS1_TP (1 << 5)
#define SPIF_STATUS1_SEC (1 << 6)
#define SPIF_STATUS1_SRP0 (1 << 7)

#define SPIF_STATUS2_SRP1 (1 << 0)
#define SPIF_STATUS2_QE (1 << 1)
#define SPIF_STATUS2_RESERVE1 (1 << 2)
#define SPIF_STATUS2_LB0 (1 << 3)
#define SPIF_STATUS2_LB1 (1 << 4)
#define SPIF_STATUS2_LB2 (1 << 5)
#define SPIF_STATUS2_CMP (1 << 6)
#define SPIF_STATUS2_SUS (1 << 7)

#define SPIF_STATUS3_RESERVE1 (1 << 0)
#define SPIF_STATUS3_RESERVE2 (1 << 1)
#define SPIF_STATUS3_WPS (1 << 2)
#define SPIF_STATUS3_RESERVE3 (1 << 3)
#define SPIF_STATUS3_RESERVE4 (1 << 4)
#define SPIF_STATUS3_DRV0 (1 << 5)
#define SPIF_STATUS3_DRV1 (1 << 6)
#define SPIF_STATUS3_HOLD (1 << 7)

#define SPIF_SECTOR_SIZE	4096
#define SPIF_SectorToAddress(SectorNumber) (SectorNumber * SPIF_SECTOR_SIZE)

// flash freq. storage
#define SPIF_MAX_FREQS	100
#define SYSSTATE_SECTOR	256
#define EMPTY_SECTOR_CODE	0xAE
#define OCCUPIED_SECTOR_CODE	0xEA
///////////////////////////////////////////////					
//				GLOBALS	
///////////////////////////////////////////////					
// Flash chip ID codes
typedef enum
{
	SPIF_MANUFACTURER_ERROR = 0,
	SPIF_MANUFACTURER_WINBOND = 0xEF,
	SPIF_MANUFACTURER_ISSI = 0x9D,
	SPIF_MANUFACTURER_MICRON = 0x20,
	SPIF_MANUFACTURER_GIGADEVICE = 0xC8,
	SPIF_MANUFACTURER_MACRONIX = 0xC2,
	SPIF_MANUFACTURER_SPANSION = 0x01,
	SPIF_MANUFACTURER_AMIC = 0x37,
	SPIF_MANUFACTURER_SST = 0xBF,
	SPIF_MANUFACTURER_HYUNDAI = 0xAD,
	SPIF_MANUFACTURER_ATMEL = 0x1F,
	SPIF_MANUFACTURER_FUDAN = 0xA1,
	SPIF_MANUFACTURER_ESMT = 0x8C,
	SPIF_MANUFACTURER_INTEL = 0x89,
	SPIF_MANUFACTURER_SANYO = 0x62,
	SPIF_MANUFACTURER_FUJITSU = 0x04,
	SPIF_MANUFACTURER_EON = 0x1C,
	SPIF_MANUFACTURER_PUYA = 0x85,
	SPIF_MANUFACTURER_BOYA = 0x9F,

} SPIF_ManufactorTypeDef;

// Flash storage object
typedef struct {
	uint8_t signature;	// const value to identify a empty/used sector
	float freq;		// stored frequency
	uint8_t vol;	// optional field (not used)
	uint8_t squelch;	// optional field (not used)
	uint8_t micgain;	// optional field (not used)
	uint8_t sidetone;	// optional field (not used)
	char Name[RECORD_NAME_LEN+1];	// Frequency label
} recordtype_t;	

					
					
#endif // END OF FILE					
