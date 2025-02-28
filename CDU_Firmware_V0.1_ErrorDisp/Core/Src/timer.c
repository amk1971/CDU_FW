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
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        // Use xQueueSendFromISR for sending data from ISR
        xQueueSendFromISR(xKeyQueue, &key, &xHigherPriorityTaskWoken);

        // If a task was woken by the queue send, then request a context switch
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


void start_timer_cursor(void)
{
	  __HAL_TIM_SET_COUNTER(&htim17, 0);  // Reset the timer counter
	__HAL_TIM_CLEAR_FLAG(&htim17, TIM_FLAG_UPDATE);
    // Start the timer in interrupt mode
    HAL_TIM_Base_Start_IT(&htim17);  // Replace htimx with your timer handle (e.g., htim2)
}

// Stop the timer if needed
void stop_timer_cursor(void)
{
    // Stop the timer and disable interrupts
    HAL_TIM_Base_Stop_IT(&htim17);
}

void cursor_notify_tft(void)
{
	char key = BLINK_CURSOR;
	if (xKeyQueue != NULL)
	    {
	        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	        // Use xQueueSendFromISR for sending data from ISR
	        xQueueSendFromISR(xKeyQueue, &key, &xHigherPriorityTaskWoken);

	        // If a task was woken by the queue send, then request a context switch
	        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	    }
}
