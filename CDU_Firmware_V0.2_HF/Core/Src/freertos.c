/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#define FREERTOS
#ifndef FREERTOS
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
#else

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"

#include "TFT_LCD.h"
#include "keyboard.h"
#include "debug_console.h"
#include "serial_1.h"
#include "data_handler.h"

#define RX_MESSAGE_LENGTH 25
#define QUEUE_LENGTH 10

// Task Size
#define KEYBOARD_STACK_SIZE	256
#define SERIAL1_STACK_SIZE	512
//#define SERIAL2_STACK_SIZE	256
//#define SERIAL3_STACK_SIZE	256
//#define SERIAL4_STACK_SIZE	256
#define TFT_LCD_STACK_SIZE	2048
#define RIGHT_P_STACK_SIZE	256
#if DEBUG_CONSOLE
#define DEBUG_STACK_SIZE	1024
#endif

// Define the queue handle
xQueueHandle xKeyQueue = NULL;
xQueueHandle xuartTXQueue = NULL;
xQueueHandle xuartRXQueue = NULL;
#if DEBUG_CONSOLE
xQueueHandle xDebugQueue = NULL;
#endif

//SemaphoreHandle_t uartRXSemaphoreHandle;

SemaphoreHandle_t xFlashMutex;

/*
 * Function: create_cdu_tasks
 * Arguments: None
 *
 * Description:
 * This function creates various tasks for handling different components of the CDU.
 * It uses the FreeRTOS xTaskCreate function to create tasks for the keyboard, right panel,
 * serial communication with 1553, and TFT LCD display. Additionally, it conditionally creates
 * a debug console task if DEBUG_CONSOLE is enabled.
 *
 * The created tasks include:
 * - KeyBoard Matrix task for handling keyboard inputs.
 * - RIGHT PANEL task for handling right panel key inputs.
 * - Serial 1553 task for serial communication.
 * - TFT_LCD task for display operations.
 * - Debug Console task for debugging purposes (conditionally created).
 */
void create_cdu_tasks(void)
{
//	// Key Board task for handling Keyboard inputs
//	xTaskCreate(keyboard_thread, "Keyboard Matrix", KEYBOARD_STACK_SIZE, (void *)1, 5, NULL);
//	// Right Panel task for handling the Right panel key inputs
//	xTaskCreate(right_panel_thread, "RIGHT PANEL", RIGHT_P_STACK_SIZE,  (void *)1, 5, NULL);
//	// Serial Task to 1553
//	xTaskCreate(serial_1553_thread, "Serial 1553", SERIAL1_STACK_SIZE, (void *) 1, 5, NULL);
//	// A display task
//	xTaskCreate(tft_lcd_thread, "TFT_LCD", TFT_LCD_STACK_SIZE, (void *)1, 5, NULL);


	// Key Board task for handling Keyboard inputs (High priority)
	xTaskCreate(keyboard_thread, "Keyboard Matrix", KEYBOARD_STACK_SIZE, (void *)1, 3, NULL);

	// Right Panel task for handling the Right panel key inputs (Medium-high priority)
	xTaskCreate(right_panel_thread, "RIGHT PANEL", RIGHT_P_STACK_SIZE, (void *)1, 2, NULL);

	// Serial Task to 1553 (Medium priority)
	xTaskCreate(serial_1553_thread, "Serial 1553", SERIAL1_STACK_SIZE, (void *) 1, 1, NULL);

	// A display task (Low priority)
	xTaskCreate(tft_lcd_thread, "TFT_LCD", TFT_LCD_STACK_SIZE, (void *)1, 0, NULL);

#if DEBUG_CONSOLE
	xTaskCreate(debug_console_thread, "Debug Console", DEBUG_STACK_SIZE, (void *)1, 5, NULL);
#endif
}

/*
 * Function: create_cdu_queus
 * Arguments: None
 *
 * Description:
 * This function creates various FreeRTOS queues for handling different components of the CDU.
 * It initializes queues for the keyboard keys, UART transmission indication, and the debug console.
 * It also includes conditional debug prints to check if the queues were created successfully.
 *
 * The created queues include:
 * - xKeyQueue: Queue for the keyboard keys.
 * - xuartTXQueue: Queue for UART transmission indication.
 * - xDebugQueue: Queue for debug messages (conditionally created if DEBUG_CONSOLE is enabled).
 */
