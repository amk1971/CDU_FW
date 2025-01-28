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

#include "Flash.h"
#include "FreeRTOS.h"
#include "NAV_funcs.h"
#include "debug_console.h"
#include "main.h"
#include "rcu_display.h"
#include "rotary_encoder.h"
#include "serial_handler.h"
#include "task.h"

// flags for rotray
extern bool volatile read_encoder_standby_mhz_flag;
extern bool volatile read_encoder_standby_khz_flag;
extern bool volatile read_encoder_volume_flag;
extern bool volatile scroll_flag;
extern bool volatile read_saved_standby_khz_flag;
extern bool volatile read_saved_standby_mhz_flag;

/* ------------------------------ Variables ------------------------------------- */
s_NAV_RCU_Parameters nav_parameters;

float saved_freqs[15] =
{ 0 };

float frequencies[NUM_FREQUENCIES_PER_PAGE] =
{ 108.00, 108.75, 0.0, 110.20, 110.95, 111.65, 112.40, 113.15, 113.85, 114.60,
		115.35, 116.05, 116.80, 117.55, 117.95 };
static uint8_t current_index_saved_freqs = 0;

uint8_t g_current_page = 0;
uint8_t g_selectedPreset = 0;

uint8_t g_standby_mhz_knob = 0;
uint8_t g_standby_khz_knob = 0;
uint8_t g_volume_knob = 0;

uint8_t g_saved_freq_khz = 0;
uint8_t g_saved_freq_mhz = 0;
/* --------------------------- Function Declarations ----------------------------- */

void update_globals(void);

void check_general_switches(void);

void send_data_to_transmission_queue_CDU(Message_ID id);
void send_data_to_transmission_queue_NAV_RECEIVER(void);

