

// TSC2046 touch-screen controller

#include "product_config.h"
#include "tsc2046.h"
#include "p32_utils.h"

// For ESI unit, at least, we swap the x,y axis to better match
// the underlying LCD controller's view of things.
#define TSC_SWAP_XY 1

// TODO: Where do these belong?
// 
// Note the ESI shares TSC I/O with LCD's data bus:
//   D0 = SCK
//   D1 = MISO  (touch-screen controller to processor)
//   D2 = BUSY
//   D3 = MOSI  (processor to TSC)
// 
#if defined ST7565_M4557_PROTOTYPE_STARTERKIT

#define TSC_CSn_LO() LATFCLR=BIT_5      // RF5 (PMA8), J10-52 on SKII
#define TSC_CSn_HI() LATFSET=BIT_5

#define TSC_SCK_LO()   LATECLR=BIT_0    // RE0 (LCD's D0), J10-12 on SKII
#define TSC_SCK_HI()   LATESET=BIT_0
#define TSC_MOSI_LO()  LATECLR=BIT_1    // RE1 (LCD's D1), J10-11 on SKII
#define TSC_MOSI_HI()  LATESET=BIT_1

#elif defined M4557_DUINOMITE

#define TSC_CSn_LO() LATBCLR=BIT_10     // RB10, GPIO-11 on Duinomite
#define TSC_CSn_HI() LATBSET=BIT_10

#define TSC_SCK_LO()   LATECLR=BIT_0    // RE0 (LCD's D0), GPIO-4 on Duinomite
#define TSC_SCK_HI()   LATESET=BIT_0
#define TSC_MOSI_LO()  LATECLR=BIT_1    // RE1 (LCD's D1), GPIO-6 on Duinomite
#define TSC_MOSI_HI()  LATESET=BIT_1

#else
  #error Need a product defined
#endif

//
//  Raw values observed for ESI M4557B
//
//  Note: The 64x128 pixel ESI unit will be treated as 128x64 display,
//        since that jives with the existing ST7565 code. Therefore,
//        the x axis is the long axis; y is the short axis.
//
//const int16_t x0 = 

// SPI clk - Idles low; DIN (to TSC) centered on rising edge.

// Define the port & bit's used for the digital serial interface


//void tscInit()
//{
//    TRISx
//}

//  Perform a 3-byte touch-screen command/response sequence.
//
//  One command byte is sent, which selects a channel to measure with the
//  ADC controller.  Two bytes are returned - typically 12 bits from an ADC
//  conversion.
//
int16_t tscXfer(uint8_t cmd)
{
	uint8_t i;
	int16_t tmp16;

    // TODO: make defines for these ports/bits...
    TRISESET = 0x000C;   // Normally these are D2 & D3 LCD outputs; Set as inputs
                         // for use with the TSC: D2=BUSY; D3=SDI (serial data from TSC)
    TSC_SCK_LO();        // Init clock line low
    delay_us(2);

    
    TSC_CSn_LO();        // Activate TSC (chip select)
    delay_us(2);


    // Send the command byte
    for(i = 0x80; i; i>>=1)   // Shift a bit-mask from left to right
    {
        // Set up the data line
        if(i & cmd)  TSC_MOSI_HI();
        else         TSC_MOSI_LO();
        delay_us(2);
        
        // Clock the data
        TSC_SCK_HI();
        delay_us(2);
        TSC_SCK_LO();
    }

    delay_us(2);

    // Clock / Read-back 16 bits. Only 12 bits are significant
	tmp16 = 0;
    for(i=0; i<16; i++)
    {
        TSC_SCK_HI();
        delay_us(2);
        TSC_SCK_LO();
        delay_us(2);
		tmp16 <<= 1;
        tmp16 |= PORTEbits.RE3;   // TODO define for port assignment
    }

    delay_us(2);
    TSC_CSn_HI();        // De-Activate TSC (chip select)

    TRISECLR = 0x000C;   // Set the LCDs data lines (D2 & D3) as outputs.

    // Shift the 12 significant bits (currently in the MS bits) down 4 bits.
    tmp16 >>= 4;

    return tmp16 & 0x0fff;
}


// touchGetXY()
//
// If the touch screen is being pressed, return true, and also return
// x,y coordinates of the touch.
//
// If touch screen is not being pressed, return false.
//


// For initial code testing / experimentation only
void tscTesting()
{
    int16_t bubba, tscX, tscY, tscZ1, tscZ2;
    int32_t pressure;

    char tmpStr[64];
    bubba = tscXfer(TSC_TEMP0);
    bubba = tscXfer(TSC_Y);
    bubba = tscXfer(TSC_VBAT);
    bubba = tscXfer(TSC_Z1);
    bubba = tscXfer(TSC_Z2);
    bubba = tscXfer(TSC_X);
    bubba = tscXfer(TSC_AUX);
    bubba = tscXfer(TSC_TEMP1);
    bubba = tscXfer(TSC_X);
    while(1)
	{
        tscX = tscXfer(TSC_X);
		delay_ms(2);
		tscY = tscXfer(TSC_Y);
        delay_ms(2);
        tscZ1 = tscXfer(TSC_Z1);
		delay_ms(2);
        tscZ2 = tscXfer(TSC_Z2);
		delay_ms(2);

        // Note: Initial tests of this pressure code, done on ESI M4557 unit, does not
        //       look great. If we want to measure pressure (we probably don't), we
        //       have more work to do.
        if (tscZ1)  //Z1 will always be zero on no touch.
        {
          pressure = 280 * tscX;   // MMc: Was 40. This is x-plate resistance; Whats ours???
          pressure /= 4096;
          pressure *= tscZ2 - tscZ1;
          pressure /= tscZ1;
        }
        else
          pressure = 0xffffffff; // Was 0; makes more sense that this is "infinity"


		sprintf(tmpStr,"x:%04x\ty:%04x\tz1:%04x\tz2:%04x\tp:%04x\n",
                tscX, tscY, tscZ1, tscZ2, pressure);
		DBPUTS(tmpStr);
        delay_ms(1000);
	}
}


// If a touch is active, read x,y values and return
//
bool touchGetXY(int16_t *x, int16_t *y)
{
    int16_t tmpX, tmpY;

    // If no touch  detect, return false.
    if(tscXfer(TSC_Z1) == 0)  return false;

    tmpX = tscXfer(TSC_X);      // Read X position
    tmpY = tscXfer(TSC_Y);      // Read Y position

    // Check the touch still active now, and again after
    // some delay (to debounce the touch)
    if(tscXfer(TSC_Z1) == 0) return false;
    delay_ms(20);
    if(tscXfer(TSC_Z1) == 0) return false;
    
    // This seems to be a valid touch. Return true, and the x,y values.
    if(TSC_SWAP_XY)
    {
        *x = tmpY;
        *y = tmpX;
    }
    else
    {
        *x = tmpX;
        *y = tmpY;
    }
    return true;
}



// Wait for a touch to be released, with debouncing
//
void touchWaitForRelease()
{
    while(1)
    {
        // Check a few times, with delays in-between, that no touch is detected.
        if(tscXfer(TSC_Z1) == 0)
        {
            delay_ms(20);
            if(tscXfer(TSC_Z1) == 0)
            {
                delay_ms(20);
                if(tscXfer(TSC_Z1) == 0) break;  // Looks like a real release
            }
        }
    }
}
