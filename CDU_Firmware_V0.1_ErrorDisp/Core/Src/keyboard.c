/*
 * keyboard.c
 *
 *  Created on: Oct 3, 2024
 *      Author: Hamza Javed
 */

/* -------------------------------------------- Include Headers --------------------------------------------*/
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "keyboard.h"
#include "main.h"
#include "debug_console.h"

/* -------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------- User Defines -------------------------------------------- */
// Define row and column pins here (as GPIO pins)
#define ROW_0_PIN    ROW1_Pin
#define ROW_1_PIN    ROW2_Pin
#define ROW_2_PIN    ROW3_Pin
#define ROW_3_PIN    ROW4_Pin
#define ROW_4_PIN    ROW5_Pin
#define ROW_5_PIN    ROW6_Pin
#define ROW_6_PIN    ROW7_Pin
#define ROW_7_PIN    ROW8_Pin

#define COL_0_PIN    COL1_Pin
#define COL_1_PIN    COL2_Pin
#define COL_2_PIN    COL3_Pin
#define COL_3_PIN    COL4_Pin
#define COL_4_PIN    COL5_Pin
#define COL_5_PIN    COL6_Pin
#define COL_6_PIN    COL7_Pin
#define COL_7_PIN    COL8_Pin

// Define ROWs and columns Ports here
#define ROW_0_PORT	 ROW1_GPIO_Port
#define ROW_1_PORT	 ROW2_GPIO_Port
#define ROW_2_PORT	 ROW3_GPIO_Port
#define ROW_3_PORT	 ROW4_GPIO_Port
#define ROW_4_PORT	 ROW5_GPIO_Port
#define ROW_5_PORT	 ROW6_GPIO_Port
#define ROW_6_PORT	 ROW7_GPIO_Port
#define ROW_7_PORT	 ROW8_GPIO_Port

#define COL_0_PORT	 COL1_GPIO_Port
#define COL_1_PORT	 COL2_GPIO_Port
#define COL_2_PORT	 COL3_GPIO_Port
#define COL_3_PORT	 COL4_GPIO_Port
#define COL_4_PORT	 COL5_GPIO_Port
#define COL_5_PORT	 COL6_GPIO_Port
#define COL_6_PORT	 COL7_GPIO_Port
#define COL_7_PORT	 COL8_GPIO_Port

#define DEBOUNCE_DELAY_MS 50  // De-bounce delay in milliseconds
#define DEBOUNCE_DELAY	  400

//
#define NO_ROWS_COLS	8
/* ----------------------------------------------------------------------------------------------------------- */

/* -------------------------------------------- Global Variables -------------------------------------------- */
// Define ROW and COLUMN pins as uint16_t (since GPIO_PIN_x constants are usually uint16_t)
uint16_t ROWS_PINS[NO_ROWS_COLS] = {ROW_0_PIN, ROW_1_PIN, ROW_2_PIN, ROW_3_PIN, ROW_4_PIN, ROW_5_PIN, ROW_6_PIN, ROW_7_PIN};
uint16_t COLS_PINS[NO_ROWS_COLS] = {COL_0_PIN, COL_1_PIN, COL_2_PIN, COL_3_PIN, COL_4_PIN, COL_5_PIN, COL_6_PIN, COL_7_PIN};

// Define corresponding GPIO ports for rows and columns (assuming all ports are GPIOA for this example)
GPIO_TypeDef* ROW_PORTS[NO_ROWS_COLS] = {ROW_0_PORT, ROW_1_PORT, ROW_2_PORT, ROW_3_PORT, ROW_4_PORT, ROW_5_PORT, ROW_6_PORT, ROW_7_PORT};
GPIO_TypeDef* COL_PORTS[NO_ROWS_COLS] = {COL_0_PORT, COL_1_PORT, COL_2_PORT, COL_3_PORT, COL_4_PORT, COL_5_PORT, COL_6_PORT, COL_7_PORT};