void get_update_data_CDU(Identifier *ident);
/* ------------------------------- NAV_RCU Functions ------------------------------ */
void NAV_RCU_main_thread(void *pvParameters)
{
	static uint32_t last_lcd_update_time = 0;
	static uint32_t last_Health_tx_time = 0;
	static const uint32_t lcd_update_interval = 200; // Interval in milliseconds (500ms)
	static bool lcd_update_needed = false;
	static bool mem_screen_update = false;

	uint32_t current_time;

	Identifier ident;

	GLCD_INIT();

	for (;;)
	{
		check_general_switches();

		if (xQueueReceive(xMainNotifyQueue, &ident, (TickType_t) (10)) == pdTRUE)
		{
			get_update_data_CDU(&ident);
			// send_data_to_transmission_queue_CDU(A_FREQ);
			// send_data_to_transmission_queue_CDU(S_FREQ);
			// send_data_to_transmission_queue_CDU(VOLUME);
			// send_data_to_transmission_queue_NAV_RECEIVER();

			// Flag to update LCD
			lcd_update_needed = true;

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		if (read_encoder_standby_mhz_flag || read_encoder_standby_khz_flag)
		{
			taskENTER_CRITICAL();
			nav_parameters.standby_freq = g_standby_mhz_knob
					+ (0.01 * g_standby_khz_knob);
			if (nav_parameters.standby_freq >= 117.95)
			{
				nav_parameters.standby_freq = 117.95;
				g_standby_mhz_knob = (uint8_t) nav_parameters.standby_freq;
				g_standby_khz_knob = (uint8_t) ((nav_parameters.standby_freq
						- g_standby_mhz_knob) * 100);
			}

			// Send data to queues

			send_data_to_transmission_queue_CDU(S_FREQ);
			vTaskDelay(pdMS_TO_TICKS(200));
			//            send_data_to_transmission_queue_CDU(A_FREQ);

			send_data_to_transmission_queue_NAV_RECEIVER();

			// Flag to update LCD
			lcd_update_needed = true;

			read_encoder_standby_mhz_flag = false;
			read_encoder_standby_khz_flag = false;
			taskEXIT_CRITICAL();
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		if (read_encoder_volume_flag)
		{
			taskENTER_CRITICAL();
			nav_parameters.volume = g_volume_knob;

			send_data_to_transmission_queue_NAV_RECEIVER();

			send_data_to_transmission_queue_CDU(VOLUME);

			// Flag to update LCD
			lcd_update_needed = true;

			read_encoder_volume_flag = false;
			taskEXIT_CRITICAL();
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		if (scroll_flag && nav_parameters.mem_page == ON)
		{
			if (g_current_page > MAX_PAGES)
			{
				g_current_page = MAX_PAGES;
			}
			mem_screen_update = true;
			g_selectedPreset = g_current_page * PRESETS_PER_PAGE;
			set_current_index(g_selectedPreset);
			scroll_flag = false;
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		if ((read_saved_standby_khz_flag || read_saved_standby_mhz_flag)
				&& nav_parameters.mem_page == ON)
		{
			taskENTER_CRITICAL();
			uint8_t index = get_current_index();
			saved_freqs[index] = g_saved_freq_mhz + (0.01 * g_saved_freq_khz);
			mem_screen_update = true;

			nav_parameters.standby_freq = saved_freqs[g_selectedPreset];
			if (nav_parameters.standby_freq < 108.00)
			{
				nav_parameters.standby_freq = 108.00;
			}
			if (nav_parameters.standby_freq > 117.95)
			{
				nav_parameters.standby_freq = 117.95;
			}
			read_saved_standby_khz_flag = false;
			read_saved_standby_mhz_flag = false;
			taskEXIT_CRITICAL();
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		// Check if the interval has passed or if an update is required
		current_time = HAL_GetTick();
		if (lcd_update_needed
				&& (current_time - last_lcd_update_time > lcd_update_interval))
		{
			LCD_Print_Home();
			last_lcd_update_time = current_time;  // Update the last update time
			lcd_update_needed = false;        // Reset the flag after the update
		}

		if (mem_screen_update
				&& (current_time - last_lcd_update_time > lcd_update_interval))
		{
			LCD_PRINT_MEM_SCREEN(g_current_page);
			last_lcd_update_time = current_time;  // Update the last update time
			mem_screen_update = false;        // Reset the flag after the update
		}

		if(current_time > (last_Health_tx_time + 200)){
			last_Health_tx_time =  HAL_GetTick();
			send_data_to_transmission_queue_CDU(Health);
		}

		vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
	}
}

void update_globals(void)
{
	g_standby_mhz_knob = (uint8_t) nav_parameters.standby_freq;
	g_standby_khz_knob = (uint8_t) ((nav_parameters.standby_freq
			- g_standby_mhz_knob) * 100);
	g_volume_knob = nav_parameters.volume;
	g_current_page = 0;

	nav_parameters.mem_page = OFF;
}

void check_general_switches(void)
{
	// Wait for any button event
	EventBits_t eventBits = xEventGroupWaitBits(buttonEventGroup,
	EVENT_GP_SW_DELETE | EVENT_GP_SW_PRESET | EVENT_GP_SW1_PRESS |
	EVENT_GP_SW2_PRESS | EVENT_GP_SW3_PRESS | EVENT_RIGHT_SW_PRESS |
	EVENT_LEFT_SW_NORMAL_PRESS | EVENT_LEFT_SW_LONG_PRESS,
	pdTRUE,   // Clear bits after processing
			pdFALSE,  // Wait for any bit
			(TickType_t) 5);

	// Handle button events

	//    uint8_t startPreset = g_current_page * PRESETS_PER_PAGE;
	if (eventBits & EVENT_GP_SW_PRESET)  // and save
	{
#if DEBUG_CONSOLE
        debug_print("GP_SW1 pressed\n");
#endif
		nav_parameters.mem_page = (nav_parameters.mem_page == OFF) ? ON : OFF;
		//           // send the update value to NAV Receiver
		// send_data_to_transmission_queue_NAV_RECEIVER();
		if (nav_parameters.mem_page == ON)
		{  //** MEMORY PAGE DISPLAY */
			glcd_clear_all();
			// read_frequencies_from_flash(FREQUENCY_FLASH_ADDRESS, frequencies, NUM_FREQUENCIES);
			set_current_index(0);
			g_current_page = 0;
			g_selectedPreset = 0;
			LCD_PRINT_MEM_SCREEN(g_current_page);
		}
		if (nav_parameters.mem_page == OFF)
		{  //*** SAVE FUNCTIONALITY */

			nav_parameters.standby_freq = saved_freqs[g_selectedPreset];
			if (nav_parameters.standby_freq == EMPTY_FREQ)
			{
				g_standby_mhz_knob = 108;
				g_standby_khz_knob = 0;
			}
			else
			{
				g_standby_mhz_knob = (uint8_t) nav_parameters.standby_freq;
				g_standby_khz_knob = (uint8_t) ((nav_parameters.standby_freq
						- g_standby_mhz_knob) * 100);
			}
			set_current_index(0);

			g_current_page = 0;
			g_selectedPreset = 0;

			write_frequencies_to_flash(saved_freqs, NUM_FREQUENCIES);

			glcd_clear_all();
			LCD_Print_Home();
		}
	}

	if (eventBits & EVENT_GP_SW1_PRESS) // Select the first preset on the current page
	{
		if (nav_parameters.mem_page == ON)
		{
			g_selectedPreset = g_current_page * PRESETS_PER_PAGE; // First preset on current page
			set_current_index(g_selectedPreset);
			LCD_PRINT_MEM_SCREEN(g_current_page);  // Refresh display
		}
	}

	if (eventBits & EVENT_GP_SW2_PRESS) // Select the second preset on the current page
	{
		if (nav_parameters.mem_page == ON)
		{
			g_selectedPreset = g_current_page * PRESETS_PER_PAGE + 1; // Second preset on current page
			set_current_index(g_selectedPreset);
			LCD_PRINT_MEM_SCREEN(g_current_page);  // Refresh display
		}
	}
	if (eventBits & EVENT_GP_SW3_PRESS) // Select the third preset on the current page
	{
		if (nav_parameters.mem_page == ON)
		{
			g_selectedPreset = g_current_page * PRESETS_PER_PAGE + 2; // Third preset on current page
			set_current_index(g_selectedPreset);
			LCD_PRINT_MEM_SCREEN(g_current_page);  // Refresh display
		}
	}
	if (eventBits & EVENT_GP_SW_DELETE)
	{  //** DELETE */
		if (nav_parameters.mem_page == ON)
		{
			set_current_index(g_selectedPreset);
			saved_freqs[current_index_saved_freqs] = EMPTY_FREQ;
			update_frequency_in_flash(current_index_saved_freqs,
					saved_freqs[current_index_saved_freqs]);
			LCD_PRINT_MEM_SCREEN(g_current_page);
		}
	}
	if (eventBits & EVENT_RIGHT_SW_PRESS)
	{
		if (nav_parameters.standby_freq == EMPTY_FREQ)
		{
			return;

		}

		swap_active_standby();  // Swap Active and Standby Frequency
		// update knob variables also
		g_standby_mhz_knob = (uint8_t) nav_parameters.standby_freq;
		g_standby_khz_knob = (uint16_t) ((nav_parameters.standby_freq
				- g_standby_mhz_knob) * 1000);

		// update LCD to new Values
		LCD_Print_Home();
		// Send the update data to queues
		//        send_data_to_transmission_queue_NAV_RECEIVER();
		send_data_to_transmission_queue_CDU(A_FREQ);
		vTaskDelay(pdMS_TO_TICKS(200));
		send_data_to_transmission_queue_CDU(S_FREQ);
	}
	if (eventBits & EVENT_LEFT_SW_NORMAL_PRESS)
	{
#if DEBUG_CONSOLE
        debug_print("LEFT_SW normal press detected\n");
#endif
		/*
		 * POWER ONNNNN

		 */
		HAL_Delay(100);
		LCD_power_on();
		HAL_Delay(100);

#if UPLOAD_DEFAULT
        write_frequencies_to_flash(frequencies, NUM_FREQUENCIES_PER_PAGE);
#endif
		read_frequencies_from_flash(FREQUENCY_FLASH_ADDRESS, saved_freqs,
		NUM_FREQUENCIES);

		Load_NAV_RCU_Parameters(&nav_parameters);

		update_globals();
		nav_parameters.power_on = ON;
		LCD_Print_Home();
		nav_parameters.power_on = OFF;
	}
	if (eventBits & EVENT_LEFT_SW_LONG_PRESS)
	{
#if DEBUG_CONSOLE
        debug_print("LEFT_SW long press detected\n");
#endif
		// LCD power off
		uint8_t power_off[3] =
		{ 0 };

		if (xNAVRecTxQueue != NULL)
		{
			if (xQueueSend(xNAVRecTxQueue, &power_off,
					(TickType_t )(10)) != pdPASS)
			{
#if DEBUG_CONSOLE
                debug_print("DH xNAVRecTxQueue Failed to queue\n");
#endif
			}
		}

		update_flash();
		write_frequencies_to_flash(saved_freqs, NUM_FREQUENCIES);
		nav_parameters.power_on = OFF;
		LCD_power_off();
	}
}

/* ----------------------------- Soft Keys Operations ------------------------------- */

static inline uint8_t readButton(GPIO_TypeDef *BUTTON_PORT, uint16_t pin)
{
	return HAL_GPIO_ReadPin(BUTTON_PORT, pin);
}

void general_purpose_switches_thread(void *pvParameters)
{
	TickType_t right_knob_press_start_time = 0;
	uint8_t left_knob_was_pressed = 0;

	for (;;)
	{
		// Handle Button 1, 2, 3 (Normal Buttons)
		if (!readButton(GP_SW1_GPIO_Port, GP_SW1_Pin))
		{  // Active-low
			xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW1_PRESS);
		}
		if (!readButton(GP_SW2_GPIO_Port, GP_SW2_Pin))
		{  // Active-low
			xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW2_PRESS);
		}

		if (!readButton(GP_SW3_GPIO_Port, GP_SW3_Pin))
		{  // Active-low
			xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW3_PRESS);
		}
		if (!readButton(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin))
		{  // Active-low
			xEventGroupSetBits(buttonEventGroup, EVENT_RIGHT_SW_PRESS);
		}
		if (!readButton(GP_SW6_GPIO_Port, GP_SW6_Pin))
		{
			xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW_PRESET);
		}

		if (!readButton(GP_SW5_GPIO_Port, GP_SW5_Pin))
		{
			xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW_DELETE);
		}

		// Handle Button 4 (Dual-Operation Button)
		uint8_t button4Pressed = !readButton(LEFT_SW_GPIO_Port, LEFT_SW_Pin); // Active-low

		if (button4Pressed)
		{
			if (!left_knob_was_pressed)
			{
				// Button just pressed
				right_knob_press_start_time = xTaskGetTickCount();
				left_knob_was_pressed = 1;
			}
			else if (left_knob_was_pressed
					&& (xTaskGetTickCount() - right_knob_press_start_time)
							>= pdMS_TO_TICKS(LONG_PRESS_THRESHOLD_MS))
			{
				// Long press detected
				xEventGroupSetBits(buttonEventGroup, EVENT_LEFT_SW_LONG_PRESS);
				left_knob_was_pressed = 0; // Reset state to avoid repeated long press events
			}
		}
		else
		{
			if (left_knob_was_pressed)
			{
				// Button just released
				if ((xTaskGetTickCount() - right_knob_press_start_time)
						< pdMS_TO_TICKS(LONG_PRESS_THRESHOLD_MS))
				{
					// Short press detected
					xEventGroupSetBits(buttonEventGroup,
					EVENT_LEFT_SW_NORMAL_PRESS);
				}
				left_knob_was_pressed = 0;
			}
		}

		// Delay for the polling interval
		vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
	}
}

