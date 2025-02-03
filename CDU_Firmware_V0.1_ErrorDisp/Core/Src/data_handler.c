/*
 * data_handler.c
 *
 *  Created on: Oct 8, 2024
 *      Author: Hamza Javed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "data_handler.h"
#include "Flash.h"
#include "TFT_LCD.h"
#include "keyboard.h"
#include "user_defaults.h"
#include <ctype.h>
#include "tacan_map.h"

#define UPLOAD_DEFAULT 		1	// 0 DIsable and 1 for Enable

#define KHZ_SCALE_FACTOR_F		0.025
#define KHZ_SCALE_FACTOR_DEC	25
#define CONVERSION_VAL_HEX		30
#define CONVERSION_VAL_DEC		48

#define NUM_CLASSES_ID		6
#define NUM_MESSAGE_ID		6


const char class_id[NUM_CLASSES_ID] = {
		'V',	// for VHF NAV
		'A',	// ADF
		'T',	// TACAN Channel and Frequency
		'H',	// HF
		'V',	// VHF
		'U'		// UHF
};

const char* message_id_nav_vhf[NUM_MESSAGE_ID] = {
		"28",
		"27",
		"73",
		"",
		"",
		"99"	//Health Message
};

const char* message_id_adf_tacan[NUM_MESSAGE_ID] = {
		"29",	// Standby Frequency Message ID
		"42",	// Active Frequency Message ID
		"71",	// Volume, Toggle ADF/ANT, Switch on BFO Message ID
		"20",	// Reset Status  Message ID
		"271",	// Communication Error Message ID
		"99"	// health message
};

CDU_Parameters cdu_parameters;

Class_Id page_class;
 /* ---------------------------- Function Declarations ------------------------------- */

typedef void (*MessageHandler)(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);

uint16_t combine_frequency_khz(uint8_t* khz_buf);
uint8_t split_frequency_khz(uint16_t f_khz, uint8_t* khz_buf);
void notify_tft(void);
Class_Id get_class_Id(void);
float get_freq( uint8_t f_mhz, uint16_t f_khz);
void handle_NAV_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);
void handle_ADF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);
void handle_TACAN_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);
void handle_HF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);
void handle_VHF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);
void handle_UHF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz);


void Read_CDU_Parameters(uint32_t address, CDU_Parameters* params);
void Write_CDU_Parameters(uint32_t address, CDU_Parameters* params);

MessageHandler message_parse[class_ID_Count] = {
		handle_NAV_incoming_message,
		handle_ADF_incoming_message,
		handle_TACAN_incoming_message,
		handle_HF_incoming_message,
		handle_VHF_incoming_message,
		handle_UHF_incoming_message,
};

/* ----------------------------------------------------------------------------------- */
void update_flash(void)
{
	Write_CDU_Parameters((uint32_t) CDU_PARA_FLASH_ADDRESS , &cdu_parameters);
}
/*
 * Function: Write_CDU_Parameters
 * Arguments: uint32_t address, CDU_Parameters* params
 *
 * Description:
 * This function writes the CDU parameters to the specified flash memory address.
 * It converts the CDU_Parameters structure to a byte array and writes the data in
 * chunks of up to 256 bytes using the Flash_WriteData function.
 *
 * - address: The flash memory address where the CDU parameters will be written.
 * - params: Pointer to the CDU_Parameters structure containing the data to be written.
 */
void Write_CDU_Parameters(uint32_t address, CDU_Parameters* params)
{
    // Get the size of the structure
    uint32_t structSize = sizeof(CDU_Parameters);

    // Cast the structure to a byte array for writing
    uint8_t* data = (uint8_t*)params;

    Flash_SectorErase(address);

    // While there is data left to write, write it in chunks of 256 bytes
    while (structSize > 0) {
        // Determine how much to write in the current iteration (maximum 256 bytes)
        uint32_t writeLength = (structSize > BY25Q16BS_PAGE_SIZE) ? BY25Q16BS_PAGE_SIZE : structSize;

        // Write the data to the current address
        Flash_WriteData(address, data, writeLength);

        // Update address, data pointer, and remaining structure size
        address += writeLength;
        data += writeLength;
        structSize -= writeLength;
    }
}

/*
 * Function: Read_CDU_Parameters
 * Arguments: uint32_t address, CDU_Parameters* params
 *
 * Description:
 * This function reads the CDU parameters from the specified flash memory address
 * into the provided CDU_Parameters structure. It reads the data in chunks of up
 * to 256 bytes using the Flash_ReadData function.
 *
 * - address: The flash memory address from which the CDU parameters will be read.
 * - params: Pointer to the CDU_Parameters structure where the read data will be stored.
 */
