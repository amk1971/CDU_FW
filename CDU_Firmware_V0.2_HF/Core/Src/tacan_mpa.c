/*
 * tacan_mpa.c
 *
 *  Created on: Oct 22, 2024
 *      Author: Hamza Javed
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "data_handler.h"

// Definations of constant Texts
#define STANDBY		"S"
#define ACTIVE		"A"
#define PRE_PRO_1	"P1"
#define PRE_PRO_2	"P2"
#define PRE_PRO_3	"P3"
#define PRE_PRO_4	"P4"
#define PRE_PRO_5	"P5"
#define PRE_PRO_6	"P6"
#define PRE_PRO_7	"P7"
#define PRE_PRO_8	"P8"

#define TACAN_FREQ_CHANNEL_MAP_SIZE 252

#define TACAN_CHANNEL_SIZE          4
typedef struct {
	char channel[TACAN_CHANNEL_SIZE];   // Channel like "1X", "1Y"
	float frequency;                    // Frequency like 134.40, 134.45
} ChannelFrequency;

ChannelFrequency tacan_freq_channel_map[TACAN_FREQ_CHANNEL_MAP_SIZE] = { { "1X",
		134.40 }, { "1Y", 134.45 }, { "2X", 134.50 }, { "2Y", 134.55 }, { "3X",
		134.60 }, { "3Y", 134.65 }, { "4X", 134.70 }, { "4Y", 134.75 }, { "5X",
		134.80 }, { "5Y", 134.85 }, { "6X", 134.90 }, { "6Y", 134.95 }, { "7X",
		135.00 }, { "7Y", 135.05 }, { "8X", 135.10 }, { "8Y", 135.15 }, { "9X",
		135.20 }, { "9Y", 135.25 }, { "10X", 135.30 }, { "10Y", 135.35 }, {
		"11X", 135.40 }, { "11Y", 135.45 }, { "12X", 135.50 },
		{ "12Y", 135.55 }, { "13X", 135.60 }, { "13Y", 135.65 },
		{ "14X", 135.70 }, { "14Y", 135.75 }, { "15X", 135.80 },
		{ "15Y", 135.85 }, { "16X", 135.90 }, { "16Y", 135.95 },
		{ "17X", 108.00 }, { "17Y", 108.05 }, { "18X", 108.10 },
		{ "18Y", 108.15 }, { "19X", 108.20 }, { "19Y", 108.25 },
		{ "20X", 108.30 }, { "20Y", 108.35 }, { "21X", 108.40 },
		{ "21Y", 108.45 }, { "22X", 108.50 }, { "22Y", 108.55 },
		{ "23X", 108.60 }, { "23Y", 108.65 }, { "24X", 108.70 },
		{ "24Y", 108.75 }, { "25X", 108.80 }, { "25Y", 108.85 },
		{ "26X", 108.90 }, { "26Y", 108.95 }, { "27X", 109.00 },
		{ "27Y", 109.05 }, { "28X", 109.10 }, { "28Y", 109.15 },
		{ "29X", 109.20 }, { "29Y", 109.25 }, { "30X", 109.30 },
		{ "30Y", 109.35 }, { "31X", 109.40 }, { "31Y", 109.45 },
		{ "32X", 109.50 }, { "32Y", 109.55 }, { "33X", 109.60 },
		{ "33Y", 109.65 }, { "34X", 109.70 }, { "34Y", 109.75 },
		{ "35X", 109.80 }, { "35Y", 109.85 }, { "36X", 109.90 },
		{ "36Y", 109.95 }, { "37X", 110.00 }, { "37Y", 110.05 },
		{ "38X", 110.10 }, { "38Y", 110.15 }, { "39X", 110.20 },
		{ "39Y", 110.25 }, { "40X", 110.30 }, { "40Y", 110.35 },
		{ "41X", 110.40 }, { "41Y", 110.45 }, { "42X", 110.50 },
		{ "42Y", 110.55 }, { "43X", 110.60 }, { "43Y", 110.65 },
		{ "44X", 110.70 }, { "44Y", 110.75 }, { "45X", 110.80 },
		{ "45Y", 110.85 }, { "46X", 110.90 }, { "46Y", 110.95 },
		{ "47X", 111.00 }, { "47Y", 111.05 }, { "48X", 111.10 },
		{ "48Y", 111.15 }, { "49X", 111.20 }, { "49Y", 111.25 },
		{ "50X", 111.30 }, { "50Y", 111.35 }, { "51X", 111.40 },
		{ "51Y", 111.45 }, { "52X", 111.50 }, { "52Y", 111.55 },
		{ "53X", 111.60 }, { "53Y", 111.65 }, { "54X", 111.70 },
		{ "54Y", 111.75 }, { "55X", 111.80 }, { "55Y", 111.85 },
		{ "56X", 111.90 }, { "56Y", 111.95 }, { "57X", 112.00 },
		{ "57Y", 112.05 }, { "58X", 112.10 }, { "58Y", 112.15 },
		{ "59X", 112.20 }, { "59Y", 112.25 }, { "60X", 133.30 },
		{ "60Y", 133.35 }, { "61X", 133.40 }, { "61Y", 133.45 },
		{ "62X", 133.50 }, { "62Y", 133.55 }, { "63X", 133.60 },
		{ "63Y", 133.65 }, { "64X", 133.70 }, { "64Y", 133.75 },
		{ "65X", 133.80 }, { "65Y", 133.85 }, { "66X", 133.90 },
		{ "66Y", 133.95 }, { "67X", 134.00 }, { "67Y", 134.05 },
		{ "68X", 134.10 }, { "68Y", 134.15 }, { "69X", 134.20 },
		{ "69Y", 134.25 }, { "70X", 112.30 }, { "70Y", 112.35 },
		{ "71X", 112.40 }, { "71Y", 112.45 }, { "72X", 112.50 },
		{ "72Y", 112.55 }, { "73X", 112.60 }, { "73Y", 112.65 },
		{ "74X", 112.70 }, { "74Y", 112.75 }, { "75X", 112.80 },
		{ "75Y", 112.85 }, { "76X", 112.90 }, { "76Y", 112.95 },
		{ "77X", 113.00 }, { "77Y", 113.05 }, { "78X", 113.10 },
		{ "78Y", 113.15 }, { "79X", 113.20 }, { "79Y", 113.25 },
		{ "80X", 113.30 }, { "80Y", 113.35 }, { "81X", 113.40 },
		{ "81Y", 113.45 }, { "82X", 113.50 }, { "82Y", 113.55 },
		{ "83X", 113.60 }, { "83Y", 113.65 }, { "84X", 113.70 },
		{ "84Y", 113.75 }, { "85X", 113.80 }, { "85Y", 113.85 },
		{ "86X", 113.90 }, { "86Y", 113.95 }, { "87X", 114.00 },
		{ "87Y", 114.05 }, { "88X", 114.10 }, { "88Y", 114.15 },
		{ "89X", 114.20 }, { "89Y", 114.25 }, { "90X", 114.30 },
		{ "90Y", 114.35 }, { "91X", 114.40 }, { "91Y", 114.45 },
		{ "92X", 114.50 }, { "92Y", 114.55 }, { "93X", 114.60 },
		{ "93Y", 114.65 }, { "94X", 114.70 }, { "94Y", 114.75 },
		{ "95X", 114.80 }, { "95Y", 114.85 }, { "96X", 114.90 },
		{ "96Y", 114.95 }, { "97X", 115.00 }, { "97Y", 115.05 },
		{ "98X", 115.10 }, { "98Y", 115.15 }, { "99X", 115.20 },
		{ "99Y", 115.25 }, { "100X", 115.30 }, { "100Y", 115.35 }, { "101X",
				115.40 }, { "101Y", 115.45 }, { "102X", 115.50 }, { "102Y",
				115.55 }, { "103X", 115.60 }, { "103Y", 115.65 }, { "104X",
				115.70 }, { "104Y", 115.75 }, { "105X", 115.80 }, { "105Y",
				115.85 }, { "106X", 115.90 }, { "106Y", 115.95 }, { "107X",
				116.00 }, { "107Y", 116.05 }, { "108X", 116.10 }, { "108Y",
				116.15 }, { "109X", 116.20 }, { "109Y", 116.25 }, { "110X",
				116.30 }, { "110Y", 116.35 }, { "111X", 116.40 }, { "111Y",
				116.45 }, { "112X", 116.50 }, { "112Y", 116.55 }, { "113X",
				116.60 }, { "113Y", 116.65 }, { "114X", 116.70 }, { "114Y",
				116.75 }, { "115X", 116.80 }, { "115Y", 116.85 }, { "116X",
				116.90 }, { "116Y", 116.95 }, { "117X", 117.00 }, { "117Y",
				117.05 }, { "118X", 117.10 }, { "118Y", 117.15 }, { "119X",
				117.20 }, { "119Y", 117.25 }, { "120X", 117.30 }, { "120Y",
				117.35 }, { "121X", 117.40 }, { "121Y", 117.45 }, { "122X",
				117.50 }, { "122Y", 117.55 }, { "123X", 117.60 }, { "123Y",
				117.65 }, { "124X", 117.70 }, { "124Y", 117.75 }, { "125X",
				117.80 }, { "125Y", 117.85 }, { "126X", 117.90 }, { "126Y",
				117.95 } };

/*
 * Function: custom_compare
 * Arguments: const char *a, const char *b
 *
 * Description:
 * This function compares two strings based on their numeric and alphabetic parts.
 * It first extracts and compares the numeric parts of the strings. If the numeric
 * parts are equal, it then compares the alphabetic parts. The function returns:
 * - -1 if the first string is less than the second.
 * - 1 if the first string is greater than the second.
 * - 0 if both strings are equal.
 *
 * - a: The first string to be compared.
 * - b: The second string to be compared.
 *
 * Returns:
 * - -1 if the numeric part of a is less than b, or if their numeric parts are equal but a's alphabetic part is less than b's.
 * - 1 if the numeric part of a is greater than b, or if their numeric parts are equal but a's alphabetic part is greater than b's.
 * - 0 if both strings are equal in numeric and alphabetic parts.
 */
