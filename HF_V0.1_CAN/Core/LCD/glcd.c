#include "glcd.h"

#include "FreeRTOS.h"
#include "stdint.h"

#define LCD_DELAY 300

uint8_t frameBuffer[128][8] = {0};

#include "FreeRTOS.h"
#include "glcd.h"
#include "main.h"
#include "task.h"
#include "timers.h"

void Delay(int t)
{
    t *= 10;
    while (t)
    {
        t--;
    }
}

void send_byte_2_LCD(uint8_t thebyte)
{
    if (thebyte & 0x01)
        LCD_DB0_HIGH;
    else
        LCD_DB0_LOW;
    if (thebyte & 0x02)
        LCD_DB1_HIGH;
    else
        LCD_DB1_LOW;
    if (thebyte & 0x04)
        LCD_DB2_HIGH;
    else
        LCD_DB2_LOW;
    if (thebyte & 0x08)
        LCD_DB3_HIGH;
    else
        LCD_DB3_LOW;
    if (thebyte & 0x10)
        LCD_DB4_HIGH;
    else
        LCD_DB4_LOW;
    if (thebyte & 0x20)
        LCD_DB5_HIGH;
    else
        LCD_DB5_LOW;
    if (thebyte & 0x40)
        LCD_DB6_HIGH;
    else
        LCD_DB6_LOW;
    if (thebyte & 0x80)
        LCD_DB7_HIGH;
    else
        LCD_DB7_LOW;
}

void send_command(unsigned char command)
{
    Delay(T);
    LCD_E_LOW;
    Delay(T);
    LCD_RW_LOW;
    Delay(T);
    LCD_DI_LOW;
    Delay(T);
    LCD_CS1_HIGH;
    Delay(T);
    LCD_CS2_HIGH;
    Delay(T);

    // Clear data lines
    send_byte_2_LCD(0);
    Delay(T);
    LCD_E_HIGH;
    Delay(T);

    // Set data lines with the command
    send_byte_2_LCD(command);
    Delay(T);
    LCD_E_LOW;
}

void glcd_on(void)
{
    Delay(T);
    send_command(0x3F);  // GLCD on
    send_command(0x40);  // SET Cursor on Y=0
    send_command(0xB8);  // SET Page on X=0, Line=0
    send_command(0xC0);  // Display Start Line=0xC0
}

void glcd_off(void)
{
    Delay(T);
    send_command(0x3E);
}

void glcd_putchar(unsigned char data, unsigned char j)
{
    if (j < 64)
    {
        LCD_E_LOW;
        Delay(T);
        LCD_RW_LOW;
        Delay(T);
        LCD_DI_HIGH;
        Delay(T);
        LCD_CS1_HIGH;
        Delay(T);
        LCD_CS2_LOW;
        Delay(T);

        // Clear data lines
        send_byte_2_LCD(0);

        Delay(T);
        LCD_E_HIGH;
        Delay(T);

        // Set data lines with the data
        send_byte_2_LCD(data);

        Delay(T);
        LCD_E_LOW;
    }
    else
    {
        LCD_E_LOW;
        Delay(T);
        LCD_RW_LOW;
        Delay(T);
        LCD_DI_HIGH;
        Delay(T);
        LCD_CS1_LOW;
        Delay(T);
        LCD_CS2_HIGH;
        Delay(T);

        // Clear data lines
        send_byte_2_LCD(0);
        Delay(T);
        LCD_E_HIGH;
        Delay(T);

        // Set data lines with the data
        send_byte_2_LCD(data);
        Delay(T);
        LCD_E_LOW;
    }
}

void glcd_puts(char *str, char x, char y)
{
    Delay(T);
    int i, j;
    while (*str != 0)
    {
        i = (8 * (*str));
        j = i + 7;
        for (; i <= j; i++)
        {
            glcd_gotoxy(x, y);
            glcd_putchar(Font[i], x);
            ++x;
            if (x > 121)
            {
                if (i % 8 == 0)
                {
                    ++y;
                    x = 0;
                }
            }
        }
        str++;
    }
}

void glcd_putImage(unsigned char data, unsigned char j)
{
    Delay(T);

    if (j < 64)
    {
        LCD_E_LOW;
        Delay(T);
        LCD_RW_LOW;
        Delay(T);
        LCD_DI_HIGH;
        Delay(T);
        LCD_CS1_HIGH;
        Delay(T);
        LCD_CS2_LOW;
        Delay(T);

        // Clear data lines
        send_byte_2_LCD(0);
        Delay(T);
        LCD_E_HIGH;
        Delay(T);

        // Set data lines with the value of data
        send_byte_2_LCD(data);

        Delay(T);
        LCD_E_LOW;
    }
    else
    {
        LCD_E_LOW;
        Delay(T);
        LCD_RW_LOW;
        Delay(T);
        LCD_DI_HIGH;
        Delay(T);
        LCD_CS1_LOW;
        Delay(T);
        LCD_CS2_HIGH;
        Delay(T);

        // Clear data lines
        send_byte_2_LCD(0);
        Delay(T);
        LCD_E_HIGH;
        Delay(T);

        // Set data lines with the value of data
        send_byte_2_LCD(data);
        Delay(T);
        LCD_E_LOW;
    }
}

