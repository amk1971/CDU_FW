/*
 * timer.c
 *
 *  Created on: Oct 18, 2024
 *      Author: Hamza Javed
 */

#include "main.h"
#include "tim.h"
#include "keyboard.h"


// Start the timer for the 30-second countdown
void start_timer(void)
{
	  __HAL_TIM_SET_COUNTER(&htim16, 0);  // Reset the timer counter
	__HAL_TIM_CLEAR_FLAG(&htim16, TIM_FLAG_UPDATE);
    // Start the timer in interrupt mode
    HAL_TIM_Base_Start_IT(&htim16);  // Replace htimx with your timer handle (e.g., htim2)
}

// Stop the timer if needed
void stop_timer(void)
{
    // Stop the timer and disable interrupts
    HAL_TIM_Base_Stop_IT(&htim16);
}

void time_up_notify_tft(void)
{
	char key = REVERT_IN;
	stop_timer();
    if (xKeyQueue != NULL)
    {
        xQueueSend(xKeyQueue, &key, portMAX_DELAY);
    }
}

