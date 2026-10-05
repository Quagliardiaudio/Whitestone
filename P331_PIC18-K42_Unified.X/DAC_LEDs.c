#include <xc.h>
#include "mcc_generated_files/mcc.h"

#include "Hardware.h"
#include "Globals.h"
#include "SPI.h"
#include "DAC_LEDs.h"
#include "GPIO.h"

LED_DAC_values_t current_LED_DAC_values;

//Either BITBANG_DAC (now works but we are not sure if this development of a bit banger was because of a pinout issue holding the bus down?) or DAC_USES_SPI_NATIVE (might be working now?)
#define BITBANG_DAC  /*it works now for old PIC (PIC18F26K20) but not for the new PIC18F26K42*/
#undef DAC_USES_SPI_NATIVE /*for the new PIC18F26K42*/

#define NEW_PIC18K42


#ifdef BITBANG_DAC
uchar_t writeSPI_DAC(uchar_t DACchannel, uchar_t data) // this is a WOM (write-only memory)
{
    uchar_t bitbang;
   
    //deconfigure SPI bus temporarily for bit banging by hand herein for this function only
#ifdef NEW_PIC18K42
    SPI1CON0bits.EN = 0;
    PIE2bits.SPI1IE = 0; // disable SPI interrupts for exchange byte
    SPI1SCKPPS = 0x00;   //RC3->SPI1:SCK1;    
    RC3PPS = 0x00;   //RC3->SPI1:SCK1;    
    RC5PPS = 0x00;   //RC5->SPI1:SDO1;    
    SPI1SDIPPS = 0x14;   //RC4->SPI1:SDI1;    
 
    uchar_t return_data = 0x00;
#else
    SSPCON1bits.SSPEN = 0;  //Turn off SPI bus
    TRISCbits.RC5 = 0;  //SDO as output
#endif
    
    __delay_us(100);
    LATA = 0xFF; //unset chip selects  (all ones; active low)
    __delay_us(100);
    
    LATBbits.LB0 = 0; //chip select for DAC (actually the "load" LD pin)
    __delay_us(100);
    
    
    uchar_t bitdata = (DACchannel & 0b00001111U);

    //bit shift out (note endianess is different for data below; we shift out D11 through D8 starting with D11 as "MSb", but LSb is really D11 for these channels in binary; e.g. channel LSb is D11, but is shift out first)
    for(bitbang = 0; bitbang <4; bitbang++)  // bang out the command (channel number, really) for the DAC
    {
        uchar_t single_bit_shift_msb;
        single_bit_shift_msb = (uchar_t) ((bitdata & 0b00000001U) > 0);
       //set data line (MOSI)
        LATCbits.LC5 =  single_bit_shift_msb;  //checks to see if the highest bit is set (endianess)
        __delay_us(100);
        LATCbits.LC3 = 0; //start clock active low
        __delay_us(100);
        LATCbits.LC3 = 1; //finish clock active low
        bitdata >>= 1; //shift it off the upper byte
        __delay_us(100);
    }
    
    bitdata = data; 
    for(bitbang = 0; bitbang <8; bitbang++)
    {
        //set data line (MOSI)
        LATCbits.LC5 =  (uchar_t) ((bitdata & 0b10000000U) > 0);  //checks to see if the highest bit is set (endianess)
        __delay_us(100);
        LATCbits.LC3 = 0; //start clock active low
        __delay_us(100);
        LATCbits.LC3 = 1; //finish clock active low
        bitdata <<= 1; //shift it off the upper byte
        __delay_us(100);
    }
    //put_SPI((uchar_t)(DACchannel & 0b00001111));     //DAC channel
    //put_SPI(data);

    //note that clock pin _should_ go low here per timing diagram and Clock idle low setting in SSPCONbits.CKP=0 in Init_SPI_Bus()
    LATCbits.LC3 = 1; //SCK needs to be high per DAC (BH2223FV) datasheet
    __delay_us(100);
    LATBbits.LB0 = 1; //latch data in the DAC's LD pin (essentially a chip select for DAC)
    
    __delay_us(100); //was 5 ms

    LATBbits.LB0 = 0; //latch data in the DAC's LD pin (essentially a chip select for DAC)
    
    current_LED_DAC_values.LED_DAC_by_INDEX[DACchannel] = data;  //remember the last value written to which DAC

    __delay_us(100); //was 10 ms
    
#ifdef NEW_PIC18K42
    SPI1SCKPPS = 0x13;   //RC3->SPI1:SCK1;    
    RC3PPS = 0x1E;   //RC3->SPI1:SCK1;    
    RC5PPS = 0x1F;   //RC5->SPI1:SDO1;    
    SPI1SDIPPS = 0x14;   //RC4->SPI1:SDI1;    
    SPI1CON0bits.EN = 1;
    PIE2bits.SPI1IE = 1; // enable SPI interrupts for exchange byte

    return return_data = data;
#else
    SSPCON1bits.SSPEN = 0;  //Turn off SPI bus
    TRISCbits.RC5 = 0;  //SDO as output
    return (SSPBUF);
    
#endif
    
}
#endif

