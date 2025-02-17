/*
 * HF_main_thread.c
 *
 *  Created on: Jan 27, 2025
 *      Author: Admin
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "..\LCD\rcu_display.h"
#include "..\SWITCHES_Encoder\general_switches.h"
#include "Flash.h"
#include "HF_funcs.h"
#include "constant_defines.h"
#include "data_handler.h"
#include "global_vars.h"
#include "serial_handler.h"
#include "tim.h"

extern bool volatile read_encoder_standby_mhz_flag;
extern bool volatile read_encoder_standby_khz_flag;
extern bool volatile read_encoder_volume_flag;
extern bool volatile scroll_flag;
extern bool volatile read_saved_standby_khz_flag;
extern bool volatile read_saved_standby_mhz_flag;
extern bool volatile move_cursor_flag;
extern bool volatile digit_change;
s_HF_Parameters HF_parameters;

void HF_main_thread(void *pvParameters)
{
	static uint32_t last_lcd_update_time = 0;
	static const uint32_t lcd_update_interval = 200; // Interval in milliseconds (500ms)
	static bool lcd_update_needed = false;
	static bool mem_screen_update = false;

	uint32_t current_time;

	Identifier ident;

	GLCD_INIT();

	HAL_Delay(100);
	LCD_power_on();
	HAL_Delay(100);

	Load_HF_RCU_Parameters(&HF_parameters);
	update_globals(&HF_parameters);

	HF_parameters.power_on = ON;
	send_data_to_transmission_queue_HF_RECEIVER(POWER_ON);
	//    float frequency = 255.141f;
	//    LCD_UpdateRegion(TL,convertToString(&frequency,TYPE_FLOAT));

//	    LCD_UpdateRegion(TL, "225.00");    // Update Top Left with frequency
//	    LCD_UpdateRegion(ML, "SQ");        // Update Middle Left with "SQ"
//	    LCD_UpdateRegion(BL, "PROG");      // Update Bottom Left with "PROG"
//	    LCD_UpdateRegion(MT, "T/GD/MN");   // Update Middle Top
//	    LCD_UpdateRegion(MM, "VOL 10");    // Update Middle Middle
//	    LCD_UpdateRegion(MB, "MN/BOTH");   // Update Middle Bottom
//	    LCD_UpdateRegion(RT, "FRQ/CH");    // Update Right Top
//	    LCD_UpdateRegion(RM, "TST");       // Update Right Middle
//	    LCD_UpdateRegion(RB, "BOTH");      // Update Right Bottom

	LCD_Print_Home();

	for (;;)
	{
		check_general_switches();

		// if (xQueueReceive(xMainNotifyQueue, &ident, (TickType_t)(10)) == pdTRUE)
		// {
		//     get_update_data_CDU(&ident);
		//     lcd_update_needed = true;
		//     vTaskDelay(pdMS_TO_TICKS(100));
		// }

		if ((read_encoder_standby_mhz_flag || read_encoder_standby_khz_flag)
				&& (HF_parameters.FRQ_CH == 0)
				&& (HF_parameters.power_on == ON))
		{
			taskENTER_CRITICAL();
			HF_parameters.standby_freq = g_vars.g_standby_mhz_knob
					+ (0.01 * g_vars.g_standby_khz_knob);
			if (HF_parameters.standby_freq >= (STANDBY_MHZ_MAX+STANDBY_KHZ_MAX/1000.0))
			{
				HF_parameters.standby_freq = (STANDBY_MHZ_MAX+STANDBY_KHZ_MAX/1000.0);
				g_vars.g_standby_mhz_knob =
						(uint8_t) HF_parameters.standby_freq;
				g_vars.g_standby_khz_knob =
						(uint8_t) ((HF_parameters.standby_freq
								- g_vars.g_standby_mhz_knob) * 100);
			}

			send_data_to_transmission_queue_CDU(S_FREQ);
			vTaskDelay(pdMS_TO_TICKS(200));		//200ms delay in a critical section  ??????????

			send_data_to_transmission_queue_HF_RECEIVER(S_FREQ);

			lcd_update_needed = true;

			read_encoder_standby_mhz_flag = false;
			read_encoder_standby_khz_flag = false;
			taskEXIT_CRITICAL();
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		if (read_encoder_volume_flag)
		{  // DONE map volume to ascii for  interface
			taskENTER_CRITICAL();
			HF_parameters.volume = g_vars.g_volume_knob;
			strcpy((char*) HF_parameters.volume_ascii,
					(char*) map_volume_into_ascii(HF_parameters.volume));

			send_data_to_transmission_queue_HF_RECEIVER(VOLUME);

			send_data_to_transmission_queue_CDU(VOLUME);

			// Flag to update LCD
			lcd_update_needed = true;

			read_encoder_volume_flag = false;
			taskEXIT_CRITICAL();
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		if (scroll_flag && HF_parameters.PROG == ON)
		{
			if (g_vars.g_current_page > MAX_PAGES)
			{
				g_vars.g_current_page = MAX_PAGES;
			}
			mem_screen_update = true;
			g_vars.g_selectedPreset = g_vars.g_current_page * PRESETS_PER_PAGE;
			//            set_current_index( g_vars.g_selectedPreset);
			scroll_flag = false;
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		if ((read_saved_standby_khz_flag || read_saved_standby_mhz_flag)
				&& HF_parameters.PROG == ON)
		{
			taskENTER_CRITICAL();
			uint8_t index = 0;  // get_current_index();
			saved_channels[index] = g_vars.g_saved_channel_mhz
					+ (0.01 * g_vars.g_saved_channel_khz);
			mem_screen_update = true;

			HF_parameters.standby_freq =
					saved_channels[g_vars.g_selectedPreset];
			if (HF_parameters.standby_freq < (STANDBY_MHZ_MIN+STANDBY_KHZ_MIN/10000))
			{
				HF_parameters.standby_freq = (STANDBY_MHZ_MIN+STANDBY_MHZ_MIN/1000);
			}
			if (HF_parameters.standby_freq > (STANDBY_MHZ_MAX+STANDBY_KHZ_MAX/1000))
			{
				HF_parameters.standby_freq = (STANDBY_MHZ_MAX+STANDBY_KHZ_MAX/1000);
			}
			read_saved_standby_khz_flag = false;
			read_saved_standby_mhz_flag = false;
			taskEXIT_CRITICAL();
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		if(move_cursor_flag && HF_parameters.Cursor==ON){
			LCD_PRINT_CURSOR(false,g_vars.previous_cursor_location);
			vTaskDelay(50);
			LCD_PRINT_CURSOR(true,g_vars.current_cursor_location);
			g_vars.previous_cursor_location=g_vars.current_cursor_location;
			move_cursor_flag=false;
		}
		if(digit_change){
			lcd_update_needed=true;
			digit_change=false;
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
			LCD_PRINT_MEM_SCREEN(g_vars.g_current_page);
			last_lcd_update_time = current_time;  // Update the last update time
			mem_screen_update = false;        // Reset the flag after the update
		}

		vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
	}
}
