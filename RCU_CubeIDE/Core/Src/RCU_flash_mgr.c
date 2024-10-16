/*! \file RCU_flash_mgr.c					
\brief RCU Flash storage manager and access methods					
*/ 					
				
#include "main.h"					
#include "FreeRTOS.h"
#include "task.h"						
#include "RCU_flash_mgr.h"				
		
///////////////////////////////////////////////					
//				GLOBALS	
///////////////////////////////////////////////	
// Default frequency struct in flash (Deleted frequencies)
const recordtype_t defaultrecord = {
	.signature = EMPTY_SECTOR_CODE,
	.freq = 118.0,
	.vol = 0,
	.squelch = 0,
	.micgain = 0,
	.sidetone = 0, 
	.Name = "EMPTY  "
};	

///////////////////////////////////////////////					
//				PROTOTYPES	
///////////////////////////////////////////////	
void spif_WriteEnable(void);
uint8_t spif_IsBusy(void);
uint8_t spif_Init_Flash(void);
void spif_WriteRecord(recordtype_t* r, uint16_t index);			
void spif_ReadRecord(recordtype_t* r, uint16_t index);
uint8_t spif_Record_Is_Empty(uint16_t index);
void spif_DeleteRecord(uint16_t index);
void spif_Reset_Flash(void);
void spif_Store_SysState(RCUsystemstate_t *s, uint16_t sector);
void spif_Load_SysState(RCUsystemstate_t *s, uint16_t sector);
uint8_t spif_SysState_in_Flash(uint16_t sector);

///////////////////////////////////////////////					
//				FUNCTIONS	
///////////////////////////////////////////////					
//-------------------------------------------------------------------								
//  CHECK FLASH STATUS								
//								
/*! \fn uint8_t spif_IsBusy(void)								
\brief Check Flash write/erase status								
								
check the Write in Progress (WIP) bit								
\return 1: busy, 0: ready								
*/								
//-------------------------------------------------------------------								
uint8_t spif_IsBusy(void)								
{								
	uint8_t tx[4] = { SPIF_CMD_READSTATUS1, 0xFF, 0xFF, 0xFF };
	uint8_t rx[4];
	
	FLASH_CS_LOW;
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 4, 10);   // read Status reg
	FLASH_CS_HIGH;
	
	return (rx[1] & 0x01);	// extract Write in Progress (WIP) bit
}								
							

//-------------------------------------------------------------------								
//  WRITE ENABLE								
//								
/*! \fn void spif_WriteEnable(void)								
\brief Enable flash writes								
								
Set the WEL status bit								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_WriteEnable(void)								
{	
	uint8_t cmd = SPIF_CMD_WRITEENABLE; 
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
	FLASH_CS_HIGH;
}								

//-------------------------------------------------------------------								
//  INIT FLASH								
//								
/*! \fn uint8_t spif_Init_Flash(void)								
\brief Initialize the Flash IC								
								
Setup pins and read Chip ID								
\return 0: OK, -1: Error								
*/								
//-------------------------------------------------------------------								
uint8_t spif_Init_Flash(void)								
{	
	uint8_t tx[4] = { SPIF_CMD_JEDECID, 0xFF, 0xFF, 0xFF };
	uint8_t rx[4];	
	
	//  wait for stable VCC
	FLASH_CS_HIGH;
	vTaskDelay(20); 		// wait 20ms
	FLASH_CS_LOW;
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 4, 10);  // read Chip ID
	FLASH_CS_HIGH;
	
	if ((rx[1] == SPIF_MANUFACTURER_BOYA) && (rx[3] != 0)) return 0;
	else return -1;
}	

//-------------------------------------------------------------------								
//  WRITE RECORD								
//								
/*! \fn void spif_WriteRecord(recordtype_t* r,uint16_t index)								
\brief Store a frequency in flash memory								
								
Write Frequency label, MHz and KHz/25 fields								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_WriteRecord(recordtype_t* r /* pointer to record to write */, uint16_t index /* Flash sector number*/)								
{								
	uint32_t addr = SPIF_SectorToAddress(index);
	uint8_t buf[4];
	buf[0] = SPIF_CMD_SECTORERASE3ADD;
	buf[1] = (addr & 0x00FF0000) >> 16;
	buf[2] = (addr & 0x0000FF00) >> 8;
	buf[3] = (addr & 0x000000FF);

	// erase sector
	spif_WriteEnable();
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10);
	FLASH_CS_HIGH;
	while (spif_IsBusy()) ;	// wait for command completion
	
	// write data
	spif_WriteEnable();
	buf[0] = SPIF_CMD_PAGEPROG3ADD;
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10);	// send command and address
	HAL_SPI_Transmit(&hspi1, (uint8_t*) r, (uint16_t) sizeof(*r), 10);    	// send data
	FLASH_CS_HIGH;
	while (spif_IsBusy());	// wait for command completion		
}								
					
//-------------------------------------------------------------------								
//  READ RECORD								
//								
/*! \fn void spif_ReadRecord(recordtype_t* r,uint16_t index)								
\brief Read a frequency from flash memory								
								
Read Frequency label, MHz and KHz/25 fields and store in record object passed by reference								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_ReadRecord(recordtype_t* r /* pointer to record to load data from flash */, uint16_t index /* Flash sector number */)								
{								
	uint32_t addr = SPIF_SectorToAddress(index);
	uint8_t buf[4];
	buf[0] = SPIF_CMD_READDATA3ADD;
	buf[1] = (addr & 0x00FF0000) >> 16;
	buf[2] = (addr & 0x0000FF00) >> 8;
	buf[3] = (addr & 0x000000FF);
	
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10); 	// send command and address
	HAL_SPI_Receive(&hspi1,(uint8_t*) r, (uint16_t) sizeof(*r), 10);   	// receive data
	FLASH_CS_HIGH;

	r->Name[RECORD_NAME_LEN] = 0;	// Null terminate string in case of flash error
}								

