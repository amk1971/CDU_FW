#include "..\HF\HF_funcs.h"

#include "flash.h"
#include "string.h"

/* ------------------------- Default Parameter Values --------------------------- */
const s_HF_Parameters HF_RCU_Default_Values = {
    .standby_freq = 2156.4,
    .volume = 5,
    .PROG = OFF,
    .power_on = OFF,
};

uint8_t inverse_volume_map[20][2] = {{0, 0},    {1, 15},   {2, 28},   {3, 42},   {4, 55},   {5, 69},   {6, 82},
                                     {7, 95},   {8, 109},  {9, 122},  {10, 136}, {11, 149}, {12, 163}, {13, 176},
                                     {14, 189}, {15, 203}, {16, 216}, {17, 230}, {18, 243}, {19, 255}};

void update_flash(void) { Write_HF_RCU_Parameters((uint32_t)PARAMETER_FLASH_ADDRESS, &HF_parameters); }

/*
 * Function: Write_HF_RCU_Parameters
 * Arguments: uint32_t address, HF_RCU_Parameters* params
 *
 * Description:
 * This function writes the HF_RCU parameters to the specified flash memory address.
 * It converts the HF_RCU_Parameters structure to a byte array and writes the data in
 * chunks of up to 256 bytes using the Flash_WriteData function.
 *
 * - address: The flash memory address where the HF_RCU parameters will be written.
 * - params: Pointer to the HF_RCU_Parameters structure containing the data to be written.
 */
void Write_HF_RCU_Parameters(uint32_t address, s_HF_Parameters *params)
{
    // Get the size of the structure
    uint32_t structSize = sizeof(s_HF_Parameters);

    // Cast the structure to a byte array for writing
    uint8_t *data = (uint8_t *)params;

    Flash_SectorErase(address);

    // While there is data left to write, write it in chunks of 256 bytes
    while (structSize > 0)
    {
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
 * Function: Read_HF_RCU_Parameters
 * Arguments: uint32_t address, HF_RCU_Parameters* params
 *
 * Description:
 * This function reads the HF_RCU parameters from the specified flash memory address
 * into the provided HF_RCU_Parameters structure. It reads the data in chunks of up
 * to 256 bytes using the Flash_ReadData function.
 *
 * - address: The flash memory address from which the HF_RCU parameters will be read.
 * - params: Pointer to the HF_RCU_Parameters structure where the read data will be stored.
 */
void Read_HF_RCU_Parameters(uint32_t address, s_HF_Parameters *params)
{
    // Get the size of the structure
    uint32_t structSize = sizeof(s_HF_Parameters);

    // Cast the structure to a byte array for reading
    uint8_t *data = (uint8_t *)params;

    // While there is data left to read, read it in chunks of 256 bytes
    while (structSize > 0)
    {
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
 * Function: Load_Default_HF_RCU_Parameters
 * Arguments: HF_RCU_Parameters* params
 *
 * Description:
 * This function loads default values into the HF_RCU_Parameters structure. It copies
 * the entire structure from the constant default values defined in HF_RCU_Default_Values.
 *
 * - params: Pointer to the HF_RCU_Parameters structure where the default values will be loaded.
 */
void Load_Default_HF_RCU_Parameters(s_HF_Parameters *params)
{
    // Copy the entire structure from the constant defaults
    memcpy(params, &HF_RCU_Default_Values, sizeof(s_HF_Parameters));
}

/*
 * Function: Load_HF_RCU_Parameters
 * Arguments: HF_RCU_Parameters* params
 *
 * Description:
 * This function loads HF_RCU parameters from flash memory or default values if the flash is empty.
 * It first checks if the flash memory at the specified address is empty. If it is empty, it loads
 * the default values into the HF_RCU_Parameters structure. Otherwise, it reads the HF_RCU parameters
 * from flash memory into the structure.
 *
 * - params: Pointer to the HF_RCU_Parameters structure where the data will be loaded.
 */
void Load_HF_RCU_Parameters(s_HF_Parameters *params)
{
    uint32_t address = (uint32_t)PARAMETER_FLASH_ADDRESS;

    // Check if flash is empty (assuming sizeof(HF_RCU_Parameters) data is stored at 'address')
    if (flash_check())
    {
#if UPLOAD_DEFAULT
        Load_Default_HF_RCU_Parameters(params);
        Write_HF_RCU_Parameters(address, params);
        s_HF_Parameters HF_RCU_test_flash;
        Read_HF_RCU_Parameters(address, &HF_RCU_test_flash);
#endif
        if (Is_Flash_Empty(address, sizeof(s_HF_Parameters)))
        {
            // Flash is empty, load default values from the global structure
            Load_Default_HF_RCU_Parameters(params);
            Write_HF_RCU_Parameters(address, params);
            // test if default parameters are written into flash.
            //			HF_RCU_Parameters HF_RCU_test_flash;
            //			Read_HF_RCU_Parameters(address, &HF_RCU_test_flash);
        }
        else
        {
            // Flash is not empty, read the structure from flash
            Read_HF_RCU_Parameters(address, params);
        }
    }
    else
    {
        // Flash is empty, load default values from the global structure
        Load_Default_HF_RCU_Parameters(params);
    }
}

void write_channels_to_flash(const float *frequencies, uint8_t num_frequencies)
{
    // Ensure the number of frequencies doesn't exceed what can fit in one page
    if (num_frequencies > NUM_CHANNELS_PER_PAGE)
    {
        num_frequencies = NUM_CHANNELS_PER_PAGE;  // Limit to the max frequencies per page
    }

    // Prepare buffer for writing
    uint8_t buffer[BY25Q16BS_PAGE_SIZE] = {0};
    memcpy(buffer, frequencies, num_frequencies * sizeof(float));

    // Erase the sector containing the frequency page
    uint32_t sector_address = CHANNEL_FLASH_ADDRESS & ~(BY25Q16BS_SECTOR_SIZE - 1);
    Flash_SectorErase(sector_address);

    // Write the buffer to the flash page
    Flash_WriteData(CHANNEL_FLASH_ADDRESS, buffer, BY25Q16BS_PAGE_SIZE);
}
void read_channels_from_flash(uint32_t address, float *frequencies, uint8_t max_count)
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
    if (index >= NUM_CHANNELS_PER_PAGE)
    {
        // Index out of range
        return;
    }

    float frequencies[NUM_CHANNELS_PER_PAGE] = {0};

    // Read existing frequencies
    read_channels_from_flash(CHANNEL_FLASH_ADDRESS, frequencies, NUM_FREQUENCIES);

    // Update the specific frequency
    frequencies[index] = new_frequency;

    // Write the updated frequencies back to flash
    write_channels_to_flash(frequencies, NUM_CHANNELS_PER_PAGE);
}

void swap_active_standby(void)
{
    //    float tmp = HF_parameters.active_freq;
    //    HF_parameters.active_freq = HF_parameters.standby_freq;
    //    HF_parameters.standby_freq = tmp;
}

const char *map_volume_into_ascii(uint8_t volume)
{
    static char mapped_values[][3] = {"00", "01", "04", "07", "12", "17", "22", "27", "32", "37", "42",
                                      "47", "52", "57", "62", "67", "72", "77", "82", "87", "99"};
    if (volume > 20) return "XX";  // Return an error indicator

    return mapped_values[volume];
}

uint8_t inverse_volume_maping(uint8_t volume)
{
    if (volume < 20)
    {
        return inverse_volume_map[volume][1];  // Return the corresponding mapped value
    }
    return 0;  // Default return if volume is out of range
}
