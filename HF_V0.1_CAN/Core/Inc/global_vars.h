/*
 * global_vars.h
 *
 *  Created on: Jan 27, 2025
 *      Author: Admin
 */

#ifndef INC_GLOBAL_VARS_H_
#define INC_GLOBAL_VARS_H_

#include "data_handler.h"
#include "global_vars.h"
#include "stdint.h"



typedef struct
{
    uint8_t g_current_page;
    uint8_t g_selectedPreset;

    uint16_t g_standby_mhz_knob;
    uint16_t g_standby_khz_knob;
    uint8_t g_volume_knob;

    uint8_t g_saved_channel_khz;
    uint8_t g_saved_channel_mhz;


    B_Status cursor_blink;
    uint8_t current_cursor_location;
    uint8_t previous_cursor_location;
    uint8_t cursor_map[5];
    uint8_t cursor_index;
} s_global_vars;

extern s_global_vars g_vars;

// Declarations
extern float saved_channels[saved_channel_SIZE];
extern uint8_t saved_channel_index;

void update_globals(s_HF_Parameters *obj);

void set_saved_channel_index(uint8_t index);
uint8_t get_saved_channel_index();

#endif /* INC_GLOBAL_VARS_H_ */
