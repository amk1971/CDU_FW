#include "rotary_encoder.h"

#include <math.h>
#include <stdbool.h>

#include "constant_defines.h"
#include "data_handler.h"
#include "global_vars.h"
#include "main.h"
#include "stdint.h"

/* ------------------------------ ENCODER Operations -------------------------------- */

// flags for rotray
bool volatile read_encoder_standby_mhz_flag = false;
bool volatile read_encoder_standby_khz_flag = false;
bool volatile read_encoder_channel_flag = false;
bool volatile read_encoder_volume_flag = false;
bool volatile move_cursor_flag = false;

bool volatile read_saved_standby_khz_flag = false;
bool volatile read_saved_standby_mhz_flag = false;
bool volatile scroll_flag = false;
bool volatile digit_change = false;

void adjust_digit(int direction)
{
	// Separate the frequency into integer and decimal parts
	int freq_int = (int) (HF_parameters.tuned_freq);  // Integer part
	int freq_decimal = (int) (((HF_parameters.tuned_freq - freq_int) + 0.01)
			* 10); // Decimal part (tenths place)

	int high_place = freq_int / 1000;

	// Determine place_value based on cursor index
	int place_value = 1;
	if (g_vars.cursor_index >= 0 && g_vars.cursor_index <= 3)
	{ // Non-decimal part (left side of the decimal)
		switch (g_vars.cursor_index)
		{
		case 0:
			place_value = 1000;
			break;
		case 1:
			place_value = 100;
			break;
		case 2:
			place_value = 10;
			break;
		case 3:
			place_value = 1;
			break;

		}
	}
	else if (g_vars.cursor_index == 4)
	{ // Decimal part (right side of the decimal)
		switch (g_vars.cursor_index)
		{

		case 4:
			place_value = 1;
			break;
		}
	}

	if (g_vars.cursor_index == 0)
	{
		int digit = (freq_int / place_value) % 10;

		if (direction > 0) // Increment
		{
			if (high_place < 29) // Ensure max limit
			{
				if (digit < 9)
				{
					digit += 1;  // Increase normally
				}
				else
				{
					digit = 0;  // Reset to 0
					if (place_value == 1000) // Carry from thousands place (e.g., 2099 → 2100)
					{
						freq_int += 1000;
					}
				}
			}
		}
		else if (direction < 0) // Decrement
		{
			if (high_place > 2) // Ensure min limit
			{
				if (digit > 0)
				{
					digit -= 1;  // Decrease normally
				}
				else
				{
					digit = 9;  // Reset to 9
					if (place_value == 1000) // Carry from thousands place (e.g., 2099 → 2100)
					{
						freq_int -= 1000;
					}
					// Borrow from next digit
				}
			}
		}

		// Ensure it stays within 2-29 range
//		if (high_place < 20)
//		{
//			freq_int = (freq_int % 1000) + (20 * 100); // Lock hundreds place to 20
//		}
//		else if (high_place > 29)
//		{
//			freq_int = (freq_int % 1000) + (29 * 100); // Lock hundreds place to 29
//		}
		freq_int = freq_int - (freq_int / place_value % 10) * place_value
				+ digit * place_value;
	}
	// Modify the correct part based on the cursor position
	else if (g_vars.cursor_index > 0 && g_vars.cursor_index <= 3)
	{
		// Non-decimal part (MHz, kHz, etc.)
		int digit = (freq_int / place_value) % 10; // Get the selected digit

		if (direction > 0)
		{
			digit = (digit == 9) ? 0 : digit + 1;
			; // Increment, no rollover past 9
		}
		else if (direction < 0)
		{
			digit = (digit == 0) ? 9 : digit - 1; // Decrement, no rollover past 0
		}

		// Update the integer part with the modified digit
		freq_int = freq_int - (freq_int / place_value % 10) * place_value
				+ digit * place_value;
		if (freq_int > 9999)
		{
			int map[] =
			{ 8, 16, 24, 32, 48 };
			memcpy(g_vars.cursor_map, map, sizeof(map));
		}
		else
		{
			uint8_t map[] =
			{ 0, 8, 16, 24, 40 }; // no 4 cause decimal is ignored
			memcpy(g_vars.cursor_map, map, sizeof(map));
		}
	}

	// Update the integer part with the modified digit

	if (g_vars.cursor_index == 4)
	{

		int digit = (freq_decimal / place_value) % 10;

		if (direction > 0)
		{
			// Rollover from 9 to 0
			digit = (digit == 9) ? 0 : digit + 1;
		}
		else if (direction < 0)
		{
			// Rollover from 0 to 9
			digit = (digit == 0) ? 9 : digit - 1;
		}

		// Modify the decimal part using the place_value for the relevant position
		freq_decimal = freq_decimal
				- (freq_decimal / place_value % 10) * place_value
				+ digit * place_value;
	}
// Combine both parts back into a fl
// Combine both parts back into a float and enforce the range limit
	float new_freq = (float) freq_int + (float) freq_decimal / 10.0; // Recombine the integer and decimal parts
	if (new_freq >= 2000.0 && new_freq <= 29999.9)
	{
		HF_parameters.tuned_freq = new_freq; // Apply new frequency if within range
	}
	digit_change = true;
}

