/*
 * rcu_display.c
 *
 *  Created on: Nov 28, 2024
 *      Author: Hamza Javed
 */

#include "rcu_display.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "constant_defines.h"
#include "data_handler.h"
#include "glcd.h"
#include "glcd_font.h"
#include "global_vars.h"
#include "main.h"

/*-----------------------------------------------------------------------------------------------
 **function code**
 ------------------------------------------------------------------------------------------------*/

// Global buffer to hold the result
char globalBuffer[100];

void glcd_axis(void)
{
	// make x axis----------------------
	for (char x = 0; x <= 127; x++)
	{
		glcd_puts_point(x, 0, 1);
	}
	// make y axis----------------------
	for (char y = 0; y <= 7; y++)
	{
		glcd_gotoxy(0, y);
		glcd_putchar(0xff, 0);
	}
}
//--------------------------------------
void glcd_plot(float number, char step)
{
	Delay(T);
	char a, s;
	s = step * 4;

	// print point on coordinates---------
	for (a = 0; a <= 32; a++)
	{
		if (number <= (0.03230 * a) && number > (0.03230 * (a - 1)))
		{
			glcd_puts_point(s, 64 - (a * 2), 2);
			glcd_puts_point(s + 1, 64 - (2 * a), 2);
		}
	}
}
//-------------------------------------
// Function For Initialize GLCD Pins
// Function For Initialize GLCD Pins
void GLCD_INIT(void)
{
	// Enable clock for GPIO ports A, B, C, D, and E
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN
			| RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;

	// Set the pin modes to output for DATA_PORT and CTRL_PORT
	// GPIOA: D5, D6, D7 (PA7, PA6, PA5), CS1 (PA4), CS2 (PA3), RESET (PA2)
	DATA5_PORT->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
	DATA5_PORT->CRL |= GPIO_CRL_MODE7_0;
	DATA6_PORT->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6);
	DATA6_PORT->CRL |= GPIO_CRL_MODE6_0;
	DATA7_PORT->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
	DATA7_PORT->CRL |= GPIO_CRL_MODE5_0;

	SECO1_PORT->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4 | GPIO_CRL_MODE3
			| GPIO_CRL_CNF3);
	SECO1_PORT->CRL |= (GPIO_CRL_MODE4_0 | GPIO_CRL_MODE3_0);

	RESET_PORT->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
	RESET_PORT->CRL |= GPIO_CRL_MODE2_0;

	// GPIOB: D0, D3 (PB1, PB0)
	DATA0_PORT->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 | GPIO_CRL_MODE0
			| GPIO_CRL_CNF0);
	DATA0_PORT->CRL |= (GPIO_CRL_MODE1_0 | GPIO_CRL_MODE0_0);

	// GPIOE: D1, DI, RW, E (PE7, PE10, PE8, PE9)
	DATA1_PORT->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
	DATA1_PORT->CRL |= GPIO_CRL_MODE7_0;
	CTRL_PORT->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10 | GPIO_CRH_MODE8
			| GPIO_CRH_CNF8 | GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
	CTRL_PORT->CRH |= (GPIO_CRH_MODE10_0 | GPIO_CRH_MODE8_0 | GPIO_CRH_MODE9_0);

	// GPIOC: D2, D4 (PC5, PC4)
	DATA2_PORT->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
	DATA2_PORT->CRL |= GPIO_CRL_MODE5_0;
	DATA4_PORT->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
	DATA4_PORT->CRL |= GPIO_CRL_MODE4_0;

	// Reset data and control ports
	DATA0_PORT->BSRR = (1 << (D0_PIN_NUMBER + 16));
	DATA1_PORT->BSRR = (1 << (D1_PIN_NUMBER + 16));
	DATA2_PORT->BSRR = (1 << (D2_PIN_NUMBER + 16));
	DATA3_PORT->BSRR = (1 << (D3_PIN_NUMBER + 16));
	DATA4_PORT->BSRR = (1 << (D4_PIN_NUMBER + 16));
	DATA5_PORT->BSRR = (1 << (D5_PIN_NUMBER + 16));
	DATA6_PORT->BSRR = (1 << (D6_PIN_NUMBER + 16));
	DATA7_PORT->BSRR = (1 << (D7_PIN_NUMBER + 16));
	SECO1_PORT->BSRR = (1 << (CS1_PIN_NUMBER + 16));
	SECO2_PORT->BSRR = (1 << (CS2_PIN_NUMBER + 16));
	RESET_PORT->BSRR = (1 << (RESET_PIN_NUMBER + 16));

	// Reset the GLCD
	RESET_PORT->BSRR = (1 << RESET_PIN_NUMBER); // Set reset pin high
	Delay(10); // Wait for a short period
	RESET_PORT->BSRR = (1 << (RESET_PIN_NUMBER + 16)); // Set reset pin low
	Delay(10); // Wait for a short period
	RESET_PORT->BSRR = (1 << RESET_PIN_NUMBER); // Set reset pin high
	Delay(10); // Wait for a short period
}

