/*
 * TFT_LCD.c
 *
 *  Created on: Oct 2, 2024
 *      Author: Hamza Javed
 */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "usart.h"
#include "keyboard.h"
#include "debug_console.h"
#include "data_handler.h"
#include "serial_1.h"
#include "timer.h"
#include "tacan_map.h"
#include "data_validation.h"

/* ------------------------------ MACRO DEGIN -------------------------------------*/
#define LCD_SCREEN_TEST	DISABLE 	// ENABLE

#if	LCD_SCREEN_TEST
#define TACAN_TEST
#endif

// TFT LCD UART Handler
#define TFT_UART	huart2

// Color Definitions
#define BLACK	0
#define WHITE	65535
#define RED		63488
#define GREEN	2024
#define BLUE	31
#define C_BLUE	23869
#define CUSTOM	53054

// Nextion TFT Commands
#define TEXT  			"txt"
#define	COLOR_CHG_TEXT	"pco"
#define COLOR_CHG_BG	"bco"
#define APH				"aph"

#define SHOW_POPUP		 127
#define	HIDE_POPUP		 0

/* Volume Limits */
#define VOLUME_MAX 	  	19
#define VOLUME_MIN		0

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
#define VOLUME	 	"Vol"

#define TOTAL_SCREENS  29

#define MAX_STANDBY_FREQ_LEN	10

#define INPUT_TIMEOUT_MS 30000 // 30 seconds in milliseconds

/* ------------------------------ MACRO END -------------------------------------*/

/* ------------------------------ Variables START -------------------------------------*/
/*
 * enum for LCD Screens
 */
typedef enum {
	HOME_SCREEN,
	NAV_P1_SCREEN,
	NAV_P2_SCREEN,
	NAV_P1_PROG_SCREEN,
	NAV_P2_PROG_SCREEN,
	ADF_P1_SCREEN,
	ADF_P2_SCREEN,
	ADF_P1_PROG_SCREEN,
	ADF_P2_PROG_SCREEN,
	TACAN_P1_CH_SCREEN,
	TACAN_P2_CH_SCREEN,
	TACAN_P1_CH_PROG_SCREEN,
	TACAN_P2_CH_PROG_SCREEN,
	TACAN_P1_FQ_SCREEN,
	TACAN_P2_FQ_SCREEN,
	TACAN_P1_FQ_PROG_SCREEN,
	TACAN_P2_FQ_PROG_SCREEN,
	HF_P1_SCREEN,
	HF_P2_SCREEN,
	HF_P1_PROG_SCREEN,
	HF_P2_PROG_SCREEN,
	VHF_P1_SCREEN,
	VHF_P2_SCREEN,
	VHF_P1_PROG_SCREEN,
	VHF_P2_PROG_SCREEN,
	UHF_P1_SCREEN,
	UHF_P2_SCREEN,
	UHF_P1_PROG_SCREEN,
	UHF_P2_PROG_SCREEN
}TFT_Screen;

// Array of page names corresponding to the screens
const char* screenNames[TOTAL_SCREENS] = {
    "Home",  				// HOME_SCREEN
    "NAV_P1",  				// NAV_P1_SCREEN
    "NAV_P2", 				// NAV_P2_SCREEN
    "NAV_P1_PROG",  		// NAV_PROG_SCREEN 1
	"NAV_P2_PROG",  		// NAV_PROG_SCREEN_2
    "ADF_P1",  				// ADF_P1_SCREEN
    "ADF_P2", 				// ADF_P2_SCREEN
    "ADF_P1_PROG",  		// ADF_PROG_SCREEN 1
	"ADF_P2_PROG",  		// ADF_PROG_SCREEN_2
    "TACAN_P1_CH",  		// TACAN_P1_CH_SCREEN
	"TACAN_P2_CH",  		// TACAN_P2_CH_SCREEN
    "TACAN_P1_CHPG",  		// TACAN_P1_CH_PROG_SCREEN 1
    "TACAN_P2_CHPG",  		// TACAN_P2_CH_PROG_SCREEN 2
    "TACAN_P1_FQ",  		// TACAN_P1_FQ_SCREEN
	"TACAN_P2_FQ",  		// TACAN_P2_FQ_SCREEN
    "TACAN_P1_FQPG",  		// TACAN_P1_FQ_PROG_SCREEN 1
    "TACAN_P2_FQPG",  		// TACAN_P2_FQ_PROG_SCREEN 2
    "HF_P1",  				// HF_P1_SCREEN
    "HF_P2",  				// HF_P2_SCREEN
	"HF_P1_PROG",			// HF_P1_PROG_SCREEN
	"HF_P2_PROG",			// HF_P2_PROG_SCREEN
    "VHF_P1",  				// VHF_P1_SCREEN
    "VHF_P2",  				// VHF_P2_SCREEN
	"VHF_P1_PROG",			// VHF_P1_PROG_SCREEN
	"VHF_P2_PROG",			// VHF_P2_PROG_SCREEN
    "UHF_P1",  				// VHF_P1_SCREEN
    "UHF_P2",  				// VHF_P2_SCREEN
	"UHF_P1_PROG",			// VHF_P1_PROG_SCREEN
	"UHF_P2_PROG"			// VHF_P2_PROG_SCREEN
};

TFT_Screen current_screen = HOME_SCREEN;	// Start on the home screen
typedef void (*ScreenHandler)(uint8_t key);

bool Input_mode = false;
bool invalid_entry = false;
char input[MAX_STANDBY_FREQ_LEN];
uint8_t input_index = 0;
uint8_t prefix_index = 0;
uint32_t lastInputTime = 0;
/* ------------------------------ Variables END -------------------------------------*/

/* ------------------------------ Function Declarations START -------------------------------------*/
void tft_lcd(uint8_t key);
void update_screen_(TFT_Screen screen);
void revert_screen_entry(TFT_Screen screen, uint8_t p_index);
void changeBackgroundToBlack(void);
void tft_lcd_Home();
void tft_lcd_change_screen(TFT_Screen screen);
void tft_lcd_send_command_float(const char* objectName, const char* command, float value, const char *identifier);
void tft_lcd_send_command_channel(const char* objectName, const char* command, char* value, const char * identifier);
void tft_lcd_send_command_int(const char* objectName, const char* command, uint32_t value);
void tft_lcd_send_command(const char* objectName, const char* command, const char* value);
void tft_lcd_send_command_text(const char* objectName, const char* command, const char* value);
#ifdef TACAN_TEST
void TACAN_SCREEN_TEST();
#endif


/* LCD Screen Handlers */
void home_screen_handler(uint8_t key);
void nav_p1_screen_handler(uint8_t key);
void nav_p2_screen_handler(uint8_t key);
void nav_p1_prog_screen_handler(uint8_t key);
void nav_p2_prog_screen_handler(uint8_t key);
void adf_p1_screen_handler(uint8_t key);
void adf_p2_screen_handler(uint8_t key);
void adf_p1_prog_screen_handler(uint8_t key);
void adf_p2_prog_screen_handler(uint8_t key);
void tacan_p1_ch_screen_handler(uint8_t key);
void tacan_p2_ch_screen_handler(uint8_t key);
void tacan_p1_ch_prog_screen_handler(uint8_t key);
void tacan_p2_ch_prog_screen_handler(uint8_t key);
void tacan_p1_fq_screen_handler(uint8_t key);
void tacan_p2_fq_screen_handler(uint8_t key);
void tacan_p1_fq_prog_screen_handler(uint8_t key);
void tacan_p2_fq_prog_screen_handler(uint8_t key);
void hf_p1_screen_handler(uint8_t key);
void hf_p2_screen_handler(uint8_t key);
void hf_p1_prog_screen_handler(uint8_t key);
void hf_p2_prog_screen_handler(uint8_t key);
void vhf_p1_screen_handler(uint8_t key);
void vhf_p2_screen_handler(uint8_t key);
void vhf_p1_prog_screen_handler(uint8_t key);
void vhf_p2_prog_screen_handler(uint8_t key);
void uhf_p1_screen_handler(uint8_t key);
void uhf_p2_screen_handler(uint8_t key);
void uhf_p1_prog_screen_handler(uint8_t key);
void uhf_p2_prog_screen_handler(uint8_t key);

ScreenHandler screen_handlers[] = {
		home_screen_handler,
		nav_p1_screen_handler,
		nav_p2_screen_handler,
		nav_p1_prog_screen_handler,
		nav_p2_prog_screen_handler,
		adf_p1_screen_handler,
		adf_p2_screen_handler,
		adf_p1_prog_screen_handler,
		adf_p2_prog_screen_handler,
		tacan_p1_ch_screen_handler,
		tacan_p2_ch_screen_handler,
		tacan_p1_ch_prog_screen_handler,
		tacan_p2_ch_prog_screen_handler,
		tacan_p1_fq_screen_handler,
		tacan_p2_fq_screen_handler,
		tacan_p1_fq_prog_screen_handler,
		tacan_p2_fq_prog_screen_handler,
		hf_p1_screen_handler,
		hf_p2_screen_handler,
		hf_p1_prog_screen_handler,
		hf_p2_prog_screen_handler,
		vhf_p1_screen_handler,
		vhf_p2_screen_handler,
		vhf_p1_prog_screen_handler,
		vhf_p2_prog_screen_handler,
		uhf_p1_screen_handler,
		uhf_p2_screen_handler,
		uhf_p1_prog_screen_handler,
		uhf_p2_prog_screen_handler,
};