const char keyMap[NO_ROWS_COLS][NO_ROWS_COLS] = {
	{BACK, HOME, OK, SWAP, PREV, NEXT, DIM, BRT},
    {'1', '2', '3', 'A', 'B', 'C', 'D', 'E'},// 'F',
    {'F', '4', '5', '6', 'G', 'H', 'I', 'J'},//,
    {'K', 'L', '7', '8', '9', 'M', 'N', 'O'},//,
    {'P', 'Q', 'R', 'p', '0', '/', 'S', 'T'}, //,
	{'U', 'V', 'W', 'X', INC, NDF, NDF, NDF},
	{NDF, 'Y', 'Z', ADD, DEC, ENT, SPACE, SPACE},
	{CLR, NDF, SUB, NDF, NDF, NDF, NDF, NDF},
    // Define the remaining rows and keys here
};

static TickType_t lastPressTime = 0;  // Store the last press time
/* ----------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------- Function Decelerations --------------------------------------------*/

char keyboard_scan(void);
char right_panel_scan(void);

/* -------------------------------------------- Application Code -------------------------------------------- */

/*
 * Function: keybord_thread
 * Arguments: void *pvParameters
 *
 * Description:
 * This function runs an infinite loop to continuously scan for key presses.
 * If a key press is detected (and not identified as `NDF`), the function sends
 * the key press to a queue. The task includes a debounce delay to prevent
 * multiple detections of the same key press. The task also yields periodically
 * to allow other tasks to run.
 *
 * - pvParameters: Pointer to any parameters passed to the thread function.
 */

