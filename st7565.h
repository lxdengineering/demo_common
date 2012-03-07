#ifndef __ST7565_H__
#define __ST7565_H__
//
// ST7565 - Sitronix ST7565x 65x132 Dot Matrix LCD  Controller/Driver
//

#include <stdint.h>

// Commands
//
// Some commands occupy a varying number of MS bits, with optional
// "arguments" contained in the lower bits. Ranges for "args"
// are shown in comments if applicable.
// Note: Enter sleep mode with Display-off and All-points-on cmds.
//
#define cDISPLAY_OFF      0xAE
#define cDISPLAY_ON       0xAF
#define cDISP_START_LINE  0x40  /* Set start line, 0..63    */
#define cPAGE             0xB0  /* Set page, 0..8           */
#define cCOL_MS           0x10  /* Set col, MS nybble       */
#define cCOL_LS           0x00  /* Sel col, LS nybble       */
#define cADC_NORMAL       0xA0  
#define cADC_REVERSE      0xA1  /* Reverse segment order    */
#define cDISP_NORMAL      0xA6  
#define cDISP_REVERSE     0xA7  /* Reverse lit/unlit pixels */
#define cALLPTS_NORMAL    0xA4  
#define cALLPTS_ON        0xA5  /* All pixels on            */
#define cBIAS_9           0xA2  /* Voltage bias ratio, 1/9  */
#define cBIAS_7           0xA3  /* Voltage bias ratio, 1/7  */
#define cRMW_BEGIN        0xE0  /* Enter Read/Modify/Write mode */
#define cRMW_END          0xEE  /* Exit  Read/Modify/Write mode */
#define cRESET            0xE2  /* Reset most things (disp RAM not affected)*/
#define cCOM_NORMAL       0xC0  
#define cCOM_REVERSE      0xC8  /* Reverse Com scan direction               */
#define cPOWER_CONTROL    0x28  /* D2:Booster; D1:Vreg; D0:Vfollower        */
#define cRESISTOR_RATIO   0x20  /* Set resistor ratio,          0..7        */
#define cVOLUME           0x81  /* 2-byte cmd: Set Vlcd (Vo). Next byte...  */
                                /*    volume-register 0..63                 */
#define cSLEEP_ENTER      0xAC  /* 2-byte cmd: Enter sleep mode (follow with 0) */
#define cSLEEP_EXIT       0xAD  /* 2-byte cmd: Exit sleep mode  (follow with 0) */

#define cBOOSTRATIO       0xF8  /* 2-byte cmd: Set V boost. Next byte...    */
                                /*    0: 2x,3x, or 4x                       */
                                /*    1: 5x                                 */
                                /*    2: 6x                                 */
#define cNOP              0xE3

// lcdInit()
//
// Inputs are "contrast" parameters: The ST7565's resistor-ratio
// and volume settings.
//
void lcdInit(
    uint8_t resistorRatio,   // Sets ST7565's resistor ratio, 0..7
    uint8_t volume);         // Sets ST7565's "volume" (contrast?), 0..0x3F


// Send one of the LCD command bytes, as defined above
uint8_t lcdCmd(uint8_t cmd);

// Send display data, one byte.
uint8_t lcdData(uint8_t data);

// Send display data, an array of bytes.
uint8_t lcdDataArray(const uint8_t data[], int n);

// LCD read-status and read-data. Only available in parallel mode (when
// operating in serial mode, the ST7565 is write-only).
uint8_t lcdReadStatus(void);
uint8_t lcdReadData(void);

// Clear the LCD's display
void    lcdClear(void);

// Copy a bitmap from memory to the LCD
void    lcdWriteBuffer(const uint8_t *buff);


#endif
