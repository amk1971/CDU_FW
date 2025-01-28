/*
 * Flash.c
 *
 *  Created on: Oct 11, 2024
 *      Author: Hamza Javed
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "spi.h"
//#include "debug_console.h"


#define FLASH_DEV_ID	0x68

// Command definitions
#define CMD_DEVICE_ID		0x90
#define CMD_READ_DATA 		0x03
#define CMD_WRITE_ENABLE 	0x06
#define CMD_PAGE_PROGRAM 	0x02
#define CMD_SECTOR_ERASE 	0x20
#define CMD_READ_STATUS 	0x05
#define CMD_CHIP_ERASE 		0xC7
#define CMD_WRITE_DISABLE 	0x04

// Select the Flash memory by pulling the CS pin low
#define FLASH_CS_LOW() HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define FLASH_CS_HIGH() HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)

/*
 * Function: Flash_ReadDeviceID
 * Arguments: None
 *
 * Description:
 * This function reads the device ID of the flash memory. It uses the JEDEC ID command to request the
 * device ID, sending 3 dummy bytes to align with the protocol requirements. The function handles the
 * SPI communication to transmit the command and receive the device ID, with chip select signals to
 * manage the communication start and end.
 *
 * Returns:
 * - id: The device ID of the flash memory.
 */
uint8_t Flash_ReadDeviceID(void) {
    uint8_t cmd = CMD_DEVICE_ID; // JEDEC ID command
    uint8_t dummy[3] = {0x00, 0x00, 0x00};
    uint8_t id;

    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);  // Send JEDEC ID command
    HAL_SPI_Transmit(&hspi1, dummy, 3, HAL_MAX_DELAY); // Send 3 dummy bytes
    HAL_SPI_Receive(&hspi1, &id, 1, HAL_MAX_DELAY);    // Receive the device ID
    FLASH_CS_HIGH();

    return id;
}

/*
 * Function: Flash_WriteEnable
 * Arguments: None
 *
 * Description:
 * This function enables write operations on the flash memory by sending the
 * Write Enable command via SPI. It handles the chip select signals to initiate
 * and conclude the SPI communication.
 */
void Flash_WriteEnable(void) {
    uint8_t cmd = CMD_WRITE_ENABLE;
    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();
}

/*
 * Function: Flash_WriteDisable
 * Arguments: None
 *
 * Description:
 * This function disables write operations on the flash memory by sending the
 * Write Disable command via SPI. It handles the chip select signals to initiate
 * and conclude the SPI communication.
 */
void Flash_WriteDisable(void) {
    uint8_t cmd = CMD_WRITE_DISABLE;
    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();
}

/*
 * Function: Flash_ReadStatus
 * Arguments: None
 *
 * Description:
 * This function reads the status register of the flash memory by sending the
 * Read Status command via SPI. It transmits the command, receives the status
 * register value, and handles the chip select signals to manage the SPI communication.
 *
 * Returns:
 * - status: The current value of the status register.
 */
uint8_t Flash_ReadStatus(void) {
    uint8_t cmd = CMD_READ_STATUS;
    uint8_t status;
    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();
    return status;
}

/*
 * Function: Flash_SectorErase
 * Arguments: uint32_t address
 *
 * Description:
 * This function erases a sector of the flash memory at the specified address.
 * It first enables write operations, then sends the Sector Erase command along
 * with the address via SPI. After sending the command, it waits for the erase
 * operation to complete by checking the BUSY bit in the status register.
 *
 * - address: The starting address of the sector to be erased.
 */
void Flash_SectorErase(uint32_t address) {
    Flash_WriteEnable();  // Enable writing
    uint8_t status = Flash_ReadStatus();
    if(!(status & 0x02))
    {
    	return;
    }

    uint8_t cmd[4];
    cmd[0] = CMD_SECTOR_ERASE;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);  // Send Sector Erase command and address
    FLASH_CS_HIGH();

    // Wait for the erase operation to complete
    while (Flash_ReadStatus() & 0x01);  // Wait for the BUSY bit to clear
}


/*
 * Function: Flash_PageProgram
 * Arguments: uint32_t address, uint8_t* data, uint16_t length
 *
 * Description:
 * This function programs a page of the flash memory at the specified address with the given data.
 * It ensures that the length of the data does not exceed the page size (256 bytes). It first enables
 * write operations, then sends the Page Program command along with the address and data via SPI.
 * After sending the command, it waits for the write operation to complete by checking the BUSY bit
 * in the status register.
 *
 * - address: The starting address where the data will be written.
 * - data: Pointer to the data to be written.
 * - length: The length of the data to be written, limited to 256 bytes.
 */
