/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f0xx_it.c
 * @brief   Interrupt Service Routines.
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart4;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */

	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */
	while (1) {
	}
	/* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
	/* USER CODE BEGIN HardFault_IRQn 0 */

	__BKPT(0);
	uint32_t *hardfault_stack = (uint32_t*) __get_MSP();
	uint32_t stacked_r0 = hardfault_stack[0];
	uint32_t stacked_r1 = hardfault_stack[1];
	uint32_t stacked_r2 = hardfault_stack[2];
	uint32_t stacked_r3 = hardfault_stack[3];
	/* USER CODE END HardFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

// Hard Fault handler wrapper in assembly
// It extracts the location of stack frame and passes it to handler
// in C as a pointer. We also extract the LR value as second
// parameter.


//void pop_registers_from_fault_stack(unsigned int * hardfault_args)
//{
//    unsigned int stacked_r0;
//    unsigned int stacked_r1;
//    unsigned int stacked_r2;
//    unsigned int stacked_r3;
//    unsigned int stacked_r12;
//    unsigned int stacked_lr;
//    unsigned int stacked_pc;
//    unsigned int stacked_psr;
//
//    // Pop the registers from the stack passed via the 'hardfault_args'
//    stacked_r0  = hardfault_args[0];
//    stacked_r1  = hardfault_args[1];
//    stacked_r2  = hardfault_args[2];
//    stacked_r3  = hardfault_args[3];
//    stacked_r12 = hardfault_args[4];
//    stacked_lr  = hardfault_args[5];
//    stacked_pc  = hardfault_args[6];
//    stacked_psr = hardfault_args[7];
//
//    // Now you can inspect these values. For debugging purposes, you could
//    // print them out or store them in a variable or memory location.
//    // For example, if you're using a debug UART, you could send these values:
//#if DEBUG_CONSOLE
//    debug_print("Hardfault occurred!\n");
//    debug_print("R0  = 0x%08X\n", stacked_r0);
//    debug_print("R1  = 0x%08X\n", stacked_r1);
//    debug_print("R2  = 0x%08X\n", stacked_r2);
//    debug_print("R3  = 0x%08X\n", stacked_r3);
//    debug_print("R12 = 0x%08X\n", stacked_r12);
//    debug_print("LR  = 0x%08X\n", stacked_lr);
//    debug_print("PC  = 0x%08X\n", stacked_pc);
//    debug_print("PSR = 0x%08X\n", stacked_psr);
//#endif
//
//    // Inspect stacked_pc to locate the offending instruction.
//    // The stacked_pc holds the address of the instruction that caused the fault.
//    // This can be useful for debugging and determining where the fault occurred.
//    // You could, for example, check the PC and stop or reset the system if needed.
//
//    // Debugging infinite loop for further inspection.
//    // You could replace this with logic to recover from the fault.
//    for( ;; );
//}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line 0 and 1 interrupts.
 */
void EXTI0_1_IRQHandler(void) {
	/* USER CODE BEGIN EXTI0_1_IRQn 0 */

	/* USER CODE END EXTI0_1_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(LEFT_SW4_Pin);
	HAL_GPIO_EXTI_IRQHandler(LEFT_SW3_Pin);
	/* USER CODE BEGIN EXTI0_1_IRQn 1 */

	/* USER CODE END EXTI0_1_IRQn 1 */
}

/**
 * @brief This function handles EXTI line 2 and 3 interrupts.
 */
void EXTI2_3_IRQHandler(void) {
	/* USER CODE BEGIN EXTI2_3_IRQn 0 */

	/* USER CODE END EXTI2_3_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(LEFT_SW2_Pin);
	HAL_GPIO_EXTI_IRQHandler(LEFT_SW1_Pin);
	/* USER CODE BEGIN EXTI2_3_IRQn 1 */

	/* USER CODE END EXTI2_3_IRQn 1 */
}

/**
 * @brief This function handles TIM6 global and DAC channel underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void) {
	/* USER CODE BEGIN TIM6_DAC_IRQn 0 */

	/* USER CODE END TIM6_DAC_IRQn 0 */
	HAL_TIM_IRQHandler(&htim6);
	/* USER CODE BEGIN TIM6_DAC_IRQn 1 */

	/* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
 * @brief This function handles TIM16 global interrupt.
 */
void TIM16_IRQHandler(void) {
	/* USER CODE BEGIN TIM16_IRQn 0 */

	/* USER CODE END TIM16_IRQn 0 */
	HAL_TIM_IRQHandler(&htim16);
	/* USER CODE BEGIN TIM16_IRQn 1 */

	/* USER CODE END TIM16_IRQn 1 */
}

/**
 * @brief This function handles TIM17 global interrupt.
 */
void TIM17_IRQHandler(void) {
	/* USER CODE BEGIN TIM17_IRQn 0 */

	/* USER CODE END TIM17_IRQn 0 */
	HAL_TIM_IRQHandler(&htim17);
	/* USER CODE BEGIN TIM17_IRQn 1 */

	/* USER CODE END TIM17_IRQn 1 */
}

/**
 * @brief This function handles USART3 to USART8 global interrupts / USART3 wake-up interrupt through EXTI line 28.
 */
void USART3_8_IRQHandler(void) {
	/* USER CODE BEGIN USART3_8_IRQn 0 */
#ifdef OS_INT
  /* USER CODE END USART3_8_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);(&huart4);
  /* USER CODE BEGIN USART3_8_IRQn 1 */
#else
	HAL_UART_IRQHandler(&huart4);
#endif
	/* USER CODE END USART3_8_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