void create_cdu_queus(void)
{
	// a queue for the keyboard keys
    xKeyQueue = xQueueCreate(QUEUE_LENGTH, sizeof(char));
#if DEBUG_CONSOLE
	if(xKeyQueue == NULL)
	{
		debug_print("Error: Failed to Create Queue. xKeyQueue \r\n");
	}
#endif
	// serial Transmit queue
	xuartTXQueue = xQueueCreate(QUEUE_LENGTH, sizeof(Identifier));
#if DEBUG_CONSOLE
	if(xuartTXQueue == NULL)
	{
		debug_print("Error: Failed to Create xuartTXQueue. \r\n");
	}
#endif
	// serial Reception queue
	xuartRXQueue = xQueueCreate(QUEUE_LENGTH, RX_MESSAGE_LENGTH + 1);
#if DEBUG_CONSOLE
	if(xuartRXQueue == NULL)
	{
		debug_print("Error: Failed to Create xuartTXQueue. \r\n");
	}
#endif
#if DEBUG_CONSOLE
	xDebugQueue = xQueueCreate(QUEUE_LENGTH, sizeof(char[100]));
	if(xDebugQueue == NULL)
	{
		debug_print("Error: Failed to Create Queue xDebugQueue. \r\n");
	}
#endif
}

/*
 * Function: creat_cdu_semaphore
 * Arguments: None
 *
 * Description:
 * This function creates a semaphore for receive indication in the CDU.
 * It initializes the semaphore using `xSemaphoreCreateBinary`. If the
 * creation fails, a debug print statement is executed to indicate the failure
 * when the DEBUG_CONSOLE flag is enabled.
 *
 * - uartRXSemaphoreHandle: Handle to the created binary semaphore.
 */
void creat_cdu_semaphore(void)
{
	//semaphore for Receive indication
//	uartRXSemaphoreHandle = xSemaphoreCreateBinary();
//#if DEBUG_CONSOLE
//	if(uartRXSemaphoreHandle == NULL)
//	{
//		debug_print("Error: Failed to Create uartRXSemaphoreHandle. \r\n");
//	}
//#endif
	// Semaphore for Flash Synchronization
	xFlashMutex = xSemaphoreCreateMutex();
#if DEBUG_CONSOLE
	if(xFlashMutex == NULL)
	{
		debug_print("Error: Failed to Create xFlashMutex. \r\n");
	}
#endif

}

/*
 * Function: MX_FREERTOS_Init
 * Arguments: None
 *
 * Description:
 * This function initializes the FreeRTOS environment for the CDU. It creates
 * the necessary tasks, queues, and semaphores required for operation. After
 * setting up the tasks, queues, and semaphores, it starts the FreeRTOS scheduler.
 *
 * Steps:
 * - Create all the tasks for CDU by calling create_cdu_tasks.
 * - Create the queues for user inputs like the keyboard by calling create_cdu_queus.
 * - Create the semaphores for serial receive and transmission by calling creat_cdu_semaphore.
 * - Start the FreeRTOS scheduler with vTaskStartScheduler.
 */
void MX_FREERTOS_Init(void)
{
	// Create all the tasks for CDU
	create_cdu_tasks();
	// Creates the queue for user inputs like for KeyBoard
	create_cdu_queus();
	// Creates the semaphore for the serial Receive and transmission
	creat_cdu_semaphore();

	// start the scheduler


    vTaskStartScheduler();
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Optional: Print the name of the task that caused the overflow (if logging is available)
//    printf("Stack Overflow in Task: %s\n", pcTaskName);

    // Disable interrupts to prevent further execution
    taskDISABLE_INTERRUPTS();

    // Infinite loop to halt execution (useful for debugging)
    while (1) {
        // You can toggle an LED here to indicate a failure if available
    }
}


void vApplicationIdleHook(void)
{
    /* This is the idle task hook function. The idle task runs whenever no other task is ready to run.
       It is often used for low-power modes like sleep or to perform low-priority background tasks.
       In this example, we put the MCU into sleep mode when the system is idle. */

    __WFI();  // Wait for interrupt (puts the MCU in low power mode until the next interrupt occurs)
}


void vApplicationMallocFailedHook(void) {
    printf("Malloc Failed!\n");
    while (1);
}
#endif

/* USER CODE END Application */