void adjust_digit_interrupt()
{

	static uint8_t old_GH = 3;                         // Lookup table index
	static int8_t encval = 0;                               // Encoder value
	static const int8_t enc_states[] =
	{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

	old_GH <<= 2;  // Remember previous state

	if (HAL_GPIO_ReadPin(RIGHT_A1_GPIO_Port, RIGHT_A1_Pin))
		old_GH |= 0x02;  // Add current state of pin G
	if (HAL_GPIO_ReadPin(RIGHT_B1_GPIO_Port, RIGHT_B1_Pin))
		old_GH |= 0x01;  // Add current state of pin H

	encval += enc_states[(old_GH & 0x0f)];

	if (encval > 0)
	{
		adjust_digit(+1); // Clockwise -> Increase digit
		encval = 0;
	}
	else if (encval < 0)
	{
		adjust_digit(-1); // Counterclockwise -> Decrease digit
		encval = 0;
	}
}

void move_cursor()
{
	if (!move_cursor_flag)
	{
		static uint8_t old_AB = 3;  // Lookup table index
		static int8_t encval = 0;   // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_AB <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A2_GPIO_Port, RIGHT_A2_Pin))
			old_AB |= 0x02;  // Add current state of pin A
		if (HAL_GPIO_ReadPin(RIGHT_B2_GPIO_Port, RIGHT_B2_Pin))
			old_AB |= 0x01;  // Add current state of pin B

		encval += enc_states[(old_AB & 0x0F)];

		if (encval > 0)  // Rotate forward (increment cursor position)
		{
			if (g_vars.cursor_index < (sizeof(g_vars.cursor_map) - 1))
			{
				g_vars.cursor_index++;  // Move to the next index
			}
			else
			{
				g_vars.cursor_index = 0;  // Wrap around to the first position
			}
			g_vars.current_cursor_location =
					g_vars.cursor_map[g_vars.cursor_index];
			move_cursor_flag = true;
			encval = 0;
		}
		else if (encval < 0)  // Rotate backward (decrement cursor position)
		{
			if (g_vars.cursor_index > 0)
			{
				g_vars.cursor_index--;  // Move to the previous index
			}
			else
			{
				g_vars.cursor_index = sizeof(g_vars.cursor_map) - 1; // Wrap to the last position
			}
			g_vars.current_cursor_location =
					g_vars.cursor_map[g_vars.cursor_index];
			move_cursor_flag = true;
			encval = 0;
		}
	}
}

void read_encoder_standby_mhz() // Controls the 10 kHz steps for larger place values (tens, hundreds, thousands)
{
//	float number = saved_channels[g_vars.g_selectedPreset];
//		g_vars.g_saved_channel_mhz = (uint16_t) number;
//		g_vars.g_saved_channel_khz =
//				(uint16_t) (((number - (uint16_t) number) + 0.0001) * 1000);

	if (!read_encoder_standby_mhz_flag)
	{
		static uint8_t old_AB = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_AB <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A2_GPIO_Port, RIGHT_A2_Pin))
			old_AB |= 0x02;  // Add current state of pin A
		if (HAL_GPIO_ReadPin(RIGHT_B2_GPIO_Port, RIGHT_B2_Pin))
			old_AB |= 0x01;  // Add current state of pin B

		encval += enc_states[(old_AB & 0x0f)];

		if (encval > 0)  // Rotate forward (increment)
		{
			if (g_vars.g_standby_mhz_knob < STANDBY_MHZ_MAX)
			{
				g_vars.g_standby_mhz_knob++;  // Increment by 1
			}
			else
			{
				g_vars.g_standby_mhz_knob = STANDBY_MHZ_MIN; // Wrap to 019 (after 179)
			}
			read_encoder_standby_mhz_flag = true;
			encval = 0;
		}
		else if (encval < 0)  // Rotate backward (decrement)
		{
			if (g_vars.g_standby_mhz_knob > STANDBY_MHZ_MIN)
			{
				g_vars.g_standby_mhz_knob--;  // Decrement by 1
			}
			else
			{
				g_vars.g_standby_mhz_knob = STANDBY_MHZ_MAX; // Wrap to 179 (after going below 019)
			}
			read_encoder_standby_mhz_flag = true;
			encval = 0;
		}
		//saved_channels[g_vars.g_selectedPreset] = g_vars.g_saved_channel_mhz + g_vars.g_saved_channel_khz / 1000.0;
	}
}

