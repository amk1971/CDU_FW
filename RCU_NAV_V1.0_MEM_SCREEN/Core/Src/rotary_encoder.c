#include <main.h>
#include "rotary_encoder.h"

#include <stdbool.h>

#include "constant_defines.h"
#include "data_handler.h"
#include "stdint.h"
#include <math.h>

/* ------------------------------ ENCODER Operations -------------------------------- */

// flags for rotray
bool volatile read_encoder_standby_mhz_flag = false;
bool volatile read_encoder_standby_khz_flag = false;
bool volatile read_encoder_volume_flag = false;

bool volatile read_saved_standby_khz_flag = false;
bool volatile read_saved_standby_mhz_flag = false;
bool volatile scroll_flag = false;

extern uint8_t g_standby_mhz_knob;
extern uint16_t g_standby_khz_knob;
extern uint8_t g_volume_knob;
extern uint8_t g_saved_freq_khz;
extern uint8_t g_saved_freq_mhz;
extern uint8_t g_current_page;
extern uint8_t g_selectedPreset;
extern float saved_freqs[15];

void read_encoder_standby_mhz() // Controls the 10 kHz steps for larger place values (tens, hundreds, thousands)
{
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
			if (g_standby_mhz_knob < STANDBY_MHZ_MAX)
			{
				g_standby_mhz_knob++;  // Increment by 1
			}
			else
			{
				g_standby_mhz_knob = STANDBY_MHZ_MIN; // Wrap to 019 (after 179)
			}
			read_encoder_standby_mhz_flag = true;
			encval = 0;
		}
		else if (encval < 0)  // Rotate backward (decrement)
		{
			if (g_standby_mhz_knob > STANDBY_MHZ_MIN)
			{
				g_standby_mhz_knob--;  // Decrement by 1
			}
			else
			{
				g_standby_mhz_knob = STANDBY_MHZ_MAX; // Wrap to 179 (after going below 019)
			}
			read_encoder_standby_mhz_flag = true;
			encval = 0;
		}
	}
}