void Read_CDU_Parameters(uint32_t address, CDU_Parameters* params)
{
    // Get the size of the structure
    uint32_t structSize = sizeof(CDU_Parameters);

    // Cast the structure to a byte array for reading
    uint8_t* data = (uint8_t*)params;

    // While there is data left to read, read it in chunks of 256 bytes
    while (structSize > 0) {
        // Determine how much to read in the current iteration (maximum 256 bytes)
        uint32_t readLength = (structSize > BY25Q16BS_PAGE_SIZE) ? BY25Q16BS_PAGE_SIZE : structSize;

        // Read the data from the current address
        Flash_ReadData(address, data, readLength);

        // Update address, data pointer, and remaining structure size
        address += readLength;
        data += readLength;
        structSize -= readLength;
    }
}

/*
 * Function: Load_Default_CDU_Parameters
 * Arguments: CDU_Parameters* params
 *
 * Description:
 * This function loads default values into the CDU_Parameters structure. It copies
 * the entire structure from the constant default values defined in CDU_Default_Values.
 *
 * - params: Pointer to the CDU_Parameters structure where the default values will be loaded.
 */
void Load_Default_CDU_Parameters(CDU_Parameters* params)
{
    // Copy the entire structure from the constant defaults
    memcpy(params, &CDU_Default_Values, sizeof(CDU_Parameters));
}

/*
 * Function: Load_CDU_Parameters
 * Arguments: CDU_Parameters* params
 *
 * Description:
 * This function loads CDU parameters from flash memory or default values if the flash is empty.
 * It first checks if the flash memory at the specified address is empty. If it is empty, it loads
 * the default values into the CDU_Parameters structure. Otherwise, it reads the CDU parameters
 * from flash memory into the structure.
 *
 * - params: Pointer to the CDU_Parameters structure where the data will be loaded.
 */
void Load_CDU_Parameters(CDU_Parameters* params)
{
	uint32_t address = (uint32_t) CDU_PARA_FLASH_ADDRESS;

    // Check if flash is empty (assuming sizeof(CDU_Parameters) data is stored at 'address')
	if( flash_check())
	{
#if UPLOAD_DEFAULT
		Load_Default_CDU_Parameters(params);
		Write_CDU_Parameters(address, params);
		CDU_Parameters cdu_test_flash;
		Read_CDU_Parameters(address, &cdu_test_flash);
#endif
		if (Is_Flash_Empty(address, sizeof(CDU_Parameters)))
		{
			// Flash is empty, load default values from the global structure
			Load_Default_CDU_Parameters(params);
			Write_CDU_Parameters(address, params);
			// test if default parameters are written into flash.
//			CDU_Parameters cdu_test_flash;
//			Read_CDU_Parameters(address, &cdu_test_flash);
		}
		else
		{
			// Flash is not empty, read the structure from flash
			Read_CDU_Parameters(address, params);
		}
	}
	else
	{
		// Flash is empty, load default values from the global structure
		Load_Default_CDU_Parameters(params);
	}
}


/*
 * Function: swap_active_standby_freq
 * Arguments: None
 *
 * Description:
 * This function swaps the active frequency with the standby frequency in the
 * cdu_parameters structure. It uses a temporary variable to hold the active
 * frequency, assigns the standby frequency to the active frequency, and then
 * sets the standby frequency to the previously stored active frequency.
 * After the swap, it sends the updated data to the NAV system.
 */

void swap_active_standby_freq(Class_Id unit)
{
	float temp;
	uint16_t  temp_adf = 0;
	switch (unit)
	{
		case NAV:
			temp = cdu_parameters.active_nav_freq;
			cdu_parameters.active_nav_freq = cdu_parameters.standby_nav_freq;
			cdu_parameters.standby_nav_freq = temp;
			break;
		case ADF:
			temp_adf = cdu_parameters.active_adf_freq;
			cdu_parameters.active_adf_freq = cdu_parameters.standby_adf_freq;
			cdu_parameters.standby_adf_freq = temp_adf;
		    break;
		case TACAN:
			temp = cdu_parameters.active_tacan_freq;
			cdu_parameters.active_tacan_freq = cdu_parameters.standby_tacan_freq;
			cdu_parameters.standby_tacan_freq = temp;
		    break;
		case HF:
			temp = cdu_parameters.active_hf_freq;
			cdu_parameters.active_hf_freq = cdu_parameters.standby_hf_freq;
			cdu_parameters.standby_hf_freq = temp;
		    break;
		case VHF:
			temp = cdu_parameters.active_vhf_freq;
			cdu_parameters.active_vhf_freq = cdu_parameters.standby_vhf_freq;
			cdu_parameters.standby_vhf_freq = temp;
		    break;
		case UHF:
			temp = cdu_parameters.active_uhf_freq;
			cdu_parameters.active_uhf_freq = cdu_parameters.standby_uhf_freq;
			cdu_parameters.standby_uhf_freq = temp;
		    break;
		default:
			break;
	}
}

/*
 * Function: swap_active_standby_channel
 * Arguments: None
 *
 * Description:
 * This function swaps the active channel with the standby channel in the
 * cdu_parameters structure. It uses a temporary array to hold the standby
 * channel and ensures that the null terminators are correctly managed during
 * the swap. After the swap, it sends a message to TACAN to reflect the changes.
 */

