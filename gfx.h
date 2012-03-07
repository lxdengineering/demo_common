#ifndef __GFX_H_
#define __GFX_H_

#include <stdint.h>

//
// Graphics primitives
//
// Currently, the routines assume a bitmap format that is in accordance
// with the ST7565 LCD controller.
//
// TODOs: - Allow for other pixel mapping arrangements.
//        - Provide a *.bmp translation util.

// gfxInit - Init some variables that the graphics routines
//           will need. Note the bitmapBuffer size is assumed to
//           be bitmapWidth * bitmapHeight / 8.
//
void gfxInit(int16_t bitmapWidth,    // Width  (pixels)
             int16_t bitmapHeight,   // Height (pixels)
             uint8_t  *bitmapBuffer); // Bitmap buffer

// The remainder of the routines will work on the bitmap buffer
// that is supplied in the call to initGfx. Most routines accept
// one or more x,y locations, and a color, where the color is
// 0 (zero) to clear, or 1 to draw.
//

void gfxFill(uint8_t fillValue);  // (Call with 0 to clear buffer)

void gfxPixel(int16_t x, int16_t y, uint8_t color);

void gfxLine(int16_t x0, int16_t y0,
             int16_t x1, int16_t y1,
             uint8_t color);

void gfxRect(int16_t x0, int16_t y0,
             int16_t x1, int16_t y1,
             uint8_t color);

void gfxFRect(int16_t x0, int16_t y0,
              int16_t x1, int16_t y1,
              uint8_t color);

void gfxCircle(int16_t x0, int16_t y0,
               int16_t r, uint8_t color);

void gfxFCircle(int16_t x0, int16_t y0,
                int16_t r, uint8_t color);

// These char and string routines plot 5x7 pixel characters at an x location
// (specified in pixels) and y location (specified in lines).
// Lines are 8 pixels high (one pixel spacing between 5x7 font).
void gfxChar(int16_t x,    // x location (in pixels)
             int16_t line, // y location (in 8-pixel lines)
             char c);       // The char to print
void gfxString(int16_t x, int16_t line, char *c);

#endif
