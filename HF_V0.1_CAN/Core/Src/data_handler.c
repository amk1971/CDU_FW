/*
 * data_handler.c
 *
 *  Created on: Nov 29, 2024
 *      Author: Hamza Javed
 */

/* ------------------------------------ Headers -------------------------------- */
#include "data_handler.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\HF\HF_funcs.h"
#include "..\LCD\rcu_display.h"
#include "..\SWITCHES_Encoder\general_switches.h"
#include "..\SWITCHES_Encoder\rotary_encoder.h"
#include "Flash.h"
#include "FreeRTOS.h"
#include "debug_console.h"
#include "global_vars.h"
#include "main.h"
#include "serial_handler.h"
#include "task.h"

// flags for rotray

/* ------------------------------ Variables ------------------------------------- */

void send_data_to_transmission_queue_HF_RECEIVER(Message_ID id)
{
    HF_parameters.ID = id;
    switch (HF_parameters.ID)
    {
        case POWER_ON:
            HF_parameters.ON_OFF = ON;
            break;
        case POWER_OFF:
            HF_parameters.ON_OFF = OFF;
            HF_parameters.CH_PAGE = OFF;
            HF_parameters.STO = OFF;
            HF_parameters.standby_freq = 0;
            HF_parameters.volume = 0;
            break;
        case STO_ON:
            HF_parameters.STO = ON;
            break;
        case STO_OFF:
            HF_parameters.STO = OFF;
            break;




        case VOLUME:
            HF_parameters.volume = g_vars.g_volume_knob;
            break;

        case S_FREQ:

            break;
        default:
            break;
    }
    if (xHFRecTxQueue != NULL)
    {
        if (xQueueSend(xHFRecTxQueue, &HF_parameters, (TickType_t)(10)) != pdPASS)
        {
#if DEBUG_CONSOLE
            debug_print("DH xHFRecTxQueue Failed to queue\n");
#endif
        }
    }
}

void send_data_to_transmission_queue_CDU(Message_ID id)
{
    Identifier ident;

    ident.ID = id;
    if (ident.ID == S_FREQ)
    {
        ident.mhz = g_vars.g_standby_mhz_knob;
        ident.khz = g_vars.g_standby_khz_knob * 10;
    }
    //    else if (ident.ID == A_FREQ)
    //    {
    //        ident.mhz = (uint8_t)HF_parameters.active_freq;                    // Extract the MHz part
    //        ident.khz = ((HF_parameters.active_freq - ident.mhz) * 100) * 10;  // Extract the kHz part
    //    }
    else if (ident.ID == VOLUME)
    {
        ident.volume = HF_parameters.volume;
    }

    if (xCDUCOMTxQueue != NULL)
    {
        if (xQueueSend(xCDUCOMTxQueue, &ident, (TickType_t)(10)) != pdPASS)
        {
#if DEBUG_CONSOLE
            debug_print("DH xCDUCOMTxQueue Failed to queue\n");
#endif
        }
    }
}

/* --------------------------- Update Helper Functions ----------------------------- */
void get_update_data_CDU(Identifier *ident)
{
    Message_ID id = ident->ID;
    switch (id)
    {
        case S_FREQ:
            HF_parameters.standby_freq = ident->mhz + (ident->khz / 1000.0);
            g_vars.g_standby_mhz_knob = (uint8_t)HF_parameters.standby_freq;
            g_vars.g_standby_khz_knob =
                (uint8_t)(((HF_parameters.standby_freq - g_vars.g_standby_mhz_knob) + 0.001) * 100);
            break;
        case VOLUME:
            HF_parameters.volume = ident->volume;
            break;

        default:
            break;
    }
}

float get_channel(uint8_t index)
{
    if (index < NUM_FREQUENCIES)
    {
        return saved_channels[index];
    }
    return 0.0f;
}  // Default value if index is invalid }
void set_channel(uint8_t index, float frequency)
{
    if (index < NUM_FREQUENCIES)			//Saved Chanel Array Bound Checking
    {
        saved_channels[index] = frequency;
    }
}
// uint8_t get_current_index() { return current_index_saved_channels; }
//
// void set_current_index(uint8_t index)
//{
//     if (index < NUM_FREQUENCIES)
//     {
//         current_index_saved_channels = index;
//     }
// }
