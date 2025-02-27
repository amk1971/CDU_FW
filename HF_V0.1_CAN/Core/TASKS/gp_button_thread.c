/* ----------------------------- Soft Keys Operations ------------------------------- */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "constant_defines.h"
#include "data_handler.h"
#include "main.h"

static inline uint8_t readButton(GPIO_TypeDef *BUTTON_PORT, uint16_t pin) { return HAL_GPIO_ReadPin(BUTTON_PORT, pin); }

void general_purpose_switches_thread(void *pvParameters)
{
    TickType_t right_knob_press_start_time = 0;
    uint8_t left_knob_was_pressed = 0;
    uint8_t left_knob_long_press = 0;

    for (;;)
    {
        // Handle Button 1, 2, 3 (Normal Buttons)
        if (!readButton(GP_SW1D10_GPIO_Port, GP_SW1D10_Pin))
        {  // FRQ / CH
            xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW1_PRESS);
//            vTaskDelay(pdMS_TO_TICKS(200));
        }
         if (!readButton(GP_SW2D11_GPIO_Port, GP_SW2D11_Pin))
         {  // Active-low
             xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW2_PRESS);
         }

         if (!readButton(GP_SW3D12_GPIO_Port, GP_SW3D12_Pin))
         {  // Active-low
             xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW3_PRESS);
         }
        if (!readButton(RIGHT_SW_GPIO_Port, RIGHT_SW_Pin))
        {  // CURSOR BLINK START
            xEventGroupSetBits(buttonEventGroup, EVENT_RIGHT_SW_PRESS);
        }
        if (!readButton(GP_SW6_GPIO_Port, GP_SW6_Pin))
        {  // PROG
            xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW_PROG_EXIT);
        }

        if (!readButton(SW5_GPIO_Port, SW5_Pin))
        {  // STO
            xEventGroupSetBits(buttonEventGroup, EVENT_GP_SW_DELETE_STO);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        // Handle Button 4 (Dual-Operation Button) // LEFT KNOB BUTTON
        uint8_t button4Pressed = !readButton(LEFT_SW_GPIO_Port, LEFT_SW_Pin);  // Active-low

        if (button4Pressed)
        {
            if (!left_knob_was_pressed)
            {
                // Button just pressed
                right_knob_press_start_time = xTaskGetTickCount();
                left_knob_was_pressed = 1;
            }
            else if (left_knob_was_pressed &&
                     (xTaskGetTickCount() - right_knob_press_start_time) >= pdMS_TO_TICKS(LONG_PRESS_THRESHOLD_MS))
            {
                // Long press detected

            	//??????!!!!
                xEventGroupSetBits(buttonEventGroup, EVENT_LEFT_SW_LONG_PRESS);
                left_knob_long_press = 1;  // Reset state to avoid repeated long press events
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
        else
        {
            if ((left_knob_was_pressed) && (left_knob_long_press == 0)) {
                // Button just released
                if ((xTaskGetTickCount() - right_knob_press_start_time) < pdMS_TO_TICKS(LONG_PRESS_THRESHOLD_MS))
                {
                    // Short press detected
                    xEventGroupSetBits(buttonEventGroup, EVENT_LEFT_SW_NORMAL_PRESS);
                }
                left_knob_was_pressed = 0;
            }
            if (left_knob_long_press == 1) {
            	left_knob_long_press = 0;
            	left_knob_was_pressed = 0;
            }
        }

        // Delay for the polling interval
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
    }
}