void read_encoder_channel_mhz() // Controls the 10 kHz steps for larger place values (tens, hundreds, thousands)
{
	float number = saved_channels[g_vars.g_selectedPreset];
	if (number <  STANDBY_MHZ_MIN) number = STANDBY_MHZ_MIN;
	if (number >  STANDBY_MHZ_MAX) number = STANDBY_MHZ_MAX;
		g_vars.g_saved_channel_mhz = (uint16_t) number;
		g_vars.g_saved_channel_khz =
				(uint16_t) (((number - (uint16_t) number) + 0.0001) * 1000);

	if (!read_encoder_channel_flag)
	{
		static uint8_t old_AB = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_AB <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A2_GPIO_Port, RIGHT_A2_Pin))
			old_AB |= 0x02;  // Add current state of pin A
		if (HAL_GPIO_ReadPin(RIGHT_B2_GPIO_Port, RIGHT_B2_Pin))
			old_AB |= 0x01;  // Add current state of pin B

		encval += enc_states[(old_AB & 0x0f)];

		if (encval > 0)  // Rotate forward (increment)
		{
			if (g_vars.g_saved_channel_mhz < STANDBY_MHZ_MAX)
			{
				g_vars.g_saved_channel_mhz++;  // Increment by 1
			}
			else
			{
				g_vars.g_saved_channel_mhz = STANDBY_MHZ_MIN; // Wrap to 019 (after 179)
			}
			read_encoder_channel_flag = true;
			encval = 0;
		}
		else if (encval < 0)  // Rotate backward (decrement)
		{
			if (g_vars.g_saved_channel_mhz > STANDBY_MHZ_MIN)
			{
				g_vars.g_saved_channel_mhz--;  // Decrement by 1
			}
			else
			{
				g_vars.g_saved_channel_mhz = STANDBY_MHZ_MAX; // Wrap to 179 (after going below 019)
			}
			read_encoder_channel_flag = true;
			encval = 0;
		}
		saved_channels[g_vars.g_selectedPreset] = g_vars.g_saved_channel_mhz + g_vars.g_saved_channel_khz / 1000.0;
	}
}


