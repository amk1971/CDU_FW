#include "general_switches.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\HF\HF_funcs.h"
#include "..\LCD\rcu_display.h"
#include "Flash.h"
#include "FreeRTOS.h"
#include "data_handler.h"
#include "debug_console.h"
#include "global_vars.h"
#include "main.h"
#include "rotary_encoder.h"
#include "serial_handler.h"
#include "task.h"
#include "tim.h"

// extern s_HF_Parameters HF_parameters;
extern bool volatile read_mode_flag;
extern bool volatile read_Test_Start_Flag;

float pre_Set_Channels[NUM_CHANNELS_PER_PAGE] =
{ 108.00, 108.75, 0.0, 110.20, 110.95, 111.65, 112.40, 113.15, 113.85, 114.60,
		115.35, 116.05, 116.80, 117.55, 117.95, 118.00, 119.00 };

void handle_prog_exit(void);
void handle_gp_sw1_press(void);
void handle_gp_sw2_press(void);
void handle_gp_sw3_press(void);
void handle_delete_STO(void);
void handle_right_sw_press(void);
void handle_left_sw_normal_press(void);
void handle_left_sw_long_press(void);

void check_general_switches(void)
{
	// Wait for any button event
	EventBits_t eventBits = xEventGroupWaitBits(buttonEventGroup,
	EVENT_GP_SW_DELETE_STO | EVENT_GP_SW_PROG_EXIT | EVENT_GP_SW1_PRESS |
	EVENT_GP_SW2_PRESS | EVENT_GP_SW3_PRESS | EVENT_RIGHT_SW_PRESS |
	EVENT_LEFT_SW_NORMAL_PRESS | EVENT_LEFT_SW_LONG_PRESS,
	pdTRUE,   // Clear bits after processing
			pdFALSE,  // Wait for any bit
			(TickType_t) 5);

	if (eventBits & EVENT_GP_SW_PROG_EXIT)
	{
		handle_prog_exit();
	}

	if (eventBits & EVENT_GP_SW1_PRESS)
	{  // FRQ /CH
		handle_gp_sw1_press();
	}

	 if (eventBits & EVENT_GP_SW2_PRESS)
	 {
	     handle_gp_sw2_press();
	 }

	 if (eventBits & EVENT_GP_SW3_PRESS)
	 {
	     handle_gp_sw3_press();
	 }

	if (eventBits & EVENT_GP_SW_DELETE_STO)
	{
		handle_delete_STO();	//Left Center Switch
	}

	if (eventBits & EVENT_RIGHT_SW_PRESS)
	{  // right knob press
		handle_right_sw_press();
	}

	if (eventBits & EVENT_LEFT_SW_NORMAL_PRESS)
	{  // ** POWER ON
		handle_left_sw_normal_press();
	}

	if (eventBits & EVENT_LEFT_SW_LONG_PRESS)
	{  //** POWER OFF */
		handle_left_sw_long_press();
	}
}

void handle_prog_exit(void)
{
#if DEBUG_CONSOLE
    debug_print("GP_SW1 pressed\n");
#endif
	// DONE display of prog screen based on prog selection

	HF_parameters.PROG = (HF_parameters.PROG == OFF) ? ON : OFF;

	if (HF_parameters.PROG == ON)
	{
		glcd_clear_all();
		set_saved_channel_index(0);
		g_vars.g_current_page = 0;
		g_vars.g_selectedPreset = 0;
		LCD_PRINT_MEM_SCREEN(g_vars.g_current_page);
	}
	else
	{
		//HF_parameters.tuned_freq = saved_channels[g_vars.g_selectedPreset];

//		if (HF_parameters.tuned_freq == EMPTY_FREQ)
//		{
//			g_vars.g_standby_mhz_knob = 108;
//			g_vars.g_standby_khz_knob = 0;
//		}
//		else
//		{
			g_vars.g_standby_mhz_knob = (uint8_t) HF_parameters.tuned_freq;
			g_vars.g_standby_khz_knob = (uint8_t) ((HF_parameters.tuned_freq
					- g_vars.g_standby_mhz_knob) * 100);
//		}

		set_saved_channel_index(0);
		g_vars.g_current_page = 0;
		g_vars.g_selectedPreset = 0;
		write_channels_to_flash(saved_channels, NUM_FREQUENCIES);
		//glcd_clear_all();
		LCD_Print_Home();
	}
}

void handle_gp_sw1_press(void)
{ /* TODO make knob select preset channels when chanels is selected and
 make it change freq when freq is selected*/
	if (HF_parameters.PROG == ON)
	{
		g_vars.g_selectedPreset = g_vars.g_current_page * PRESETS_PER_PAGE;
		set_saved_channel_index(g_vars.g_selectedPreset);
		LCD_PRINT_MEM_SCREEN(g_vars.g_current_page);
	}
	else
	{ // TODO make sure right hand knob acts accordingly based on value of FRQ_CH
		/**
		 * @brief if value is frq then change mhz and khz of freq
		 * if value is ch then select preset from saved
		 * if GD then knob is rendered useless
		 */
		switch (HF_parameters.FRQ_CH)
		{
		case FRQ:
			HF_parameters.FRQ_CH = CH;
			break;
		case CH:
			HF_parameters.FRQ_CH = FRQ;
			break;
		default:
			HF_parameters.FRQ_CH = FRQ;
			break;
		}
		LCD_Print_Home();
	}
}

