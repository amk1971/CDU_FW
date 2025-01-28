/*
 * Flash.h
 *
 *  Created on: Oct 11, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdint.h>
// Flash size in bytes (16 Mbit = 2 MB)
#define BY25Q16BS_FLASH_SIZE          0x200000  // 2 MB (16 Mbit)

// Page size in bytes
#define BY25Q16BS_PAGE_SIZE           256       // 256 bytes per page

// Sector size in bytes
#define BY25Q16BS_SECTOR_SIZE         0x1000    // 4 KB per sector

// Block size in bytes (64 KB blocks)
#define BY25Q16BS_BLOCK_SIZE          0x10000   // 64 KB per block

// Total number of pages
#define BY25Q16BS_PAGE_COUNT          (BY25Q16BS_FLASH_SIZE / BY25Q16BS_PAGE_SIZE)   // 8192 pages

// Total number of sectors
#define BY25Q16BS_SECTOR_COUNT        (BY25Q16BS_FLASH_SIZE / BY25Q16BS_SECTOR_SIZE) // 512 sectors

// Total number of blocks
#define BY25Q16BS_BLOCK_COUNT         (BY25Q16BS_FLASH_SIZE / BY25Q16BS_BLOCK_SIZE)  // 32 blocks

// Start and maximum addresses
#define BY25Q16BS_START_ADDRESS       0x000000  // Starting address of flash
#define BY25Q16BS_END_ADDRESS         0x1FFFFF  // Ending address (2 MB - 1)

// Sector address calculation
#define BY25Q16BS_SECTOR_ADDRESS(sector)   ((sector) * BY25Q16BS_SECTOR_SIZE)
#define BY25Q16BS_PAGE_ADDRESS(page)       ((page) * BY25Q16BS_PAGE_SIZE)

// Maximum number of bytes per operation (256 bytes for page writes)
#define BY25Q16BS_MAX_WRITE_SIZE      BY25Q16BS_PAGE_SIZE  // Maximum bytes per write (page size)

#define CDU_PARA_FLASH_ADDRESS		  BY25Q16BS_START_ADDRESS + BY25Q16BS_PAGE_SIZE

extern void Flash_ReadData(uint32_t address, uint8_t* buffer, uint16_t length);
extern void Flash_WriteData(uint32_t address, uint8_t* data, uint16_t length);
extern void Flash_Test();
extern uint8_t Is_Flash_Empty(uint32_t address, uint16_t length);
extern uint8_t flash_check(void);
extern void Flash_SectorErase(uint32_t address);

#endif /* INC_FLASH_H_ */
