/*
 * Flash.h
 *
 *  Created on: Oct 11, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdint.h>
// Flash Memory Configuration
#define BY25Q16BS_FLASH_SIZE          0x200000  // 2 MB (16 Mbit)
#define BY25Q16BS_PAGE_SIZE           256       // 256 bytes per page
#define BY25Q16BS_SECTOR_SIZE         0x1000    // 4 KB per sector
#define BY25Q16BS_BLOCK_SIZE          0x10000   // 64 KB per block
#define BY25Q16BS_PAGE_COUNT          (BY25Q16BS_FLASH_SIZE / BY25Q16BS_PAGE_SIZE)   // 8192 pages
#define BY25Q16BS_SECTOR_COUNT        (BY25Q16BS_FLASH_SIZE / BY25Q16BS_SECTOR_SIZE) // 512 sectors
#define BY25Q16BS_BLOCK_COUNT         (BY25Q16BS_FLASH_SIZE / BY25Q16BS_BLOCK_SIZE)  // 32 blocks

// Flash Address Calculation
#define BY25Q16BS_START_ADDRESS       0x000000  // Starting address of flash
#define BY25Q16BS_END_ADDRESS         0x1FFFFF  // Ending address (2 MB - 1)
#define BY25Q16BS_SECTOR_ADDRESS(sector)   ((sector) * BY25Q16BS_SECTOR_SIZE)
#define BY25Q16BS_PAGE_ADDRESS(page)       (BY25Q16BS_START_ADDRESS + ((page) * BY25Q16BS_PAGE_SIZE))
#define BY25Q16BS_MAX_WRITE_SIZE      BY25Q16BS_PAGE_SIZE  // Maximum bytes per write (page size)

// Parameter Structure Configuration
#define PARAMETER_FLASH_PAGE          0                 // Page for s_NAV_RCU_Parameters
#define PARAMETER_FLASH_ADDRESS       BY25Q16BS_PAGE_ADDRESS(PARAMETER_FLASH_PAGE) // Address: 0x000000

// Frequency Storage Configuration
#define FREQUENCY_FLASH_PAGE          2                 // Page for storing standby frequencies
#define FREQUENCY_FLASH_ADDRESS       BY25Q16BS_PAGE_ADDRESS(FREQUENCY_FLASH_PAGE) // Address: 0x000100
#define FREQUENCIES_PER_PAGE          (BY25Q16BS_PAGE_SIZE / sizeof(float)) // 64 frequencies per page

// General Macros for Flash
#define FLASH_PAGE_ADDRESS(page)      (BY25Q16BS_START_ADDRESS + ((page) * BY25Q16BS_PAGE_SIZE))

#define NUM_FREQUENCIES_PER_PAGE 15


extern void Flash_ReadData(uint32_t address, uint8_t* buffer, uint16_t length);
extern void Flash_WriteData(uint32_t address, uint8_t* data, uint16_t length);
extern void Flash_Test();
extern uint8_t Is_Flash_Empty(uint32_t address, uint16_t length);
extern uint8_t flash_check(void);
extern void Flash_SectorErase(uint32_t address);

#endif /* INC_FLASH_H_ */