int8_t custom_compare(const char *a, const char *b) {
	// Extract the numeric parts from both strings
	int num_a = 0, num_b = 0;

	// Parse the numeric parts from the strings
	num_a = atoi(a);  // Convert initial part of a to integer
	num_b = atoi(b);  // Convert initial part of b to integer

	// Compare the numeric parts
	if (num_a < num_b) {
		return -1;
	} else if (num_a > num_b) {
		return 1;
	}

	// If numeric parts are equal, compare the alphabetic parts
	const char *alpha_a = a;  // Pointer to alphabetic part of a
	const char *alpha_b = b;  // Pointer to alphabetic part of b

	// Move the pointer to the first alphabet character in the string
	while (isdigit((unsigned char )*alpha_a))
		alpha_a++;
	while (isdigit((unsigned char )*alpha_b))
		alpha_b++;

	// Compare the alphabetic part (using strcmp)
	if (*alpha_a == *alpha_b) {
		return 0;
	} else if (*alpha_a < *alpha_b) {
		return -1;
	}

	return 1;
}

/*
 * Function: search_tacan_channel_
 * Arguments: const char *key, uint16_t low, uint16_t high
 *
 * Description:
 * This function performs a binary search on the `tacan_freq_channel_map` array to find the
 * specified key. It compares the key with the mid-point of the current search range and adjusts
 * the range accordingly. If the key is found, the function returns the index of the key in the
 * array. If the key is not found, the function returns -1.
 *
 * - key: The key to search for in the `tacan_freq_channel_map` array.
 * - low: The lower bound of the search range.
 * - high: The upper bound of the search range.
 *
 * Returns:
 * - The index of the key if found.
 * - -1 if the key is not found in the array.
 */