//----------------------------------------------------------------------------------------------
void glcd_line(int x1, int x2, int y)
{
	for (int k = x1; k <= x2; k++)
	{
		static char arr[] = "-";
		glcd_puts(arr, k, y);
	}
}

void LCD_power_on(void)
{
	glcd_on();
	HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_SET);
	glcd_clear_all();
}

void LCD_power_off(void)
{
	if (HF_parameters.power_on == OFF)
	{
		glcd_off();
		HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_RESET);
	}
}
void LCD_Print_Home(void)
{

	char strS[12];
	char strV[8];

	//    static float prev_active_freq = 0;
	static float prev_standby_freq = 0;
	static uint8_t prev_volume = 0;

	memset(strS, 0, sizeof(strS));
	memset(strV, 0, sizeof(strV));

	// Check if standby frequency has changed

	if (HF_parameters.FRQ_CH == FRQ)
	{
		glcd_clear_text("FRQ", 75, 1);
		glcd_puts("/", 98, 1);
		glcd_puts("CH", 105, 1);

	}
	if (HF_parameters.FRQ_CH == CH)
	{
		glcd_puts("FRQ", 75, 1);
		glcd_puts("/", 98, 1);
		glcd_clear_text("CH", 105, 1);

	}

	glcd_puts("STO", 2, 3);

	if (HF_parameters.STO == ON && HF_parameters.FRQ_CH == CH)
	{
		glcd_puts("T", 70, 1);
	}
	else
	{
		glcd_clear_here(70, 75, 1, 1);
	}

	glcd_puts("PROG", 2, 6);

	if (HF_parameters.standby_freq != prev_standby_freq
			|| HF_parameters.power_on == ON || HF_parameters.PROG == OFF)
	{
		if (HF_parameters.standby_freq == EMPTY_FREQ)

		{
			glcd_clear_here(2, 68, 1, 1); // Clear specific area for standby frequency
			glcd_puts("NO MEM", 1, 1);
		}
		else
		{
			snprintf(strS, sizeof(strS), "%.1f", HF_parameters.standby_freq); // coulmn first

			glcd_clear_here(2, 68, 1, 1); // Clear specific area for standby frequency
			glcd_puts(strS, 1, 1);          // Update standby frequency
			prev_standby_freq = HF_parameters.standby_freq;
		}
	}
	// Check if volume has changed
	if (HF_parameters.volume != prev_volume || HF_parameters.power_on == ON
			|| HF_parameters.PROG == OFF)
	{
		glcd_clear_here(50, 70, 6, 6);  // Clear specific area for volume
		snprintf(strV, sizeof(strV), "Vol%02d", HF_parameters.volume);
		glcd_puts(strV, 50, 6);  // Update volume
		prev_volume = HF_parameters.volume;
	}
}

