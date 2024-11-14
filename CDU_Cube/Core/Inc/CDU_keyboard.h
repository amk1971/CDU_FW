#pragma once

/**
  ******************************************************************************	
		KEYBOARD DRIVER
  *
  ******************************************************************************
  */

#ifndef __CDU_KEYBOARD_H
#define __CDU_KEYBOARD_H


typedef struct {
	uint8_t code;
	char *label;
} keyboard_t;

extern const keyboard_t keymap[];
extern uint16_t key_counter[59];
extern buttonmode key_mode[59];

///////////////////////////////////////////////
//				PROTOTYPES
///////////////////////////////////////////////
uint8_t KBD_scan_row(uint8_t col);
uint8_t KBD_decode_columns(uint8_t col);

	
#endif

