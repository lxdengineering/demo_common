//
// gfx.c - Simple graphics primitives
//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gfx.h"

// Byte swap macro
#define swap(a, b) { uint8_t t = a; a = b; b = t; }

// 
// Private variables
//
static int16_t bmapWidth  = 0;  // Bitmap width (pixels)
static int16_t bmapHeight = 0;  // Bitmap height (pixels)
static uint8_t *bmap;            // The bitmap buffer
static int16_t bmapSize;        // Size of the bitmap buffer (bytes)

// 5x7 pixel character font definitions
extern uint8_t font[];

// "Constructor" - Init size variables, pointer to active bitmap buffer, and
//                 clear the buffer.
void gfxInit(int16_t width, int16_t height, uint8_t *_bmap)
{
	bmapWidth = width;
	bmapHeight = height;
    bmap = _bmap;
    bmapSize = width * height / 8;  // Byte size of bitmap buffer

    gfxFill(0);  // Clear buffer
}


// Fill bitmap buffer (clear with gfxFill(0));
void gfxFill(uint8_t fillValue) {
    memset(bmap, fillValue, bmapSize);
}

// gfxPixel()
//
// Given x,y coords in pixels, with top-left being 0,0, set/clear the
// appropriate bit in a bitmap buffer to turn-on (or off) that pixel.
//
// This version of gfxPixel has byte orientation arranged in "ST7565"
// LCD controller format. TODO: arrange for other formats? Provide
// conversion from *.bmp to this format?
//
void gfxPixel(int16_t x, int16_t y, uint8_t color)
{
    int16_t byteIdx;
    uint8_t bitMask;

    if(x < 0 || x >= bmapWidth) return;   // Range check
    if(y < 0 || y >= bmapHeight) return;

    byteIdx = y / 8 * bmapWidth + x;
    bitMask = 0x80 >> (y & 7);

    if(color)
        bmap[byteIdx] |= bitMask;   // color != 0: Set the pixel
    else
        bmap[byteIdx] &= ~bitMask;  // color == 0: Clear the pixel
}


//
//  Plot a character from the 5x8 font array, at the specified location.
//
void gfxChar(int16_t x,      // Starting x location (in pixels)
             int16_t line,   // Starting line (0..7)
             char c)          // Character
{
    int16_t i, bmapIdx, fontIdx;

    bmapIdx = (int16_t)line * bmapWidth + x;
    fontIdx = (int16_t)c * 5;

    for (i =0; i<5; i++ ) // 5 bytes per character
    {
        bmap[bmapIdx++] = font[fontIdx++];
    }
}


void gfxString(int16_t x, int16_t line, char *c)
{
    while(*c)  // Until string null-terminator...
    {
        gfxChar(x, line, *c++); // Plot one character
        x += 6;                       // x-position for next char
        if (x + 6 >= bmapWidth)       // Will it fit on this line?
        {
            x = 0;                    // If not, go to next line
            line++;
        }
        if (line >= (bmapHeight/8))   // All out of lines?
            return;                   // If so, quit.
    }
}

// Draw a line (using Bresenham's line algorithm)
// TODO: Should the line include the endpoint?
//
void gfxLine(int16_t x0, int16_t y0,
             int16_t x1, int16_t y1,
             uint8_t color) 
{
    int16_t dx, dy;
    char     steep;

    steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }
    
    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }
    
    dx = x1 - x0;
    dy = abs(y1 - y0);
    
    int16_t err = dx / 2;
    int8_t  ystep;
    
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;}
    
    while(x0 < x1)
    {
        if (steep) {
            gfxPixel(y0, x0, color);
        } else {
            gfxPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
        x0++;
    }
}

//
// Rectangle
//
void gfxRect(int16_t x0, int16_t y0,
             int16_t x1, int16_t y1,
             uint8_t color)
{
    gfxLine(x0,y0, x1,y0, color);
    gfxLine(x0,y1, x1,y1, color);
    gfxLine(x0,y0, x0,y1, color);
    // The line routine does not include the last point. The extra "+1"
    // in the call below is required to fill in a missing pixel on the last line.
    gfxLine(x1,y0, x1,y1+1, color);
}

//
// Filled rectangle
//
void gfxFRect(int16_t x0, int16_t y0,
			  int16_t x1, int16_t y1,
              uint8_t color)
{
    int16_t i,j;

    for (i=x0; i<=x1; i++) {
        for (j=y0; j<=y1; j++) {
            gfxPixel(i, j, color);
        }
    }
}


void gfxCircle(int16_t x0, int16_t y0,  // Center coord
               int16_t r,                // Radius
               uint8_t color)
{
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    gfxPixel(x0, y0+r, color);
    gfxPixel(x0, y0-r, color);
    gfxPixel(x0+r, y0, color);
    gfxPixel(x0-r, y0, color);

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
  
        gfxPixel(x0 + x, y0 + y, color);
        gfxPixel(x0 - x, y0 + y, color);
        gfxPixel(x0 + x, y0 - y, color);
        gfxPixel(x0 - x, y0 - y, color);
    
        gfxPixel(x0 + y, y0 + x, color);
        gfxPixel(x0 - y, y0 + x, color);
        gfxPixel(x0 + y, y0 - x, color);
        gfxPixel(x0 - y, y0 - x, color);
    }
}

// Filled circle
void gfxFCircle(int16_t x0, int16_t y0, // Center coord
                int16_t r,               // Radius
                uint8_t color)
{
    uint8_t i;

    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    for (i=y0-r; i<=y0+r; i++) {
        gfxPixel(x0, i, color);
    }

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
  
        for (i=y0-y; i<=y0+y; i++) {
            gfxPixel(x0+x, i, color);
            gfxPixel(x0-x, i, color);
        }
        for (i=y0-x; i<=y0+x; i++) {
            gfxPixel(x0+y, i, color);
            gfxPixel(x0-y, i, color);
        }
    }
}