/* ---------------------------- Transmission Queues Handler --------------------------- */
void send_data_to_transmission_queue_NAV_RECEIVER(void)
{
	if (xNAVRecTxQueue != NULL)
	{
		if (xQueueSend(xNAVRecTxQueue, &nav_parameters,
				(TickType_t )(10)) != pdPASS)
		{
#if DEBUG_CONSOLE
            debug_print("DH xNAVRecTxQueue Failed to queue\n");
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
//		ident.mhz = g_standby_mhz_knob;
//		ident.khz = g_standby_khz_knob * 10;

		ident.mhz = (uint8_t) nav_parameters.standby_freq;
		ident.khz = (uint16_t) (((nav_parameters.standby_freq - ident.mhz) + 0.001) * 1000);
	}
	else if (ident.ID == A_FREQ)
	{
		ident.mhz = (uint8_t) nav_parameters.active_freq; // Extract the MHz part
		ident.khz = (uint16_t)(((nav_parameters.active_freq - ident.mhz) +0.001) * 1000); // Extract the kHz part
	}
	else if (ident.ID == VOLUME)
	{
		ident.volume = nav_parameters.volume;
	} else if (ident.ID == Health){

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
		nav_parameters.standby_freq = ident->mhz + (ident->khz / 1000.0);
		g_standby_mhz_knob = (uint8_t) nav_parameters.standby_freq;
		g_standby_khz_knob = (uint8_t) (((nav_parameters.standby_freq
				- g_standby_mhz_knob) + 0.001) * 100);
		break;
	case A_FREQ:
		nav_parameters.active_freq = ident->mhz + (ident->khz / 1000.0);
		break;
	case VOLUME:
		nav_parameters.volume = ident->volume;
		break;

	default:
		break;
	}
}

float get_frequency(uint8_t index)
{
	if (index < NUM_FREQUENCIES)
	{
		return saved_freqs[index];
	}
	return 0.0f;
}  // Default value if index is invalid }
void set_frequency(uint8_t index, float frequency)
{
	if (index < NUM_FREQUENCIES)
	{
		saved_freqs[index] = frequency;
	}
}
uint8_t get_current_index()
{
	return current_index_saved_freqs;
}

void set_current_index(uint8_t index)
{
	if (index < NUM_FREQUENCIES)
	{
		current_index_saved_freqs = index;
	}
}