void LCD_PRINT_MEM_SCREEN(uint8_t page)
{
	glcd_clear_here(85, 120, 2, 2);
	glcd_clear_here(85, 120, 4, 4);
	glcd_clear_here(85, 120, 6, 6);
	glcd_clear_here(2, 60, 2, 2);

	uint8_t start_preset = page * PRESETS_PER_PAGE;
	for (int i = 0; i < PRESETS_PER_PAGE; i++)
	{
		int presetIndex = start_preset + i;  // Calculate preset index
		char presetLabel[8];
		char strS[10];

		if (presetIndex <= TOTAL_PRESETS)
		{
			snprintf(presetLabel, sizeof(presetLabel), "CH%d", presetIndex + 1); // Presets are 1-based
		}

		else  // should not come here since preset should not exceed  15
		{
			snprintf(presetLabel, sizeof(presetLabel), "EMPTY");
			snprintf(strS, sizeof(strS), "no here");  // No frequency
		}

		if (presetIndex == g_vars.g_selectedPreset)
		{
			glcd_clear_text(presetLabel, 90, 2 + (i * 2)); // Highlighting rectangle for selected preset
		}
		else
		{
			// Display the preset normally
			glcd_puts(presetLabel, 90, 2 + (i * 2));  // Normal text
		}
	}

	if (g_vars.g_selectedPreset < NUM_FREQUENCIES)
	{
		char selectedFreq[9];
		glcd_clear_here(2, 60, 2, 2);

		if (saved_channels[g_vars.g_selectedPreset] == EMPTY_FREQ)
		{
			glcd_puts("EMPTY", 2, 2);
		}
		else
		{
			snprintf(selectedFreq, sizeof(selectedFreq), "%0.2f",
					saved_channels[g_vars.g_selectedPreset]);
			glcd_puts(selectedFreq, 2, 2);
		}
	}

	glcd_puts("DELETE", 2, 4);
	glcd_puts("EXIT", 2, 6);
}

void LCD_PRINT_CURSOR(uint8_t status, uint8_t cursor_location)
{
	uint8_t loc = cursor_location;
	if (status == true)
	{
		glcd_puts("_", loc, 2);
	}
	else
	{
		glcd_puts("   ", loc, 2);

//		glcd_clear_here(loc,loc+2, 2, 2);
	}
}

// Generic function to update values in any region
void LCD_UpdateRegion(LCD_Region region, const char* value) {
    int x1, x2, y1, y2;

    switch (region) {
        case TL:  x1 = TL_X1;  x2 = TL_X2;  y1 = TL_Y1;  y2 = TL_Y2;  break;
        case ML:  x1 = ML_X1;  x2 = ML_X2;  y1 = ML_Y1;  y2 = ML_Y2;  break;
        case BL:  x1 = BL_X1;  x2 = BL_X2;  y1 = BL_Y1;  y2 = BL_Y2;  break;
        case MT:  x1 = MT_X1;  x2 = MT_X2;  y1 = MT_Y1;  y2 = MT_Y2;  break;
        case MM:  x1 = MM_X1;  x2 = MM_X2;  y1 = MM_Y1;  y2 = MM_Y2;  break;
        case MB:  x1 = MB_X1;  x2 = MB_X2;  y1 = MB_Y1;  y2 = MB_Y2;  break;
        case RT:  x1 = RT_X1;  x2 = RT_X2;  y1 = RT_Y1;  y2 = RT_Y2;  break;
        case RM:  x1 = RM_X1;  x2 = RM_X2;  y1 = RM_Y1;  y2 = RM_Y2;  break;
        case RB:  x1 = RB_X1;  x2 = RB_X2;  y1 = RB_Y1;  y2 = RB_Y2;  break;
        default: return;  // Invalid region
    }

     glcd_clear_here(x1, x2, y1, y2);  // Clear previous text
     glcd_puts(value, x1, y1);         // Print new value
}

// Generic function to convert any data type to a string
char* convertToString(void* data, DataType type) {
    switch (type) {
        case TYPE_INT:
            snprintf(globalBuffer, sizeof(globalBuffer), "%d", *(int*)data);
            break;
        case TYPE_FLOAT:
            snprintf(globalBuffer, sizeof(globalBuffer), "%.3f", *(float*)data);
            break;
        case TYPE_DOUBLE:
            snprintf(globalBuffer, sizeof(globalBuffer), "%.3lf", *(double*)data);
            break;
        case TYPE_CHAR:
            snprintf(globalBuffer, sizeof(globalBuffer), "%c", *(char*)data);
            break;
        case TYPE_STRING:
            snprintf(globalBuffer, sizeof(globalBuffer), "%s", (char*)data);
            break;
    }

    return globalBuffer;
}