void read_encoder_standby_khz()  // KHz right inner knob
{
	//g_vars.g_current_page
//	float number = saved_channels[g_vars.g_selectedPreset];
//		g_vars.g_saved_channel_mhz = (uint16_t) number;
//		g_vars.g_saved_channel_khz =
//				(uint16_t) (((number - (uint16_t) number) + 0.0001) * 1000);
// Encoder interrupt routine for both pins. Updates counter
// if they are valid and have rotated a full indent
	if (!read_encoder_standby_khz_flag)
	{
		static uint8_t old_GH = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_GH <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A1_GPIO_Port, RIGHT_A1_Pin))
			old_GH |= 0x02;  // Add current state of pin G
		if (HAL_GPIO_ReadPin(RIGHT_B1_GPIO_Port, RIGHT_B1_Pin))
			old_GH |= 0x01;  // Add current state of pin H

		encval += enc_states[(old_GH & 0x0f)];

		// Update counter if encoder has rotated a full indent, that is at least 4 steps
		if (encval > 0)  // Four steps forward
		{
			g_vars.g_standby_khz_knob += CHANGE_KHZ;  // Update kHz counter

			// Check if kHz exceeds max and adjust MHz
			if (g_vars.g_standby_khz_knob > STANDBY_KHZ_MAX)
			{
				g_vars.g_standby_khz_knob = STANDBY_KHZ_MIN; // Reset kHz to minimum
				if (g_vars.g_standby_mhz_knob < STANDBY_MHZ_MAX)
				{
					g_vars.g_standby_mhz_knob++;  // Increment MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Incremented: %d\n", g_vars.g_standby_mhz_knob);
#endif
			}
			read_encoder_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_vars.g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		else if (encval < 0)  // Four steps backward
		{

			if (g_vars.g_standby_khz_knob == STANDBY_KHZ_MIN)
			{
				g_vars.g_standby_khz_knob = STANDBY_KHZ_MAX; // Reset kHz to maximum
				if (g_vars.g_standby_mhz_knob > STANDBY_MHZ_MIN)
				{
					g_vars.g_standby_mhz_knob--;  // Decrement MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Decremented: %d\n", g_vars.g_standby_mhz_knob);
#endif
			}
			else
			{
				g_vars.g_standby_khz_knob -= CHANGE_KHZ;  // Update kHz counter
			}
			read_encoder_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_vars.g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		//saved_channels[g_vars.g_selectedPreset] = g_vars.g_saved_channel_mhz + g_vars.g_saved_channel_khz / 1000.0;
		g_vars.g_standby_khz_knob = ((g_vars.g_standby_khz_knob+CHANGE_KHZ/2)/CHANGE_KHZ)*CHANGE_KHZ;
	}
}

void read_encoder_channel_khz()  // KHz right inner knob
{
	//g_vars.g_current_page
	float number = saved_channels[g_vars.g_selectedPreset];
	if (number <  STANDBY_MHZ_MIN) number = STANDBY_MHZ_MIN;
	if (number >  STANDBY_MHZ_MAX) number = STANDBY_MHZ_MAX;
		g_vars.g_saved_channel_mhz = (uint16_t) number;
		g_vars.g_saved_channel_khz =
				(uint16_t) (((number - (uint16_t) number) + 0.0001) * 1000);
// Encoder interrupt routine for both pins. Updates counter
// if they are valid and have rotated a full indent
	if (!read_encoder_channel_flag)
	{
		static uint8_t old_GH = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_GH <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A1_GPIO_Port, RIGHT_A1_Pin))
			old_GH |= 0x02;  // Add current state of pin G
		if (HAL_GPIO_ReadPin(RIGHT_B1_GPIO_Port, RIGHT_B1_Pin))
			old_GH |= 0x01;  // Add current state of pin H

		encval += enc_states[(old_GH & 0x0f)];

		// Update counter if encoder has rotated a full indent, that is at least 4 steps
		if (encval > 0)  // Four steps forward
		{
			g_vars.g_saved_channel_khz += CHANGE_KHZ;  // Update kHz counter

			// Check if kHz exceeds max and adjust MHz
			if (g_vars.g_saved_channel_khz > STANDBY_KHZ_MAX)
			{
				g_vars.g_saved_channel_khz = STANDBY_KHZ_MIN; // Reset kHz to minimum
				if (g_vars.g_saved_channel_mhz < STANDBY_MHZ_MAX)
				{
					g_vars.g_saved_channel_mhz++;  // Increment MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Incremented: %d\n", g_vars.g_standby_mhz_knob);
#endif
			}
			read_encoder_channel_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_vars.g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		else if (encval < 0)  // Four steps backward
		{

			if (g_vars.g_saved_channel_khz == STANDBY_KHZ_MIN)
			{
				g_vars.g_saved_channel_khz = STANDBY_KHZ_MAX; // Reset kHz to maximum
				if (g_vars.g_saved_channel_mhz > STANDBY_MHZ_MIN)
				{
					g_vars.g_saved_channel_mhz--;  // Decrement MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Decremented: %d\n", g_vars.g_standby_mhz_knob);
#endif
			}
			else
			{
				g_vars.g_saved_channel_khz -= CHANGE_KHZ;  // Update kHz counter
			}
			read_encoder_channel_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_vars.g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		g_vars.g_saved_channel_khz = ((g_vars.g_saved_channel_khz+CHANGE_KHZ/2)/CHANGE_KHZ)*CHANGE_KHZ;
		saved_channels[g_vars.g_selectedPreset] = g_vars.g_saved_channel_mhz + g_vars.g_saved_channel_khz / 1000.0;
	}
}


void read_encoder_volume()  // volume left inner knob
{                           // DONE volume without roll over
	if (!read_encoder_volume_flag)
	{
		static uint8_t old_CD = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_CD <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin))
			old_CD |= 0x02;  // Add current state of pin C
		if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin))
			old_CD |= 0x01;  // Add current state of pin D

		encval += enc_states[(old_CD & 0x0F)];

		if (encval > 1)  // Four steps forward
		{
			if (g_vars.g_volume_knob + CHANGE_VALUE <= VOLUME_MAX)
			{
				g_vars.g_volume_knob += CHANGE_VALUE;  // Increase volume
				read_encoder_volume_flag = true;
#if DEBUG_CONSOLE
                debug_print("E_VOL = %d \n", g_vars.g_volume_knob);
#endif
				encval = 0;  // Reset encoder counter after successful update
			}
		}
		else if (encval < -1)  // Four steps backward
		{
			if (g_vars.g_volume_knob - CHANGE_VALUE >= VOLUME_MIN)
			{
				g_vars.g_volume_knob -= CHANGE_VALUE;  // Decrease volume
				read_encoder_volume_flag = true;
#if DEBUG_CONSOLE
                debug_print("E_VOL = %d \n", g_vars.g_volume_knob);
#endif
				encval = 0;  // Reset encoder counter after successful update
			}
		}
	}
}

