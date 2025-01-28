#ifndef D__JIM_CDU_NAV_RCU_RCU_WORKSPACE_RCU_FIRMWARE_V0_0_CORE_INC_NAV_FUNCS_H_
#define D__JIM_CDU_NAV_RCU_RCU_WORKSPACE_RCU_FIRMWARE_V0_0_CORE_INC_NAV_FUNCS_H_
#include <stdint.h>

#include "data_handler.h"

// functions for nav parameter structure
void update_flash(void);
void Write_NAV_RCU_Parameters(uint32_t address, s_NAV_RCU_Parameters *params);
void Read_NAV_RCU_Parameters(uint32_t address, s_NAV_RCU_Parameters *params);
void Load_Default_NAV_RCU_Parameters(s_NAV_RCU_Parameters *params);
void Load_NAV_RCU_Parameters(s_NAV_RCU_Parameters *params);
// functions for memory of frequency

void write_frequencies_to_flash(const float *frequencies, uint8_t count);
void read_frequencies_from_flash(uint32_t address, float *frequencies, uint8_t max_count);
void update_frequency_in_flash(uint8_t index, float new_frequency);

uint8_t map_volume(uint8_t volume);

uint8_t inverse_volume_maping(uint8_t volume);

void swap_active_standby(void);

#endif  // D__JIM_CDU_NAV_RCU_RCU_WORKSPACE_RCU_FIRMWARE_V0_0_CORE_INC_NAV_FUNCS_H_