void swap_active_standby_channel(void)
{
    char temp[TACAN_S];  // Temp array to hold standby channel with null terminator


	memset(temp, 0, TACAN_S);
    // Copy active_channel to temp (ensure the null terminator is included)
    strncpy(temp, cdu_parameters.active_channel, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';  // Ensure temp is null-terminated

    // Copy standby_channel to active_channel
	memset(cdu_parameters.active_channel, 0, TACAN_S);
    strncpy(cdu_parameters.active_channel, cdu_parameters.standby_channel, sizeof(cdu_parameters.active_channel) - 1);
    cdu_parameters.active_channel[sizeof(cdu_parameters.active_channel) - 1] = '\0';  // Ensure active_channel is null-terminated


    // Copy temp (previous active_channel) to standby_channel
	memset(cdu_parameters.standby_channel, 0, TACAN_S);
    strncpy(cdu_parameters.standby_channel, temp, sizeof(cdu_parameters.standby_channel) - 1);
    cdu_parameters.standby_channel[sizeof(cdu_parameters.standby_channel) - 1] = '\0';  // Ensure standby_channel is null-terminated

    // send Message to TACAN
}

/*
 * Function: copy_preset_to_standby_freq
 * Arguments: char index
 *
 * Description:
 * This function copies the preset frequency to the standby frequency based
 * on the given index.
 *
 * Depending on the index ('1' to '8'), it handles different preset frequencies
 * (p1_freq to p8_freq).
 */
void copy_preset_to_standby_freq(char index, Class_Id unit)
{
	switch(index)
	{
		case '1':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p1_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p1_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p1_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p1_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p1_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p1_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '2':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p2_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p2_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p2_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p2_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p2_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p2_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '3':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p3_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p3_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p3_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p3_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p3_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p3_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '4':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p4_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p4_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p4_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p4_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p4_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p4_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '5':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p5_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p5_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p5_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p5_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p5_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p5_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '6':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p6_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p6_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p6_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p6_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p6_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p6_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '7':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p7_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p7_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p7_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p7_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p7_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p7_uhf_freq;
					break;
				default:
					break;
			}
			break;
		case '8':
			switch(unit)
			{
				case NAV:
					cdu_parameters.standby_nav_freq = cdu_parameters.p8_nav_freq;
					break;
				case ADF:
					cdu_parameters.standby_adf_freq = cdu_parameters.p8_adf_freq;
					break;
				case TACAN:
					cdu_parameters.standby_tacan_freq = cdu_parameters.p8_tacan_freq;
					break;
				case HF:
					cdu_parameters.standby_hf_freq = cdu_parameters.p8_hf_freq;
					break;
				case VHF:
					cdu_parameters.standby_vhf_freq = cdu_parameters.p8_vhf_freq;
					break;
				case UHF:
					cdu_parameters.standby_uhf_freq = cdu_parameters.p8_uhf_freq;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}


/*
 * Function: copy_preset_to_standby_channel
 * Arguments: char index
 *
 * Description:
 * This function copies the preset channel to the standby channel based
 * on the given index.
 *
 * Depending on the index ('1' to '8'), it handles different preset channels
 * (p1_freq to p8_freq).
 */
void copy_preset_to_standby_channel(char index)
{

	switch(index)
	{
		case '1':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p1_channel, strlen(cdu_parameters.p1_channel));
			break;
		case '2':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p2_channel, strlen(cdu_parameters.p2_channel));
			break;
		case '3':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p3_channel, strlen(cdu_parameters.p3_channel));
			break;
		case '4':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p4_channel, strlen(cdu_parameters.p4_channel));
			break;
		case '5':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p5_channel, strlen(cdu_parameters.p5_channel));
			break;
		case '6':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p6_channel, strlen(cdu_parameters.p6_channel));
			break;
		case '7':
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p7_channel, strlen(cdu_parameters.p7_channel));
			break;
		case '8':
				memset(cdu_parameters.standby_channel, 0, TACAN_S);
				memcpy(cdu_parameters.standby_channel, cdu_parameters.p8_channel, strlen(cdu_parameters.p8_channel));
			break;
		default:
			break;
	}
}

/*
 * Function: calculate_checksum
 * Arguments: const char *msg, uint8_t *checksum_high, uint8_t *checksum_low
 *
 * Description:
 * This function calculates a checksum for a given message by summing all characters and
 * applying a mask to ignore carry bits. It then separates the high and low 4-bit nibbles
 * and converts them to ASCII.
 *
 * - msg: The message for which the checksum is being calculated.
 * - checksum_high: Pointer to store the high nibble of the checksum.
 * - checksum_low: Pointer to store the low nibble of the checksum.
 *
 * Process:
 * 1. Sum all characters in the message.
 * 2. Apply a mask to ignore carry bits, keeping the result within 6 bits.
 * 3. Separate the high and low 4-bit nibbles.
 */
