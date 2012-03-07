//
// ST7565 - Sitronix ST7565x 65x132 Dot Matrix LCD  Controller/Driver
//

// TODOs
//
// Datasheet recommends 'operating modes be refreshed periodically', in case the
// part has lost it's mind??


#include <plib.h>

#include "product_config.h"
#include "p32_utils.h"
#include "st7565.h"

// Hardware line definitions (TODO: Where does it make best sense
//    to place these defs - in a "hal.h" ?)
//


#if defined ST7565_NHD_PROTOTYPE_STARTERKIT || defined ST7565_M4557_PROTOTYPE_STARTERKIT

  // Port assignemnt for using a parallel ST7565 interface on the PIC32 Starter Kit
  //
  //               Starter-Kit 2,    J10-
  #define CS1n_LO()  LATBCLR=BIT_10   // 47
  #define CS1n_HI()  LATBSET=BIT_10
  #define RESn_LO()  LATBCLR=BIT_11   // 48
  #define RESn_HI()  LATBSET=BIT_11
  #define A0_LO()    LATBCLR=BIT_12   // 49
  #define A0_HI()    LATBSET=BIT_12
  #define WRn_LO()   LATBCLR=BIT_13   // 50
  #define WRn_HI()   LATBSET=BIT_13
  #define RDn_LO()   LATBCLR=BIT_14   // 59
  #define RDn_HI()   LATBSET=BIT_14
    
  #define LCD_DB     PORTE  /* Data lines on bits 0..7 (SKII J10-12..5) */ 

#elif defined M4557_DUINOMITE

  // Port assignemnt for using parallel ST7565 interface on Olimex Duinomite
  //
  //                 Duinomite,    GPIO-
  #define CS1n_LO()  LATBCLR=BIT_3    // 21
  #define CS1n_HI()  LATBSET=BIT_3
  #define RESn_LO()  LATBCLR=BIT_4    // 19
  #define RESn_HI()  LATBSET=BIT_4
  #define A0_LO()    LATBCLR=BIT_6    // 17
  #define A0_HI()    LATBSET=BIT_6
  #define WRn_LO()   LATBCLR=BIT_7    // 15
  #define WRn_HI()   LATBSET=BIT_7
  #define RDn_LO()   LATBCLR=BIT_9    // 13
  #define RDn_HI()   LATBSET=BIT_9
    
  #define LCD_DB     PORTE  /* Data lines on bits 0..7 (GPIO-4,6,8,..18) */ 

#elif defined ST7565_M4492_PROTOTYPE_OLIMEX_UEXTPORT

  // Olimex PIC32-MX460
  // Port assignment for using a serial ST7565 interface via the
  // Olimex UEXT connector.  In addition to the SPI lines, MISO1,
  // MOSI1, and SCK1 on pins 7,8,9, we use:
  //
  #define A0_LO()   LATFCLR=BIT_8  /* UEXT-3 (RF8/TXD1) */
  #define A0_HI()   LATFSET=BIT_8
  #define RESn_LO() LATFCLR=BIT_2  /* UEXT-4 (RF2/RXD1)*/
  #define RESn_HI() LATFSET=BIT_2
  #define CS1n_LO() LATDCLR=BIT_9  /* UEXT-10 (RD9/CS_UEXT) */
  #define CS1n_HI() LATDSET=BIT_9

  #define LCD_SPI_CH  SPI_CHANNEL1
  #define LCD_SPIBUSY SPI1STATbits.SPIBUSY

#elif defined ST7565_M4492_OLIMEX_PINGUINO_OTG

  // Olimex PIC32-PINGUINO-OTG
  // Port assignment for using a serial ST7565 interface via the
  // Olimex UEXT connector.  In addition to the SPI lines, MISO1,
  // MOSI1, and SCK1 on pins 7,8,9, we use:
  //
  #define A0_LO()   LATFCLR=BIT_5  /* UEXT-3 (RF8/TXD1) */
  #define A0_HI()   LATFSET=BIT_5
  #define RESn_LO() LATFCLR=BIT_4  /* UEXT-4 (RF2/RXD1)*/
  #define RESn_HI() LATFSET=BIT_4
  #define CS1n_LO() LATFCLR=BIT_0  /* UEXT-10 (RD9/CS_UEXT) */
  #define CS1n_HI() LATFSET=BIT_0

  #define LCD_SPI_CH  SPI_CHANNEL2
  #define LCD_SPIBUSY SPI2STATbits.SPIBUSY

#else
  #error must define port setup macro
#endif


