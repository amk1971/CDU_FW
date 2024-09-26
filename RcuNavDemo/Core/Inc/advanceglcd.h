#ifndef _ADVANCEGLCD_H
#define _ADVANCEGLCD_H

#include "stm32f4xx.h"
#include "glcd.h"
#include "stdio.h"

/*-----------------------------------------------------------------------------------------------
                                  **advance glcd function**
------------------------------------------------------------------------------------------------*/
void GLCD_INIT( void );
void glcd_axis(void);
void glcd_plot(float number,char step);
void GLCD_INIT( void );
void line (int x1,int x2,int y);
/*-----------------------------------------------------------------------------------------------
                                  **function code**
------------------------------------------------------------------------------------------------*/

void glcd_axis(void){
	//make x axis----------------------
	for (char x=0;x<=127;x++)
	{
		glcd_puts_point (x,0,1);	
	}
	//make y axis----------------------
	for (char y=0;y<=7;y++)
	{
		glcd_gotoxy (0,y);
		glcd_putchar (0xff,0);
	}
}
//--------------------------------------
void glcd_plot(float number,char step){
	Delay(T);
	char a,s;
	s=step*4;
	
//print point on coordinates---------
	for (a=0;a<=32;a++)
	{
		if(number <=(0.03230*a) && number>(0.03230*(a-1)))
		{
			glcd_puts_point (s,64-(a*2),2);
			glcd_puts_point (s+1,64-(2*a),2);
		}
	}
}
//-------------------------------------
//Function For Initialize GLCD Pins
// Function For Initialize GLCD Pins
void GLCD_INIT(void) {
//    // Enable clock for GPIO ports A, B, C, D, and E
//    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;
//
//    // Set the pin modes to output for DATA_PORT and CTRL_PORT
//    // GPIOA: D5, D6, D7 (PA7, PA6, PA5), CS1 (PA4), CS2 (PA3), RESET (PA2)
//    DATA5_PORT->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
//    DATA5_PORT->CRL |= GPIO_CRL_MODE7_0;
//    DATA6_PORT->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6);
//    DATA6_PORT->CRL |= GPIO_CRL_MODE6_0;
//    DATA7_PORT->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
//    DATA7_PORT->CRL |= GPIO_CRL_MODE5_0;
//
//    SECO1_PORT->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4 | GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
//    SECO1_PORT->CRL |= (GPIO_CRL_MODE4_0 | GPIO_CRL_MODE3_0);
//
//    RESET_PORT->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
//    RESET_PORT->CRL |= GPIO_CRL_MODE2_0;
//
//    // GPIOB: D0, D3 (PB1, PB0)
//    DATA0_PORT->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 | GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
//    DATA0_PORT->CRL |= (GPIO_CRL_MODE1_0 | GPIO_CRL_MODE0_0);
//
//    // GPIOE: D1, DI, RW, E (PE7, PE10, PE8, PE9)
//    DATA1_PORT->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
//    DATA1_PORT->CRL |= GPIO_CRL_MODE7_0;
//    CTRL_PORT->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10 | GPIO_CRH_MODE8 | GPIO_CRH_CNF8 | GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
//    CTRL_PORT->CRH |= (GPIO_CRH_MODE10_0 | GPIO_CRH_MODE8_0 | GPIO_CRH_MODE9_0);
//
//    // GPIOC: D2, D4 (PC5, PC4)
//    DATA2_PORT->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
//    DATA2_PORT->CRL |= GPIO_CRL_MODE5_0;
//    DATA4_PORT->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
//    DATA4_PORT->CRL |= GPIO_CRL_MODE4_0;
//
//    // Reset data and control ports
//    DATA0_PORT->BSRR = (1 << (D0_PIN_NUMBER + 16));
//    DATA1_PORT->BSRR = (1 << (D1_PIN_NUMBER + 16));
//    DATA2_PORT->BSRR = (1 << (D2_PIN_NUMBER + 16));
//    DATA3_PORT->BSRR = (1 << (D3_PIN_NUMBER + 16));
//    DATA4_PORT->BSRR = (1 << (D4_PIN_NUMBER + 16));
//    DATA5_PORT->BSRR = (1 << (D5_PIN_NUMBER + 16));
//    DATA6_PORT->BSRR = (1 << (D6_PIN_NUMBER + 16));
//    DATA7_PORT->BSRR = (1 << (D7_PIN_NUMBER + 16));
//    SECO1_PORT->BSRR = (1 << (CS1_PIN_NUMBER + 16));
//    SECO2_PORT->BSRR = (1 << (CS2_PIN_NUMBER + 16));
//    RESET_PORT->BSRR = (1 << (RESET_PIN_NUMBER + 16));
//
//    // Reset the GLCD
//    RESET_PORT->BSRR = (1 << RESET_PIN_NUMBER); // Set reset pin high
//    Delay(10); // Wait for a short period
//    RESET_PORT->BSRR = (1 << (RESET_PIN_NUMBER + 16)); // Set reset pin low
//    Delay(10); // Wait for a short period
//    RESET_PORT->BSRR = (1 << RESET_PIN_NUMBER); // Set reset pin high
    Delay(10); // Wait for a short period
}



//----------------------------------------------------------------------------------------------
 void glcd_line(int x1,int x2,int y)
 {
	 for(int k=x1;k<=x2;k++)
 {
			static char arr[]="-";
			glcd_puts(arr,k,y);

 } 
 }
 //---------------------------------------------------------------------------------------------
 
 
 
#endif