int16_t search_tacan_channel_(const char *key, uint16_t low, uint16_t high) {
	uint16_t mid = 0;
	while (low <= high) {
		mid = low + (high - low) / 2;
		int8_t cmp_result = custom_compare(tacan_freq_channel_map[mid].channel,
				key);   //strcmp(key, tacan_freq_channel_map[mid].channel);
		if (cmp_result == 0) {
			return mid;
		}

		if (cmp_result < 0) {
			low = mid + 1;
		} else {
			high = mid - 1;
		}
	}
//    printf("low: %d, mid: %d, high: %d\n", low, mid, high);
	return -1;  // Key not found in the array
}

/*
 * Function: find_value
 * Arguments: const char *key
 *
 * Description:
 * This function searches for a specified key in the `tacan_freq_channel_map` array
 * and returns its associated frequency value. If the key is not found, the function
 * returns -1.0f to indicate that the key was not found.
 *
 * - key: The key to search for in the `tacan_freq_channel_map` array.
 *
 * Returns:
 * - The frequency value associated with the key if found.
 * - -1.0f if the key is not found in the array.
 */
float find_value(const char *key) {
	uint16_t len = sizeof(tacan_freq_channel_map)
			/ sizeof(tacan_freq_channel_map[0]);
	int16_t index = 0;
	index = search_tacan_channel_(key, 0, len - 1);

	if (index == -1) {
		return -1.0f;  // Key not found in the array
	} else {
		return tacan_freq_channel_map[index].frequency; // Return the value if key is found
	}
}

