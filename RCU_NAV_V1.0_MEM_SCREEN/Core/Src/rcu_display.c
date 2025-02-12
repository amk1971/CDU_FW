/*
 * rcu_display.c
 *
 *  Created on: Nov 28, 2024
 *      Author: Hamza Javed
 */

#include <main.h>
#include "rcu_display.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "constant_defines.h"
#include "data_handler.h"
#include "glcd.h"
#include "glcd_font.h"

/*-----------------------------------------------------------------------------------------------
 **function code**
 ------------------------------------------------------------------------------------------------*/

extern float saved_freqs[NUM_FREQUENCIES];
extern uint8_t g_selectedPreset;

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
	RCC->APB2ENR |=
	RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN
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
	RESET_PORT->BSRR = (1 << RESET_PIN_NUMBER);         // Set reset pin high
	Delay(10);                                        // Wait for a short period
	RESET_PORT->BSRR = (1 << (RESET_PIN_NUMBER + 16));  // Set reset pin low
	Delay(10);                                        // Wait for a short period
	RESET_PORT->BSRR = (1 << RESET_PIN_NUMBER);         // Set reset pin high
	Delay(10);                                        // Wait for a short period
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
	if (nav_parameters.power_on == OFF)
	{
		glcd_off();
		HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_RESET);
	}
}
void LCD_Print_Home(void)
{
	char strA[10];
	char strS[10];
	char strV[8];

	static float prev_active_freq = 0;
	static float prev_standby_freq = 0;
	static uint8_t prev_volume = 0;

	memset(strA, 0, sizeof(strA));
	memset(strS, 0, sizeof(strS));
	memset(strV, 0, sizeof(strV));

	// Check if active frequency has changed
	if (nav_parameters.active_freq != prev_active_freq
			|| nav_parameters.power_on == ON || nav_parameters.mem_page == OFF)
	{
		glcd_clear_here(2, 60, 2, 2); // Clear specific area for active frequency
		snprintf(strA, sizeof(strA), "A%.2f", nav_parameters.active_freq);
		glcd_puts(strA, 2, 2);  // Update active frequency
		prev_active_freq = nav_parameters.active_freq;
	}

	// Check if standby frequency has changed
	if (nav_parameters.standby_freq != prev_standby_freq
			|| nav_parameters.power_on == ON || nav_parameters.mem_page == OFF)
	{
		if (nav_parameters.standby_freq == EMPTY_FREQ)

		{	glcd_clear_here(68, 125, 2, 2); // Clear specific area for standby frequency
			glcd_puts("NO MEM", 68, 2);
		}
		else
		{
			snprintf(strS, sizeof(strS), "S%.2f", nav_parameters.standby_freq); // coulmn first

			glcd_clear_here(68, 125, 2, 2); // Clear specific area for standby frequency
			glcd_puts(strS, 68, 2); // Update standby frequency
			prev_standby_freq = nav_parameters.standby_freq;
		}
	}
// Check if volume has changed
	if (nav_parameters.volume != prev_volume || nav_parameters.power_on == ON
			|| nav_parameters.mem_page == OFF)
	{
		glcd_clear_here(2, 50, 4, 4);  // Clear specific area for volume
		snprintf(strV, sizeof(strV), "Vol %02d", nav_parameters.volume);
		glcd_puts(strV, 2, 4);  // Update volume
		prev_volume = nav_parameters.volume;
	}

	glcd_puts("PRESET", 2, 6);
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
			snprintf(presetLabel, sizeof(presetLabel), "P%d", presetIndex + 1); // Presets are 1-based
		}

		else  // should not come here since preset should not exceed  15
		{
			snprintf(presetLabel, sizeof(presetLabel), "EMPTY");
			snprintf(strS, sizeof(strS), "no here");  // No frequency
		}

		if (presetIndex == g_selectedPreset)
		{
			glcd_clear_text(presetLabel, 90, 2 + (i * 2)); // Highlighting rectangle for selected preset
		}
		else
		{
			// Display the preset normally
			glcd_puts(presetLabel, 90, 2 + (i * 2));  // Normal text
		}
	}

	if (g_selectedPreset < NUM_FREQUENCIES)
	{
		char selectedFreq[9];
		glcd_clear_here(2, 60, 2, 2);

		if (saved_freqs[g_selectedPreset] == EMPTY_FREQ)
		{
			glcd_puts("EMPTY", 2, 2);
		}
		else
		{
			snprintf(selectedFreq, sizeof(selectedFreq), "%0.2f",
					saved_freqs[g_selectedPreset]);
			glcd_puts(selectedFreq, 2, 2);
		}
	}

	glcd_puts("DELETE", 2, 4);
	glcd_puts("SAVE", 2, 6);
}

char* convert_to_str(uint8_t value) {
    static char str[4];
    sprintf(str, "%u", value);
    return str;
}

void update_following(uint8_t UPDATE_FLAGS, char* sValue)
{
	switch (UPDATE_FLAGS)
	{
		case TF:
		glcd_clear_here(TF_X1,TF_X2,TF_Y1,TF_Y2);
		glcd_puts(sValue,TF_X1,TF_Y2);
		break;

		case SQ:
		glcd_clear_here(SQ_X1,SQ_X2,SQ_Y1,SQ_Y2);
		glcd_puts(sValue,SQ_X1,SQ_Y2);
		break;

		case PROG:
		glcd_clear_here(PROG_X1,PROG_X2,PROG_Y1,PROG_Y2);
		glcd_puts(sValue,PROG_X1,PROG_Y2);
		break;

		case T_GD_MV:
		glcd_clear_here(T_GD_MV_X1,T_GD_MV_X2,T_GD_MV_Y1,T_GD_MV_Y2);
		glcd_puts(sValue,T_GD_MV_X1,T_GD_MV_Y2);
		break;

		case FREQ_CH:
		glcd_clear_here(FREQ_CH_X1,FREQ_CH_X2,FREQ_CH_Y1,FREQ_CH_Y2);
		glcd_puts(sValue,FREQ_CH_X1,FREQ_CH_Y2);
		break;

		case TST:
		glcd_clear_here(TST_X1,TST_X2,TST_Y1,TST_Y2);
		glcd_puts(sValue,TST_X1,TST_Y2);
		break;

		case VOL:
		glcd_clear_here(VOL_X1,VOL_X2,VOL_Y1,VOL_Y2);
		glcd_puts(sValue,VOL_X1,VOL_Y2);
		break;

		case MN_BOTH:
		glcd_clear_here(MN_BOTH_X1,MN_BOTH_X2,MN_BOTH_Y1,MN_BOTH_Y2);
		glcd_puts(sValue,MN_BOTH_X1,MN_BOTH_Y2);
		break;
	}

}

void LCD_Print_Dummy(uint8_t UPDATE_FLAGS,uint8_t sValue)
{
		char* strValue = convert_to_str(sValue);
		update_following(UPDATE_FLAGS,strValue);
}
