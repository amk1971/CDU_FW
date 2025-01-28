#include "nav_funcs.h"

#include "flash.h"
#include "string.h"

/* ------------------------- Default Parameter Values --------------------------- */
const s_NAV_RCU_Parameters NAV_RCU_Default_Values =
{ .standby_freq = 109.350, .active_freq = 108.50, .volume = 5, .mem_page = OFF,
		.power_on = OFF, };

uint8_t inverse_volume_map[20][2] =
{
{ 0, 0 },
{ 1, 15 },
{ 2, 28 },
{ 3, 42 },
{ 4, 55 },
{ 5, 69 },
{ 6, 82 },
{ 7, 95 },
{ 8, 109 },
{ 9, 122 },
{ 10, 136 },
{ 11, 149 },
{ 12, 163 },
{ 13, 176 },
{ 14, 189 },
{ 15, 203 },
{ 16, 216 },
{ 17, 230 },
{ 18, 243 },
{ 19, 255 } };

void update_flash(void)
{
	Write_NAV_RCU_Parameters((uint32_t) PARAMETER_FLASH_ADDRESS,
			&nav_parameters);
}

/*
 * Function: Write_NAV_RCU_Parameters
 * Arguments: uint32_t address, NAV_RCU_Parameters* params
 *
 * Description:
 * This function writes the NAV_RCU parameters to the specified flash memory address.
 * It converts the NAV_RCU_Parameters structure to a byte array and writes the data in
 * chunks of up to 256 bytes using the Flash_WriteData function.
 *
 * - address: The flash memory address where the NAV_RCU parameters will be written.
 * - params: Pointer to the NAV_RCU_Parameters structure containing the data to be written.
 */
void Write_NAV_RCU_Parameters(uint32_t address, s_NAV_RCU_Parameters *params)
{
	// Get the size of the structure
	uint32_t structSize = sizeof(s_NAV_RCU_Parameters);

	// Cast the structure to a byte array for writing
	uint8_t *data = (uint8_t*) params;

	Flash_SectorErase(address);

	// While there is data left to write, write it in chunks of 256 bytes
	while (structSize > 0)
	{
		// Determine how much to write in the current iteration (maximum 256 bytes)
		uint32_t writeLength = (structSize > BY25Q16BS_PAGE_SIZE) ?
		BY25Q16BS_PAGE_SIZE :
																	structSize;

		// Write the data to the current address
		Flash_WriteData(address, data, writeLength);

		// Update address, data pointer, and remaining structure size
		address += writeLength;
		data += writeLength;
		structSize -= writeLength;
	}
}

/*
 * Function: Read_NAV_RCU_Parameters
 * Arguments: uint32_t address, NAV_RCU_Parameters* params
 *
 * Description:
 * This function reads the NAV_RCU parameters from the specified flash memory address
 * into the provided NAV_RCU_Parameters structure. It reads the data in chunks of up
 * to 256 bytes using the Flash_ReadData function.
 *
 * - address: The flash memory address from which the NAV_RCU parameters will be read.
 * - params: Pointer to the NAV_RCU_Parameters structure where the read data will be stored.
 */
void Read_NAV_RCU_Parameters(uint32_t address, s_NAV_RCU_Parameters *params)
{
	// Get the size of the structure
	uint32_t structSize = sizeof(s_NAV_RCU_Parameters);

	// Cast the structure to a byte array for reading
	uint8_t *data = (uint8_t*) params;

	// While there is data left to read, read it in chunks of 256 bytes
	while (structSize > 0)
	{
		// Determine how much to read in the current iteration (maximum 256 bytes)
		uint32_t readLength = (structSize > BY25Q16BS_PAGE_SIZE) ?
		BY25Q16BS_PAGE_SIZE :
																	structSize;

		// Read the data from the current address
		Flash_ReadData(address, data, readLength);

		// Update address, data pointer, and remaining structure size
		address += readLength;
		data += readLength;
		structSize -= readLength;
	}
}

/*
 * Function: Load_Default_NAV_RCU_Parameters
 * Arguments: NAV_RCU_Parameters* params
 *
 * Description:
 * This function loads default values into the NAV_RCU_Parameters structure. It copies
 * the entire structure from the constant default values defined in NAV_RCU_Default_Values.
 *
 * - params: Pointer to the NAV_RCU_Parameters structure where the default values will be loaded.
 */
void Load_Default_NAV_RCU_Parameters(s_NAV_RCU_Parameters *params)
{
	// Copy the entire structure from the constant defaults
	memcpy(params, &NAV_RCU_Default_Values, sizeof(s_NAV_RCU_Parameters));
}

/*
 * Function: Load_NAV_RCU_Parameters
 * Arguments: NAV_RCU_Parameters* params
 *
 * Description:
 * This function loads NAV_RCU parameters from flash memory or default values if the flash is empty.
 * It first checks if the flash memory at the specified address is empty. If it is empty, it loads
 * the default values into the NAV_RCU_Parameters structure. Otherwise, it reads the NAV_RCU parameters
 * from flash memory into the structure.
 *
 * - params: Pointer to the NAV_RCU_Parameters structure where the data will be loaded.
 */