#ifdef DAC_USES_SPI_NATIVE  
uchar_t writeSPI_DAC(uchar_t DACchannel, uchar_t data) // this is a WOM (write-only memory)
{
    //Note: the DAC is opposite from the XAR GPIO expansion chips in that it must idle high on the SCK (inverted clock polarity from the other chips)
//    SSPCON1bits.CKP = 1;  // SCK idle state high
    SPI1CON0bits.EN = 0;
    SPI1CON1bits.CKP = 1;
//    SPI1CON0bits.LSBF = 0; //Note DAC is endian different than the other chips
    SPI1CON0bits.EN = 1;
    
    LATA = 0xFF; //unset chip selects  (all ones; active low)
    LATBbits.LB0 = 0; //chip select for DAC (actually the "load" LD pin)
    __delay_ms(1);
    
    while(SPI1CON2bits.BUSY);
    SPI1TWIDTHbits.TWIDTH = 4; // only the first 4 bits (starting from the Lsb below)
    
    put_SPI((uchar_t)(DACchannel & 0b00001111)<<4 );     //DAC channel

    while(SPI1CON2bits.BUSY);
    SPI1TWIDTHbits.TWIDTH = 1; // 8 bits to transfer
    put_SPI(data);

    
    //note that clock pin _should_ go low here per timing diagram and Clock idle low setting in SSPCONbits.CKP=0 in Init_SPI_Bus()
//    LATCbits.LATC3 = 1; //SCK needs to be high per DAC (BH2223FV) datasheet
    while(SPI1CON2bits.BUSY);
    __delay_us(10);
    LATBbits.LB0 = 1; //latch data in the DAC's LD pin (essentially a chip select for DAC)
    
    __delay_us(10);

    LATBbits.LB0 = 0; //latch data in the DAC's LD pin (essentially a chip select for DAC)
    
    current_LED_DAC_values.LED_DAC_by_INDEX[DACchannel] = data;  //remember the last value written to which DAC

  //  SSPCON1bits.CKP = 0;  //SCK idle state low
     SPI1CON0bits.EN = 0;
   SPI1CON1bits.CKP = 0;
 //  SPI1CON0bits.LSBF = 1; //reset the SPI endianness
    SPI1CON0bits.EN = 1;
    //SSPSTATbits.CKE = 1;  //
    
    return (data);
    
}
#endif

void set_Pilot_Brightness(uchar_t level)
{   
    BRIGHTNESS_PILOT = level;
    writeSPI_DAC(Pilot, level);
}

void set_LED_bias(uchar_t level)
{
    writeSPI_DAC(_bias, level);
}

