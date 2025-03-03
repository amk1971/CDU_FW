/*
 * debug_console.c
 *
 *  Created on: Oct 8, 2024
 *      Author: Hamza Javed
 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include "debug_console.h"
#include "keyboard.h"

void debug_print(const char *format, ...);

#if DEBUG_CONSOLE

#define PRINT_CONSOLE	huart1
char newLine[2] = "\r\n";
/*
 * Function: uart_send
 * Arguments: const char *msg
 *
 * Description:
 * This function sends a message over UART. It transmits the provided message followed
 * by a newline. The function uses the HAL_UART_Transmit function to handle the UART
 * communication, ensuring the message is sent before the function returns.
 *
 * - msg: Pointer to the null-terminated string to be sent over UART.
 */
void uart_send(const char *msg)
{
    HAL_UART_Transmit(&PRINT_CONSOLE, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    HAL_UART_Transmit(&PRINT_CONSOLE, (uint8_t*)newLine, strlen(newLine), HAL_MAX_DELAY);
}

/*
 * Function: debug_console_thread
 * Arguments: void *pvParameter
 *
 * Description:
 * This function runs an infinite loop to continuously receive chunks of debug messages
 * from the queue. It accumulates these chunks into a full message buffer and sends the
 * complete message via UART when a newline character is detected as the end marker.
 * It handles buffer overflow and ensures proper concatenation of message chunks.
 *
 * - pvParameter: Pointer to any parameters passed to the thread function.
 */
void debug_console_thread(void *pvParameter)
{
    char p_msg[100];  // Chunk received from the queue
    char full_message[500] = {0};  // Buffer for full message
    int full_msg_offset = 0;
    uart_send("-----CDU Start ---- \n");
//    UART_StartReceive_1();

    for (;;)
    {
    	// Wait for a chunk to arrive in the queue
        if (xQueueReceive(xDebugQueue, p_msg, portMAX_DELAY) == pdTRUE)
        {
            // Append the chunk to the full message buffer
            int chunk_len = strlen(p_msg);
            if (full_msg_offset + chunk_len < sizeof(full_message))
            {
                // Concatenate the chunk to the full message
                strcpy(full_message + full_msg_offset, p_msg);
                full_msg_offset += chunk_len;
            }
            else
            {
                // Handle overflow (optional)
                uart_send("Error: Message buffer overflow\n");
                full_msg_offset = 0;  // Reset buffer
            }

            // Check if the message is complete (newline \n as an end marker)
            if (strchr(p_msg, '\n') != NULL)
            {
                // Send the complete message via UART
                uart_send(full_message);

                // Reset the full message buffer after sending
                memset(full_message, 0, sizeof(full_message));
                full_msg_offset = 0;
            }
        }

        // Task delay for some time (optional)
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


#endif


/*
 * Function: debug_print
 * Arguments: const char *format, ...
 *
 * Description:
 * This function formats a debug message using a variable argument list and sends it
 * to a queue in chunks of 100 characters. It first initializes the argument list and
 * formats the message into a buffer. The message is then split into chunks and sent
 * to the debug queue, ensuring each chunk is null-terminated.
 *
 * - format: A formatted string similar to printf.
 * - ... : Additional arguments for the formatted string.
 */
void debug_print(const char *format, ...)
{
#if DEBUG_CONSOLE
    char buffer[500];  // Buffer large enough for multiple statements
    char chunk[100];   // Chunk to send to the queue
    va_list args;

    // Initialize the argument list
    va_start(args, format);

    // Format the string (similar to printf)
    vsnprintf(buffer, sizeof(buffer), format, args);

    // End using variable argument list
    va_end(args);

    // Send the buffer in chunks of 100 characters
    int buffer_length = strlen(buffer);
    int offset = 0;

    while (offset < buffer_length)
    {
        // Copy a chunk of 100 characters (or remaining part)
        strncpy(chunk, buffer + offset, sizeof(chunk) - 1);
        chunk[sizeof(chunk) - 1] = '\0';  // Null-terminate the chunk

        // Send the chunk to the queue
        if (xDebugQueue != NULL)
        {
            if (xQueueSend(xDebugQueue, chunk, portMAX_DELAY) != pdPASS)
            {
                // Handle queue full scenario
            }
        }

        // Increment offset by 100 for the next chunk
        offset += sizeof(chunk) - 1;
    }
#endif
}