//-------------------------------------------------------------------								
//  CHECK IF EMPTY								
//								
/*! \fn uint8_t spif_Record_Is_Empty(uint16_t index)								
\brief Peek a record to check if it is empty								
								
Read the first byte and check if the record is empty								
\return 1: empty, 0: occupied								
*/								
//-------------------------------------------------------------------								
uint8_t spif_Record_Is_Empty(uint16_t index /* Flash sector number */)								
{								
	recordtype_t check;

	spif_ReadRecord(&check, index);	// copy flash contents to 'check' object

	if(check.signature == EMPTY_SECTOR_CODE) return 1;
	else return 0;
}								

//-------------------------------------------------------------------								
//  DELETE RECORD								
//								
/*! \fn void spif_DeleteRecord(uint16_t index)								
\brief Erase a record contents								
								
Replace label with "EMPTY" string, and clear all other values								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_DeleteRecord(uint16_t index /* Flash sector number */)								
{	
	uint32_t addr = SPIF_SectorToAddress(index);
	uint8_t buf[4];
	buf[0] = SPIF_CMD_SECTORERASE3ADD;
	buf[1] = (addr & 0x00FF0000) >> 16;
	buf[2] = (addr & 0x0000FF00) >> 8;
	buf[3] = (addr & 0x000000FF);
	
	spif_WriteEnable();
	
	// erase sector
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10);
	FLASH_CS_HIGH;
	while (spif_IsBusy());	// wait for command completion
	
	// write default record data
	spif_WriteRecord((uint8_t*)&defaultrecord, index);
}								
					
//-------------------------------------------------------------------								
//  SOFT RESET FLASH								
//								
/*! \fn void spif_Reset_Flash(void)								
\brief Software reset								
								
Sends the 2-byte Reset command to the Flash IC								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_Reset_Flash(void)								
{								
	uint8_t cmd = SPIF_CMD_RESET1; 
	
	while (spif_IsBusy()) ;	// wait for pending commands
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
	FLASH_CS_HIGH;
	cmd = SPIF_CMD_RESET2; 
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
	FLASH_CS_HIGH; 
}								
					
//-------------------------------------------------------------------								
//  STORE SYSTEM STATE								
//								
/*! \fn void spif_Store_SysState(RCUsystemstate_t *s, , uint16_t sector)								
\brief Store System State parameters in Flash								
								7
Write SysState struct to Flash								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_Store_SysState(RCUsystemstate_t *s /* Pointer to RCUsystemstate_t object */, uint16_t sector /* destination sector number */)						
{								
	uint32_t addr = SPIF_SectorToAddress(sector);
	uint8_t buf[4];

	buf[0] = SPIF_CMD_SECTORERASE3ADD;
	buf[1] = (addr & 0x00FF0000) >> 16;
	buf[2] = (addr & 0x0000FF00) >> 8;
	buf[3] = (addr & 0x000000FF);

	// erase sector
	spif_WriteEnable();
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10);
	FLASH_CS_HIGH;
	while (spif_IsBusy()) ;	// wait for command completion
	
	// write data
	spif_WriteEnable();
	buf[0] = SPIF_CMD_PAGEPROG3ADD;
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10); 	// send command and address
	HAL_SPI_Transmit(&hspi1, (uint8_t*)s, (uint16_t)sizeof(*s), 10);    	// send data
	FLASH_CS_HIGH;
	while (spif_IsBusy()) ;	// wait for command completion										
}								
					
//-------------------------------------------------------------------								
//  LOAD SYSTEM STATE								
//								
/*! \fn void spif_Load_SysState(RCUsystemstate_t *s,uint16_t sector)								
\brief Load System State parameters from Flash								
								
Read struct from Flash and copy to RCUsystemstate_t object passed as reference								
\return None								
*/								
//-------------------------------------------------------------------								
void spif_Load_SysState(RCUsystemstate_t *s /* Pointer to RCUsystemstate_t object to write data to */, uint16_t sector /* source sector number */)								
{								
							
	uint32_t addr = SPIF_SectorToAddress(sector);
	uint8_t buf[4];	

	buf[0] = SPIF_CMD_READDATA3ADD;
	buf[1] = (addr & 0x00FF0000) >> 16;
	buf[2] = (addr & 0x0000FF00) >> 8;
	buf[3] = (addr & 0x000000FF);
	
	FLASH_CS_LOW;
	HAL_SPI_Transmit(&hspi1, buf, 4, 10); 	// send command and address
	HAL_SPI_Receive(&hspi1,(uint8_t*)s, (uint16_t)sizeof(*s), 10);      	// receive data
	FLASH_CS_HIGH;							
}								

//-------------------------------------------------------------------								
//  SYS STATE AVAILABLE IN FLASH								
//								
/*! \fn uint8_t spif_SysState_in_Flash(uint16_t sector)								
\brief Check that a SysState exists in flash								
								
Check if a SysState object is available in flash								
\return 1: true, 0: false								
*/								
//-------------------------------------------------------------------								
uint8_t spif_SysState_in_Flash(uint16_t sector /* sector number */)								
{								
	RCUsystemstate_t s;
	
	spif_Load_SysState(&s, sector);
	if (s.signature == OCCUPIED_SECTOR_CODE) return 1;
	else return 0;
}								