/*
 * Function: search_tacan_frequency_
 * Arguments: float freq, uint16_t len
 *
 * Description:
 * This function searches for a specific frequency in the `tacan_freq_channel_map` array.
 * It iterates through the array and compares each entry's frequency with the specified
 * frequency. If the frequency is found, the function returns the index of the entry.
 * If the frequency is not found, the function returns -1.
 *
 * - freq: The frequency to search for in the array.
 * - len: The length of the array to search within.
 *
 * Returns:
 * - The index of the entry if the frequency is found.
 * - -1 if the frequency is not found in the array.
 */
int16_t search_tacan_frequency_(float freq, uint16_t len) {
	uint16_t i = 0;
	while (i <= len) {
		if (tacan_freq_channel_map[i].frequency == freq) {
			return i;
		}
		i++;
	}
	return -1;  // Key not found in the array
}

/*
 * Function: find_channel
 * Arguments: float freq, char *key
 *
 * Description:
 * This function finds the channel corresponding to a specified frequency in the
 * `tacan_freq_channel_map` array. It uses the `search_tacan_frequency_` function
 * to find the index of the frequency in the array. If the frequency is found, the
 * function copies the corresponding channel string to the `key` buffer. If the
 * frequency is not found, it prints an error message and returns -1.
 *
 * - freq: The frequency to search for in the array.
 * - key: Pointer to the buffer where the channel string will be stored.
 *
 * Returns:
 * - 0 if the channel is found and copied to the key buffer.
 * - -1 if the frequency is not found in the array.
 */
int8_t find_channel(float freq, char *key) {

	if (key == NULL) {
		printf("Error: key buffer is NULL!\n");
		return -1;
	}

	uint16_t len = sizeof(tacan_freq_channel_map)
			/ sizeof(tacan_freq_channel_map[0]);
	int16_t index = search_tacan_frequency_(freq, len - 1);

	if (index == -1) {
		printf("Key not found in the array!\n");
		return -1;
	}

	uint8_t size = strnlen(tacan_freq_channel_map[index].channel,
			sizeof(tacan_freq_channel_map[index].channel));

	// Ensure the copied string does not overflow the key buffer
	strncpy(key, tacan_freq_channel_map[index].channel, TACAN_S - 1);
	key[TACAN_S - 1] = '\0';  // Always null-terminate

	return 0;
}

