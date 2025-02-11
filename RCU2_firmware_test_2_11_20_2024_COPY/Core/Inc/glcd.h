#ifndef _GLCD_H
#define _GLCD_H

#include "font.h"
#include "stm32f1xx.h"

#define T  1
// Data Pins
#define D0_PIN_NUMBER  13
#define D1_PIN_NUMBER  14
#define D2_PIN_NUMBER  11
#define D3_PIN_NUMBER  12
#define D4_PIN_NUMBER  10
#define D5_PIN_NUMBER  9
#define D6_PIN_NUMBER  8
#define D7_PIN_NUMBER  7
// Data Port
#define DATA0_PORT  GPIOE
#define DATA1_PORT  GPIOE
#define DATA2_PORT  GPIOE
#define DATA3_PORT  GPIOE
#define DATA4_PORT  GPIOE
#define DATA5_PORT  GPIOE
#define DATA6_PORT  GPIOE
#define DATA7_PORT  GPIOE
// Control Port
#define CTRL_PORT  GPIOB
// Select Column Port
#define SECO1_PORT  GPIOB
#define SECO2_PORT  GPIOB
// Control Pins
#define DI_PIN_NUMBER  11
#define RW_PIN_NUMBER  15
#define E_PIN_NUMBER   10
// Select Column Pins
#define CS1_PIN_NUMBER   1
#define CS2_PIN_NUMBER   0
// Reset Port
#define RESET_PORT  GPIOC
#define RESET_PIN_NUMBER   5

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

#endif

