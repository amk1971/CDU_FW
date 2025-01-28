/*
 * glcd.h
 *
 *  Created on: Nov 28, 2024
 *      Author: RCU_NAV_NoErrorMsg_V3nobs
 */

#ifndef INC_GLCD_H_
#define INC_GLCD_H_

#include "glcd_font.h"
#include "stm32f1xx.h"

#define T  1
// Data Pins
#define D0_PIN_NUMBER  1
#define D1_PIN_NUMBER  7
#define D2_PIN_NUMBER  5
#define D3_PIN_NUMBER  0
#define D4_PIN_NUMBER  4
#define D5_PIN_NUMBER  7
#define D6_PIN_NUMBER  6
#define D7_PIN_NUMBER  5
// Data Port
#define DATA0_PORT  GPIOB
#define DATA1_PORT  GPIOE
#define DATA2_PORT  GPIOC
#define DATA3_PORT  GPIOB
#define DATA4_PORT  GPIOC
#define DATA5_PORT  GPIOA
#define DATA6_PORT  GPIOA
#define DATA7_PORT  GPIOA
// Control Port
#define CTRL_PORT  GPIOE
// Select Column Port
#define SECO1_PORT  GPIOA
#define SECO2_PORT  GPIOA
// Control Pins
#define DI_PIN_NUMBER  10
#define RW_PIN_NUMBER  8
#define E_PIN_NUMBER   9
// Select Column Pins
#define CS1_PIN_NUMBER   4
#define CS2_PIN_NUMBER   3
// Reset Port
#define RESET_PORT  GPIOA
#define RESET_PIN_NUMBER   2

// GLCD Function Prototypes
void glcd_on(void);
void glcd_off(void);
void glcd_putchar(unsigned char data, unsigned char j);
void glcd_putImage(unsigned char data, unsigned char j);
void glcd_puts(char *str, char x, char y);
void glcd_puts_point(char x, char y, char w);
void glcd_disp_Image(unsigned char *str, char x1, char x2, char y1, char y2);
void glcd_gotoxy(unsigned char x, unsigned char y);
void glcd_clear_all(void);
void glcd_clearline(unsigned line);
void Delay(int t);
void send_command(unsigned char data);
void goto_row(unsigned char y);
void goto_column(unsigned char x);
void glcd_clear_here(char x1, char x2, char y1, char y2);

// isf func
void glcd_fill_rectangle(unsigned char start_column, unsigned char start_row,
                         unsigned char end_column, unsigned char end_row,
                         unsigned char fill_color);

void glcd_setPixel(uint8_t x, uint8_t y, uint8_t color);



void glcd_clear_text_area(char *str, uint8_t start_column, uint8_t start_row,
                          uint8_t end_column, uint8_t end_row);
void glcd_clear_text(char *str, char x, char y);


void glcd_puts_pixel_loc(char *str, char x, char y);
#endif /* INC_GLCD_H_ */
