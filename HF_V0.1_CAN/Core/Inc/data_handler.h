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
#include "serial_handler.h"

/* ------------------------------------ MACROS --------------------------------- */

// Event group bits
#define EVENT_GP_SW1_PRESS (1 << 0)
#define EVENT_GP_SW2_PRESS (1 << 1)
#define EVENT_GP_SW3_PRESS (1 << 2)

#define EVENT_RIGHT_SW_PRESS (1 << 3)        // right rotary
#define EVENT_LEFT_SW_NORMAL_PRESS (1 << 4)  // left rotary
#define EVENT_LEFT_SW_LONG_PRESS (1 << 5)    // left rotary

#define EVENT_GP_SW_PROG_EXIT (1 << 6)  // preset button
#define EVENT_GP_SW_DELETE_STO (1 << 7)

#define NUM_FREQUENCIES 15

// #define RCU_ID 'A'  // A for HF
typedef enum
{
    S_FREQ,
    VOLUME,
    STO_ON,
    STO_OFF,
    POWER_ON,
    POWER_OFF,
    INVALID
} Message_ID;

typedef enum
{
    OFF = 0,
    ON
} B_Status;

typedef enum
{
    CDU,
    HF_REC
} COM_Unit;

typedef struct
{
    uint8_t mhz;
    uint16_t khz;
    Message_ID ID;
    B_Status lock_s;
    bool freq_flag;
    B_Status BFO;
    B_Status HF;
    uint8_t volume;

} Identifier;

typedef struct
{
    float tuned_freq;
    uint8_t Channel;
    uint8_t volume;
    uint8_t FRQ_CH : 2;
    char volume_ascii[2];
    B_Status power_on;
    B_Status STO;
    B_Status PROG;
    B_Status CH_PAGE;
    B_Status ON_OFF;
    B_Status Cursor;
    Message_ID ID;
} s_HF_Parameters;

extern xQueueHandle xMainNotifyQueue;

extern s_HF_Parameters HF_parameters;

extern EventGroupHandle_t buttonEventGroup;

float get_channel(uint8_t index);
void set_channel(uint8_t index, float frequency);

void get_update_data_CDU(Identifier *ident);
void send_data_to_transmission_queue_CDU(Message_ID id);
void send_data_to_transmission_queue_HF_RECEIVER(Message_ID id);

uint8_t get_current_index();
void set_current_index(uint8_t index);

#endif /* INC_DATA_HANDLER_H_ */