void update_Counters(void)
{
	if(cdu_parameters.NAV_message_counter < 50000) cdu_parameters.NAV_message_counter += 50;			// Counts the time after the last nav health message
	if(cdu_parameters.TACAN_message_counter < 50000) cdu_parameters.TACAN_message_counter += 50;			// Counts the time after the last tacan health message
	if(cdu_parameters.HF_message_counter < 50000) cdu_parameters.HF_message_counter += 50;			// Counts the time after the last hf health message
	if(cdu_parameters.UHF_message_counter < 50000) cdu_parameters.UHF_message_counter += 50;			// Counts the time after the last uhf health message
	if(cdu_parameters.VHF_message_counter < 50000) cdu_parameters.VHF_message_counter += 50;			// Counts the time after the last vhf health message
	if(cdu_parameters.ADF_message_counter < 50000) cdu_parameters.ADF_message_counter += 50;
}
/* ------------------------------ Function Declarations END -------------------------------------*/
/*
 * Function: tft_lcd_thread
 * Arguments: void * pvParameters
 *
 * Description:
 * This function runs an infinite loop to continuously receive key presses
 * from the queue and process them for the TFT LCD. It initially sets up the
 * TFT LCD by changing the background and setting the home screen.
 * In each iteration, it waits for a key press from the queue and calls
 * the `tft_lcd` function to handle the received key. The task yields periodically
 * to allow other tasks to run.
 *
 * - pvParameters: Pointer to any parameters passed to the thread function.
 */