void handle_gp_sw2_press(void)
{
	if (HF_parameters.PROG == ON)
	{
		g_vars.g_selectedPreset = g_vars.g_current_page * PRESETS_PER_PAGE + 1;
		set_saved_channel_index(g_vars.g_selectedPreset);
		LCD_PRINT_MEM_SCREEN(g_vars.g_current_page);
	} else {
		if(HF_parameters.Test == OFF){
			HF_parameters.Test = ON;//(HF_parameters.Test == ON)? OFF : ON;
			HF_parameters.TestTick = HAL_GetTick();
			read_Test_Start_Flag = true;
		} else {
			HF_parameters.TestTick = HAL_GetTick() - 6000;
		}
	}

}

void handle_gp_sw3_press(void)
{
	if (HF_parameters.PROG == ON)
	{
		g_vars.g_selectedPreset = g_vars.g_current_page * PRESETS_PER_PAGE + 2;
		set_saved_channel_index(g_vars.g_selectedPreset);
		LCD_PRINT_MEM_SCREEN(g_vars.g_current_page);
	}
	else
	{  // DONE highlight selection and set 1 for on and 0 for off in data byte 2
		HF_parameters.Mode = (HF_parameters.Mode == ON)?OFF:ON;
		read_mode_flag = true;
		LCD_Print_Home();
	}
}

void handle_delete_STO(void)
{
	if (HF_parameters.PROG == ON)
	{
		saved_channels[g_vars.g_selectedPreset] = EMPTY_FREQ;
		update_frequency_in_flash(g_vars.g_selectedPreset,
				saved_channels[g_vars.g_selectedPreset]);
		LCD_PRINT_MEM_SCREEN(g_vars.g_current_page);
	}
	else
	{  // DONE turn on STO and set byte 4 1 or 0 in data stream
		HF_parameters.STO = (HF_parameters.STO == OFF) ? ON : OFF;

		if (HF_parameters.STO == ON)
		{
			send_data_to_transmission_queue_HF_RECEIVER(STO_ON);
		}
		else
		{
			send_data_to_transmission_queue_HF_RECEIVER(STO_OFF);
		}
		LCD_Print_Home();
	}
}

void handle_right_sw_press(void)
{
	if (HF_parameters.tuned_freq == EMPTY_FREQ)
	{
		return;
	}
	// TODO cursor blink start and check the cursor start flag
	/*
	 *
	 * */

	HF_parameters.Cursor = (HF_parameters.Cursor == OFF) ? ON : OFF;
	vTaskDelay(50);
	if(HF_parameters.Cursor== ON){
	// start cursor
		HAL_TIM_Base_Start_IT(&htim3);  // Stop blinking

	}
	else{
		// stop cursor

		HAL_TIM_Base_Stop_IT(&htim3);  // Stop blinking
		LCD_PRINT_CURSOR(false,g_vars.current_cursor_location);
		g_vars.current_cursor_location=START_CURSOR_LOCATION;
		vTaskDelay(50);
	}
}

void handle_left_sw_normal_press(void)
{  // DONE send byte 5 as on power on
// HAL_Delay(100);
// LCD_power_on();
// HAL_Delay(100);

#if UPLOAD_DEFAULT
	write_channels_to_flash(pre_Set_Channels, NUM_CHANNELS_PER_PAGE);
#endif

	// read_channels_from_flash(CHANNEL_FLASH_ADDRESS, saved_channels, NUM_FREQUENCIES);
	// Load_HF_RCU_Parameters(&HF_parameters);
	// update_globals(&HF_parameters);
	if (HF_parameters.power_on == OFF){
		LCD_power_on();
		 HF_parameters.power_on = ON;
		 HF_parameters.PROG = OFF;
		 HF_parameters.FRQ_CH = FRQ;
		 HF_parameters.volume = 05;
		 send_data_to_transmission_queue_HF_RECEIVER(POWER_ON);
		 LCD_Print_Home();
	}
}

void handle_left_sw_long_press(void)
{  // DONE send byte 5 as off
#if DEBUG_CONSOLE
    debug_print("LEFT_SW long press detected\n");
#endif

	update_flash();
	write_channels_to_flash(saved_channels, NUM_FREQUENCIES);
	send_data_to_transmission_queue_HF_RECEIVER(POWER_OFF);

	vTaskDelay(pdMS_TO_TICKS(200));
	HF_parameters.power_on = OFF;
	LCD_power_off();
}