/*
 * Function: update_corresponding_frequency
 * Arguments: const char *identifier, const char *key, CDU_Parameters *param
 *
 * Description:
 * This function updates the corresponding TACAN frequency in the CDU_Parameters structure
 * based on the provided identifier and key. It searches for the frequency associated with
 * the given key using the `find_value` function. If the frequency is found, it updates the
 * appropriate field (standby, active, or one of the preset channels) in the CDU_Parameters structure.
 *
 * - identifier: The identifier for the frequency to be updated (e.g., STANDBY, ACTIVE, PRE_PRO_1).
 * - key: The key to search for in the `tacan_freq_channel_map` array to get the corresponding frequency.
 * - param: Pointer to the CDU_Parameters structure to be updated.
 */
void update_corresponding_frequency(const char *identifier, const char *key,
		CDU_Parameters *param) {
	float f_val = 0;

	f_val = find_value(key);
	if (f_val >= 0) {
		if (strcmp(identifier, STANDBY) == 0) {
			param->standby_tacan_freq = f_val;
		} else if (strcmp(identifier, ACTIVE) == 0) {
			param->active_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_1) == 0) {
			param->p1_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_2) == 0) {
			param->p2_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_3) == 0) {
			param->p3_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_4) == 0) {
			param->p4_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_5) == 0) {
			param->p5_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_6) == 0) {
			param->p6_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_7) == 0) {
			param->p7_tacan_freq = f_val;
		} else if (strcmp(identifier, PRE_PRO_8) == 0) {
			param->p8_tacan_freq = f_val;
		}
	} else {
		return;
	}
}

/*
 * Function: update_corresponding_channel
 * Arguments: const char *identifier, float f_val, CDU_Parameters *param
 *
 * Description:
 * This function updates the corresponding TACAN channel in the CDU_Parameters structure
 * based on the provided identifier and frequency value. It searches for the channel associated
 * with the given frequency using the `find_channel` function. If the channel is found, it updates
 * the appropriate field (standby, active, or one of the preset channels) in the CDU_Parameters structure.
 *
 * - identifier: The identifier for the channel to be updated (e.g., STANDBY, ACTIVE, PRE_PRO_1).
 * - f_val: The frequency value to search for and associate with the channel.
 * - param: Pointer to the CDU_Parameters structure to be updated.
 */
void update_corresponding_channel(const char *identifier, float f_val,
		CDU_Parameters *param) {
	char tacan_channel[TACAN_S];
	uint8_t length = 0;
	int8_t status = 0;
	memset(tacan_channel, 0, TACAN_S);
	status = find_channel(f_val, tacan_channel);
	length = strlen(tacan_channel);
	if (status >= 0) {
		if (strcmp(identifier, STANDBY) == 0) {
			memset(param->standby_channel, 0, TACAN_S);
			memcpy(param->standby_channel, tacan_channel, length);
		} else if (strcmp(identifier, ACTIVE) == 0) {
			memset(param->active_channel, 0, TACAN_S);
			memcpy(param->active_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_1) == 0) {
			memset(param->p1_channel, 0, TACAN_S);
			memcpy(param->p1_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_2) == 0) {
			memset(param->p2_channel, 0, TACAN_S);
			memcpy(param->p2_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_3) == 0) {
			memset(param->p3_channel, 0, TACAN_S);
			memcpy(param->p3_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_4) == 0) {
			memset(param->p4_channel, 0, TACAN_S);
			memcpy(param->p4_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_5) == 0) {
			memset(param->p5_channel, 0, TACAN_S);
			memcpy(param->p5_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_6) == 0) {
			memset(param->p6_channel, 0, TACAN_S);
			memcpy(param->p6_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_7) == 0) {
			memset(param->p7_channel, 0, TACAN_S);
			memcpy(param->p7_channel, tacan_channel, length);
		} else if (strcmp(identifier, PRE_PRO_8) == 0) {
			memset(param->p8_channel, 0, TACAN_S);
			memcpy(param->p8_channel, tacan_channel, length);
		}

	}
}