void calculate_checksum(const char *msg, uint8_t *checksum_high, uint8_t *checksum_low) {
    uint16_t checksum = 0;

    // Calculate the raw checksum by summing all characters
    while (*msg) {
        checksum += *msg++;
    }

    // Apply the mask to ignore the carry bits, keeping the result within 6 bits
    checksum &= 0xFF;

    // Separate the high and low 4-bit nibbles
    *checksum_high = (checksum >> 4) & 0x0F;
    *checksum_low = checksum & 0x0F;
}

/*
 * Function: hex_to_uint16
 * Arguments: const char *hex_str
 *
 * Description:
 * This function converts a hexadecimal string to a 16-bit unsigned integer. It iterates
 * through each character of the input string, converts it to its corresponding integer
 * value, and accumulates the result by shifting and combining the bits. The function
 * handles both numeric (0-9) and alphabetic (A-F) characters.
 *
 * - hex_str: Pointer to the null-terminated string representing the hexadecimal value.
 *
 * Returns:
 * - The 16-bit unsigned integer result of the conversion.
 */
uint16_t hex_to_uint16(const char *hex_str) {
    uint16_t result = 0;
    for (int i = 0; i < 4; i++) {
        char c = toupper(hex_str[i]);
        if (c >= '0' && c <= '9') {
            result = (result << 4) | (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            result = (result << 4) | (c - 'A' + 10);
        }
    }
    return result;
}



Message_Id get_message_ID(const char* message, Class_Id class)
{
    Message_Id id = INVALID;
    if((class == NAV) || (class == VHF))
    {
        for(int i = 0; i < NUM_MESSAGE_ID; i++)
        {
            if(strcmp(message, message_id_nav_vhf[i]) == 0)
            {
                id = i;
                break;
            }
        }
    }
    else if((class == ADF) || (class == TACAN))
    {
        for(int i = 0; i < NUM_MESSAGE_ID; i++)
        {
            if(strcmp(message, message_id_adf_tacan[i]) == 0)
            {
                id = i;
                break;
            }
        }
    }
    else
    {
        return INVALID;
    }

    return id;
}
/*
 * Function: decode_message
 * Arguments: uint8_t *rx_buffer, Identifier *ident, uint8_t *mhz, uint16_t *khz, uint8_t *f_flag
 *
 * Description:
 * This function decodes a received message stored in `rx_buffer` and extracts relevant information, such as the class ID,
 * message ID, MHz and kHz values, and a frequency flag. It also verifies the checksum to ensure message integrity.
 *
 * - rx_buffer: Pointer to the buffer containing the received message.
 * - ident: Pointer to the `Identifier` structure where the class and message ID will be stored.
 * - mhz: Pointer to a variable where the MHz value will be stored, if applicable.
 * - khz: Pointer to a variable where the kHz value will be stored, if applicable.
 * - f_flag: Pointer to a flag indicating whether frequency information is included in the message.
 *
 * Process:
 * 1. Copy the received message into a local buffer and ensure it starts with '$PATC' or '$PMRR' and ends with '\r\n'.
 * 2. Extract the class ID from the message and store it in `ident`.
 * 3. Extract the message ID and store it in `ident`.
 * 4. If the message ID indicates frequency information (`S_FREQ` or `A_FREQ`), extract the MHz and kHz values and set the frequency flag.
 * 5. Extract and calculate the checksum to verify the message integrity.
 * 6. Compare the received checksum with the calculated checksum.
 *
 * Returns:
 * - true if the message is valid and the checksum matches.
 * - false if the message is invalid or the checksum does not match.
 */
bool decode_message(uint8_t *rx_buffer, Identifier *ident, uint8_t *mhz, uint16_t *khz, uint8_t *f_flag)
{
    char received_msg[25];  // Buffer to extract the message data
    char cID;
    char message[3];
    uint8_t khz_length = 0;
    uint8_t received_checksum;
    uint8_t received_checksum_l;
    uint8_t received_checksum_h;
    uint8_t calculated_checksum_l;
    uint8_t calculated_checksum_h;
    uint8_t calculated_checksum;
    uint8_t len = strlen((char*)rx_buffer);
    memcpy(received_msg, (char*)rx_buffer, len);
    // Ensure the message starts with '$PATC' and ends with '\r\n'
    if ( received_msg[len - 2] == '\r' || received_msg[len - 1] == '\n') {
        if(strncmp(received_msg, "$PATC", 5) != 0 && strncmp(received_msg, "$PMRR", 5) != 0 && strncmp(received_msg, "$PATN", 5) != 0)
        {
            return false;  // Invalid format
        }
    }
    else
    {
        return false;  // Invalid format
    }

    cID = received_msg[5];
    for(int i = 0; i < NUM_CLASSES_ID; i++)
    {
        if(cID == class_id[i])
        {
            ident->class = i;
            break;
        }
    }
    sscanf(&received_msg[6], "%2s", message);

    ident->msg_id = get_message_ID(message, ident->class);
    if(ident->msg_id == INVALID)
    {
        return false;  // Invalid message ID
    }
    // Extract mhz and khz if applicable (freq_flag == true)
    if ((ident->msg_id == S_FREQ) || (ident->msg_id == A_FREQ)) {
        *f_flag = true;

        if((ident->class == NAV) || (ident->class == VHF) || (ident->class == TACAN) || (ident->class == ADF))
        {
            char mhz_c, khz_c;
            sscanf(&received_msg[8], "%c%c", &mhz_c, &khz_c);
            *mhz = mhz_c + CONVERSION_VAL_DEC;  // Reverse the conversion done during encoding
            int k = (khz_c - CONVERSION_VAL_DEC);
            k *= 25;
            *khz = k;  // Reverse the conversion done during encoding
        }
        else if((ident->class == HF) || (ident->class == ADF))
        {
            char mhz_c;
            sscanf(&received_msg[8], "%c", &mhz_c);
            *mhz = mhz_c - CONVERSION_VAL_DEC;  // Reverse the conversion done during encoding
            uint8_t comb_khz[5];
            for(uint8_t i=0; i<5; i ++)
            {
                comb_khz[i] = received_msg[9+i];
                if(received_msg[10+i] == 'N')
                {
                    comb_khz[i+1] = 0;
                    *khz = combine_frequency_khz(comb_khz);
                    khz_length = i+1;
                    break;
                }
                else if(i+1 == 5)
                {
                    return false;  // Invalid format
                }
            }
        }
    }
    else
    {
        *f_flag = false;
    }

    // Extract checksum from the message (it's always at the 2nd last and 3rd last position before '\r\n')
    sscanf(&received_msg[len - 4], "%c", &received_checksum_h);
    sscanf(&received_msg[len - 3], "%c", &received_checksum_l);
    received_checksum = ((received_checksum_h >= 'A') ? (received_checksum_h - 'A' + 10) : (received_checksum_h - '0')) << 4;
    received_checksum |= (received_checksum_l >= 'A') ? (received_checksum_l - 'A' + 10) : (received_checksum_l - '0');


    // Calculate the checksum of the received message (from 'P' to last data character before checksum)
    char cal_sum[10];
    strncpy(cal_sum, &received_msg[6], len-2-6);  // Extract the checksum
    cal_sum[len-10] =0;
    calculate_checksum(cal_sum, &calculated_checksum_h, &calculated_checksum_l);  // Use the same checksum function as during encoding
    calculated_checksum = (calculated_checksum_h << 4) | calculated_checksum_l;
    // Compare the checksums
    if (calculated_checksum != received_checksum) {
        return false;  // Checksum mismatch
    }
    return true;
}

/*
 * Function: decode_receive_data
 * Arguments: uint8_t* buffer
 *
 * Description:
 * This function decodes a received NMEA message. It creates an Identifier
 * structure and variables for MHz, kHz, and a frequency flag. The function
 * then calls decode_message to interpret the buffer contents.
 *
 * If decoding is successful, the function updates values based on the
 * class of the identifier and processes the incoming message accordingly
 * using the appropriate handler functions for NAV, VHF, ADF, or TACAN.
 */

void decode_receive_data(uint8_t *buffer)
{
	// Decode the NMEA Message
	Identifier ident;
    uint8_t mhz = 0;
    uint16_t khz = 0;
    uint8_t freq_flag = '\0';

    // Call the decode function with the message
    bool decode_success = decode_message(buffer, &ident, &mhz, &khz, &freq_flag);
    if(decode_success)
    {
    	message_parse[ident.class](&ident, mhz, khz);
		notify_tft();
		page_class = ident.class;
    }
}

/*
 * Function: encode_message
 * Arguments: uint8_t *tx_buffer, Class_Id c_index, Message_Id m_index, uint8_t mhz, uint16_t khz, char f_flag
 *
 * Description:
 * This function constructs and encodes a message into the transmission buffer `tx_buffer`.
 * It creates a proprietary message identifier, calculates a checksum, and encodes the message
 * with the checksum appended.
 *
 * - tx_buffer: Pointer to the buffer where the encoded message will be stored.
 * - c_index: The class index to be included in the message.
 * - m_index: The message identifier index.
 * - mhz: The megahertz part of the frequency to be encoded.
 * - khz: The kilohertz part of the frequency to be encoded.
 * - f_flag: Flag indicating whether to include frequency information in the message.
 *
 * Process:
 * 1. If `f_flag` is set, adjust `mhz` and convert `khz` to ASCII, then construct the message with frequency info.
 * 2. If `f_flag` is not set, construct the message without frequency info.
 * 3. Calculate the checksum from the message.
 * 4. Encode the checksum into hex characters and store the final message in `tx_buffer`.
 *
 * Note:
 * - The proprietary message identifier is constructed using `message_id` and the provided indices.
 * - The checksum is calculated from the message excluding start character, checksum, and end characters.
 */
void encode_message(uint8_t *tx_buffer, Class_Id c_index, Message_Id m_index, uint8_t mhz , uint16_t khz,  char f_flag)
{
    char msg[20]; // Buffer for constructing the message (excluding start, checksum, and end)


    // Construct the proprietary message identifier ("$PATCcNNdd")
    if((f_flag) && (c_index != ADF) && (c_index != HF))
    {
		mhz -= CONVERSION_VAL_DEC;
		khz = (khz /25);
		khz += CONVERSION_VAL_DEC;
		char khz_ascii = (char)khz;
		if((c_index == NAV) || (c_index == VHF))
		{
			sprintf(msg, "%s%c%cN", message_id_nav_vhf[m_index],(unsigned char)mhz,khz_ascii);
		}
		else if((c_index == TACAN) || (c_index == UHF))
		{
            sprintf(msg, "%s%c%cN", message_id_adf_tacan[m_index],(unsigned char)mhz,khz_ascii);
        }
    }
	else if((c_index == ADF) || (c_index == HF))
	{
		mhz += CONVERSION_VAL_DEC;
		uint8_t split_khz[5];
		uint8_t size = split_frequency_khz(khz, split_khz);
		split_khz[size] = 0;
		sprintf(msg, "%s%c%sN", message_id_adf_tacan[m_index],(unsigned char)mhz,split_khz);
	}
    else
    {
    	if((c_index == NAV) || (c_index == VHF))
		{
			sprintf(msg, "%s", message_id_nav_vhf[m_index]);
		}
		else if((c_index == ADF) || (c_index == TACAN) || (c_index == HF) || (c_index == UHF))
		{
            sprintf(msg, "%s", message_id_adf_tacan[m_index]);
        }
    }

    // Calculate checksum from the message (from 'P' to last data character)
    uint8_t checksum_h = 0;
	uint8_t checksum_l = 0;

	calculate_checksum(msg, &checksum_h, &checksum_l);

    // Encode the checksum into two hex characters (0-9, A-F) and store the final message in TX buffer
	if((c_index == NAV) || (c_index == VHF))
	{
    	sprintf((char*)tx_buffer, "$PATN%c%s%c%c\r\n",class_id[c_index], msg, (checksum_h+0x30), (checksum_l+0x30));
	}
	else if((c_index == ADF) || (c_index == TACAN) || (c_index == HF) || (c_index == UHF))
	{
    	sprintf((char*)tx_buffer, "$PATC%c%s%c%c\r\n",class_id[c_index], msg, (checksum_h+0x30), (checksum_l+0x30));
	}
}


/*
 * Function: handle_NAV_incoming_message
 * Arguments: Identifier* ident, uint8_t f_mhz, uint16_t f_khz
 *
 * Description:
 * This function processes incoming NAV messages based on the message ID.
 * Depending on the message ID, it updates the standby or active frequency,
 * handles volume, toggle, and squelch updates, resets the status, or handles
 * communication errors.
 *
 * - ident: Pointer to an Identifier structure containing message information.
 * - f_mhz: Frequency in MHz.
 * - f_khz: Frequency in kHz.
 */
void handle_NAV_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz)
{
	switch(ident->msg_id)
	{
		case S_FREQ:
				 // get the frequency and update standby
			cdu_parameters.standby_nav_freq = get_freq(f_mhz, f_khz);
			break;
		case A_FREQ:
				// get the frequency and update active
			cdu_parameters.active_nav_freq = get_freq(f_mhz, f_khz);
			break;
		case V_T_S:
				// update Volume, Toggle, squelch
			break;
		case Health:
			cdu_parameters.NAV_message_counter = 0;
			break;
		case RESET_STATUS:

				// update status
			break;
		case COM_ERROR:
				// handle com error
			break;
		default:
			break;
	}
}

