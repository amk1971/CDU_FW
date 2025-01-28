#include "glcd.h"
#include "stdint.h"
#include"FreeRTOS.h"


#define LCD_DELAY 300

uint8_t frameBuffer[128][8]={0};

void Delay(int t) {
    t *= 10;
    while (t) {
        t--;
    }
}

void send_command(unsigned char command) {
    Delay(T);
    CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER);  // E_PIN_NUMBER=RESET
    Delay(T);
    CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER); // RW_PIN_NUMBER=RESET
    Delay(T);
    CTRL_PORT->ODR &= ~(1 << DI_PIN_NUMBER); // DI_PIN_NUMBER=RESET
    Delay(T);
    SECO1_PORT->ODR |= (1 << CS1_PIN_NUMBER); // CS1_PIN_NUMBER=SET
    Delay(T);
    SECO2_PORT->ODR |= (1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=SET
    Delay(T);

    // Clear data lines
    DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
    DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
    DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
    DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
    DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
    DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
    DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
    DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

    Delay(T);
    CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
    Delay(T);

    // Set data lines with the command
    if (command & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
    if (command & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
    if (command & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
    if (command & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
    if (command & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
    if (command & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
    if (command & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
    if (command & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

    Delay(T);
    CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
}

void glcd_on(void) {
    Delay(T);
    send_command(0x3F);     // GLCD on
    send_command(0x40);     // SET Cursor on Y=0
    send_command(0xB8);     // SET Page on X=0, Line=0
    send_command(0xC0);     // Display Start Line=0xC0
}

void glcd_off(void) {
    Delay(T);
    send_command(0x3E);
}

void glcd_putchar(unsigned char data, unsigned char j) {
    if (j < 64) {
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER);   // E_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER);  // RW_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR |= (1 << DI_PIN_NUMBER);   // DI_PIN_NUMBER=SET
        Delay(T);
        SECO1_PORT->ODR |= (1 << CS1_PIN_NUMBER); // CS1_PIN_NUMBER=SET
        Delay(T);
        SECO2_PORT->ODR &= ~(1 << CS2_PIN_NUMBER);// CS2_PIN_NUMBER=RESET
        Delay(T);

        // Clear data lines
        DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
        DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
        DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
        DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
        DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
        DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
        DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
        DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
        Delay(T);

        // Set data lines with the data
        if (data & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
        if (data & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
        if (data & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
        if (data & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
        if (data & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
        if (data & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
        if (data & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
        if (data & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
    } else {
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER);   // E_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER);  // RW_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR |= (1 << DI_PIN_NUMBER);   // DI_PIN_NUMBER=SET
        Delay(T);
        SECO1_PORT->ODR &= ~(1 << CS1_PIN_NUMBER);// CS1_PIN_NUMBER=RESET
        Delay(T);
        SECO2_PORT->ODR |= (1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=SET
        Delay(T);

        // Clear data lines
        DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
        DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
        DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
        DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
        DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
        DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
        DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
        DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
        Delay(T);

        // Set data lines with the data
        if (data & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
        if (data & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
        if (data & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
        if (data & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
        if (data & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
        if (data & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
        if (data & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
        if (data & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
    }
}

void glcd_puts(char *str, char x, char y) {
    Delay(T);
    int i, j;
    while (*str != 0) {
        i = (8 * (*str));
        j = i + 7;
        for (; i <= j; i++) {
            glcd_gotoxy(x, y);
            glcd_putchar(Font[i], x);
            ++x;
            if (x > 121) {
                if (i % 8 == 0) {
                    ++y;
                    x = 0;
                }
            }
        }
        str++;
    }
}

void glcd_putImage(unsigned char data, unsigned char j) {
    Delay(T);

    if (j < 64) {
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER); // RW_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR |= (1 << DI_PIN_NUMBER); // DI_PIN_NUMBER=SET
        Delay(T);
        SECO1_PORT->ODR |= (1 << CS1_PIN_NUMBER); // CS1_PIN_NUMBER=SET
        Delay(T);
        SECO2_PORT->ODR &= ~(1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=RESET
        Delay(T);

        // Clear data lines
        DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
        DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
        DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
        DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
        DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
        DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
        DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
        DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
        Delay(T);

        // Set data lines with the value of data
        if (data & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
        if (data & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
        if (data & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
        if (data & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
        if (data & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
        if (data & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
        if (data & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
        if (data & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
    } else {
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER); // RW_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR |= (1 << DI_PIN_NUMBER); // DI_PIN_NUMBER=SET
        Delay(T);
        SECO1_PORT->ODR &= ~(1 << CS1_PIN_NUMBER); // CS1_PIN_NUMBER=RESET
        Delay(T);
        SECO2_PORT->ODR |= (1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=SET
        Delay(T);

        // Clear data lines
        DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
        DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
        DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
        DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
        DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
        DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
        DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
        DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
        Delay(T);

        // Set data lines with the value of data
        if (data & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
        if (data & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
        if (data & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
        if (data & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
        if (data & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
        if (data & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
        if (data & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
        if (data & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
    }
}

void glcd_disp_Image(unsigned char *str, char x1, char x2, char y1, char y2) {
    Delay(T);
    unsigned char i, j;
    for (i = y1; i <= y2; i++) {
        for (j = x1; j <= x2; j++) {
            glcd_gotoxy(j, i);
            glcd_putImage(*str, j);
            str++;
        }
    }
}



void glcd_puts_point(char x, char y, char w) {
    unsigned char point = 0x00, dot, i = 0;
    dot = y % 8;
    y = (y / 8);
    if (dot == 0) y -= 1;
    for (i = 0; i < w; i++) {
        switch (dot) {
            case 0: point |= 0x80; break;
            case 1: point |= 0x01; break;
            case 2: point |= 0x02; break;
            case 3: point |= 0x04; break;
            case 4: point |= 0x08; break;
            case 5: point |= 0x10; break;
            case 6: point |= 0x20; break;
            case 7: point |= 0x40; break;
        }
        if (dot == 1) {
            glcd_gotoxy(x, y);
            glcd_putImage(point, x);
            y -= 1;
            point = 0x00;
        }
        if (dot == 0) dot = 8;
        dot -= 1;
    }
    if (point != 0x00) {
        glcd_gotoxy(x, y);
        glcd_putImage(point, x);
    }
}

void glcd_gotoxy(unsigned char x, unsigned char y) {
    Delay(T);
    goto_row(y);
    goto_column(x);
}

void goto_row(unsigned char y) {
    Delay(T);
    y = (y | 0xB8) & 0xBF;
    send_command(y);
}

void goto_column(unsigned char x) {
    Delay(T);

    if (x < 64) {
        x = (x | 0x40) & 0x7F;
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER); // RW_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << DI_PIN_NUMBER); // DI_PIN_NUMBER=RESET
        Delay(T);
        SECO1_PORT->ODR |= (1 << CS1_PIN_NUMBER); // CS1_PIN_NUMBER=SET
        Delay(T);
        SECO2_PORT->ODR &= ~(1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=RESET
        Delay(T);

        // Clear data lines
        DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
        DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
        DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
        DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
        DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
        DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
        DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
        DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
        Delay(T);

        // Set data lines with the value of x
        if (x & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
        if (x & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
        if (x & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
        if (x & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
        if (x & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
        if (x & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
        if (x & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
        if (x & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
    } else {
        x -= 64;
        x = (x | 0x40) & 0x7F;
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << RW_PIN_NUMBER); // RW_PIN_NUMBER=RESET
        Delay(T);
        CTRL_PORT->ODR &= ~(1 << DI_PIN_NUMBER); // DI_PIN_NUMBER=RESET
        Delay(T);
        SECO1_PORT->ODR &= ~(1 << CS1_PIN_NUMBER); // CS1_PIN_NUMBER=RESET
        Delay(T);
        SECO2_PORT->ODR |= (1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=SET
        Delay(T);

        // Clear data lines
        DATA0_PORT->ODR &= ~(1 << D0_PIN_NUMBER);
        DATA1_PORT->ODR &= ~(1 << D1_PIN_NUMBER);
        DATA2_PORT->ODR &= ~(1 << D2_PIN_NUMBER);
        DATA3_PORT->ODR &= ~(1 << D3_PIN_NUMBER);
        DATA4_PORT->ODR &= ~(1 << D4_PIN_NUMBER);
        DATA5_PORT->ODR &= ~(1 << D5_PIN_NUMBER);
        DATA6_PORT->ODR &= ~(1 << D6_PIN_NUMBER);
        DATA7_PORT->ODR &= ~(1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR |= (1 << E_PIN_NUMBER); // E_PIN_NUMBER=SET
        Delay(T);

        // Set data lines with the value of x
        if (x & 0x01) DATA0_PORT->ODR |= (1 << D0_PIN_NUMBER);
        if (x & 0x02) DATA1_PORT->ODR |= (1 << D1_PIN_NUMBER);
        if (x & 0x04) DATA2_PORT->ODR |= (1 << D2_PIN_NUMBER);
        if (x & 0x08) DATA3_PORT->ODR |= (1 << D3_PIN_NUMBER);
        if (x & 0x10) DATA4_PORT->ODR |= (1 << D4_PIN_NUMBER);
        if (x & 0x20) DATA5_PORT->ODR |= (1 << D5_PIN_NUMBER);
        if (x & 0x40) DATA6_PORT->ODR |= (1 << D6_PIN_NUMBER);
        if (x & 0x80) DATA7_PORT->ODR |= (1 << D7_PIN_NUMBER);

        Delay(T);
        CTRL_PORT->ODR &= ~(1 << E_PIN_NUMBER); // E_PIN_NUMBER=RESET
    }
}

void glcd_clearline(unsigned line) {
    Delay(T);
    int i;
    glcd_gotoxy(0, line); // At start of line of left side
    SECO1_PORT->ODR = SECO1_PORT->ODR | 1 << (CS1_PIN_NUMBER); // CS1_PIN_NUMBER=SET
    Delay(T);
    SECO2_PORT->ODR = SECO2_PORT->ODR & ~(1 << CS2_PIN_NUMBER); // CS2_PIN_NUMBER=RESET

    for (i = 0; i < 64; i++) {
        glcd_putchar(0, 0); // Print 0 for Delete Left section
        glcd_putchar(0, 64); // Print 0 for Delete Right section
    }
    glcd_gotoxy(64, line); // At start of line of right side
    for (i = 0; i < 64; i++) {
        glcd_putchar(0, 0); // Print 0 for Delete Left section
        glcd_putchar(0, 64); // Print 0 for Delete Right section
    }
}

void glcd_clear_all() {
    int i;
    for (i = 0; i < 8; i++)
        glcd_clearline(i);
    send_command(0x40); // SET Cursor on Y=0
    send_command(0xB8); // SET Page on X=0, Line=0

}

void glcd_clear_here(char x1, char x2, char y1, char y2) {
    Delay(T);
    unsigned char i, j;
    for (i = y1; i <= y2; i++) {
        for (j = x1; j <= x2; j++) {
            glcd_gotoxy(j, i);
            glcd_putImage(0, j);
        }
    }
}




// isf functions



void glcd_fill_rectangle(unsigned char start_column, unsigned char start_row, unsigned char end_column,
                         unsigned char end_row, unsigned char fill_color)
{
    for (unsigned char row = start_row; row <= end_row; row++)
    {
        for (unsigned char column = start_column; column <= end_column; column++)
        {
            glcd_gotoxy(column, row);          // Move to position (column, row)
            glcd_putImage(fill_color, column); // Fill with specified color (0xFF or 0x00)
        }
    }
}


void glcd_puts_pixel_loc(char *str, char x, char y) {
    Delay(T);
    while (*str != 0) {
        // Get the starting index of the current character in the font array
        int char_index = 8 * (*str);  // 8 bits per character
        int char_end = char_index + 7;  // We have 8 bits per character (height of the character)

        // Loop through each column of the character's bitmap
        for (int i = char_index; i <= char_end; i++) {
            uint8_t pixel_data = Font[i];  // Get the byte representing the column of the character

            // Loop through each bit in the byte (this corresponds to individual pixels)
            for (int bit = 0; bit < 8; bit++) {
                // If the bit is set, it means the pixel is ON
                if (pixel_data & (1 << bit)) {
                    // Calculate the exact pixel position for this bit (x, y)
                    uint8_t pixel_x = x + bit;
                    uint8_t pixel_y = y;
                    glcd_gotoxy(pixel_x, pixel_y);
                    glcd_putImage(0, pixel_y);
                    // Call your function to turn on the pixel at (pixel_x, pixel_y)
//                    glcd_setPixel(pixel_x, pixel_y, 1);  // Turn on the pixel
                }
            }
            // Move to the next character column
            x++;
        }

        // Move to the next character (based on the character width)
        x += CHAR_WIDTH;

        str++;  // Move to the next character in the string
    }
}


void glcd_clear_text(char *str, char x, char y) {
    Delay(T);  // Optional delay for timing if needed
    int i, j;
    while (*str != 0) {
        i = (8 * (*str));   // Start index for the current character in the font array
        j = i + 7;          // End index for the current character

        for (; i <= j; i++) {
            glcd_gotoxy(x, y);        // Move to position where the character is to be erased
            glcd_putImage(~Font[i], x);   // Turn off the pixels for the current character (0x00 means no pixels)
            ++x;  // Move to the next horizontal position

            if (x > 121) {            // If x exceeds 121, wrap to the next line
                if (i % 8 == 0) {
                    ++y;
                    x = 0;
                }
            }
        }
        str++;  // Move to the next character in the string
    }
}