// lcdInit()
//
// Inputs are "contrast" parameters: The ST7565's resistor-ratio
// and volume settings.
//
void lcdInit(
    uint8_t resistorRatio,   // Sets ST7565's resistor ratio, 0..7
    uint8_t volume)          // Sets ST7565's "volume" (contrast?), 0..0x3F
{
    CS1n_HI();         // De-select controller
    RESn_LO();         // Activate reset 
    delay_ms(5);
    RESn_HI();         // Release reset
    delay_ms(5);

    lcdCmd(cADC_NORMAL);            // Normal segment order
    lcdCmd(cCOM_NORMAL);            // Normal common order
    lcdCmd(cBIAS_9);                // Set 1/9 bias

    delay_ms(2);

    //lcdCmd(cDISP_START_LINE | 0);   // Start line is line 0
 
    lcdCmd(cBOOSTRATIO);            // Enter boost ratio set mode, and then...
    lcdCmd(0);                      //   set boost ratio to 2x/3x/4x

    // In some example code (incl lxd's 8051 asm test code), power circuits
    // are brought up one at a time (in other examples, this is not done, and
    // is probably not req'd. i.e. You can just turn on boost, regulator, and
    // follower all at once).
    lcdCmd(cPOWER_CONTROL | 4);     // Booster on.
    delay_ms(5);
    lcdCmd(cPOWER_CONTROL | 6);     // Boost, regulator on.
    delay_ms(5);
    lcdCmd(cPOWER_CONTROL | 7);     // Power boost, regulator, and follower all on
    delay_ms(5);

 	// Contrast/Brightness settings
    resistorRatio &= 0x07;          // Limit to 0..7 or less
    lcdCmd(cRESISTOR_RATIO | resistorRatio);
    delay_ms(2);
    lcdCmd(cVOLUME);                // Volume register set (LCD voltage, Vo). Next byte is...
    lcdCmd(volume & 0x3F);          //    the "volume", 0..63 (0x00..0x3f).
/*
#if   defined ST7565_NHD_PROTOTYPE_STARTERKIT
    lcdCmd(cRESISTOR_RATIO | 5 );   //
    delay_ms(2);                    //
    lcdCmd(cVOLUME);                // Volume register set (LCD voltage, Vo). Next byte is...
    lcdCmd(35);                     //    the "volume", 0..63 (0x00..0x3f).
#elif defined ST7565_M4557_PROTOTYPE_STARTERKIT
    lcdCmd(cRESISTOR_RATIO | 5 );   //
    delay_ms(2);                    //
    lcdCmd(cVOLUME);                // Volume register set (LCD voltage, Vo). Next byte is...
    lcdCmd(35);                     //    the "volume", 0..63 (0x00..0x3f).
#elif defined ST7565_M4492_PROTOTYPE_OLIMEX_UEXTPORT
    lcdCmd(cRESISTOR_RATIO | 4 );   // (values copied from test-board 8052 code)
    delay_ms(2);                    //
    lcdCmd(cVOLUME);                // Volume register set (LCD voltage, Vo). Next byte is...
    lcdCmd(28);                     //    the "volume", 0..63 (0x00..0x3f).
#else
  #error must define port setup macro
#endif
*/
    delay_ms(2);
    lcdCmd(cDISPLAY_ON);            // Turn display on
 
    lcdClear();                     // Write all zeros to display RAM
}

// Serial (SPI) Mode Interface
//
// With the PS line held low, the ST7565 operates in serial (SPI) mode.
// The controller is write only in this mode.
//
// Signals:
//   /CS1 - Chip select
//   A0   - H:Display Data; L:Command. Sampled with last/8th bit (D0)
//   SI   - Serial In (sampled on rising edge of SCL); MSbit 1st
//   SCL  - Serial Clock (max @ Vdd 2.7v is T=100ns; F=10MHz)
//   /RES - Reset (>1us pulse; wait 1us after)

uint8_t lcdCmd(uint8_t cmd)
{
    A0_LO();
    delay_us(5);

    CS1n_LO();
    delay_us(5);
    
#if defined LCD_SERIAL
    SpiChnPutC(LCD_SPI_CH, cmd);
	// TODO:  YUCK!! We have to wait for the SPI byte to have
	//        been sent, before we continue (with raising CS1n).
	//        Find a way to wait for Tx complete, or rework.
	//delay_us(250);
    while(LCD_SPIBUSY);   // Seems to work, but still have to wait some...
    delay_us(15);                  // This fails at 5us; passes at 10 (M4492)
#elif defined LCD_PARALLEL
    uint16_t tmp16 = LCD_DB & 0xff00;
    tmp16 |= cmd;
    LCD_DB = tmp16;
	delay_us(5);
    WRn_LO();
	delay_us(5);
    WRn_HI();
	delay_us(5);
#else
    #error Need to define LCD_SERIAL or LCD_PARALLEL
#endif

    CS1n_HI();
	delay_us(25);  // Atmel test board has ~28us here

    return 0;
}