/*
 * Function: handle_ADF_incoming_message
 * Arguments: Identifier* ident, uint8_t f_mhz, uint16_t f_khz
 *
 * Description:
 * This function processes incoming ADF messages based on the message ID.
 * Depending on the message ID, it updates the standby or active frequency,
 * handles volume, toggle, and squelch updates, resets the status, or handles
 * communication errors.
 *
 * - ident: Pointer to an Identifier structure containing message information.
 * - f_mhz: Frequency in MHz.
 * - f_khz: Frequency in kHz.
 */
void handle_ADF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz)
{
	switch(ident->msg_id)
	{
		case S_FREQ:
				 // get the frequency and update standby
			cdu_parameters.standby_adf_freq = get_freq(f_mhz, f_khz);
			break;
		case A_FREQ:
				// get the frequency and update active
			cdu_parameters.active_adf_freq = get_freq(f_mhz, f_khz);
			break;
		case V_T_S:
				// update Volume, Toggle, squelch
			break;
		case Health:
			cdu_parameters.ADF_message_counter = 0;
			break;
		case RESET_STATUS:
				// update status
			break;
		case COM_ERROR:
				// handle com error
			break;
		default:
			break;
	}
}

/*
 * Function: handle_VHF_incoming_message
 * Arguments: Identifier* ident, uint8_t f_mhz, uint16_t f_khz
 *
 * Description:
 * This function processes incoming VHF messages based on the message ID.
 * Depending on the message ID, it updates the standby or active frequency,
 * handles volume, toggle, and squelch updates, resets the status, or handles
 * communication errors.
 *
 * - ident: Pointer to an Identifier structure containing message information.
 * - f_mhz: Frequency in MHz.
 * - f_khz: Frequency in kHz.
 */