void read_encoder_standby_khz()  // KHz right inner knob
{
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
			g_standby_khz_knob += CHANGE_KHZ;  // Update kHz counter

			// Check if kHz exceeds max and adjust MHz
			if (g_standby_khz_knob > STANDBY_KHZ_MAX)
			{
				g_standby_khz_knob = STANDBY_KHZ_MIN;  // Reset kHz to minimum
				if (g_standby_mhz_knob < STANDBY_MHZ_MAX)
				{
					g_standby_mhz_knob++;  // Increment MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Incremented: %d\n", g_standby_mhz_knob);
#endif
			}
			read_encoder_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		else if (encval < 0)  // Four steps backward
		{
			if (g_standby_khz_knob == STANDBY_KHZ_MIN)
			{
				g_standby_khz_knob = STANDBY_KHZ_MAX;  // Reset kHz to maximum
				if (g_standby_mhz_knob > STANDBY_MHZ_MIN)
				{
					g_standby_mhz_knob--;  // Decrement MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Decremented: %d\n", g_standby_mhz_knob);
#endif
			}
			else
			{
				g_standby_khz_knob -= CHANGE_KHZ;  // Update kHz counter
			}
			read_encoder_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
	}
}

void read_encoder_volume()  // volume left inner knob
{
	// Encoder interrupt routine for both pins. Updates counter
	// if they are valid and have rotated a full indent
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

		encval += enc_states[(old_CD & 0x0f)];

		// Update counter if encoder has rotated a full indent, that is at least 4 steps
		if (encval > 1)
		{  // Four steps forward
			if (g_volume_knob < VOLUME_MAX)
			{
				g_volume_knob += CHANGE_VALUE;  // Update counter
				read_encoder_volume_flag = true;
#if DEBUG_CONSOLE
                debug_print("E_VOL = %d \n", volume_knob);
#endif
			}
			encval = 0;
		}
		else if (encval < -1)
		{  // Four steps backward
			if (g_volume_knob > VOLUME_MIN)
			{
				g_volume_knob -= CHANGE_VALUE;  // Update counter
				read_encoder_volume_flag = true;
#if DEBUG_CONSOLE
                debug_print("E_VOL = %d \n", volume_knob);
#endif
			}
			encval = 0;
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
		if (HAL_GPIO_ReadPin(LEFT_A1_GPIO_Port, LEFT_A1_Pin))
			old_AB |= 0x02;  // Add pin A state
		if (HAL_GPIO_ReadPin(LEFT_B1_GPIO_Port, LEFT_B1_Pin))
			old_AB |= 0x01;  // Add pin B state

		encval += enc_states[(old_AB & 0x0f)]; // Update encoder value based on state

		// Check if the encoder has completed one full step (4 state transitions)
		if (encval > 1)
		{  // Full step forward
			if (g_current_page < MAX_PAGES)
			{
				g_current_page++;
			}
			else
			{
				g_current_page = 0;
			}
			// Update the display with the new frequency
			scroll_flag = true;
			encval = 0;  // Reset the encoder value
		}
		else if (encval < -1)
		{
			if (g_current_page == 0)
			{
				g_current_page = MAX_PAGES;
			}
			else
			{
				g_current_page -= 1;  // Update kHz counter
			}
			scroll_flag = true;  // Update the display with the new frequency

			encval = 0;  // Reset the encoder value
		}
	}
}

void change_saved_freq_khz()
{
	// Encoder interrupt routine for both pins. Updates counter
	// if they are valid and have rotated a full indent
	float number = saved_freqs[g_selectedPreset];
	g_saved_freq_mhz = (uint8_t) number;
	g_saved_freq_khz = (uint8_t) (((number - (uint8_t) number) + 0.001) * 100);

	if (g_saved_freq_mhz == 0)
	{
		g_saved_freq_mhz = 108;
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
			g_saved_freq_khz += CHANGE_KHZ;  // Update kHz counter

			// Check if kHz exceeds max and adjust MHz
			if (g_saved_freq_khz > STANDBY_KHZ_MAX)
			{
				g_saved_freq_khz = STANDBY_KHZ_MIN;  // Reset kHz to minimum
				if (g_saved_freq_mhz < STANDBY_MHZ_MAX)
				{
					g_saved_freq_mhz++;  // Increment MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Incremented: %d\n", g_standby_mhz_knob);
#endif
			}
			read_saved_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
		else if (encval < 0)  // Four steps backward
		{
			if (g_saved_freq_khz == STANDBY_KHZ_MIN)
			{
				g_saved_freq_khz = STANDBY_KHZ_MAX;  // Reset kHz to maximum
				if (g_saved_freq_mhz > STANDBY_MHZ_MIN)
				{
					g_saved_freq_mhz--;  // Decrement MHz
				}
#if DEBUG_CONSOLE
                debug_print("E_MHz Decremented: %d\n", g_standby_mhz_knob);
#endif
			}
			else
			{
				g_saved_freq_khz -= CHANGE_KHZ;  // Update kHz counter
			}
			read_saved_standby_khz_flag = true;
#if DEBUG_CONSOLE
            debug_print("E_KHz = %d \n", g_standby_khz_knob);
#endif
			encval = 0;  // Reset encoder value
		}
	}
}

void change_saved_freq_mhz()
{

	g_saved_freq_mhz = saved_freqs[g_selectedPreset];
	if (g_saved_freq_mhz == 0)
	{
		g_saved_freq_mhz = 108;
	}

//	g_saved_freq_khz = (uint8_t)(((number - (uint8_t) number) + 0.001) * 100);
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
			if (g_saved_freq_mhz < STANDBY_MHZ_MAX)
			{
				g_saved_freq_mhz++;  // Increment by 1
			}
			else
			{
				g_saved_freq_mhz = STANDBY_MHZ_MIN;  // Wrap to 019 (after 179)
			}
			read_saved_standby_mhz_flag = true;
			encval = 0;
		}
		else if (encval < 0)  // Rotate backward (decrement)
		{
			if (g_saved_freq_mhz > STANDBY_MHZ_MIN)
			{
				g_saved_freq_mhz--;  // Decrement by 1
			}
			else
			{
				g_saved_freq_mhz = STANDBY_MHZ_MAX; // Wrap to 179 (after going below 019)
			}
			read_saved_standby_mhz_flag = true;
			encval = 0;
		}
	}
}
