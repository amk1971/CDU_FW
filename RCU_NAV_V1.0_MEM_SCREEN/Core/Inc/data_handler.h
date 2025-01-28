/*
 * data_handler.h
 *
 *  Created on: Nov 29, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_DATA_HANDLER_H_
#define INC_DATA_HANDLER_H_

#include <stdint.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"

/* ------------------------------------ MACROS --------------------------------- */

// Event group bits
#define EVENT_GP_SW1_PRESS (1 << 0)
#define EVENT_GP_SW2_PRESS (1 << 1)
#define EVENT_GP_SW3_PRESS (1 << 2)

#define EVENT_RIGHT_SW_PRESS (1 << 3)        // right rotary
#define EVENT_LEFT_SW_NORMAL_PRESS (1 << 4)  // left rotary
#define EVENT_LEFT_SW_LONG_PRESS (1 << 5)    // left rotary

#define EVENT_GP_SW_PRESET (1 << 6)  // preset button
#define EVENT_GP_SW_DELETE (1 << 7)

#define NUM_FREQUENCIES 15
typedef enum
{
    OFF = 0,
    ON
} B_Status;

typedef enum
{
    CDU,
    NAV_REC
} COM_Unit;

typedef struct
{
    float standby_freq;
    float active_freq;
    float default_freq;
    uint8_t memory_number;
    uint8_t volume;
    B_Status memory_status;
    B_Status power_on;
    B_Status mem_page;
} s_NAV_RCU_Parameters;

extern xQueueHandle xMainNotifyQueue;

extern s_NAV_RCU_Parameters nav_parameters;

extern EventGroupHandle_t buttonEventGroup;

void general_purpose_switches_thread(void *pvParameters);
void NAV_RCU_main_thread(void *pvParameters);

float get_frequency(uint8_t index);
void set_frequency(uint8_t index, float frequency);
uint8_t get_current_index();
void set_current_index(uint8_t index);

#endif /* INC_DATA_HANDLER_H_ */
