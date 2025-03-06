/*
 * data_validation.c
 *
 *  Created on: Oct 30, 2024
 *      Author: Hamza Javed
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "sys_defines.h"
#include "data_handler.h"
#include "debug_console.h"


#define TWO_DEC			100
#define THREE_DEC		1000

#define PRECISION_FACTOR_4     0.0001
#define PRECISION_FACTOR_3     0.001
#define PRECISION_FACTOR_2     0.01

#define ROUNDING_FACTOR     0.5

/* NAV FREQUENCY RANGE*/
#define NAV_MIN_FREQ        108.00
#define NAV_MAX_FREQ        117.95 + PRECISION_FACTOR_2

#define NAV_INCREMENT       50

/* TACAN FREQUENCY RANGE*/
#define TACAN_MIN_FREQ_0    108.00
#define TACAN_MAX_FREQ_0    117.95 + PRECISION_FACTOR_2
#define TACAN_MIN_FREQ_1	133.40
#define TACAN_MAX_FREQ_1	134.25 + PRECISION_FACTOR_2
#define TACAN_MIN_FREQ_2    134.40
#define TACAN_MAX_FREQ_2    135.95 + PRECISION_FACTOR_2

#define TACAN_INCREMENT     5

/* TACAN CHANAL RANGE */
#define TACAN_MIN_CHANNEL   1
#define TACAN_MAX_CHANNEL   126
#define TACAN_CHANNEL_X     'X'
#define TACAN_CHANNEL_Y     'Y'

/* ADF FREQUENCY RANGE*/
#define ADF_MIN_FREQ        190
#define ADF_MAX_FREQ        1799 //+ PRECISION_FACTOR_2

#define ADF_INCREMENT       1

/* HF FREQUENCY RANGE*/
#define HF_MIN_FREQ         2.0000
#define HF_MAX_FREQ         2.9999 + PRECISION_FACTOR_4

#define HF_INCREMENT        100

/* VHF FREQUENCY RANGE*/
#define VHF_MIN_FREQ        118.000
#define VHF_MAX_FREQ        137.975 + PRECISION_FACTOR_3

#define VHF_INCREMENT       25

/* UHF FREQUENCY RANGE*/
#define UHF_MIN_FREQ        108.00
#define UHF_MAX_FREQ        117.975 + PRECISION_FACTOR_3

#define UHF_INCREMENT       25

/* Volume Range*/
#define MINI_VOL	0
#define MAX_VOL		20
/* --------------------------- - --------------------------- */


/*
 * Function: has_two_decimal_places
 * Arguments: float value
 *
 * Description:
 * This function checks if a given float value has exactly two decimal places.
 * It scales the fractional part of the value and rounds it to both 2 and 3 decimal places.
 * It then compares the scaled values with a small tolerance to account for floating-point precision errors.
 * If the value has exactly two decimal places, the function returns true; otherwise, it returns false.
 *
 * - value: The float value to be checked.
 *
 * Returns:
 * - true if the value has exactly two decimal places.
 * - false if the value has more than two decimal places.
 */
bool has_two_decimal_places(float value)
{
    float fractional_part = value - (int)value;

    // Scale the fractional part and round to desired precision
    int scaled_100 = (int)(fractional_part * TWO_DEC + ROUNDING_FACTOR);   // Rounding to 2 decimal places
    int scaled_1000 = (int)(fractional_part * THREE_DEC + ROUNDING_FACTOR); // Rounding to 3 decimal places

    // Compare with a small tolerance to account for floating-point precision errors
    if (fabs(scaled_100 * 10 - scaled_1000) < 1) { // Tolerance threshold of < 1
        return true;   // Has exactly 2 decimal places
    } else {
        return false;  // Has more than 2 decimal places (like 3)
    }
}

/*
 * Function: input_data_validation_channel
 * Arguments: Class_Id input_class, const char* channel
 *
 * Description:
 * This function validates the input channel for a given class. It checks if the input
 * string representing the channel is valid based on the specified class. For TACAN,
 * it ensures the channel ends with 'X' or 'Y' and is within the valid channel range.
 * For other classes, additional validation logic can be added as required.
 *
 * - input_class: The class of the channel (e.g., TACAN, HF).
 * - channel: The string representing the channel to be validated.
 *
 * Returns:
 * - true if the input channel is valid for the specified class.
 * - false if the input channel is not valid for the specified class.
 */