void keyboard_thread(void *pvParameters)
{
	char key = 0;
	/* Infinite loop */
	for(;;)
	{
		key = keyboard_scan();
		if(key != NDF)
		{
			// Send the key press to the queue
	        if (xKeyQueue != NULL) {
	            xQueueSend(xKeyQueue, &key, portMAX_DELAY);
	        }
			vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

/*
 * Function: keyboard_scan
 * Arguments: None
 *
 * Description:
 * This function scans an 8x8 keyboard matrix for a pressed key. It grounds each row one at
 * a time and checks each column for a pressed key. If a key press is detected, it resets the
 * row before returning the key. Debounce handling can be added where indicated. The function
 * ensures that the row is reset before moving to the next row.
 *
 * If no key is pressed, it returns the value `NDF`.
 */

char keyboard_scan(void)
{
    for (int row = 0; row < 8; row++) {
        // Ground one row at a time
        HAL_GPIO_WritePin(ROW_PORTS[row], ROWS_PINS[row], GPIO_PIN_RESET);

        for (int col = 0; col < 8; col++) {
            // Check each column for a pressed key
            if (HAL_GPIO_ReadPin(COL_PORTS[col], COLS_PINS[col]) == GPIO_PIN_RESET) {
                // Debounce handling can be added here

                // Reset the row to high before returning the key
                HAL_GPIO_WritePin(ROW_PORTS[row], ROWS_PINS[row], GPIO_PIN_SET);

                debug_print("I_ Key Pressed :: %c", keyMap[row][col]);

                // Return the key based on the row and column
                return keyMap[row][col];
            }
        }

        // Reset the row to high before moving to the next
        HAL_GPIO_WritePin(ROW_PORTS[row], ROWS_PINS[row], GPIO_PIN_SET);
    }

    // Return '\0' if no key is pressed
    return NDF;
}

/*
 * Function: right_panel_thread
 * Arguments: void *pvParameters
 *
 * Description:
 * This function runs an infinite loop to continuously scan for key presses
 * on the right panel. If a key press is detected (and not identified as `NDF`),
 * the function sends the key press to a queue. The task includes a debounce delay
 * to prevent multiple detections of the same key press. The task also yields periodically
 * to allow other tasks to run.
 *
 * - pvParameters: Pointer to any parameters passed to the thread function.
 */

void right_panel_thread(void *pvParameters)
{
	char key = 0;
	/* Infinite loop */
	for(;;)
	{
		key = right_panel_scan();
		if(key != NDF)
		{
			// Send the key press to the queue
	        if (xKeyQueue != NULL) {
	            xQueueSend(xKeyQueue, &key, portMAX_DELAY);
	        }
			vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

/*
 * Function: right_panel_scan
 * Arguments: None
 *
 * Description:
 * This function scans the right panel for key presses. It checks each switch
 * (RIGHT_SW1 to RIGHT_SW4) to see if it is pressed. If a key press is detected,
 * the corresponding key is assigned to `panel_key`, and a debug message is printed.
 * If no key is pressed, `panel_key` is set to '\0'.
 *
 * Returns:
 * - panel_key: The character representing the key that was pressed, or '\0' if no key is pressed.
 */
char right_panel_scan(void)
{
	char panel_key = '\0';
	if(HAL_GPIO_ReadPin(RIGHT_SW1_GPIO_Port, RIGHT_SW1_Pin) == 0)
	{
		panel_key = RIGHT_SW1;
//		debug_print("I_ Key Pressed RIGHT_SW1 :: %c", panel_key);
	}
	else if(HAL_GPIO_ReadPin(RIGHT_SW2_GPIO_Port, RIGHT_SW2_Pin) == 0)
	{
		panel_key = RIGHT_SW2;
//		debug_print("I_ Key Pressed RIGHT_SW2 :: %c", panel_key);
	}
	else if(HAL_GPIO_ReadPin(RIGHT_SW3_GPIO_Port, RIGHT_SW3_Pin) == 0)
	{
		panel_key = RIGHT_SW3;
//		debug_print("I_ Key Pressed RIGHT_SW3 :: %c", panel_key);
	}
	else if(HAL_GPIO_ReadPin(RIGHT_SW4_GPIO_Port, RIGHT_SW4_Pin) == 0)
	{
		panel_key = RIGHT_SW4;
//		debug_print("I_ Key Pressed RIGHT_SW4 :: %c", panel_key);
	}
	else
	{
		panel_key = '\0';
	}

	return panel_key;
}

/*
 * Function: HAL_GPIO_EXTI_Callback
 * Arguments: uint16_t GPIO_Pin
 *
 * Description:
 * This function is the external interrupt callback handler for GPIO pins. It identifies
 * which key was pressed based on the GPIO pin that triggered the interrupt, debounces
 * the key press, and sends the key press to a queue if it is valid.
 *
 * - GPIO_Pin: The pin that triggered the external interrupt.
 *
 * The function:
 * - Identifies the key pressed based on the pin.
 * - Implements debounce logic to prevent multiple detections.
 * - Sends the key press to a queue if it is a valid key.
 * - Updates the last press time to implement debouncing.
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	char panel_key = NDF;
    TickType_t currentTime = xTaskGetTickCount();  // Get the current tick count

	switch(GPIO_Pin)
	{
		case LEFT_SW1_Pin:
			panel_key = LEFT_SW1;
//        	debug_print("I_ Key Pressed LEFT_SW1 :: %c\n", panel_key);
			break;
		case LEFT_SW2_Pin:
			panel_key = LEFT_SW2;
//        	debug_print("I_ Key Pressed LEFT_SW2 :: %c\n", panel_key);
			break;
		case LEFT_SW3_Pin:
			panel_key = LEFT_SW3;
//        	debug_print("I_ Key Pressed LEFT_SW3 :: %c\n", panel_key);
			break;
		case LEFT_SW4_Pin:
			panel_key = LEFT_SW4;
//        	debug_print("I_ Key Pressed LEFT_SW4 :: %c\n", panel_key);
			break;
		default:
			panel_key = NDF;
			break;
	}

    // De-bounce logic
    if (panel_key != NDF && (currentTime - lastPressTime) * portTICK_PERIOD_MS >= DEBOUNCE_DELAY_MS)
    {
        lastPressTime = currentTime;  // Update the last press time

        // Send to queue only if a valid key was pressed
        if (xKeyQueue != NULL)
        {
        	debug_print("I_ Queue_Send :: %c\n", panel_key);
            xQueueSend(xKeyQueue, &panel_key, portMAX_DELAY);  // Send the panel key to the queue
        }
    }
}
/* -------------------------------------------- END of File -------------------------------------------- */