void tft_lcd_thread(void * pvParameters)
{
	char receivedKey = NDF;
	Load_CDU_Parameters(&cdu_parameters);
	cdu_parameters.NAV_message_counter = 5050;			// Counts the time after the last nav health message
	cdu_parameters.TACAN_message_counter = 5050;			// Counts the time after the last tacan health message
	cdu_parameters.HF_message_counter = 5050;			// Counts the time after the last hf health message
	cdu_parameters.UHF_message_counter = 5050;			// Counts the time after the last uhf health message
	cdu_parameters.VHF_message_counter = 5050;			// Counts the time after the last vhf health message
	cdu_parameters.ADF_message_counter = 5050;

//	changeBackgroundToBlack();
 	tft_lcd_Home(1);
 	//tft_lcd_change_screen(ADF_P1_SCREEN);
	//tft_lcd_send_command_text("t10", "txt", "TACAN Error");
		//tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);


	/* Infinite loop */
	for(;;)
	{
		if (xQueueReceive(xKeyQueue, &receivedKey, (TickType_t) 20) == pdTRUE)
		{
			if(xSemaphoreTake(xFlashMutex, portMAX_DELAY) == pdTRUE)
			{
#if DEBUG_CONSOLE
				debug_print("L_ Queue_Recv :: %c\n", receivedKey);
#endif
				if(receivedKey == DATA_1553)
				{
					update_screen_(current_screen);
				}
				else if(receivedKey == REVERT_IN)
				{
					revert_screen_entry(current_screen, prefix_index);
					Input_mode = false;
				}
				else
				{
					tft_lcd(receivedKey);
				}

				xSemaphoreGive(xFlashMutex);
			}
		}
		update_Counters();
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

/*
 * Function: tft_lcd_send_command
 * Arguments: const char* objectName
 * 			  const char* command
 * 			  const char* value
 *
 * Description:
 * 				This function is used to send data and commands to specified object on the
 * 				TFT display.
 *
 */
void tft_lcd_send_command(const char* objectName, const char* command, const char* value)
{
    char fullCommand[100];  // Buffer to store the complete command

    // Format the command: "<objectName>.<command>=<value>"
    snprintf(fullCommand, sizeof(fullCommand), "%s.%s=%s", objectName, command, value);
//    snprintf(fullCommand, sizeof(fullCommand), "%s.%s.%s=%s", screenNames[screen_idx], objectName, command, value);

    // Append the 0xFF terminator required by Nextion
    char endCommand[] = {0xFF, 0xFF, 0xFF};

    // Send the command via UART
    HAL_UART_Transmit(&TFT_UART, (uint8_t*)fullCommand, strlen(fullCommand), HAL_MAX_DELAY);
    HAL_UART_Transmit(&TFT_UART, (uint8_t*)endCommand, 3, HAL_MAX_DELAY);  // Send the end command

	vTaskDelay(pdMS_TO_TICKS(5));
}

/*
 * Function: tft_lcd_send_command_text
 * Arguments: const char* objectName
 * 			  const char* command
 * 			  const char* value
 *
 * Description:
 * 				This function is used to send data and commands to specified object on the
 * 				TFT display.
 *
 */
void tft_lcd_send_command_text(const char* objectName, const char* command, const char* value)
{
    char fullCommand[100];  // Buffer to store the complete command

    // Format the command: "<objectName>.<command>=<value>"
    snprintf(fullCommand, sizeof(fullCommand), "%s.%s=\"%s\"", objectName, command, value);
//    snprintf(fullCommand, sizeof(fullCommand), "%s.%s.%s=%s", screenNames[screen_idx], objectName, command, value);

    // Append the 0xFF terminator required by Nextion
    char endCommand[] = {0xFF, 0xFF, 0xFF};

    // Send the command via UART
    HAL_UART_Transmit(&TFT_UART, (uint8_t*)fullCommand, strlen(fullCommand), HAL_MAX_DELAY);
    HAL_UART_Transmit(&TFT_UART, (uint8_t*)endCommand, 3, HAL_MAX_DELAY);  // Send the end command

	vTaskDelay(pdMS_TO_TICKS(5));
}

/*
 * Function: tft_lcd_send_command_int
 * Arguments: const char* objectName
 * 			  const char* command
 * 			  uint32_t value
 *
 * Description:
 * 				This function is used to convert the decimal values to string for sending it
 * 				to the TFT display.
 *
 */
void tft_lcd_send_command_int(const char* objectName, const char* command, uint32_t value)
{
    char valueStr[20];
    snprintf(valueStr, sizeof(valueStr), "%ld", value);  // Convert integer value to string
    tft_lcd_send_command(objectName, command, valueStr);
}
/*
 * Function: tft_lcd_send_command_
 * Arguments: const char* objectName
 * 			  const char* command
 * 			  float value
 * 			  const char *identifier
 *
 * Description:
 * 				This function is used to convert the float values to string for sending it
 * 				to the TFT display.
 *
 */
void tft_lcd_send_command_(const char* objectName, const char* command, uint16_t value, const char *identifier)
{
    char valueStr[20];

    if (identifier == NULL || *identifier == '\0')  // Check if identifier is NULL or empty
    {
        snprintf(valueStr, sizeof(valueStr), "Vol %2d", value);  // Convert float value to string without identifier
    }
    else
    {
    	snprintf(valueStr, sizeof(valueStr), "%s %4d", identifier, value);  // Convert with identifier
    }

    tft_lcd_send_command_text(objectName, command, valueStr);  // Send formatted string
}

/*
 * Function: tft_lcd_send_command_float
 * Arguments: const char* objectName
 * 			  const char* command
 * 			  float value
 * 			  const char *identifier
 *
 * Description:
 * 				This function is used to convert the float values to string for sending it
 * 				to the TFT display.
 *
 */
void tft_lcd_send_command_float(const char* objectName, const char* command, float value, const char *identifier)
{
    char valueStr[20];

    if (identifier == NULL || *identifier == '\0')  // Check if identifier is NULL or empty
    {
        snprintf(valueStr, sizeof(valueStr), "%.3f", value);  // Convert float value to string without identifier
    }
    else
    {
    	if((current_screen == TACAN_P1_FQ_SCREEN) || (current_screen == TACAN_P1_FQ_PROG_SCREEN) || (current_screen == TACAN_P2_FQ_SCREEN) || (current_screen == TACAN_P2_FQ_PROG_SCREEN))
    	{
    		snprintf(valueStr, sizeof(valueStr), "%s %.2f", identifier, value);  // Convert with identifier
    	}
    	else if((current_screen == HF_P1_PROG_SCREEN) || (current_screen == HF_P1_SCREEN) || (current_screen == HF_P2_PROG_SCREEN) || (current_screen == HF_P2_SCREEN))
    	{
    		snprintf(valueStr, sizeof(valueStr), "%s %.4f", identifier, value);  // Convert with identifier
    	}
    	else
    	{
    		snprintf(valueStr, sizeof(valueStr), "%s %.3f", identifier, value);  // Convert with identifier
    	}
    }

    tft_lcd_send_command_text(objectName, command, valueStr);  // Send formatted string
}


/*
 * Function: tft_lcd_send_command_channel
 * Arguments: const char* objectName
 * 			  const char* command
 * 			  char* value
 * 			  const char *identifier
 *
 * Description:
 * 				This function is used to send the TACAN channel data to the
 * 				TFT display.
 *
 */
void tft_lcd_send_command_channel(const char* objectName, const char* command, char* value, const char * identifier)
{
    char valueStr[20];

    if (identifier == NULL || *identifier == '\0')  // Check if identifier is NULL or empty
    {
        snprintf(valueStr, sizeof(valueStr), "%s", value);  // Convert float value to string without identifier
    }
    else
    {
        snprintf(valueStr, sizeof(valueStr), "%s %s", identifier, value);  // Convert with identifier
    }

    tft_lcd_send_command_text(objectName, command, valueStr);  // Send formatted string
}

/*
 * Function: tft_lcd_change_screen
 * Arguments: TFT_Screen screen
 *
 * Description:
 * 				This function is used to change the TFT displays page
 *
 */
void tft_lcd_change_screen(TFT_Screen screen)
{
    char endCommand[] = {0xFF, 0xFF, 0xFF};  // End of command terminator
    char command[20];
    snprintf(command, sizeof(command), "page %s", screenNames[screen]);
    HAL_UART_Transmit(&TFT_UART, (uint8_t*)command, strlen(command), HAL_MAX_DELAY);
    HAL_UART_Transmit(&TFT_UART, (uint8_t*)endCommand, 3, HAL_MAX_DELAY);
    current_screen = screen;

	vTaskDelay(pdMS_TO_TICKS(10));
}

/*
 * Function: tft_lcd_Home
 * Arguments: void
 *
 * Description:
 * 				This function is used to display the home page on the TFT display.
 *
 */
void tft_lcd_Home(int mode)
{
//	tft_lcd_change_screen(NAV_P1_SCREEN);	//Set the screen to Home Screeen At the Initialization


	if (mode == 1) tft_lcd_change_screen(HOME_SCREEN);
	vTaskDelay(pdMS_TO_TICKS(50)); // Delay for 50 milliseconds
	//Check which units are enabled/ operational of all displayed on Home Screen
	if(cdu_parameters.NAV_message_counter > 5000)
	tft_lcd_send_command_int("t1", "pco",  RED); else
		tft_lcd_send_command_int("t1", "pco",  BLACK);
	vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
	if(cdu_parameters.ADF_message_counter > 5000)
	tft_lcd_send_command_int("t2", "pco",  RED); else
		tft_lcd_send_command_int("t2", "pco",  BLACK);
	vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
	if(cdu_parameters.TACAN_message_counter > 5000)
	tft_lcd_send_command_int("t3", "pco",  RED); else
		tft_lcd_send_command_int("t3", "pco",  BLACK);
	vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
	if(cdu_parameters.HF_message_counter > 5000)
		tft_lcd_send_command_int("t5", "pco",  RED); else
	tft_lcd_send_command_int("t5", "pco",  BLACK);
	vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
	if(cdu_parameters.VHF_message_counter > 5000)
		tft_lcd_send_command_int("t6", "pco",  RED); else
	tft_lcd_send_command_int("t6", "pco",  BLACK);
	vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
	if(cdu_parameters.UHF_message_counter > 5000)
		tft_lcd_send_command_int("t7", "pco",  RED); else
	tft_lcd_send_command_int("t7", "pco",  BLACK);
}

/* ------------------------ HOME SCREEN ---------------------------*/
/*
 * Function: home_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the HOME page according to the Keyboards input.
 *
 */
void home_screen_handler(uint8_t key)
{
	//current_screen = HOME_SCREEN;
	switch(key)
	{
		case LEFT_SW1:
			if(cdu_parameters.NAV_message_counter > 5000) {
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_text("t9", "txt", "NAV");
				tft_lcd_send_command_text("t10", "txt", "Error");
			} else {
				tft_lcd_change_screen(NAV_P1_SCREEN);
			}
			tft_lcd_send_command_float("t1","txt", cdu_parameters.standby_nav_freq, STANDBY);
			tft_lcd_send_command_float("t4", "txt", cdu_parameters.active_nav_freq, ACTIVE);
			tft_lcd_send_command_("t3", "txt", cdu_parameters.volume_nav, NULL);
			break;
		case LEFT_SW2:
			if(cdu_parameters.ADF_message_counter > 5000) {
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_text("t9", "txt", "ADF");
				tft_lcd_send_command_text("t10", "txt", "Error");
			} else {
				tft_lcd_change_screen(ADF_P1_SCREEN);
			}
			tft_lcd_send_command_("t1","txt", cdu_parameters.standby_adf_freq, STANDBY);
			tft_lcd_send_command_("t4", "txt", cdu_parameters.active_adf_freq, ACTIVE);
			tft_lcd_send_command_("t3", "txt", cdu_parameters.volume_adf, NULL);

			break;
		case LEFT_SW3:
			if(cdu_parameters.TACAN_message_counter > 5000) {
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_text("t9", "txt", "TACAN");
				tft_lcd_send_command_text("t10", "txt", "Error");
			} else {
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
			}
			tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
			tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
			tft_lcd_send_command_("t3", "txt", cdu_parameters.volume_tacan, NULL);
			break;
		case LEFT_SW4:
			break;
		case RIGHT_SW1:
			if(cdu_parameters.HF_message_counter > 5000) {
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_text("t9", "txt", "HF");
				tft_lcd_send_command_text("t10", "txt", "Error");
			} else {
				tft_lcd_change_screen(HF_P1_SCREEN);
			}
			tft_lcd_send_command_float("t1","txt", cdu_parameters.standby_hf_freq, STANDBY);
			tft_lcd_send_command_float("t4", "txt", cdu_parameters.active_hf_freq, ACTIVE);
			break;
		case RIGHT_SW2:
			if(cdu_parameters.VHF_message_counter > 5000) {
				tft_lcd_change_screen(VHF_P1_SCREEN);
				tft_lcd_send_command_text("t9", "txt", "VHF");
				tft_lcd_send_command_text("t10", "txt", "Error");
			} else {
				tft_lcd_change_screen(VHF_P1_SCREEN);
			}
			tft_lcd_send_command_float("t1","txt", cdu_parameters.standby_vhf_freq, STANDBY);
			tft_lcd_send_command_float("t4", "txt", cdu_parameters.active_vhf_freq, ACTIVE);
			break;
		case RIGHT_SW3:
			if(cdu_parameters.UHF_message_counter > 5000) {
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_text("t9", "txt", "UHF");
				tft_lcd_send_command_text("t10", "txt", "Error");
			} else {
				tft_lcd_change_screen(UHF_P1_SCREEN);
			}
			tft_lcd_send_command_float("t1","txt", cdu_parameters.standby_uhf_freq, STANDBY);
			tft_lcd_send_command_float("t4", "txt", cdu_parameters.active_uhf_freq, ACTIVE);
			break;
		case RIGHT_SW4:
			break;
		case HOME:
		case BACK:
		case OK:
		case SWAP:
		case PREV:
			break;
		case DIM:
			break;
		case BRT:
			break;
		default:
			break;
	}
}

/* ------------------------------------ NAV -------------------------------------- */
/*
 * Function: nav_p1_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the NAV Page 1 according to the Keyboards input.
 *
 */
void nav_p1_screen_handler(uint8_t key)
{
	//current_screen = NAV_P1_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
		tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(NAV,value);
			if(status == true)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_nav_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(NAV, A_FREQ, cdu_parameters.active_nav_freq);
					send_to_uart_queue_freq(NAV, S_FREQ, cdu_parameters.standby_nav_freq);
				}
			}
			else
			{
				// display pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_nav_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_nav_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_nav_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case NEXT:
				tft_lcd_change_screen(NAV_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(NAV);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(NAV, A_FREQ, cdu_parameters.active_nav_freq);
				send_to_uart_queue_freq(NAV, S_FREQ, cdu_parameters.standby_nav_freq);
				break;
			case ADD:
				if (cdu_parameters.volume_nav < (VOLUME_MAX+1))
				{
					cdu_parameters.volume_nav++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_nav > VOLUME_MIN)
				{
					cdu_parameters.volume_nav--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: nav_p2_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the NAV Page 2 according to the Keyboards input.
 *
 */
void nav_p2_screen_handler(uint8_t key)
{
	//current_screen = NAV_P2_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
		tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(NAV, value);
			if(status == true)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_nav_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(NAV, A_FREQ, cdu_parameters.active_nav_freq);
					send_to_uart_queue_freq(NAV, S_FREQ, cdu_parameters.standby_nav_freq);
				}
			}
			else
			{
				// show pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_nav_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_nav_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_nav_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_nav_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case BACK:
				tft_lcd_change_screen(NAV_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(NAV);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(NAV, A_FREQ, cdu_parameters.active_nav_freq);
				send_to_uart_queue_freq(NAV, S_FREQ, cdu_parameters.standby_nav_freq);
				break;
			case ADD:
				if (cdu_parameters.volume_nav < (VOLUME_MAX+1))
				{
					cdu_parameters.volume_nav++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_nav > VOLUME_MIN)
				{
					cdu_parameters.volume_nav--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: nav_p1_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the NAV Page 1 Program screen according to the Keyboards
 * 				 input.
 *
 */
void nav_p1_prog_screen_handler(uint8_t key)
{
	//current_screen = NAV_P1_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
		switch(prefix_index)
		{
			case '1':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);
				break;
			case '2':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_nav_freq, PRE_PRO_2);
				break;
			case '3':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_nav_freq, PRE_PRO_3);
				break;
			case '4':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_nav_freq, PRE_PRO_4);
				break;
		}
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(NAV, value);
			if(status == true)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					cdu_parameters.p1_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					cdu_parameters.p2_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p2_nav_freq, PRE_PRO_2);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					cdu_parameters.p3_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p3_nav_freq, PRE_PRO_3);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					cdu_parameters.p4_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p4_nav_freq, PRE_PRO_4);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_nav_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_nav_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(NAV_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_nav_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(NAV_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, NAV);
				update_flash();
				send_to_uart_queue_freq(NAV, A_FREQ, cdu_parameters.active_nav_freq);
				send_to_uart_queue_freq(NAV, S_FREQ, cdu_parameters.standby_nav_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_nav_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_nav_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_nav_freq, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(NAV_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: nav_p2_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the NAV Page 2 Program screen according to the Keyboards
 * 				 input.
 *
 */
void nav_p2_prog_screen_handler(uint8_t key)
{
	//current_screen = NAV_P2_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
		switch(prefix_index)
		{
			case '5':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_nav_freq, PRE_PRO_5);
				break;
			case '6':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_nav_freq, PRE_PRO_6);
				break;
			case '7':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_nav_freq, PRE_PRO_7);
				break;
			case '8':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_nav_freq, PRE_PRO_8);
				break;
		}
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(NAV, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					cdu_parameters.p5_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p5_nav_freq, PRE_PRO_5);

				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					cdu_parameters.p6_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p6_nav_freq, PRE_PRO_6);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					cdu_parameters.p7_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p7_nav_freq, PRE_PRO_7);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					cdu_parameters.p8_nav_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p8_nav_freq, PRE_PRO_8);

				}
				update_flash();
			}
			else
			{
				// show pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_nav_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_nav_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_nav_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(NAV_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_nav_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(NAV_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, NAV);
				update_flash();
				send_to_uart_queue_freq(NAV, A_FREQ, cdu_parameters.active_nav_freq);
				send_to_uart_queue_freq(NAV, S_FREQ, cdu_parameters.standby_nav_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				switch(prefix_index)
				{
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_nav_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_nav_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_nav_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_nav_freq, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(NAV_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				break;
			default:
				break;
		}
	}

}


/* ----------------------------------- ADF ------------------------------------ */
/*
 * Function: adf_p1_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the ADF Page 1 according to the Keyboards input.
 *
 */
void adf_p1_screen_handler(uint8_t key)
{
	//current_screen = ADF_P1_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
		tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
		tft_lcd_send_command_("t3",TEXT, cdu_parameters.volume_adf, NULL);
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			int value = 0;
			stop_timer();
			sscanf(input, "%s %d", prefix, &value);
//			if(strcmp(prefix, VOLUME) == 0)
//			{
//				bool v_status = input_volume_validation(value);
//				if(v_status)
//				{
//					cdu_parameters.volume_adf = value;
//					tft_lcd_send_command_("t3",TEXT, cdu_parameters.volume_adf, NULL);
//					update_flash();
//					// TODO: add the  Transmission functions
//				}
//				else
//				{
//					// show popup
//					tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
//					invalid_entry = true;
//				}
//			}
			if(strcmp(prefix,STANDBY) == 0) {
				bool status = input_data_validation_freq(ADF, ((float)value/1000));
				if(status)
				{
					cdu_parameters.standby_adf_freq = value;
					tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(ADF, A_FREQ, ((float)cdu_parameters.active_adf_freq/1000));
					send_to_uart_queue_freq(ADF, S_FREQ, ((float)cdu_parameters.standby_adf_freq/1000));
				}
				else
				{
					// show popup
					tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
					invalid_entry = true;
				}
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					input[input_index++] = key;
				}
				if(input[0] == 'S')
				{
					tft_lcd_send_command_text("t1",TEXT, input);
				}
//				else if(input[0] == 'V')
//				{
//					tft_lcd_send_command_text("t3",TEXT, input);
//				}
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
//				memset(input, 0, MAX_STANDBY_FREQ_LEN);
//				input_index = 0;
//				Input_mode = true;
//				input[input_index ++] = 'V';
//				input[input_index ++] = 'o';
//				input[input_index ++] = 'l';
//				input[input_index ++] = ' ';
//				tft_lcd_send_command_text("t3",TEXT, input);
//				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p1_adf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p2_adf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p3_adf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p4_adf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case NEXT:
				tft_lcd_change_screen(ADF_P2_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(ADF);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(ADF, A_FREQ, ((float)cdu_parameters.active_adf_freq/1000));
				send_to_uart_queue_freq(ADF, S_FREQ, ((float)cdu_parameters.standby_adf_freq/1000));
				break;
			case ADD:
				if (cdu_parameters.volume_adf < VOLUME_MAX)
				{
					cdu_parameters.volume_adf++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_adf > VOLUME_MIN)
				{
					cdu_parameters.volume_adf--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: adf_p2_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the ADF Page 2 according to the Keyboards input.
 *
 */
void adf_p2_screen_handler(uint8_t key)
{
	//current_screen = ADF_P2_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
		tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
		tft_lcd_send_command_("t3",TEXT, cdu_parameters.volume_adf, NULL);
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			int value = 0;
			stop_timer();
			sscanf(input, "%s %d", prefix, &value);

			if(strcmp(prefix,STANDBY) == 0) {
				bool status = input_data_validation_freq(ADF, ((float)value/1000));
				if(status)
				{
					cdu_parameters.standby_adf_freq = value;
					tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(ADF, A_FREQ, ((float)cdu_parameters.active_adf_freq/1000));
					send_to_uart_queue_freq(ADF, S_FREQ, ((float)cdu_parameters.standby_adf_freq/1000));
				}
				else
				{
					// show popup
					tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
					invalid_entry = true;
				}
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					input[input_index++] = key;
				}
				if(input[0] == 'S')
				{
					tft_lcd_send_command_text("t1",TEXT, input);
				}
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p5_adf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p6_adf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p7_adf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p8_adf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case BACK:
				tft_lcd_change_screen(ADF_P1_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(ADF);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(ADF, A_FREQ, ((float)cdu_parameters.active_adf_freq/1000));
				send_to_uart_queue_freq(ADF, S_FREQ, ((float)cdu_parameters.standby_adf_freq/1000));
				break;
			case ADD:
				if (cdu_parameters.volume_adf < VOLUME_MAX)
				{
					cdu_parameters.volume_adf++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_adf > VOLUME_MIN)
				{
					cdu_parameters.volume_adf--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: adf_p1_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the ADF Page 1 Program screen according to the Keyboards
 * 				 input.
 *
 */
void adf_p1_prog_screen_handler(uint8_t key)
{
	//current_screen = ADF_P1_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
		tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
		switch(prefix_index)
		{
			case '1':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p1_adf_freq, PRE_PRO_1);
				break;
			case '2':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p2_adf_freq, PRE_PRO_2);
				break;
			case '3':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p3_adf_freq, PRE_PRO_3);
				break;
			case '4':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p4_adf_freq, PRE_PRO_4);
				break;
		}
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			int value = 0;
			stop_timer();
			sscanf(input, "%s %d", prefix, &value);
			bool status = input_data_validation_freq(ADF, ((float)value/1000));
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					cdu_parameters.p1_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p1_adf_freq, PRE_PRO_1);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					cdu_parameters.p2_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p2_adf_freq, PRE_PRO_2);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					cdu_parameters.p3_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p3_adf_freq, PRE_PRO_3);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					cdu_parameters.p4_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p4_adf_freq, PRE_PRO_4);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					input[input_index++] = key;
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p1_adf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p2_adf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p3_adf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(ADF_P1_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p4_adf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(ADF_P1_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, ADF);
				update_flash();
				send_to_uart_queue_freq(ADF, A_FREQ, ((float)cdu_parameters.active_adf_freq/1000));
				send_to_uart_queue_freq(ADF, S_FREQ, ((float)cdu_parameters.standby_adf_freq/1000));
				tft_lcd_send_command_("t1", TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p1_adf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p2_adf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p3_adf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p4_adf_freq, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(ADF_P1_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: adf_p2_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the ADF Page 2 Program screen according to the Keyboards
 * 				 input.
 *
 */
void adf_p2_prog_screen_handler(uint8_t key)
{
	//current_screen = ADF_P2_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
		tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
		switch(prefix_index)
		{
			case '5':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p5_adf_freq, PRE_PRO_5);
				break;
			case '6':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p6_adf_freq, PRE_PRO_6);
				break;
			case '7':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p7_adf_freq, PRE_PRO_7);
				break;
			case '8':
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p8_adf_freq, PRE_PRO_8);
				break;
		}
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			int value = 0;
			stop_timer();
			sscanf(input, "%s %d", prefix, &value);
			bool status = input_data_validation_freq(ADF, ((float)value/1000));
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					cdu_parameters.p5_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p5_adf_freq, PRE_PRO_5);

				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					cdu_parameters.p6_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p6_adf_freq, PRE_PRO_6);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					cdu_parameters.p7_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p7_adf_freq, PRE_PRO_7);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					cdu_parameters.p8_adf_freq = value;
					tft_lcd_send_command_("t10",TEXT, cdu_parameters.p8_adf_freq, PRE_PRO_8);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					input[input_index++] = key;
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p5_adf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p6_adf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p7_adf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(ADF_P2_PROG_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t10", TEXT, cdu_parameters.p8_adf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(ADF_P2_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, ADF);
				update_flash();
				send_to_uart_queue_freq(ADF, A_FREQ, ((float)cdu_parameters.active_adf_freq/1000));
				send_to_uart_queue_freq(ADF, S_FREQ, ((float)cdu_parameters.standby_adf_freq/1000));
				tft_lcd_send_command_("t1", TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				switch(prefix_index)
				{
					case '5':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p5_adf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p6_adf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p7_adf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p8_adf_freq, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(ADF_P1_SCREEN);
				tft_lcd_send_command_("t1",TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;
			default:
				break;
		}
	}

}

/* --------------------------------- TACAN ---------------------------------- */
/*
 * Function: tacan_p1_ch_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN Page 1 screen according to the Keyboards
 * 				 input.
 *
 */
void tacan_p1_ch_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P1_CH_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
		tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
		tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			char value[TACAN_S];
			stop_timer();
			memset(value, 0, TACAN_S);
			sscanf(input, "%s %s", prefix, value);
			bool status = input_data_validation_channel(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					memset(cdu_parameters.standby_channel, 0, TACAN_S);
					memcpy(cdu_parameters.standby_channel, value, strlen(value));
					tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
					update_corresponding_frequency(STANDBY, cdu_parameters.standby_channel, &cdu_parameters);
					update_flash();
					send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.active_channel);
					send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.standby_channel);
				}
			}
			else
			{
				// display Invalid pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'X') || (key == 'Y'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					input[input_index++] = key;
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				tft_lcd_change_screen(TACAN_P1_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p1_channel, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p2_channel, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p3_channel, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p4_channel, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case NEXT:
				tft_lcd_change_screen(TACAN_P2_CH_SCREEN);
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_channel();
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				update_corresponding_frequency(STANDBY, cdu_parameters.standby_channel, &cdu_parameters);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				update_corresponding_frequency(ACTIVE, cdu_parameters.active_channel, &cdu_parameters);
				update_flash();
				send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.active_channel);
				send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.standby_channel);
				break;
			case ADD:
				if (cdu_parameters.volume_tacan < VOLUME_MAX)
				{
					cdu_parameters.volume_tacan++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_tacan > VOLUME_MIN)
				{
					cdu_parameters.volume_tacan--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: tacan_p2_ch_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN Page 2 screen according to the Keyboards
 * 				 input.
 *
 */
void tacan_p2_ch_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P2_CH_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
		tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
		tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			char value[TACAN_S];
			stop_timer();
			memset(value, 0, TACAN_S);
			sscanf(input, "%s %s", prefix, value);
			bool status = input_data_validation_channel(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					memset(cdu_parameters.standby_channel, 0, TACAN_S);
					memcpy(cdu_parameters.standby_channel, value, strlen(value));
					tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
					update_corresponding_frequency(STANDBY, cdu_parameters.standby_channel, &cdu_parameters);
					update_flash();
					send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.active_channel);
					send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.standby_channel);
				}
			}
			else
			{
				// show pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'X') || (key == 'Y'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					input[input_index++] = key;
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				tft_lcd_change_screen(TACAN_P1_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p5_channel, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p6_channel, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p7_channel, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p8_channel, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case BACK:
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_channel();
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				update_corresponding_frequency(STANDBY, cdu_parameters.standby_channel, &cdu_parameters);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				update_corresponding_frequency(ACTIVE, cdu_parameters.active_channel, &cdu_parameters);
				update_flash();
				send_to_uart_queue_channel(TACAN, A_FREQ, cdu_parameters.active_channel);
				send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.standby_channel);
				break;
			case ADD:
				if (cdu_parameters.volume_tacan < VOLUME_MAX)
				{
					cdu_parameters.volume_tacan++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_tacan > VOLUME_MIN)
				{
					cdu_parameters.volume_tacan--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: tacan_p1_ch_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN program Page screen according to the Keyboards
 * 				 input.
 *
 */
void tacan_p1_ch_prog_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P1_CH_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
		tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
		switch(prefix_index)
		{
			case '1':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p1_channel, PRE_PRO_1);
				break;
			case '2':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p2_channel, PRE_PRO_2);
				break;
			case '3':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p3_channel, PRE_PRO_3);
				break;
			case '4':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p4_channel, PRE_PRO_4);
				break;
		}
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			char value[TACAN_S];
			stop_timer();
			memset(value, 0, TACAN_S);
			sscanf(input, "%s %s", prefix, value);
			bool status = input_data_validation_channel(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					memset(cdu_parameters.p1_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p1_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p1_channel, PRE_PRO_1);
					update_corresponding_frequency(PRE_PRO_1, cdu_parameters.p1_channel, &cdu_parameters);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					memset(cdu_parameters.p2_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p2_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p2_channel, PRE_PRO_2);
					update_corresponding_frequency(PRE_PRO_2, cdu_parameters.p2_channel, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					memset(cdu_parameters.p3_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p3_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p3_channel, PRE_PRO_3);
					update_corresponding_frequency(PRE_PRO_3, cdu_parameters.p3_channel, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					memset(cdu_parameters.p4_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p4_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p4_channel, PRE_PRO_4);
					update_corresponding_frequency(PRE_PRO_4, cdu_parameters.p4_channel, &cdu_parameters);

				}
				update_flash();
			}
			else
			{
				// show pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'X') || (key == 'Y'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
						input[input_index++] = key;
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p1_channel, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p2_channel, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p3_channel, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P1_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p4_channel, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				prefix_index = 0;
				break;
			case OK:
				copy_preset_to_standby_channel(prefix_index);
				update_flash();
				send_to_uart_queue_channel(TACAN, A_FREQ, cdu_parameters.active_channel);
				send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.standby_channel);
				update_corresponding_frequency(STANDBY, cdu_parameters.standby_channel, &cdu_parameters);
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p1_channel, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p2_channel, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p3_channel, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p4_channel, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: tacan_p2_ch_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN program Page screen according to the Keyboards
 * 				 input.
 *
 */
void tacan_p2_ch_prog_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P2_CH_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
		tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
		switch(prefix_index)
		{
			case '1':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p5_channel, PRE_PRO_5);
				break;
			case '2':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p6_channel, PRE_PRO_6);
				break;
			case '3':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p7_channel, PRE_PRO_7);
				break;
			case '4':
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p8_channel, PRE_PRO_8);
				break;
		}
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			char value[TACAN_S];
			stop_timer();
			memset(value, 0, TACAN_S);
			sscanf(input, "%s %s", prefix, value);
			bool status = input_data_validation_channel(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					memset(cdu_parameters.p5_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p5_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p5_channel, PRE_PRO_5);
					update_corresponding_frequency(PRE_PRO_5, cdu_parameters.p5_channel, &cdu_parameters);
				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					memset(cdu_parameters.p6_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p6_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p6_channel, PRE_PRO_6);
					update_corresponding_frequency(PRE_PRO_6, cdu_parameters.p6_channel, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					memset(cdu_parameters.p7_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p7_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p7_channel, PRE_PRO_7);
					update_corresponding_frequency(PRE_PRO_7, cdu_parameters.p7_channel, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					memset(cdu_parameters.p8_channel, 0, TACAN_S);
					memcpy(cdu_parameters.p8_channel, value, strlen(value));
					tft_lcd_send_command_channel("t10",TEXT, cdu_parameters.p8_channel, PRE_PRO_8);
					update_corresponding_frequency(PRE_PRO_8, cdu_parameters.p8_channel, &cdu_parameters);

				}
				update_flash();
			}
			else
			{
				// show pop up
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'X') || (key == 'Y'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
						input[input_index++] = key;
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p5_channel, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p6_channel, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p7_channel, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P2_CH_PROG_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p8_channel, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(TACAN_P2_CH_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				prefix_index = 0;
				break;
			case OK:
				copy_preset_to_standby_channel(prefix_index);
				update_flash();
				send_to_uart_queue_channel(TACAN, A_FREQ, cdu_parameters.active_channel);
				send_to_uart_queue_channel(TACAN, S_FREQ, cdu_parameters.standby_channel);
				update_corresponding_frequency(STANDBY, cdu_parameters.standby_channel, &cdu_parameters);
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p5_channel, PRE_PRO_5);
						break;
					case '2':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p6_channel, PRE_PRO_6);
						break;
					case '3':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p7_channel, PRE_PRO_7);
						break;
					case '4':
						tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p8_channel, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
				tft_lcd_send_command_channel("t1",TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			default:
				break;
		}
	}
}

