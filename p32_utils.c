

#include <plib.h>
#include <stdint.h>

#include "product_config.h"

// TODO: See uSec and mSec defs in Duinomite code. May be better
//       replacements of our delay_ms & delay_us.

/******************************************************************************
*	delay_ms()
*
*	This functions provides a software millisecond delay
******************************************************************************/
void delay_ms(uint32_t msec)
{
	uint32_t tWait, tStart;
		
    tWait = (CPU_HZ/2000)*msec;
    tStart = ReadCoreTimer();
    while((ReadCoreTimer() - tStart) < tWait);
}


/******************************************************************************
*	delay_us()
*
*	This functions provides a software microsecond delay
    Ref: http://www.microchip.com/forums/m425906.aspx
*     For short delays, the CP0 Count register is useful - it ticks at 1/2 the
*     CPU clock frequency.  The __builtin_mfc0(reg,sel) functions/macros
*     can be used to read the CP0 COUNT register.
******************************************************************************/

#define TICK_HZ (CPU_HZ/2)
void delay_us(uint32_t usec)
{
    uint32_t  t, stop;

    // Calculate number of ticks for the given number of microseconds
    stop = usec * (TICK_HZ / 1000000);  // 40 ticks/us for 80MHz

    // Get current tick-time, and add to our tick-interval
    stop += _mfc0(_CP0_COUNT, _CP0_COUNT_SELECT);

    // Wait till Count reaches the stop valu
    while (1)
	{
        t = _mfc0(_CP0_COUNT, _CP0_COUNT_SELECT);
		if(t >= stop) break;
	}
} 