void handle_VHF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz)
{
	switch(ident->msg_id)
	{
		case S_FREQ:
				 // get the frequency and update standby
			cdu_parameters.standby_vhf_freq = get_freq(f_mhz, f_khz);
			break;
		case A_FREQ:
				// get the frequency and update active
			cdu_parameters.active_vhf_freq = get_freq(f_mhz, f_khz);
			break;
		case V_T_S:
				// update Volume, Toggle, squelch
			break;
		case Health:
			cdu_parameters.VHF_message_counter = 0;
			break;
		case RESET_STATUS:
				// update status
			break;
		case COM_ERROR:
				// handle com error
			break;
		default:
			break;
	}
}

/*
 * Function: handle_HF_incoming_message
 * Arguments: Identifier* ident, uint8_t f_mhz, uint16_t f_khz
 *
 * Description:
 * This function processes incoming HF messages based on the message ID.
 * Depending on the message ID, it updates the standby or active frequency,
 * handles volume, toggle, and squelch updates, resets the status, or handles
 * communication errors.
 *
 * - ident: Pointer to an Identifier structure containing message information.
 * - f_mhz: Frequency in MHz.
 * - f_khz: Frequency in kHz.
 */
void handle_HF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz)
{
	switch(ident->msg_id)
	{
		case S_FREQ:
				 // get the frequency and update standby
			cdu_parameters.standby_hf_freq = get_freq(f_mhz, f_khz);
			break;
		case A_FREQ:
				// get the frequency and update active
			cdu_parameters.active_hf_freq = get_freq(f_mhz, f_khz);
			break;
		case V_T_S:
				// update Volume, Toggle, squelch
			break;
		case Health:
			cdu_parameters.HF_message_counter = 0;
			break;
		case RESET_STATUS:
				// update status
			break;
		case COM_ERROR:
				// handle com error
			break;
		default:
			break;
	}
}