/* -------------------------------------- TACAN FREQ ------------------------------ */
/*
 * Function: tacan_p1_fq_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN Frequency Page 1 according to the Keyboards input.
 *
 */
void tacan_p1_fq_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P1_FQ_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
		tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_tacan_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
					update_corresponding_channel(STANDBY, cdu_parameters.standby_tacan_freq, &cdu_parameters);
					update_flash();
					send_to_uart_queue_freq(TACAN, A_FREQ, cdu_parameters.active_tacan_freq);
					send_to_uart_queue_freq(TACAN, S_FREQ, cdu_parameters.standby_tacan_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_tacan_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_tacan_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_tacan_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_tacan_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case NEXT:
				tft_lcd_change_screen(TACAN_P2_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(TACAN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				update_corresponding_channel(STANDBY, cdu_parameters.standby_tacan_freq, &cdu_parameters);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				update_corresponding_channel(ACTIVE, cdu_parameters.active_tacan_freq, &cdu_parameters);
				update_flash();
				send_to_uart_queue_freq(TACAN, A_FREQ, cdu_parameters.active_tacan_freq);
				send_to_uart_queue_freq(TACAN, S_FREQ, cdu_parameters.standby_tacan_freq);
				break;
			case ADD:
				if (cdu_parameters.volume_tacan < VOLUME_MAX)
				{
					cdu_parameters.volume_tacan++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_tacan > VOLUME_MIN)
				{
					cdu_parameters.volume_tacan--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: tacan_p2_fq_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN Frequency Page 2 according to the Keyboards input.
 *
 */
void tacan_p2_fq_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P2_FQ_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
		tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_tacan_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
					update_corresponding_channel(STANDBY, cdu_parameters.standby_tacan_freq, &cdu_parameters);
					update_flash();
					send_to_uart_queue_freq(TACAN, A_FREQ, cdu_parameters.active_tacan_freq);
					send_to_uart_queue_freq(TACAN, S_FREQ, cdu_parameters.standby_tacan_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_tacan_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_tacan_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_tacan_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_tacan_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case BACK:
				tft_lcd_change_screen(TACAN_P1_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(TACAN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				update_corresponding_channel(STANDBY, cdu_parameters.standby_tacan_freq, &cdu_parameters);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				update_corresponding_channel(ACTIVE, cdu_parameters.active_tacan_freq, &cdu_parameters);
				update_flash();
				send_to_uart_queue_freq(TACAN, A_FREQ, cdu_parameters.active_tacan_freq);
				send_to_uart_queue_freq(TACAN, S_FREQ, cdu_parameters.standby_tacan_freq);
				break;
			case ADD:
				if (cdu_parameters.volume_tacan < VOLUME_MAX)
				{
					cdu_parameters.volume_tacan++;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			case SUB:
				if (cdu_parameters.volume_tacan > VOLUME_MIN)
				{
					cdu_parameters.volume_tacan--;
					tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
					update_flash();
					/* TODO :  Send the volume message */
				}
				break;
			default:
				break;
		}
	}
}

/*
 * Function: tacan_p1_fq_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN Frequency Page 1 Program screen according to the Keyboards
 * 				 input.
 *
 */
void tacan_p1_fq_prog_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P1_FQ_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
		switch(prefix_index)
		{
			case '1':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_tacan_freq, PRE_PRO_1);
				break;
			case '2':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_tacan_freq, PRE_PRO_2);
				break;
			case '3':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_tacan_freq, PRE_PRO_3);
				break;
			case '4':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_tacan_freq, PRE_PRO_4);
				break;
		}
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					cdu_parameters.p1_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p1_tacan_freq, PRE_PRO_1);
					update_corresponding_channel(PRE_PRO_1, cdu_parameters.p1_tacan_freq, &cdu_parameters);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					cdu_parameters.p2_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p2_tacan_freq, PRE_PRO_2);
					update_corresponding_channel(PRE_PRO_2, cdu_parameters.p2_tacan_freq, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					cdu_parameters.p3_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p3_tacan_freq, PRE_PRO_3);
					update_corresponding_channel(PRE_PRO_3, cdu_parameters.p3_tacan_freq, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					cdu_parameters.p4_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p4_tacan_freq, PRE_PRO_4);
					update_corresponding_channel(PRE_PRO_4, cdu_parameters.p4_tacan_freq, &cdu_parameters);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_tacan_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_tacan_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_tacan_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P1_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_tacan_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(TACAN_P1_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, TACAN);
				update_flash();
				update_corresponding_channel(STANDBY, cdu_parameters.standby_tacan_freq, &cdu_parameters);
				send_to_uart_queue_freq(TACAN, A_FREQ, cdu_parameters.active_tacan_freq);
				send_to_uart_queue_freq(TACAN, S_FREQ, cdu_parameters.standby_tacan_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_tacan_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_tacan_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_tacan_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_tacan_freq, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(TACAN_P1_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: tacan_p2_fq_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the TACAN Frequency Page 2 Program screen according to the Keyboards
 * 				 input.
 *
 */
void tacan_p2_fq_prog_screen_handler(uint8_t key)
{
	//current_screen = TACAN_P2_FQ_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
		switch(prefix_index)
		{
			case '5':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_tacan_freq, PRE_PRO_5);
				break;
			case '6':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_tacan_freq, PRE_PRO_6);
				break;
			case '7':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_tacan_freq, PRE_PRO_7);
				break;
			case '8':
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_tacan_freq, PRE_PRO_8);
				break;
		}
		invalid_entry = false;
		return;
	}
	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(TACAN, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					cdu_parameters.p5_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p5_tacan_freq, PRE_PRO_5);
					update_corresponding_channel(PRE_PRO_5, cdu_parameters.p5_tacan_freq, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					cdu_parameters.p6_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p6_tacan_freq, PRE_PRO_6);
					update_corresponding_channel(PRE_PRO_6, cdu_parameters.p6_tacan_freq, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					cdu_parameters.p7_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p7_tacan_freq, PRE_PRO_7);
					update_corresponding_channel(PRE_PRO_7, cdu_parameters.p7_tacan_freq, &cdu_parameters);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					cdu_parameters.p8_tacan_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p8_tacan_freq, PRE_PRO_8);
					update_corresponding_channel(PRE_PRO_8, cdu_parameters.p8_tacan_freq, &cdu_parameters);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_tacan_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_tacan_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_tacan_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(TACAN_P2_FQ_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_tacan_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(TACAN_P2_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, TACAN);
				update_flash();
				update_corresponding_channel(STANDBY, cdu_parameters.standby_tacan_freq, &cdu_parameters);
				send_to_uart_queue_freq(TACAN, A_FREQ, cdu_parameters.active_tacan_freq);
				send_to_uart_queue_freq(TACAN, S_FREQ, cdu_parameters.standby_tacan_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				switch(prefix_index)
				{
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_tacan_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_tacan_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_tacan_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_tacan_freq, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(TACAN_P1_FQ_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;
			default:
				break;
		}
	}

}

/* ---------------------------------------- HF ------------------------------------ */
/*
 * Function: hf_p1_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the HF Page 1 screen according to the Keyboards
 * 				 input.
 *
 */
void hf_p1_screen_handler(uint8_t key)
{
	//current_screen = HF_P1_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(HF, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_hf_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(HF, A_FREQ, cdu_parameters.active_hf_freq);
					send_to_uart_queue_freq(HF, S_FREQ, cdu_parameters.standby_hf_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_hf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_hf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_hf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_hf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case NEXT:
				tft_lcd_change_screen(HF_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(HF);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(HF, A_FREQ, cdu_parameters.active_hf_freq);
				send_to_uart_queue_freq(HF, S_FREQ, cdu_parameters.standby_hf_freq);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: hf_p2_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the HF Page 2 screen according to the Keyboards
 * 				 input.
 *
 */
void hf_p2_screen_handler(uint8_t key)
{
	//current_screen = HF_P2_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(HF, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_hf_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(HF, A_FREQ, cdu_parameters.active_hf_freq);
					send_to_uart_queue_freq(HF, S_FREQ, cdu_parameters.standby_hf_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_hf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_hf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_hf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_hf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case BACK:
				tft_lcd_change_screen(HF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(HF);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(HF, A_FREQ, cdu_parameters.active_hf_freq);
				send_to_uart_queue_freq(HF, S_FREQ, cdu_parameters.standby_hf_freq);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: hf_p1_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the HF Page 1 Program screen according to the Keyboards
 * 				 input.
 *
 */
void hf_p1_prog_screen_handler(uint8_t key)
{
	//current_screen = HF_P1_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(HF, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					cdu_parameters.p1_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p1_hf_freq, PRE_PRO_1);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					cdu_parameters.p2_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p2_hf_freq, PRE_PRO_2);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					cdu_parameters.p3_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p3_hf_freq, PRE_PRO_3);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					cdu_parameters.p4_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p4_hf_freq, PRE_PRO_4);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_hf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_hf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_hf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(HF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_hf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(HF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, HF);
				update_flash();
				send_to_uart_queue_freq(HF, A_FREQ, cdu_parameters.active_hf_freq);
				send_to_uart_queue_freq(HF, S_FREQ, cdu_parameters.standby_hf_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_hf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_hf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_hf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_hf_freq, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(HF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: hf_p2_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the HF Page 2 Program screen according to the Keyboards
 * 				 input.
 *
 */
void hf_p2_prog_screen_handler(uint8_t key)
{
	//current_screen = HF_P2_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(HF, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					cdu_parameters.p5_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p5_hf_freq, PRE_PRO_5);

				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					cdu_parameters.p6_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p6_hf_freq, PRE_PRO_6);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					cdu_parameters.p7_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p7_hf_freq, PRE_PRO_7);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					cdu_parameters.p8_hf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p8_hf_freq, PRE_PRO_8);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_hf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_hf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_hf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(HF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_hf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(HF_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, HF);
				update_flash();
				send_to_uart_queue_freq(HF, A_FREQ, cdu_parameters.active_hf_freq);
				send_to_uart_queue_freq(HF, S_FREQ, cdu_parameters.standby_hf_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				switch(prefix_index)
				{
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_hf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_hf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_hf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_hf_freq, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(HF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;
			default:
				break;
		}
	}

}

/* --------------------------------------- VHF ------------------------------------ */
/*
 * Function: vhf_p1_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the VHF Page 1 screen according to the Keyboards
 * 				 input.
 *
 */
void vhf_p1_screen_handler(uint8_t key)
{
	//current_screen = VHF_P1_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(VHF, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_vhf_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_vhf_freq);
					send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_vhf_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_vhf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_vhf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_vhf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_vhf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case NEXT:
				tft_lcd_change_screen(VHF_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(VHF);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_vhf_freq);
				send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_vhf_freq);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: vhf_p2_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the VHF Page 2 screen according to the Keyboards
 * 				 input.
 *
 */
void vhf_p2_screen_handler(uint8_t key)
{
	//current_screen = VHF_P2_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(VHF, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_vhf_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_vhf_freq);
					send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_vhf_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_vhf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_vhf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_vhf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_vhf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case BACK:
				tft_lcd_change_screen(VHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(VHF);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_vhf_freq);
				send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_vhf_freq);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: vhf_p1_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the VHF Page 1 Program screen according to the Keyboards
 * 				 input.
 *
 */
void vhf_p1_prog_screen_handler(uint8_t key)
{
	//current_screen = VHF_P1_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(VHF, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					cdu_parameters.p1_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p1_vhf_freq, PRE_PRO_1);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					cdu_parameters.p2_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p2_vhf_freq, PRE_PRO_2);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					cdu_parameters.p3_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p3_vhf_freq, PRE_PRO_3);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					cdu_parameters.p4_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p4_vhf_freq, PRE_PRO_4);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_vhf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_vhf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_vhf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(VHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_vhf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(VHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, VHF);
				update_flash();
				send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_vhf_freq);
				send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_vhf_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_vhf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_vhf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_vhf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_vhf_freq, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(VHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: vhf_p2_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the VHF Page 2 Program screen according to the Keyboards
 * 				 input.
 *
 */
void vhf_p2_prog_screen_handler(uint8_t key)
{
	//current_screen = VHF_P2_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(VHF, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					cdu_parameters.p5_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p5_vhf_freq, PRE_PRO_5);

				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					cdu_parameters.p6_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p6_vhf_freq, PRE_PRO_6);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					cdu_parameters.p7_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p7_vhf_freq, PRE_PRO_7);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					cdu_parameters.p8_vhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p8_vhf_freq, PRE_PRO_8);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_vhf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_vhf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_vhf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(VHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_vhf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(VHF_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, VHF);
				update_flash();
				send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_vhf_freq);
				send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_vhf_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				switch(prefix_index)
				{
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_vhf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_vhf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_vhf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_vhf_freq, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(VHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				break;
			default:
				break;
		}
	}

}

/* --------------------------------------------- UHF ------------------------------------------------- */
/*
 * Function: uhf_p1_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the UHF Page 1 screen according to the Keyboards
 * 				 input.
 *
 */
void uhf_p1_screen_handler(uint8_t key)
{
	//current_screen = UHF_P1_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(UHF, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_uhf_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_uhf_freq);
					send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_uhf_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_uhf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case NEXT:
				tft_lcd_change_screen(UHF_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(UHF);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_uhf_freq);
				send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_uhf_freq);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: uhf_p2_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the UHF Page 2 screen according to the Keyboards
 * 				 input.
 *
 */
void uhf_p2_screen_handler(uint8_t key)
{
	//current_screen = UHF_P2_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 2)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t1",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(UHF, value);
			if(status)
			{
				if(strcmp(prefix,STANDBY) == 0) {
					cdu_parameters.standby_uhf_freq = value;
					tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
					update_flash();
					send_to_uart_queue_freq(VHF, A_FREQ, cdu_parameters.active_uhf_freq);
					send_to_uart_queue_freq(VHF, S_FREQ, cdu_parameters.standby_uhf_freq);
				}
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t1",TEXT, input);
			}
		}

	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index ++] = 'S';
				input[input_index ++] = ' ';
				tft_lcd_send_command_text("t1",TEXT, input);
				start_timer();
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				break;
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_uhf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_uhf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_uhf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_uhf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case BACK:
				tft_lcd_change_screen(UHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case SWAP:
				swap_active_standby_freq(UHF);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				update_flash();
				send_to_uart_queue_freq(UHF, A_FREQ, cdu_parameters.active_uhf_freq);
				send_to_uart_queue_freq(UHF, S_FREQ, cdu_parameters.standby_uhf_freq);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: uhf_p1_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the UHF Page 1 Program screen according to the Keyboards
 * 				 input.
 *
 */
void uhf_p1_prog_screen_handler(uint8_t key)
{
	//current_screen = UHF_P1_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(UHF, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_1) == 0) {
					cdu_parameters.p1_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p1_uhf_freq, PRE_PRO_1);
				}
				if(strcmp(prefix,PRE_PRO_2) == 0) {
					cdu_parameters.p2_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_2);

				}
				if(strcmp(prefix,PRE_PRO_3) == 0) {
					cdu_parameters.p3_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p3_uhf_freq, PRE_PRO_3);

				}
				if(strcmp(prefix,PRE_PRO_4) == 0) {
					cdu_parameters.p4_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p4_uhf_freq, PRE_PRO_4);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_uhf_freq, PRE_PRO_1);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '1';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_2);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '2';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_uhf_freq, PRE_PRO_3);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '3';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(UHF_P1_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_uhf_freq, PRE_PRO_4);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '4';
				break;
			case HOME:
				tft_lcd_Home(1);
				prefix_index = 0;
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(UHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, UHF);
				update_flash();
				send_to_uart_queue_freq(UHF, A_FREQ, cdu_parameters.active_uhf_freq);
				send_to_uart_queue_freq(UHF, S_FREQ, cdu_parameters.standby_uhf_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				switch(prefix_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_uhf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_uhf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_uhf_freq, PRE_PRO_4);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(UHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;
			default:
				break;
		}
	}
}