bool input_data_validation_channel(Class_Id input_class, const char* channel)
{
    bool status = false;
    uint8_t len = 0;
    uint8_t inp_Number = 0;
    len = strlen(channel);

    if (len <= 1) {
        return status;
    }

    switch (input_class)
    {
		case TACAN:
			if((channel[len - 1] == TACAN_CHANNEL_X) || (channel[len - 1] == TACAN_CHANNEL_Y))
			{
				inp_Number = atoi(channel);
				if((inp_Number >= TACAN_MIN_CHANNEL) && (inp_Number <= TACAN_MAX_CHANNEL))
				{
					status = true;
				}
			}
		break;
		case HF:
		break;
		default:
			status = false;
		break;
    }
    return status;
}

bool input_volume_validation(uint8_t inp_vol)
{
	bool validate = false;
	if((inp_vol  >= MINI_VOL) && (inp_vol  <= MAX_VOL))
	{
		validate = true;
	}
	return validate;
}

/*
 * Function: input_data_validation_freq
 * Arguments: Class_Id input_class, float input_val
 *
 * Description:
 * This function validates the input frequency for a given class. It checks if the input
 * value falls within the acceptable range for the specified class and whether it aligns
 * with the required increments. The function handles different frequency classes (NAV, ADF,
 * TACAN, HF, VHF, UHF) and calculates the integer (MHz) and fractional (kHz) parts of the
 * input frequency to validate the increments.
 *
 * - input_class: The class of the frequency (e.g., NAV, ADF, TACAN, HF, VHF, UHF).
 * - input_val: The frequency value to be validated.
 *
 * Returns:
 * - true if the input frequency is valid for the specified class.
 * - false if the input frequency is not valid for the specified class.
 */
bool input_data_validation_freq(Class_Id input_class, float input_val)
{
	bool status = false;
	uint16_t mhz_p = 0;
	uint16_t khz_p = 0;

	mhz_p = (uint16_t)(input_val);

    khz_p = (uint16_t)((input_val - mhz_p)*1000 + ROUNDING_FACTOR);
//    if(has_two_decimal_places(input_val))
//    {
//        khz_p = (uint16_t)((input_val - mhz_p)*100 + ROUNDING_FACTOR);
//    }
//    else
//    {
//        khz_p = (uint16_t)((input_val - mhz_p)*1000 + ROUNDING_FACTOR);
//    }
#if DEBUG_CONSOLE
    debug_print("input = %f\n", input_val);
    debug_print("mhz_p = %d\n", mhz_p);
    debug_print("khz_p = %d\n", khz_p);
#endif
    switch(input_class)
    {
		case NAV:
			if (input_val >= NAV_MIN_FREQ && input_val < NAV_MAX_FREQ)
			{
				if(khz_p % NAV_INCREMENT == 0)
				{
					status = true;
				}
			}
		break;
		case ADF:
			uint16_t val=(uint16_t)((input_val)*1000 );;
			if ((val >= ADF_MIN_FREQ )&& (val <= ADF_MAX_FREQ))
			{
				status = true;
			}
        break;
		case TACAN:
			if ((input_val >= TACAN_MIN_FREQ_0 && input_val < TACAN_MAX_FREQ_0) || (input_val >= TACAN_MIN_FREQ_1 && input_val < TACAN_MAX_FREQ_1) || (input_val >= TACAN_MIN_FREQ_2 && input_val < TACAN_MAX_FREQ_2))
			{
				if(khz_p % TACAN_INCREMENT == 0)
				{
					status = true;
				}
			}
        break;
		case HF:
			if (input_val >= HF_MIN_FREQ && input_val < HF_MAX_FREQ)
			{
				status = true;
			}
        break;
		case VHF:
			if (input_val >= VHF_MIN_FREQ && input_val < VHF_MAX_FREQ)
			{
				if(khz_p % VHF_INCREMENT == 0)
				{
					status = true;
				}
			}
    	break;
		case UHF:
			if (input_val >= UHF_MIN_FREQ && input_val < UHF_MAX_FREQ)
			{
				if(khz_p % UHF_INCREMENT == 0)
				{
					status = true;
				}
			}
    	break;
		default:
			status = false;
		break;
    }
    return status;
}