void scroll_freqs_memory()
{
	if (!scroll_flag)
	{
		static uint8_t old_AB = 3;  // Previous state of encoder
		static int8_t encval = 0;   // Accumulated encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 }; // State transition table

		old_AB <<= 2; // Shift the previous state to make space for the new state

		// Read the current state of encoder pins
		if (HAL_GPIO_ReadPin(LEFT_A2_GPIO_Port, LEFT_A2_Pin))
			old_AB |= 0x02;  // Add pin A state
		if (HAL_GPIO_ReadPin(LEFT_B2_GPIO_Port, LEFT_B2_Pin))
			old_AB |= 0x01;  // Add pin B state

		encval += enc_states[(old_AB & 0x0f)]; // Update encoder value based on state

		// Check if the encoder has completed one full step (4 state transitions)
		if (encval > 1)
		{  // Full step forward
			if (g_vars.g_current_page < MAX_PAGES)
			{
				g_vars.g_current_page++;
			}
			else
			{
				g_vars.g_current_page = 0;
			}
			// Update the display with the new frequency
			scroll_flag = true;
			encval = 0;  // Reset the encoder value
		}
		else if (encval < -1)
		{
			if (g_vars.g_current_page == 0)
			{
				g_vars.g_current_page = MAX_PAGES;
			}
			else
			{
				g_vars.g_current_page -= 1;  // Update kHz counter
			}
			scroll_flag = true;  // Update the display with the new frequency

			encval = 0;  // Reset the encoder value
		}
	}
}