/*
 * Function: uhf_p2_prog_screen_handler
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is used to navigate on the UHF Page 2 Program screen according to the Keyboards
 * 				 input.
 *
 */
void uhf_p2_prog_screen_handler(uint8_t key)
{
	//current_screen = UHF_P2_PROG_SCREEN;
	if(invalid_entry)
	{
		tft_lcd_send_command_int("t11", APH, HIDE_POPUP);
		tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
		tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
		invalid_entry = false;
		return;
	}

	if(Input_mode)
	{
		if(key == BACK)
		{
			if(input_index > 3)
			{
				input_index--;
				input[input_index] = '\0';
			}
			tft_lcd_send_command_text("t10",TEXT, input);
		}
		else if(key == OK)
		{
			char prefix[3];
			float value = 0.0;
			stop_timer();
			sscanf(input, "%s %f", prefix, &value);
			bool status = input_data_validation_freq(UHF, value);
			if(status)
			{
				if(strcmp(prefix,PRE_PRO_5) == 0) {
					cdu_parameters.p5_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p5_uhf_freq, PRE_PRO_5);

				}
				if(strcmp(prefix,PRE_PRO_6) == 0) {
					cdu_parameters.p6_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p6_uhf_freq, PRE_PRO_6);

				}
				if(strcmp(prefix,PRE_PRO_7) == 0) {
					cdu_parameters.p7_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p7_uhf_freq, PRE_PRO_7);

				}
				if(strcmp(prefix,PRE_PRO_8) == 0) {
					cdu_parameters.p8_uhf_freq = value;
					tft_lcd_send_command_float("t10",TEXT, cdu_parameters.p8_uhf_freq, PRE_PRO_8);

				}
				update_flash();
			}
			else
			{
				// show popup
				tft_lcd_send_command_int("t11", APH, SHOW_POPUP);
				invalid_entry = true;
			}
			Input_mode = false;
		}
		else
		{
			if((key >= '0' && key <= '9') || (key == 'p'))
			{
				if(input_index < MAX_STANDBY_FREQ_LEN)
				{
					if(key == 'p') {
						input[input_index++] = '.';
					}
					else {
						input[input_index++] = key;
					}
				}
				tft_lcd_send_command_text("t10",TEXT, input);
			}
		}
	}
	else
	{
		switch(key)
		{
			case LEFT_SW1:
				break;
			case LEFT_SW2:
				break;
			case LEFT_SW3:
				break;
			case LEFT_SW4:
				memset(input, 0, MAX_STANDBY_FREQ_LEN);
				input_index = 0;
				Input_mode = true;
				input[input_index++] = 'P';
				input[input_index++] =  prefix_index;
				input[input_index++] = ' ';
				tft_lcd_send_command_text("t10",TEXT, input);
				start_timer();
				break;
			case RIGHT_SW1:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_uhf_freq, PRE_PRO_5);
				tft_lcd_send_command_int("t5", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t5", COLOR_CHG_TEXT, WHITE);
				prefix_index = '5';
				break;
			case RIGHT_SW2:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_uhf_freq, PRE_PRO_6);
				tft_lcd_send_command_int("t6", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t6", COLOR_CHG_TEXT, WHITE);
				prefix_index = '6';
				break;
			case RIGHT_SW3:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_uhf_freq, PRE_PRO_7);
				tft_lcd_send_command_int("t7", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t7", COLOR_CHG_TEXT, WHITE);
				prefix_index = '7';
				break;
			case RIGHT_SW4:
				tft_lcd_change_screen(UHF_P2_PROG_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_uhf_freq, PRE_PRO_8);
				tft_lcd_send_command_int("t8", COLOR_CHG_BG, BLACK);
				tft_lcd_send_command_int("t8", COLOR_CHG_TEXT, WHITE);
				prefix_index = '8';
				break;
			case HOME:
				tft_lcd_Home(1);
				break;
			case DIM:
				break;
			case BRT:
				break;
			case BACK:
				tft_lcd_change_screen(UHF_P2_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;
			case OK:
				copy_preset_to_standby_freq(prefix_index, UHF);
				update_flash();
				send_to_uart_queue_freq(UHF, A_FREQ, cdu_parameters.active_uhf_freq);
				send_to_uart_queue_freq(UHF, S_FREQ, cdu_parameters.standby_uhf_freq);
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				switch(prefix_index)
				{
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_uhf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_uhf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_uhf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_uhf_freq, PRE_PRO_8);
						break;
				}
				prefix_index = 0;
				vTaskDelay(50);
				tft_lcd_change_screen(UHF_P1_SCREEN);
				tft_lcd_send_command_float("t1",TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;
			default:
				break;
		}
	}

}


