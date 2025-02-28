/*
 * keyboard.h
 *
 *  Created on: Oct 3, 2024
 *      Author: Hamza Javed
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "FreeRTOS.h"
#include "queue.h"


// KEYBOARD Events
// Navigation and Decision Keys
// Special Function Keys
#define		BACK		'!'  // Key for BACK action
#define		HOME		'@'  // Key for HOME action
#define		OK			'#'  // Key for OK action
#define		SWAP		'$'  // Key for SWAP action
#define 	PREV		'%'  // Key for PREV action
#define		NEXT		'^'  // Key for NEXT action
#define		DIM			'('  // Key for DIM action
#define 	BRT			')'  // Key for BRT action
#define		INC			'<'  // Key for INCREASE action
#define 	DEC			'>'  // Key for DECREASE action
#define 	ENT			'='  // Key for ENTER action
#define 	SPACE		'_'  // Key for SPACE action
#define 	CLR			';'  // Key for CLEAR action
#define		ADD			'+'  // Key for ADD action
#define		SUB			'-'  // Key for SUBTRACT action
#define		NDF			'\0' // Not Defined (not marked on keypad)

// Panel Keys
#define 	LEFT_SW1	'a'  // Left Panel Key 1
#define 	LEFT_SW2	'b'  // Left Panel Key 2
#define 	LEFT_SW3	'c'  // Left Panel Key 3
#define 	LEFT_SW4	'd'  // Left Panel Key 4
#define 	RIGHT_SW1	'e'  // Right Panel Key 1
#define 	RIGHT_SW2	'f'  // Right Panel Key 2
#define 	RIGHT_SW3	'g'  // Right Panel Key 3
#define 	RIGHT_SW4	'h'  // Right Panel Key 4


// Alphabet and Numeric Keys
#define		ALPHA_A		'A'  // Keyboard Key A
#define		ALPHA_B		'B'  // Keyboard Key B
#define		ALPHA_C		'C'  // Keyboard Key C
#define		ALPHA_D		'D'  // Keyboard Key D
#define		ALPHA_E		'E'  // Keyboard Key E
#define		ALPHA_F		'F'  // Keyboard Key F
#define		ALPHA_G		'G'  // Keyboard Key G
#define		ALPHA_H		'H'  // Keyboard Key H
#define		ALPHA_I		'I'  // Keyboard Key I
#define		ALPHA_J		'J'  // Keyboard Key J
#define		ALPHA_K		'K'  // Keyboard Key K
#define		ALPHA_L		'L'  // Keyboard Key L
#define		ALPHA_M		'M'  // Keyboard Key M
#define		ALPHA_N		'N'  // Keyboard Key N
#define		ALPHA_O		'O'  // Keyboard Key O
#define		ALPHA_P		'P'  // Keyboard Key P
#define		ALPHA_Q		'Q'  // Keyboard Key Q
#define		ALPHA_R		'R'  // Keyboard Key R
#define		ALPHA_S		'S'  // Keyboard Key S
#define		ALPHA_T		'T'  // Keyboard Key T
#define		ALPHA_U		'U'  // Keyboard Key U
#define		ALPHA_V		'V'  // Keyboard Key V
#define		ALPHA_W		'W'  // Keyboard Key W
#define		ALPHA_X		'X'  // Keyboard Key X
#define		ALPHA_Y		'Y'  // Keyboard Key Y
#define		ALPHA_Z		'Z'  // Keyboard Key Z


#define 	NUM_0		'0'  // Keyboard Key 0
#define 	NUM_1		'1'  // Keyboard Key 1
#define 	NUM_2		'2'  // Keyboard Key 2
#define 	NUM_3		'3'  // Keyboard Key 3
#define 	NUM_4		'4'  // Keyboard Key 4
#define 	NUM_5		'5'  // Keyboard Key 5
#define 	NUM_6		'6'  // Keyboard Key 6
#define 	NUM_7		'7'  // Keyboard Key 7
#define 	NUM_8		'8'  // Keyboard Key 8
#define 	NUM_9		'9'  // Keyboard Key 9
#define		PERIOD		'p'  // Keyboard Key .
#define 	SLASH		'/'	 // Keyboard Key /

// Special Firmware Purpose Keys
#define DATA_1553		'z'
#define REVERT_IN		'y'
#define BLINK_CURSOR	'x'
#define SWI_CHANNEL	'w'
#define SWI_FREQUENCY	'v'


// Declare the event group handle as extern so it can be shared
extern xQueueHandle xKeyQueue;

// Declaration of Rtos Thread
extern void keyboard_thread(void *pvParameters);
extern void right_panel_thread(void *pvParameters);

#endif/* INC_KEYBOARD_H_ */