void breathe_autostandby()
{
    signed char direction = 1;  // either 1 or -1 for increment or decrement
    signed char breathe_index;
    int keep_breathing = 10 * sizeof(breathe_LUT);
    int timeout = 30 * sizeof(breathe_LUT);
    uchar_t LUTEnd = sizeof(breathe_LUT) - 40;  //don't completely extinguish Pilot LED during breathing
    uchar_t LUTStep = 2;  //pseudo speed of breathing 
    //TODO: Check GPIO easter egg switch to bypass this autostandby breathing Pilot LED routine...
//    for(breathe_index = 0; breathe_index <= DEBOUNCE_THRESHOLD; breathe_index++)  //include n+1 times for debounce where n = DEBOUNCE_THRESHOLD
//    {
//        ClrWdt();  // clear the watchdog timer
//
//        read_All_GPIO_Bytes(); //read the entire field of GPIO bytes
//        debounce_Changed();  //debounce determines if controls changed since last time through the loop
//    }
    
    //easter egg to defeat auto standby...
    read_All_GPIO_Bytes(); //read the entire field of GPIO bytes
    if(bouncyGPIO_Values[RIGHT].Bits.pMODE == 0) return;  //hold the right MODE switch while powering up and we bypass this long process for tubes to warm up...
     
    //breathe in autostandby mode
    for(breathe_index = 0; (keep_breathing && timeout); ) /*no increment clause, as it is handled within the loop */
    {
        ClrWdt();  // clear the watchdog timer
        
        set_Pilot_Brightness(breathe_LUT[(uchar_t) breathe_index]);
        
        //manual increment control
        breathe_index += direction * LUTStep;  //manual increment or decrement instead of doing it in the for loop statement
        if (breathe_index >= LUTEnd) direction *= -1;  // reverse direction
        if (breathe_index <= 0 ) direction *= -1;  // reverse direction

        HT_READY ? keep_breathing-- : timeout--;  //HT_READY is the power supply high tension (HT) power supply (+300V or so)

        //bits and parts of the main loop except that we override the bypass until we are warmed up with the tubes eh...
        read_All_GPIO_Bytes(); //read the entire field of GPIO bytes
        debounce_Changed();  //debounce determines if controls changed since last time through the loop
        calculate_Channel_Relay_Outputs();  // only calculates what the relay/LED outputs should be based upon switches/controls inputs and modes of operation; does _not_ actually write out to GPIO chips
        debouncedGPIO_Values[LEFT].Bits.dK_Lineamp_IN  = 0;
        debouncedGPIO_Values[RIGHT].Bits.dK_Lineamp_IN = 0;
        write_All_GPIO_Bytes(); //write out the entire set of shadowed GPIO bytes calculated above

//        __delay_ms(20); // 256 steps into 1 second is about 4ms, yet we have a quarter of a sine or periodic waveform (half of the 4ms)...  here we walk the LUT of 128 1/4-periodic waveform values back and forth for 1/2 sine being 256 steps
    }
}

void set_Mode_LED_by_Color_Map(uchar_t channel, uchar_t *RGB)
{
    set_Mode_LED_Color(channel, RGB[0], RGB[1], RGB[2]);
}

void set_Mode_LED_Color(uchar_t channel, uchar_t R, uchar_t G, uchar_t B)
{
    //TBD: link mode can change the right channel LED to "white" to indicate link????  feature...
//    if (gLinkChannels && (channel == RIGHT)) {R = 200; G = 200; B = 200;} //White

    writeSPI_DAC(GET_LED_OFFSET(channel, LED_RED),   R);
    writeSPI_DAC(GET_LED_OFFSET(channel, LED_GREEN), G);
    writeSPI_DAC(GET_LED_OFFSET(channel, LED_BLUE),  B);
   
}

#define BRIGHTNESS_MAX  128
#if DEAD_CODE
void set_System_LED_Brightness()
{
    uchar_t brightness_LED_bias = BRIGHTNESS_MAX / current_control_value[LEFT].loading_knob;
    
    writeSPI_DAC(_bias, brightness_LED_bias);
    
}
#endif