/*
 * Function: handle_UHF_incoming_message
 * Arguments: Identifier* ident, uint8_t f_mhz, uint16_t f_khz
 *
 * Description:
 * This function processes incoming UHF messages based on the message ID.
 * Depending on the message ID, it updates the standby or active frequency,
 * handles volume, toggle, and squelch updates, resets the status, or handles
 * communication errors.
 *
 * - ident: Pointer to an Identifier structure containing message information.
 * - f_mhz: Frequency in MHz.
 * - f_khz: Frequency in kHz.
 */
void handle_UHF_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz)
{
	switch(ident->msg_id)
	{
		case S_FREQ:
				 // get the frequency and update standby
			cdu_parameters.standby_uhf_freq = get_freq(f_mhz, f_khz);
			break;
		case A_FREQ:
				// get the frequency and update active
			cdu_parameters.active_uhf_freq = get_freq(f_mhz, f_khz);
			break;
		case V_T_S:
				// update Volume, Toggle, squelch
			break;
		case Health:
			cdu_parameters.UHF_message_counter = 0;
			break;
		case RESET_STATUS:
				// update status
			break;
		case COM_ERROR:
				// handle com error
			break;
		default:
			break;
	}
}
/*
 * Function: handle_TACAN_incoming_message
 * Arguments: Identifier* ident, uint8_t f_mhz, uint16_t f_khz
 *
 * Description:
 * This function processes incoming TACAN messages based on the message ID.
 * Depending on the message ID, it updates the standby or active frequency,
 * handles volume, toggle, and squelch updates, resets the status, or handles
 * communication errors.
 *
 * - ident: Pointer to an Identifier structure containing message information.
 * - f_mhz: Frequency in MHz.
 * - f_khz: Frequency in kHz.
 */