void change_saved_channel_khz()
{
// Encoder interrupt routine for both pins. Updates counter
// if they are valid and have rotated a full indent
	float number = saved_channels[g_vars.g_selectedPreset];
	g_vars.g_saved_channel_mhz = (uint8_t) number;
	g_vars.g_saved_channel_khz =
			(uint8_t) (((number - (uint8_t) number) + 0.0001) * 1000);

	if (g_vars.g_saved_channel_mhz == 0)
	{
		g_vars.g_saved_channel_mhz = STANDBY_MHZ_MIN;
	}
	if (!read_saved_standby_khz_flag)
	{
		static uint8_t old_GH = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_GH <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A1_GPIO_Port, RIGHT_A1_Pin))
			old_GH |= 0x02;  // Add current state of pin G
		if (HAL_GPIO_ReadPin(RIGHT_B1_GPIO_Port, RIGHT_B1_Pin))
			old_GH |= 0x01;  // Add current state of pin H

		encval += enc_states[(old_GH & 0x0f)];

		// Update counter if encoder has rotated a full indent, that is at least 4 steps
		if (encval > 0)  // Four steps forward
		{
			g_vars.g_saved_channel_khz += CHANGE_KHZ;  // Update kHz counter

			// Check if kHz exceeds max and adjust MHz
			if (g_vars.g_saved_channel_khz > STANDBY_KHZ_MAX)
			{
				g_vars.g_saved_channel_khz = STANDBY_KHZ_MIN; // Reset kHz to minimum
				if (g_vars.g_saved_channel_mhz < STANDBY_MHZ_MAX)
				{
					g_vars.g_saved_channel_mhz++;  // Increment MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Incremented: %d\n", g_vars.g_standby_mhz_knob);
#endif
			}
			read_saved_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_vars.g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		else if (encval < 0)  // Four steps backward
		{
			if (g_vars.g_saved_channel_khz == STANDBY_KHZ_MIN)
			{
				g_vars.g_saved_channel_khz = STANDBY_KHZ_MAX; // Reset kHz to maximum
				if (g_vars.g_saved_channel_mhz > STANDBY_MHZ_MIN)
				{
					g_vars.g_saved_channel_mhz--;  // Decrement MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Decremented: %d\n", g_vars.g_standby_mhz_knob);
#endif
			}
			else
			{
				g_vars.g_saved_channel_khz -= CHANGE_KHZ;  // Update kHz counter
			}
			read_saved_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_vars.g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
	}
}

void change_saved_channel_mhz()
{
	g_vars.g_saved_channel_mhz = saved_channels[g_vars.g_selectedPreset];
	if (g_vars.g_saved_channel_mhz == 0)
	{
		g_vars.g_saved_channel_mhz = STANDBY_MHZ_MIN;
	}

//	g_vars. g_saved_channel_khz = (uint8_t)(((number - (uint8_t) number) + 0.001) * 100);
	if (!read_saved_standby_mhz_flag)
	{
		static uint8_t old_AB = 3;                         // Lookup table index
		static int8_t encval = 0;                               // Encoder value
		static const int8_t enc_states[] =
		{ 0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 1 };  // Lookup table

		old_AB <<= 2;  // Remember previous state

		if (HAL_GPIO_ReadPin(RIGHT_A2_GPIO_Port, RIGHT_A2_Pin))
			old_AB |= 0x02;  // Add current state of pin A
		if (HAL_GPIO_ReadPin(RIGHT_B2_GPIO_Port, RIGHT_B2_Pin))
			old_AB |= 0x01;  // Add current state of pin B

		encval += enc_states[(old_AB & 0x0f)];

		if (encval > 0)  // Rotate forward (increment)
		{
			if (g_vars.g_saved_channel_mhz < STANDBY_MHZ_MAX)
			{
				g_vars.g_saved_channel_mhz++;  // Increment by 1
			}
			else
			{
				g_vars.g_saved_channel_mhz = STANDBY_MHZ_MIN; // Wrap to 019 (after 179)
			}
			read_saved_standby_mhz_flag = true;
			encval = 0;
		}
		else if (encval < 0)  // Rotate backward (decrement)
		{
			if (g_vars.g_saved_channel_mhz > STANDBY_MHZ_MIN)
			{
				g_vars.g_saved_channel_mhz--;  // Decrement by 1
			}
			else
			{
				g_vars.g_saved_channel_mhz = STANDBY_MHZ_MAX; // Wrap to 179 (after going below 019)
			}
			read_saved_standby_mhz_flag = true;
			encval = 0;
		}
	}
}
