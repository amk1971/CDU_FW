#include "global_vars.h"

#include <stdint.h>

float saved_channels[saved_channel_SIZE] =
{ 0 };
uint8_t saved_channel_index = 0;

s_global_vars g_vars =
{ 0 };

void update_globals(s_HF_Parameters *obj)
{
	g_vars.g_volume_knob = obj->volume;
	g_vars.g_current_page = 0;
	g_vars.g_standby_mhz_knob = (uint16_t) obj->standby_freq;
	g_vars.g_standby_khz_knob = (uint16_t) ((obj->standby_freq
			- g_vars.g_standby_mhz_knob) * 1000);
	g_vars.cursor_blink = OFF;
	g_vars.current_cursor_location = START_CURSOR_LOCATION;
	g_vars.previous_cursor_location = 0;

	uint8_t map[] =
	{ 0, 8, 16, 24, 40}; // no 4 cause decimal is ignored
	memcpy(g_vars.cursor_map, map, sizeof(map));
}

void set_saved_channel_index(uint8_t index)
{
	if (index < saved_channel_SIZE)
	{
		saved_channel_index = index;
	}
	else
	{
		// Handle out-of-bounds index (e.g., reset to 0 or max limit)
		saved_channel_index =
				(index >= saved_channel_SIZE) ? saved_channel_SIZE - 1 : 0;
	}
}

// Get the current global index
uint8_t get_saved_channel_index()
{
	return saved_channel_index;
}