void Flash_PageProgram(uint32_t address, uint8_t* data, uint16_t length) {
    // Ensure the length doesn't exceed 256 bytes (page size)
    if (length > 256) length = 256;

    Flash_WriteEnable(); // Enable writing
    uint8_t status = Flash_ReadStatus();
    if(!(status & 0x02))
    {
    	return;
    }

    uint8_t cmd[4];
    cmd[0] = CMD_PAGE_PROGRAM;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);  // Send Page Program command and address
    HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY);  // Send data to be written
    FLASH_CS_HIGH();

    // Wait for the write operation to complete
    while (Flash_ReadStatus() & 0x01);  // Wait for the BUSY bit to clear
}


/*
 * Function: Flash_ChipErase
 * Arguments: None
 *
 * Description:
 * This function erases the entire chip of the flash memory. It first enables
 * write operations, then sends the Chip Erase command via SPI. After sending
 * the command, it waits for the erase operation to complete by checking the
 * BUSY bit in the status register.
 */
void Flash_ChipErase(void)
{
    Flash_WriteEnable();  // Enable writing
    uint8_t status = Flash_ReadStatus();
    if(!(status & 0x02))
    {
    	return;
    }

    uint8_t cmd = CMD_CHIP_ERASE;

    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);  // Send Chip Erase command
    FLASH_CS_HIGH();

    // Wait for the erase operation to complete
    while (Flash_ReadStatus() & 0x01);  // Wait for the BUSY bit to clear
}

/*
 * Function: Flash_SoftReset
 * Arguments: None
 *
 * Description:
 * This function performs a soft reset of the flash memory. It first sends the
 * Enable Reset command (0x66) to prepare the device for a reset, and then sends
 * the Reset command (0x99) to initiate the reset. This ensures the flash memory
 * is brought back to its initial state.
 *
 * Steps:
 * - Step 1: Send the Enable Reset command (0x66) to the flash memory.
 * - Step 2: Send the Reset command (0x99) to the flash memory.
 */
void Flash_SoftReset()
{
    uint8_t enable_reset_cmd = 0x66;
    uint8_t reset_cmd = 0x99;

    // Step 1: Send the Enable Reset command (0x66)
    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &enable_reset_cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();

    // Step 2: Send the Reset command (0x99)
    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &reset_cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();
}
/*  ----------- READ/WRITE Options ------------------------- */

/*
 * Function: Flash_WaitForReady
 * Arguments: None
 *
 * Description:
 * This function waits for the flash memory to become ready by polling the
 * status register's BUSY bit. It continuously reads the status register
 * until the BUSY bit is cleared, indicating that the flash memory is ready
 * for the next operation.
 */
void Flash_WaitForReady(void) {
    while (Flash_ReadStatus() & 0x01);  // Wait for the BUSY bit to clear
}


/*
 * Function: Flash_ReadData
 * Arguments: uint32_t address, uint8_t* buffer, uint16_t length
 *
 * Description:
 * This function reads data from the flash memory starting at the specified
 * address. It sends the Read Data command along with the address via SPI,
 * and then receives the requested amount of data into the provided buffer.
 * The function manages the chip select signals to initiate and conclude
 * the SPI communication.
 *
 * - address: The starting address for the data read operation.
 * - buffer: Pointer to the buffer where the read data will be stored.
 * - length: The length of the data to be read.
 */
void Flash_ReadData(uint32_t address, uint8_t* buffer, uint16_t length)
{
    uint8_t cmd[4];
    cmd[0] = CMD_READ_DATA;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);  // Send Read Data command and address
    HAL_SPI_Receive(&hspi1, buffer, length, HAL_MAX_DELAY);  // Receive data
    FLASH_CS_HIGH();
}


/*
 * Function: Flash_WriteData
 * Arguments: uint32_t address, uint8_t* data, uint32_t length
 *
 * Description:
 * This function writes data to the flash memory starting at the specified
 * address. It splits the data into chunks of up to 256 bytes (page size)
 * and programs each page using the Flash_PageProgram function. It manages
 * the address and data pointers and decrements the length accordingly until
 * all data is written.
 *
 * - address: The starting address for the data write operation.
 * - data: Pointer to the data to be written.
 * - length: The total length of the data to be written.
 */
void Flash_WriteData(uint32_t address, uint8_t* data, uint32_t length)
{
//	Flash_SectorErase(address);

    while (length > 0) {
        uint16_t writeLength = (length > 256) ? 256 : length;
        Flash_PageProgram(address, data, writeLength);
        address += writeLength;
        data += writeLength;
        length -= writeLength;
    }
}

/*
 * Function: Is_Flash_Empty
 * Arguments: uint32_t address, uint32_t length
 *
 * Description:
 * This function checks if a specified area of flash memory is empty. It reads
 * the flash memory in chunks and checks each byte to see if it is equal to 0xFF.
 * If any byte in the specified range is not 0xFF, the function returns 0, indicating
 * that the flash is not empty. If all bytes are 0xFF, it returns 1, indicating that
 * the flash is empty.
 *
 * - address: The starting address of the flash memory to check.
 * - length: The length of the flash memory area to check.
 *
 * Returns:
 * - 0 if the flash memory is not empty.
 * - 1 if the flash memory is empty.
 */
