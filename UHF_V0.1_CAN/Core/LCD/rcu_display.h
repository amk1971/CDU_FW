/*
 * rcu_display.h
 *
 *  Created on: Nov 28, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_RCU_DISPLAY_H_
#define INC_RCU_DISPLAY_H_

#include "glcd.h"
#include "stdio.h"
#include "stm32f1xx.h"

// Coordinate definitions (adjust as needed)
#define TL_X1 0
#define TL_X2 48
#define TL_Y1 0
#define TL_Y2 0

#define ML_X1 0
#define ML_X2 48
#define ML_Y1 3
#define ML_Y2 3

#define BL_X1 0
#define BL_X2 32
#define BL_Y1 6
#define BL_Y2 6

#define MT_X1 68
#define MT_X2 68
#define MT_Y1 0
#define MT_Y2 0

#define MM_X1 68
#define MM_X2 68
#define MM_Y1 3
#define MM_Y2 3

#define MB_X1 68
#define MB_X2 68
#define MB_Y1 6
#define MB_Y2 6

#define RT_X1 128
#define RT_X2 125
#define RT_Y1 0
#define RT_Y2 0

#define RM_X1 128
#define RM_X2 125
#define RM_Y1 3
#define RM_Y2 3

#define RB_X1 128
#define RB_X2 125
#define RB_Y1 6
#define RB_Y2 6

// Define X, Y coordinates for each region
typedef enum {
    TL,  // Top Left
    ML,  // Middle Left
    BL,  // Bottom Left
    MT,  // Middle Top
    MM,  // Middle Middle
    MB,  // Middle Bottom
    RT,  // Right Top
    RM,  // Right Middle
    RB   // Right Bottom
} LCD_Region;

// Enum to represent different data types
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_CUSTOM // For custom data types (e.g., structs)
} DataType;

/*-----------------------------------------------------------------------------------------------
                                  **advance glcd function**
------------------------------------------------------------------------------------------------*/

void GLCD_INIT(void);
void glcd_axis(void);
void glcd_plot(float number, char step);
void GLCD_INIT(void);
void line(int x1, int x2, int y);
void LCD_power_on(void);
void LCD_power_off(void);
void LCD_Print_Home(void);
void LCD_PRINT_MEM_SCREEN(uint8_t page);
void LCD_PRINT_CURSOR(uint8_t status,uint8_t cursor_location);

void LCD_UpdateRegion(LCD_Region region, const char* value);
char* convertToString(void* data, DataType type);
#endif /* INC_RCU_DISPLAY_H_ */