uint8_t lcdData(uint8_t data)
{
    A0_HI();
    delay_us(5);

    CS1n_LO();
    delay_us(5);
    
#if defined LCD_SERIAL
    SpiChnPutC(LCD_SPI_CH, data);
	// TODO:  YUCK!! We have to wait for the SPI byte to have
	//        been sent, before we continue (with raising CS1n).
	//        Find a way to wait for Tx complete, or rework.
	//delay_us(250);
    while(LCD_SPIBUSY);   // Seems to work, but still have to wait some...
    delay_us(15);                  // This fails at 5us; passes at 10 (M4492)
#elif defined LCD_PARALLEL
    uint16_t tmp16 = LCD_DB & 0xff00;
    tmp16 |= data;
    LCD_DB = tmp16;
	delay_us(5);
    WRn_LO();
	delay_us(5);
    WRn_HI();
	delay_us(5);
#else
    #error Need to define LCD_SERIAL or LCD_PARALLEL
#endif

    CS1n_HI();
	delay_us(10);

    return 0;
}


uint8_t lcdDataArray(const uint8_t data[], int n)
{
    int i;

    A0_HI();
    delay_us(5);

    CS1n_LO();
    delay_us(5);
    
#if defined LCD_SERIAL
    for(i=0; i<n; i++)
    {
        SpiChnPutC(LCD_SPI_CH, data[i]);
        // TODO:  YUCK!! We have to wait for the SPI byte to have
        //        been sent, before we continue (with raising CS1n).
        //        Find a way to wait for Tx complete, or rework.
        //delay_us(250);
        while(LCD_SPIBUSY);   // Seems to work, but still have to wait some...
    }
    delay_us(15);                  // This fails at 5us; passes at 10 (M4492)
#elif defined LCD_PARALLEL
    uint16_t tmp16;
    for(i=0; i<n; i++)
    {
        tmp16 = LCD_DB & 0xff00;
        tmp16 |= data[i];
        LCD_DB = tmp16;
        delay_us(5);
        WRn_LO();
        delay_us(5);
        WRn_HI();
        delay_us(5);
    }
#else
    #error Need to define LCD_SERIAL or LCD_PARALLEL
#endif

    CS1n_HI();
	delay_us(10);

    return 0;
}


uint8_t lcdReadStatus()
{
#ifdef LCD_SERIAL
    return 0;
#else
    uint16_t tmp16;

    TRISESET = 0x00ff;  // TODO: move this

    A0_LO();
    delay_us(5);

    CS1n_LO();
    delay_us(5);

    RDn_LO();
	delay_us(5);
    tmp16 = LCD_DB;
    RDn_HI();
	delay_us(5);

    CS1n_HI();
	delay_us(5);

    TRISECLR = 0x00ff;  // TODO: move this

    return (uint8_t)tmp16;
#endif
}

uint8_t lcdReadData()
{
#if defined LCD_SERIAL
    return 0;
#else
    uint16_t tmp16;

    TRISESET = 0x00ff;  // TODO: move this

    A0_HI();
    delay_us(5);

    CS1n_LO();
    delay_us(5);

    RDn_LO();
	delay_us(5);
    tmp16 = LCD_DB;
    RDn_HI();
	delay_us(5);

    CS1n_HI();
	delay_us(5);

    TRISECLR = 0x00ff;  // TODO: move this

    return (uint8_t)tmp16;
#endif
}


// Copy a buffer from our memory to LCD's display RAM.
// TODO: Different LCD sizes, orientations, etc.
//
void lcdWriteBuffer(const uint8_t *buff)
{
    //uint8_t col
	uint8_t page;

    for(page = 0; page < 8; page++)
    {
        lcdCmd(cPAGE    | (7 - page));  // Lines need to be reversed. (Why?)
        lcdCmd(cCOL_MS  | 0);
        lcdCmd(cCOL_LS  | 0);

        //lcdCmd(cRMW);   // Why was this here?
        //lcdData(0xff);     //

        //for(col=0; col<128; col++)
        //    lcdData(*buff++);
        lcdDataArray(buff, 128);
        buff += 128;
    }
}

// lcdClear() - Write all zeros to display RAM
//
void lcdClear(void)
{
    uint16_t seg, page;

    for(page=0; page<8; page++)
    {
        lcdCmd(cPAGE | page);
        lcdCmd(cCOL_MS);
        lcdCmd(cCOL_LS);
        //lcdCmd(cRMW);    // TODO: Why was this here?
        //lcdData(0xff);   //

        for(seg=0; seg<132; seg++)
            lcdData(0);
    }
}