/* ------------------------------------------ END of SCREENS ----------------------------------------- */
/*
 * Function: tft_lcd
 * Arguments: uint8_t key : Input from the keyboard
 *
 * Description:
 * 				This function is responsible for calling the correct screen handler and pass the keyboards
 * 				input.
 *
 */
void tft_lcd(uint8_t key)
{
	if(key != NDF)
	{
		if (current_screen < sizeof(screen_handlers) / sizeof(ScreenHandler))
		{
			// Call the function associated with the current screen
			screen_handlers[current_screen](key);
		}
	}
}



void update_screen_(TFT_Screen screen)
{
	Class_Id cls = get_class_Id();
	switch(cls)
	{
		case NAV:
			if (screen == HOME_SCREEN){
				tft_lcd_Home(0);
			}
			if((screen == NAV_P1_SCREEN) || (screen == NAV_P2_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				if(cdu_parameters.NAV_message_counter > 5000) {
					tft_lcd_send_command_("t10", TEXT, "NAV RCU Error", NULL);
				}
				update_flash();
			}
			if((screen == NAV_P1_PROG_SCREEN) || (screen == NAV_P2_PROG_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				if(cdu_parameters.NAV_message_counter > 5000) {
					tft_lcd_send_command_("t10", TEXT, "NAV RCU Error", NULL);
				}
				update_flash();
			}
			break;
		case ADF:
			if((screen == ADF_P1_SCREEN) || (screen == ADF_P2_SCREEN))
			{
				tft_lcd_send_command_("t1", TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
				if(cdu_parameters.NAV_message_counter > 5000) {
					tft_lcd_send_command_("t10", TEXT, "ADF RCU Error", NULL);
				}
				update_flash();
			}
			if  ((screen == ADF_P1_PROG_SCREEN) || (screen == ADF_P2_PROG_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_nav_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
				if(cdu_parameters.NAV_message_counter > 5000) {
					tft_lcd_send_command_("t10", TEXT, "ADF RCU Error", NULL);
				}
				update_flash();
			}
			break;
		case TACAN:
			if((screen == TACAN_P1_CH_SCREEN) || (screen == TACAN_P2_CH_SCREEN))
			{
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				update_flash();
			}
			if((screen == TACAN_P1_CH_PROG_SCREEN) || (screen == TACAN_P2_CH_PROG_SCREEN))
			{
				tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
				tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
				update_flash();
			}
			if((screen == TACAN_P1_FQ_SCREEN) || (screen == TACAN_P2_FQ_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				update_flash();
			}
			if ((screen == TACAN_P1_FQ_PROG_SCREEN) || (screen == TACAN_P2_FQ_PROG_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				update_flash();
			}
			break;
		case HF:
			if((screen == HF_P1_SCREEN) || (screen == HF_P2_SCREEN) || (screen == HF_P1_PROG_SCREEN) || (screen == HF_P2_PROG_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				update_flash();
			}
			break;
		case VHF:
			if((screen == VHF_P1_SCREEN) || (screen == VHF_P2_SCREEN) || (screen == VHF_P1_PROG_SCREEN) || (screen == VHF_P2_PROG_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
				update_flash();
			}
			break;
		case UHF:
			if((screen == UHF_P1_SCREEN) || (screen == UHF_P2_SCREEN) || (screen == UHF_P1_PROG_SCREEN) || (screen == UHF_P2_PROG_SCREEN))
			{
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				update_flash();
			}
			break;
		default:
			update_flash();
			break;
	}
}


void revert_screen_entry(TFT_Screen screen, uint8_t p_index)
{
	switch(screen)
	{
		case NAV_P1_SCREEN:
		case NAV_P2_SCREEN:
			tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_nav_freq, STANDBY);
			tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_nav_freq, ACTIVE);
			tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_nav, NULL);
			break;

		case NAV_P1_PROG_SCREEN:
		case NAV_P2_PROG_SCREEN:
			switch(p_index)
			{
				case '1':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_nav_freq, PRE_PRO_1);
					break;
				case '2':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_nav_freq, PRE_PRO_2);
					break;
				case '3':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_nav_freq, PRE_PRO_3);
					break;
				case '4':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_nav_freq, PRE_PRO_4);
					break;
				case '5':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_nav_freq, PRE_PRO_5);
					break;
				case '6':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_nav_freq, PRE_PRO_6);
					break;
				case '7':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_nav_freq, PRE_PRO_7);
					break;
				case '8':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_nav_freq, PRE_PRO_8);
					break;
				default:
					break;
			}
			break;

			case ADF_P1_SCREEN:
			case ADF_P2_SCREEN:
				tft_lcd_send_command_("t1", TEXT, cdu_parameters.standby_adf_freq, STANDBY);
				tft_lcd_send_command_("t4", TEXT, cdu_parameters.active_adf_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_adf, NULL);
				break;

			case ADF_P1_PROG_SCREEN:
			case ADF_P2_PROG_SCREEN:
				switch(p_index)
				{
					case '1':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p1_adf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p2_adf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p3_adf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p4_adf_freq, PRE_PRO_4);
						break;
					case '5':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p5_adf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p6_adf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p7_adf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_("t10", TEXT, cdu_parameters.p8_adf_freq, PRE_PRO_8);
						break;
					default:
						break;
				}
				break;

		case TACAN_P1_CH_SCREEN:
		case TACAN_P2_CH_SCREEN:
			tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
			tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
			tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
			break;

		case TACAN_P1_CH_PROG_SCREEN:
		case TACAN_P2_CH_PROG_SCREEN:
			switch(p_index)
			{
				case '1':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p1_channel, PRE_PRO_1);
					break;
				case '2':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p2_channel, PRE_PRO_2);
					break;
				case '3':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p3_channel, PRE_PRO_3);
					break;
				case '4':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p4_channel, PRE_PRO_4);
					break;
				case '5':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p5_channel, PRE_PRO_5);
					break;
				case '6':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p6_channel, PRE_PRO_6);
					break;
				case '7':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p7_channel, PRE_PRO_7);
					break;
				case '8':
					tft_lcd_send_command_channel("t10", TEXT, cdu_parameters.p8_channel, PRE_PRO_8);
					break;
				default:
					break;
			}
			break;

			case TACAN_P1_FQ_SCREEN:
			case TACAN_P2_FQ_SCREEN:
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_tacan_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_tacan_freq, ACTIVE);
				tft_lcd_send_command_("t3", TEXT, cdu_parameters.volume_tacan, NULL);
				break;

			case TACAN_P1_FQ_PROG_SCREEN:
			case TACAN_P2_FQ_PROG_SCREEN:
				switch(p_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_tacan_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_tacan_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_tacan_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_tacan_freq, PRE_PRO_4);
						break;
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_tacan_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_tacan_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_tacan_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_tacan_freq, PRE_PRO_8);
						break;
					default:
						break;
				}
				break;

			case HF_P1_SCREEN:
			case HF_P2_SCREEN:
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_hf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_hf_freq, ACTIVE);
				break;

			case HF_P1_PROG_SCREEN:
			case HF_P2_PROG_SCREEN:
				switch(p_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_hf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_hf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_hf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_hf_freq, PRE_PRO_4);
						break;
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_hf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_hf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_hf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_hf_freq, PRE_PRO_8);
						break;
					default:
						break;
				}
				break;

		case VHF_P1_SCREEN:
		case VHF_P2_SCREEN:
			tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_vhf_freq, STANDBY);
			tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_vhf_freq, ACTIVE);
			break;

		case VHF_P1_PROG_SCREEN:
		case VHF_P2_PROG_SCREEN:
			switch(p_index)
			{
				case '1':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_vhf_freq, PRE_PRO_1);
					break;
				case '2':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_vhf_freq, PRE_PRO_2);
					break;
				case '3':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_vhf_freq, PRE_PRO_3);
					break;
				case '4':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_vhf_freq, PRE_PRO_4);
					break;
				case '5':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_vhf_freq, PRE_PRO_5);
					break;
				case '6':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_vhf_freq, PRE_PRO_6);
					break;
				case '7':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_vhf_freq, PRE_PRO_7);
					break;
				case '8':
					tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_vhf_freq, PRE_PRO_8);
					break;
				default:
					break;
			}
			break;

			case UHF_P1_SCREEN:
			case UHF_P2_SCREEN:
				tft_lcd_send_command_float("t1", TEXT, cdu_parameters.standby_uhf_freq, STANDBY);
				tft_lcd_send_command_float("t4", TEXT, cdu_parameters.active_uhf_freq, ACTIVE);
				break;

			case UHF_P1_PROG_SCREEN:
			case UHF_P2_PROG_SCREEN:
				switch(p_index)
				{
					case '1':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p1_uhf_freq, PRE_PRO_1);
						break;
					case '2':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p2_uhf_freq, PRE_PRO_2);
						break;
					case '3':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p3_uhf_freq, PRE_PRO_3);
						break;
					case '4':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p4_uhf_freq, PRE_PRO_4);
						break;
					case '5':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p5_uhf_freq, PRE_PRO_5);
						break;
					case '6':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p6_uhf_freq, PRE_PRO_6);
						break;
					case '7':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p7_uhf_freq, PRE_PRO_7);
						break;
					case '8':
						tft_lcd_send_command_float("t10", TEXT, cdu_parameters.p8_uhf_freq, PRE_PRO_8);
						break;
					default:
						break;
				}
				break;

			default:
				break;
	}
	memset(input, 0, MAX_STANDBY_FREQ_LEN);
	input_index = 0;
}


#ifdef TACAN_TEST
void TACAN_SCREEN_TEST()
{
	tft_lcd_change_screen(TACAN_P1_CH_SCREEN);
	tft_lcd_send_command_channel("t1", TEXT, cdu_parameters.standby_channel, STANDBY);
	tft_lcd_send_command_channel("t4", TEXT, cdu_parameters.active_channel, ACTIVE);
	vTaskDelay(pdMS_TO_TICKS(2000));

	tft_lcd_send_command_channel("t1", TEXT, "1X", STANDBY);


	vTaskDelay(pdMS_TO_TICKS(50));
}
#endif
