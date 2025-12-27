/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: You
 */ 

#include "LCD_GFX.h"
#include "ST7735.h"
#include <stdlib.h>

/******************************************************************************
* Local Functions
******************************************************************************/



/******************************************************************************
* Global Functions
******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((((31*(red+4))/255)<<11) | (((63*(green+2))/255)<<5) | ((31*(blue+4))/255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
	LCD_setAddr(x,y,x,y);
	SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
* @brief		Draw a character starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor){
	uint16_t row = character - 0x20;		//Determine row of ASCII table starting at space
	int i, j;
	if ((LCD_WIDTH-x>7)&&(LCD_HEIGHT-y>7)){
		for(i=0;i<5;i++){
			uint8_t pixels = ASCII[row][i]; //Go through the list of pixels
			for(j=0;j<8;j++){
				if ((pixels>>j)&1==1){
					LCD_drawPixel(x+i,y+j,fColor);
				}
				else {
					LCD_drawPixel(x+i,y+j,bColor);
				}
			}
		}
	}
}


/******************************************************************************
* LAB 4 TO DO. COMPLETE THE FUNCTIONS BELOW.
* You are free to create and add any additional files, libraries, and/or
*  helper function. All code must be authentically yours.
******************************************************************************/

/**************************************************************************//**
* @fn			void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
* @brief		Draw a colored circle of set radius at coordinates
* @note         i used the midpoint circle algorithm (8-way symmetry)
                                                                             
*****************************************************************************/
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color)
{
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    // Initial 4 cardinal points
    LCD_drawPixel(x0, y0 + radius, color);
    LCD_drawPixel(x0, y0 - radius, color);
    LCD_drawPixel(x0 + radius, y0, color);
    LCD_drawPixel(x0 - radius, y0, color);

    // Midpoint circle algorithm
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        // 8-way symmetry points
        LCD_drawPixel(x0 + x, y0 + y, color);
        LCD_drawPixel(x0 - x, y0 + y, color);
        LCD_drawPixel(x0 + x, y0 - y, color);
        LCD_drawPixel(x0 - x, y0 - y, color);
        LCD_drawPixel(x0 + y, y0 + x, color);
        LCD_drawPixel(x0 - y, y0 + x, color);
        LCD_drawPixel(x0 + y, y0 - x, color);
        LCD_drawPixel(x0 - y, y0 - x, color);
    }
}



/**************************************************************************//**
* @fn			void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
* @brief		Draw a line from and to a point with a color
* @note
*****************************************************************************/
/**************************************************************************//**
* @fn			void LCD_drawLine(short x0, short y0, short x1, short y1, uint16_t color)
* @brief		Draws a line between (x0,y0) and (x1,y1) using Bresenham algorithm
* @note			Uses LCD_drawPixel() for plotting
*****************************************************************************/
/**************************************************************************//**
* @fn			void LCD_drawLine(short x0, short y0, short x1, short y1, uint16_t c)
* @brief		Draw a line from (x0, y0) to (x1, y1) safely within LCD bounds Bresenham’s algorithm
******************************************************************************/
void LCD_drawLine(short x0, short y0, short x1, short y1, uint16_t c)
{
    // --- Clamp coordinates to LCD boundaries ---
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x0 >= LCD_WIDTH)  x0 = LCD_WIDTH - 1;
    if (x1 >= LCD_WIDTH)  x1 = LCD_WIDTH - 1;
    if (y0 >= LCD_HEIGHT) y0 = LCD_HEIGHT - 1;
    if (y1 >= LCD_HEIGHT) y1 = LCD_HEIGHT - 1;

  
    short dx = abs(x1 - x0);
    short sx = (x0 < x1) ? 1 : -1;
    short dy = -abs(y1 - y0);
    short sy = (y0 < y1) ? 1 : -1;
    short err = dx + dy;

    while (1)
    {
        LCD_drawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1)
            break;

        short e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}






/**************************************************************************//**
* @fn			void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
* @brief		Draw a colored block at coordinates
* @note
*****************************************************************************/
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
    if (x1 < x0) { uint8_t t = x0; x0 = x1; x1 = t; }
    if (y1 < y0) { uint8_t t = y0; y0 = y1; y1 = t; }
    if (x0 >= LCD_WIDTH)  return;
    if (y0 >= LCD_HEIGHT) return;
    if (x1 >= LCD_WIDTH)  x1 = LCD_WIDTH - 1;
    if (y1 >= LCD_HEIGHT) y1 = LCD_HEIGHT - 1;

    uint16_t w = (uint16_t)(x1 - x0 + 1);
    uint16_t h = (uint16_t)(y1 - y0 + 1);
    uint32_t n = (uint32_t)w * (uint32_t)h;

    LCD_setAddr(x0, y0, x1, y1);
    while (n--) {
        SPI_ControllerTx_16bit(color);
    }
}

/**************************************************************************//**
* @fn      void LCD_setScreen(uint16_t color)
* @brief   Fill entire screen with a solid color
******************************************************************************/
void LCD_setScreen(uint16_t color) 
{
    LCD_drawBlock(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, color);
}

/**************************************************************************//**
* @fn			void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
* @brief		Draw a string starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
{
    uint8_t startX = x;  

    while (*str)  
    {
      
        if (*str == '\n') {
            y += 8;          // move down one character height
            x = startX;      // reset to start of line
        }
        else {
            LCD_drawChar(x, y, *str, fg, bg);  // draw the character
            x += 6; // move cursor right (~5px font + 1px space)
        }

  
        if (x + 6 >= LCD_WIDTH) {
            x = startX;
            y += 8;  
        }
        if (y + 8 >= LCD_HEIGHT)
            break;  

        str++;  
    }
}