void Load_NAV_RCU_Parameters(s_NAV_RCU_Parameters *params)
{
	uint32_t address = (uint32_t) PARAMETER_FLASH_ADDRESS;

	// Check if flash is empty (assuming sizeof(NAV_RCU_Parameters) data is stored at 'address')
	if (flash_check())
	{
#if UPLOAD_DEFAULT
        Load_Default_NAV_RCU_Parameters(params);
        Write_NAV_RCU_Parameters(address, params);
        NAV_RCU_Parameters NAV_RCU_test_flash;
        Read_NAV_RCU_Parameters(address, &NAV_RCU_test_flash);
#endif
		if (Is_Flash_Empty(address, sizeof(s_NAV_RCU_Parameters)))
		{
			// Flash is empty, load default values from the global structure
			Load_Default_NAV_RCU_Parameters(params);
			Write_NAV_RCU_Parameters(address, params);
			// test if default parameters are written into flash.
			//			NAV_RCU_Parameters NAV_RCU_test_flash;
			//			Read_NAV_RCU_Parameters(address, &NAV_RCU_test_flash);
		}
		else
		{
			// Flash is not empty, read the structure from flash
			Read_NAV_RCU_Parameters(address, params);
		}
	}
	else
	{
		// Flash is empty, load default values from the global structure
		Load_Default_NAV_RCU_Parameters(params);
	}
}

void write_frequencies_to_flash(const float *frequencies,
		uint8_t num_frequencies)
{
	// Ensure the number of frequencies doesn't exceed what can fit in one page
	if (num_frequencies > NUM_FREQUENCIES_PER_PAGE)
	{
		num_frequencies = NUM_FREQUENCIES_PER_PAGE; // Limit to the max frequencies per page
	}

	// Prepare buffer for writing
	uint8_t buffer[BY25Q16BS_PAGE_SIZE] =
	{ 0 };
	memcpy(buffer, frequencies, num_frequencies * sizeof(float));

	// Erase the sector containing the frequency page
	uint32_t sector_address = FREQUENCY_FLASH_ADDRESS
			& ~(BY25Q16BS_SECTOR_SIZE - 1);
	Flash_SectorErase(sector_address);

	// Write the buffer to the flash page
	Flash_WriteData(FREQUENCY_FLASH_ADDRESS, buffer, BY25Q16BS_PAGE_SIZE);
}
void read_frequencies_from_flash(uint32_t address, float *frequencies, uint8_t max_count)
{
    uint8_t buffer[BY25Q16BS_PAGE_SIZE] = {0};

    // Read one page of data from flash
    Flash_ReadData(address, buffer, BY25Q16BS_PAGE_SIZE);

    // Calculate the number of floats stored in the page
    uint8_t num_frequencies = BY25Q16BS_PAGE_SIZE / sizeof(float);

    // Limit the number of frequencies to the smaller of max_count or the maximum possible in a page
    num_frequencies = (num_frequencies > max_count) ? max_count : num_frequencies;

    // Copy the floats from the buffer into the output array
    memcpy(frequencies, buffer, num_frequencies * sizeof(float));
}

void update_frequency_in_flash(uint8_t index, float new_frequency)
{
	if (index >= NUM_FREQUENCIES_PER_PAGE)
	{
		// Index out of range
		return;
	}

	float frequencies[NUM_FREQUENCIES_PER_PAGE] =
	{ 0 };

	// Read existing frequencies
	read_frequencies_from_flash(FREQUENCY_FLASH_ADDRESS, frequencies, NUM_FREQUENCIES);

	// Update the specific frequency
	frequencies[index] = new_frequency;

	// Write the updated frequencies back to flash
	write_frequencies_to_flash(frequencies, NUM_FREQUENCIES_PER_PAGE);
}

void swap_active_standby(void)
{
	float tmp = nav_parameters.active_freq;
	nav_parameters.active_freq = nav_parameters.standby_freq;
	nav_parameters.standby_freq = tmp;
}

uint8_t map_volume(uint8_t volume)
{
	if (volume <= 13)
	{
		return 0;
	}
	else if (14 <= volume && volume <= 26)
	{
		return 1;
	}
	else if (27 <= volume && volume <= 40)
	{
		return 2;
	}
	else if (41 <= volume && volume <= 53)
	{
		return 3;
	}
	else if (54 <= volume && volume <= 67)
	{
		return 4;
	}
	else if (68 <= volume && volume <= 80)
	{
		return 5;
	}
	else if (81 <= volume && volume <= 93)
	{
		return 6;
	}
	else if (94 <= volume && volume <= 107)
	{
		return 7;
	}
	else if (108 <= volume && volume <= 120)
	{
		return 8;
	}
	else if (121 <= volume && volume <= 134)
	{
		return 9;
	}
	else if (135 <= volume && volume <= 147)
	{
		return 10;
	}
	else if (148 <= volume && volume <= 161)
	{
		return 11;
	}
	else if (162 <= volume && volume <= 174)
	{
		return 12;
	}
	else if (175 <= volume && volume <= 187)
	{
		return 13;
	}
	else if (188 <= volume && volume <= 201)
	{
		return 14;
	}
	else if (202 <= volume && volume <= 214)
	{
		return 15;
	}
	else if (215 <= volume && volume <= 228)
	{
		return 16;
	}
	else if (229 <= volume && volume <= 241)
	{
		return 17;
	}
	else if (242 <= volume && volume <= 254)
	{
		return 18;
	}
	else if (volume == 255)
	{
		return 19;
	}
	return 0; // Default return if no condition is met (shouldn't happen for valid input)
}

uint8_t inverse_volume_maping(uint8_t volume)
{
	if (volume < 20)
	{
		return inverse_volume_map[volume][1]; // Return the corresponding mapped value
	}
	return 0;  // Default return if volume is out of range
}