void glcd_disp_Image(unsigned char *str, char x1, char x2, char y1, char y2)
{
    Delay(T);
    unsigned char i, j;
    for (i = y1; i <= y2; i++)
    {
        for (j = x1; j <= x2; j++)
        {
            glcd_gotoxy(j, i);
            glcd_putImage(*str, j);
            str++;
        }
    }
}

void glcd_puts_point(char x, char y, char w)
{
    unsigned char point = 0x00, dot, i = 0;
    dot = y % 8;
    y = (y / 8);
    if (dot == 0) y -= 1;
    for (i = 0; i < w; i++)
    {
        switch (dot)
        {
            case 0:
                point |= 0x80;
                break;
            case 1:
                point |= 0x01;
                break;
            case 2:
                point |= 0x02;
                break;
            case 3:
                point |= 0x04;
                break;
            case 4:
                point |= 0x08;
                break;
            case 5:
                point |= 0x10;
                break;
            case 6:
                point |= 0x20;
                break;
            case 7:
                point |= 0x40;
                break;
        }
        if (dot == 1)
        {
            glcd_gotoxy(x, y);
            glcd_putImage(point, x);
            y -= 1;
            point = 0x00;
        }
        if (dot == 0) dot = 8;
        dot -= 1;
    }
    if (point != 0x00)
    {
        glcd_gotoxy(x, y);
        glcd_putImage(point, x);
    }
}

void glcd_gotoxy(unsigned char x, unsigned char y)
{
    Delay(T);
    goto_row(y);
    goto_column(x);
}

void goto_row(unsigned char y)
{
    Delay(T);
    y = (y | 0xB8) & 0xBF;
    send_command(y);
}

void goto_column(unsigned char x)
{
    Delay(T);

    if (x < 64)
    {
        x = (x | 0x40) & 0x7F;
        LCD_E_LOW;
        Delay(T);
        LCD_RW_LOW;
        Delay(T);
        LCD_DI_LOW;
        Delay(T);
        LCD_CS1_HIGH;
        Delay(T);
        LCD_CS2_LOW;
        Delay(T);

        // Clear data lines
        send_byte_2_LCD(0);

        Delay(T);
        LCD_E_HIGH;
        Delay(T);

        // Set data lines with the value of x
        send_byte_2_LCD(x);

        Delay(T);
        LCD_E_LOW;
    }
    else
    {
        x -= 64;
        x = (x | 0x40) & 0x7F;
        LCD_E_LOW;
        Delay(T);
        LCD_RW_LOW;
        Delay(T);
        LCD_DI_LOW;
        Delay(T);
        LCD_CS1_LOW;
        Delay(T);
        LCD_CS2_HIGH;
        Delay(T);

        // Clear data lines
        send_byte_2_LCD(0);

        Delay(T);
        LCD_E_HIGH;
        Delay(T);

        // Set data lines with the value of x
        send_byte_2_LCD(x);
        Delay(T);
        LCD_E_LOW;
    }
}

void glcd_clearline(unsigned line)
{
    Delay(T);
    int i;
    glcd_gotoxy(0, line);  // At start of line of left side
    LCD_CS1_HIGH;
    Delay(T);
    LCD_CS2_LOW;

    for (i = 0; i < 64; i++)
    {
        glcd_putchar(0, 0);   // Print 0 for Delete Left section
        glcd_putchar(0, 64);  // Print 0 for Delete Right section
    }
    glcd_gotoxy(64, line);  // At start of line of right side
    for (i = 0; i < 64; i++)
    {
        glcd_putchar(0, 0);   // Print 0 for Delete Left section
        glcd_putchar(0, 64);  // Print 0 for Delete Right section
    }
}

void glcd_clear_all()
{
    int i;
    for (i = 0; i < 8; i++) glcd_clearline(i);
    send_command(0x40);  // SET Cursor on Y=0
    send_command(0xB8);  // SET Page on X=0, Line=0
}

void glcd_clear_here(char x1, char x2, char y1, char y2)
{
    Delay(T);
    unsigned char i, j;
    for (i = y1; i <= y2; i++)
    {
        for (j = x1; j <= x2; j++)
        {
            glcd_gotoxy(j, i);
            glcd_putImage(0, j);
        }
    }
}

void glcd_clear_text(char *str, char x, char y)
{
    Delay(T);  // Optional delay for timing if needed
    int i, j;
    while (*str != 0)
    {
        i = (8 * (*str));  // Start index for the current character in the font array
        j = i + 7;         // End index for the current character

        for (; i <= j; i++)
        {
            glcd_gotoxy(x, y);           // Move to position where the character is to be erased
            glcd_putImage(~Font[i], x);  // Turn off the pixels for the current character (0x00 means no pixels)
            ++x;                         // Move to the next horizontal position

            if (x > 121)
            {  // If x exceeds 121, wrap to the next line
                if (i % 8 == 0)
                {
                    ++y;
                    x = 0;
                }
            }
        }
        str++;  // Move to the next character in the string
    }
}
