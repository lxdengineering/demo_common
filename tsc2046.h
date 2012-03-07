#ifndef __TSC2046_H__
#define __TSC2046_H__

// Texas Instruments Touch screen controller, TSC2046
//

#include <stdint.h>
#include <plib.h>

// Control byte
//
// Bit  Ref       Desc
// ---  ---       -------------------------------------
//   7  S         Start bit (always high to indicate cmd)
//
// 6-4  A2-A0     Channel select
//
//   3  MODE      L:12-bit  H:8-bit conversion
//
//   2  SER/DFRn  Single-ended(Hi)/Differential(Lo) select. Diff mode preferred
//                for x, y, pressure conversions (single ended would
//                require external ref).  Single mode req'd for all
//                other measurements.
//
// 1-0  PD1-PD0   Power down mode select:
//                  00:  PenIRQ enabled;  Power-down enabled
//                  01:  PenIRQ disabled; Ref off; ADC on
//                  10:  PenIRQ enabled;  Ref on ; ADC off
//                  11:  PenIRQ disabled; Ref on ; ADC on (always powered)

// A "base" command for us is 0x83 : Start bit set; PenIRQ disabled, ref on, adc on.
// To that, we select the appropriate channel, and diff/single-ended mode:
//                 (Base | Chnl | Single/Diff mode)
#define TSC_TEMP0  (0x83 | 0x00 | 0x00)
#define TSC_Y      (0x83 | 0x10 | 0x04)
#define TSC_VBAT   (0x83 | 0x20 | 0x00)
#define TSC_Z1     (0x83 | 0x30 | 0x04)
#define TSC_Z2     (0x83 | 0x40 | 0x04)
#define TSC_X      (0x83 | 0x50 | 0x04)
#define TSC_AUX    (0x83 | 0x60 | 0x00)
#define TSC_TEMP1  (0x83 | 0x70 | 0x00)

//void tscInit()

// Do a 3-byte transfer with the TSC
int16_t tscXfer(uint8_t cmd);

// For code development / test only
void tscTesting();

// See if a touch is active; if so, get x,y coords of the touch
bool touchGetXY(int16_t *x, int16_t *y);

// Wait for a touch to go in-active (with debouncing)
void touchWaitForRelease();

#endif
