/*! \file CDU_keyboard.h
\brief CDU Keyboard key codes, scan and decode functions
*/ 
			
#ifndef CDU_KEYBOARD_H				
#define CDU_KEYBOARD_H							

#include "main.h"
#include "CDU_keyboard.h"				
				
				
///////////////////////////////////////////////				
//				DEFINITIONS
///////////////////////////////////////////////				
				
///////////////////////////////////////////////				
//				OBJECTS
///////////////////////////////////////////////		

const keyboard_t keymap[] = {
	{ 17, "BACK" },
	{ 18, "HOME" },
	{ 19, "OK" },
	{ 20, "SWAP" },
	{ 21, "PREV" },
	{ 22, "NEXT" },
	{ 23, "DIM" },
	{ 24, "BRT" },
	{ 33, "1" },
	{ 34, "2" },
	{ 35, "3" },
	{ 50, "4" },
	{ 51, "5" },
	{ 52, "6" },
	{ 67, "7" },
	{ 68, "8" },
	{ 69, "9" },
	{ 84, "*" },
	{ 85, "0" },
	{ 86, "/" },
	{ 36, "A" },
	{ 37, "B" },
	{ 38, "C" },
	{ 39, "D" },
	{ 40, "E" },
	{ 49, "F" },
	{ 53, "G" },
	{ 54, "H" },
	{ 55, "I" },
	{ 56, "J" },
	{ 65, "K" },
	{ 66, "L" },
	{ 70, "M" },
	{ 71, "N" },
	{ 72, "O" },
	{ 81, "P" },
	{ 82, "Q" },
	{ 83, "R" },
	{ 87, "S" },
	{ 88, "T" },
	{ 97, "U" },
	{ 98, "V" },
	{ 99, "W" },
	{ 100, "X" },
	{ 114, "Y" },
	{ 115, "Z" },
	{ 101, "ARROW_UP" },
	{ 117, "ARROW_DOWN" },
	{ 102, "KEY1" },
	{ 103, "KEY2" },
	{ 104, "KEY3" },
	{ 113, "KEY4" },
	{ 130, "KEY5" },
	{ 116, "+" },
	{ 131, "-" },
	{ 118, "ENT" },
	{ 119, " " },
	{ 120, " " },
	{ 129, "CLR" },
};

uint16_t key_counter[59];
buttonmode key_mode[59];
///////////////////////////////////////////////				
//				FUNCTIONS
///////////////////////////////////////////////				
				
//-------------------------------------------------------------------
//  SCAN KEYBOARD ROW
//
/*! \fn uint8_t KBD_scan_row(uint8_t col)
\brief Scan single row in CDU keyboard 

Scan the keyboard by rows, reading the columns states as bits, where '1': key pulsed,'0': not pulsed.
\return uint8_t containing the row and column encoded as (row number) x 16 + (col number).
*/
//-------------------------------------------------------------------
uint8_t KBD_scan_row(uint8_t row /* row index (1-8) */)
{
	
	uint8_t col_index = 0;;
	// pull up all rows (default)
	ROW1_HIGH;
	ROW2_HIGH;
	ROW3_HIGH;
	ROW4_HIGH;
	ROW5_HIGH;
	ROW6_HIGH;
	ROW7_HIGH;
	ROW8_HIGH;
	
	// select ROW pin for scan
	switch(row) {
		
	case 1:
		ROW1_LOW;
		break;
		
	case 2:
		ROW2_LOW;
		break;
		
	case 3:
		ROW3_LOW;
		break;
		
	case 4:
		ROW4_LOW;
		break;
		
	case 5:
		ROW5_LOW;
		break;
		
	case 6:
		ROW6_LOW;
		break;
		
	case 7:
		ROW7_LOW;
		break;
		
	case 8:
		ROW8_LOW;
		break;
	}

	// decode COL values and assemble as 8-bit integer
	// bit set = key pulsed

	if (COL1_IS_LOW)	col_index = 1;
	if (COL2_IS_LOW)	col_index = 2;
	if (COL3_IS_LOW)	col_index = 3;
	if (COL4_IS_LOW)	col_index = 4;
	if (COL5_IS_LOW)	col_index = 5;
	if (COL6_IS_LOW)	col_index = 6;
	if (COL7_IS_LOW)	col_index = 7;
	if (COL8_IS_LOW)	col_index = 8;

	if (col_index != 0) return row * 16 + col_index;
	else return 0;
}


#endif // END OF FILE				