void handle_TACAN_incoming_message(Identifier *ident, uint8_t f_mhz, uint16_t f_khz)
{
	int8_t status;
	char tacan_channel[TACAN_S];
	switch(ident->msg_id)
	{
		case S_FREQ:
				 // get the frequency and update standby
				cdu_parameters.standby_tacan_freq = get_freq(f_mhz, f_khz);
				memset(tacan_channel, 0, TACAN_S);
				status = find_channel(cdu_parameters.standby_tacan_freq, tacan_channel);
				if(status >= 0)
				{
					memcpy(cdu_parameters.standby_channel, tacan_channel, strlen(tacan_channel));
				}
			break;
		case A_FREQ:
				// get the frequency and update active
				cdu_parameters.active_tacan_freq = get_freq(f_mhz, f_khz);
				memset(tacan_channel, 0, TACAN_S);
				status = find_channel(cdu_parameters.active_tacan_freq, tacan_channel);
				if(status >= 0)
				{
					memcpy(cdu_parameters.active_channel, tacan_channel, strlen(tacan_channel));
				}
			break;
		case V_T_S:
				// update Volume, Toggle, squelch
			break;
		case Health:
			cdu_parameters.TACAN_message_counter = 0;
			break;
		case RESET_STATUS:
				// update status
			break;
		case COM_ERROR:
				// handle com error
			break;
		default:
			break;
	}
}

/*
 * @brief This function calculates the frequency in MHz from given MHz and KHz parts.
 *
 * @param f_mhz The MHz part of the frequency.
 * @param f_khz The KHz part of the frequency.
 *+
 * @return The calculated frequency in MHz as a float.
 *
 * The function takes two parameters, f_mhz and f_khz, which represent the MHz and KHz parts of the frequency, respectively.//+
 * It converts the KHz part to a float and adds it to the MHz part to calculate the frequency in MHz.//+
 * The result is then returned as a float.
 */
float get_freq( uint8_t f_mhz, uint16_t f_khz)
{
    float f = f_mhz;
    f += ((float)f_khz / 1000);
    return f;
}

/*
 * Function: notify_tft
 * Arguments: None
 *
 * Description:
 * This function sends a notification to the TFT task by placing a specific key
 * (DATA_1553) into the key queue. It ensures that the key is sent to the queue
 * only if the queue handle (xKeyQueue) is not NULL.
 *
 * - key: The character key representing the notification to be sent to the TFT task.
 */
void notify_tft(void)
{
	char key = DATA_1553;
    if (xKeyQueue != NULL)
    {
        xQueueSend(xKeyQueue, &key, portMAX_DELAY);
    }
}

/*
 * Function: get_class_Id
 * Arguments: None
 *
 * Description:
 * This function returns the current value of the `page_class` variable.
 * The return type is `Class_Id`, which presumably is an enumerated type
 * or a type definition representing different class IDs.
 *
 * Returns:
 * - The current value of the `page_class` variable.
 */
Class_Id get_class_Id(void)
{
	return page_class;
}

/*
 * Function: combine_frequency_khz
 * Arguments: uint8_t* khz_buf
 *
 * Description:
 * This function combines a buffer of individual digit characters into a single
 * 16-bit integer representing a frequency in kHz.
 *
 * - khz_buf: Pointer to the buffer containing the individual digit characters.
 *
 * Process:
 * 1. Initialize the result variable to 0.
 * 2. Iterate over each character in the buffer:
 *    a. Convert the character to its integer value by subtracting '0'.
 *    b. Multiply the current result by 10 and add the integer value of the character.
 * 3. Return the combined result.
 *
 * Returns:
 * - The combined 16-bit integer frequency value.
 */

uint16_t combine_frequency_khz(uint8_t* khz_buf)
{
    uint16_t res = 0;
    for(uint8_t i = 0; i < strlen((char*)khz_buf); i++)
    {
        res = res * 10 + (khz_buf[i] - '0');
    }
    return res;
}


/*
 * Function: split_frequency_khz
 * Arguments: uint16_t f_khz, uint8_t* khz_buf
 *
 * Description:
 * This function converts a frequency value in kHz (represented as a 16-bit integer)
 * to its decimal representation and stores each digit in the provided buffer.
 *
 * - f_khz: The frequency value in kHz to be split into individual digits.
 * - khz_buf: Pointer to the buffer where the individual digits will be stored as characters.
 *
 * Process:
 * 1. Check if the frequency value is 0. If so, store '0' in the buffer and return a size of 1.
 * 2. Initialize temporary storage and a size counter.
 * 3. While the frequency value is not 0:
 *    a. Get the last digit of the frequency value by taking the modulus 10.
 *    b. Store the digit in the temporary storage.
 *    c. Divide the frequency value by 10 to remove the last digit.
 *    d. Increment the size counter.
 * 4. Reverse the order of the digits in the temporary storage and store them in the buffer.
 * 5. Return the size of the buffer.
 *
 * Returns:
 * - The number of digits stored in the buffer.
 */
uint8_t split_frequency_khz(uint16_t f_khz, uint8_t* khz_buf)
{
    uint16_t res = 0, size = 0;
    uint8_t tmp[5];
    if(f_khz == 0)
    {
        khz_buf[0] = '0';
        return 1;
    }

    while(f_khz != 0)
    {
        res = f_khz % 10;
        f_khz /= 10;
        tmp[size] = res;
        size++;
    }
    for(uint8_t i = 0; i < size; i++)
    {
        khz_buf[size - i - 1] = tmp[i] + '0';
    }
    return size;
}