uint8_t Is_Flash_Empty(uint32_t address, uint16_t length)
{
    uint8_t buffer[256];  // Temporary buffer for checking
    uint32_t remaining = length;

    while (remaining > 0) {
        uint16_t readLength = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;
        Flash_ReadData(address, buffer, readLength);

        for (uint16_t i = 0; i < readLength; i++) {
            if (buffer[i] != 0xFF) {
                return 0;  // Flash is not empty
            }
        }

        address += readLength;
        remaining -= readLength;
    }

    return 1;  // Flash is empty
}

/*
 * Function: flash_check
 * Arguments: None
 *
 * Description:
 * This function checks whether the flash memory's device ID matches the expected ID.
 * It reads the device ID using the Flash_ReadDeviceID function. If the device ID matches
 * the expected FLASH_DEV_ID, the function returns 1. Otherwise, it returns 0.
 *
 * Returns:
 * - 1 if the device ID matches FLASH_DEV_ID.
 * - 0 if the device ID does not match FLASH_DEV_ID.
 */
uint8_t flash_check(void)
{
    uint8_t deviceID;

    Flash_SoftReset(); // resest the flash once to be sure the flash is ready with default settings
    HAL_Delay(5);	// flash needs 30 microseconds for the reset. but i am giving it a 5 ms. its not issue as its on a startup.

    // 1. Get and print the Device ID
    deviceID = Flash_ReadDeviceID();

    if(deviceID == FLASH_DEV_ID)
    {
    	return 1;
    }

    return 0;
}
/* ------------------------------- Flash Test --------------------------------------- */

/*
 * Function: Flash_Test
 * Arguments: None
 *
 * Description:
 * This function tests the basic functionalities of the flash memory. It performs
 * several operations including reading the device ID, writing data, reading back
 * and verifying the data, erasing a sector, and confirming the erase operation.
 *
 * The test involves:
 * 1. Reading and printing the Device ID.
 * 2. Filling a buffer with 'A' to 'Z' and writing it to a specific address.
 * 3. Reading back the data and comparing it with what was written to verify
 *    the write operation.
 * 4. Erasing the sector containing the written data.
 * 5. Reading back the data after erasing to confirm the erase operation.
 */
//void Flash_Test() {
//	// Buffer for testing
//	uint8_t writeBuffer[26]; // To hold 'A' to 'Z'
//	uint8_t readBuffer[26];  // To hold the read-back data
//
//    uint8_t deviceID;
//
//    // 1. Get and print the Device ID
//    deviceID = Flash_ReadDeviceID();
//    debug_print("Device ID: 0x%X\n", deviceID);
//
//    // Fill the writeBuffer with 'A' to 'Z'
//    for (uint8_t i = 0; i < 26; i++) {
//        writeBuffer[i] = 'A' + i;
//    }
//
//    // 2. Write 'A' to 'Z' to a specific address (e.g., 0x000000)
//    uint32_t address = 0x000000;
//    Flash_WriteData(address, writeBuffer, 26);
//    debug_print("Written A-Z to flash memory.\n");
//
//    // 3. Read back the data and compare with what was written
//    memset(readBuffer, 0, sizeof(readBuffer)); // Clear the readBuffer
//    Flash_ReadData(address, readBuffer, 26);
//
//    debug_print("Read back data after write: ");
//    for (uint8_t i = 0; i < 26; i++) {
//        printf("%c ", readBuffer[i]);
//    }
//    debug_print("\n");
//
//    // Check if the written and read data match
//    if (memcmp(writeBuffer, readBuffer, 26) == 0) {
//    	debug_print("Data matches: Write successful.\n");
//    } else {
//    	debug_print("Data mismatch: Write failed.\n");
//    }
//
//    // 4. Erase the sector (assuming 4KB sector erase)
//    Flash_SectorErase(address);
//    debug_print("Flash sector erased.\n");
//
//    // 5. Read back the data after erasing to confirm it's cleared
//    memset(readBuffer, 0, sizeof(readBuffer)); // Clear the readBuffer
//    Flash_ReadData(address, readBuffer, 26);
//
//    debug_print("Read back data after erase: ");
//    for (uint8_t i = 0; i < 26; i++) {
//    	debug_print("%02X ", readBuffer[i]); // Expecting erased bytes (usually 0xFF)
//    }
//    debug_print("\n");
//
//    // Check if the flash memory is empty (expecting all 0xFF after erase)
//    uint8_t emptyFlag = 1;
//    for (uint8_t i = 0; i < 26; i++) {
//        if (readBuffer[i] != 0xFF) {
//            emptyFlag = 0;
//            break;
//        }
//    }
//
//    if (emptyFlag) {
//    	debug_print("Flash erase successful.\n");
//    } else {
//    	debug_print("Flash erase failed.\n");
//    }
//}

/* -------------------------------------- End Flash Test ------------------------------------------------ */
