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
    for (;;)
    {
        osDelay(1);
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
#else

/* ---------------------------------- Includes --------------------------------
 */
#include "FreeRTOS.h"
#include "data_handler.h"
#include "debug_console.h"
#include "event_groups.h"
#include "gp_button_thread.h"
#include "main.h"
#include "queue.h"
#include "serial_handler.h"
#include "task.h"
#include "HF_main_thread.h"

// #include ""

/* ----------------------------------- MACROS
 * ----------------------------------- */

#define CDU_COM_STACK_SIZE 256
#define HF_RECEIVER_COM_STACK_SIZE 256
#define DATA_HANDLER_STACK_SIZE 1024
#define USER_INPUTS_STACK_SIZE 128
#if DEBUG_CONSOLE
#define DEBUG_CONSOLE_STACK_SIZE 2096
#endif

/* ----------------------------------- Queue Handles
 * ----------------------------- */
xQueueHandle xCDUCOMRxQueue = NULL;
xQueueHandle xCDUCOMTxQueue = NULL;
xQueueHandle xHFRecRxQueue = NULL;
xQueueHandle xHFRecTxQueue = NULL;
xQueueHandle xMainNotifyQueue = NULL;
#if DEBUG_CONSOLE
xQueueHandle xDebugQueue = NULL;
#endif

/* ----------------------------------- Event group
 * ------------------------------- */
EventGroupHandle_t buttonEventGroup;

/* ----------------------------------- Functions
 * --------------------------------- */

void create_HF_RCU_Tasks(void)
{
    xTaskCreate(serial_CDU_com_thread, "CDU Communication", CDU_COM_STACK_SIZE, (void *)1, 10, NULL);
    xTaskCreate(serial_HF_Receiver_com_thread, "HF Receiver Communication", HF_RECEIVER_COM_STACK_SIZE, (void *)1,
                10, NULL);
    xTaskCreate(general_purpose_switches_thread, " Switches Polling ", USER_INPUTS_STACK_SIZE, (void *)1, 10, NULL);
    xTaskCreate(HF_main_thread, "Main", DATA_HANDLER_STACK_SIZE, (void *)1, 10, NULL);

#if DEBUG_CONSOLE
    xTaskCreate(debug_console_thread, "Debug Console", DEBUG_CONSOLE_STACK_SIZE, (void *)1, 5, NULL);
#endif
}

void create_HF_RCU_Queues(void)
{
    xCDUCOMRxQueue = xQueueCreate(10, sizeof(char[25]));
#if DEBUG_CONSOLE
    if (xCDUCOMRxQueue == NULL)
    {
        debug_print("Error: Failed to Create Queue. xCDUCOMRxQueue \r\n");
    }
#endif

    xCDUCOMTxQueue = xQueueCreate(10, sizeof(Identifier));
#if DEBUG_CONSOLE
    if (xCDUCOMTxQueue == NULL)
    {
        debug_print("Error: Failed to Create Queue. xCDUCOMTxQueue \r\n");
    }
#endif

    xHFRecRxQueue = xQueueCreate(10, sizeof(char[25]));
#if DEBUG_CONSOLE
    if (xHFRecRxQueue == NULL)
    {
        debug_print("Error: Failed to Create Queue. xHFRecRxQueue \r\n");
    }
#endif

    xHFRecTxQueue = xQueueCreate(10, sizeof(s_HF_Parameters));
#if DEBUG_CONSOLE
    if (xHFRecTxQueue == NULL)
    {
        debug_print("Error: Failed to Create Queue. xHFRecTxQueue \r\n");
    }
#endif

    xMainNotifyQueue = xQueueCreate(10, sizeof(Identifier));
#if DEBUG_CONSOLE
    if (xMainNotifyQueue == NULL)
    {
        debug_print("Error: Failed to Create Queue. xMainNotifyQueue \r\n");
    }
#endif

#if DEBUG_CONSOLE
    xDebugQueue = xQueueCreate(10, sizeof(char[100]));
    if (xDebugQueue == NULL)
    {
        debug_print("Error: Failed to Create Queue xDebugQueue. \r\n");
    }
#endif
}

void MX_FREERTOS_Init(void)
{
    // Create HF-RCU Tasks
    create_HF_RCU_Tasks();

    // Create Queues for user Inputs and Data Transmission/Reception
    create_HF_RCU_Queues();

    // Create the event group
    buttonEventGroup = xEventGroupCreate();

    // start the scheduler
    vTaskStartScheduler();
}

#endif
/* USER CODE END Application */

