
#include "Hardware.h"
#include <string.h>                                            
#include <stdlib.h>
#include <stdio.h>
#include <xc.h>
#include "SPI.h"
#include "GPIO.h"
#include "Controls.h"
#include "Globals.h"

void GPIO_INIT(void)
{

//     while(PS_OPERATE_outlo == 0);
//                        //Reset GPIO chips
    __delay_ms(10);
    RST_GPIO_outhi = 0;  // Note this is for daughter card
    __delay_ms(10);
    RST_GPIO_outhi = 1;  // note this is for daughter, real design should be opposite
    //************************************
 
    __delay_ms(10);

#if GPIO_EXP_IS_MCP23SXX
    //set the MCP23S17 / MCP23S18 configuration register first on all 8 chips (see MCP_IOCON_INIT in GPIO.h)
    for(uchar_t chip_select = CALCULATE_CS(LEFT, CS_A); chip_select <= CALCULATE_CS(RIGHT, CS_D); chip_select++)
    {
        writeSPI_GPIO_EXP(chip_select, MCP_IOCON, MCP_IOCON_INIT);
    }
#endif

    //loop through to keep settings for both channels identical
    for(uchar_t channel = LEFT; channel <= RIGHT; channel ++)
    {
                writeSPI_GPIO_EXP( (((uchar_t)( channel << 2)) +1U) ,  GPX_PULLUP1, 0b00000011U);      // Set pull-up resistors bits 0,1   (Output Level gain mode switch on front panel, BOOST or CUT; center-off OUT is calculated from BOOST _and_ CUT not being engaged)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_A),  GPX_PULLUP1, 0b00000011U);      // Set pull-up resistors bits 0,1   (Output Level gain mode switch on front panel, BOOST or CUT; center-off OUT is calculated from BOOST _and_ CUT not being engaged)
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_A),  GPX_PULLUP2, 0b11000000U);      // Set pull-up resistors bits 14,15 (Fractional dB resolution switch, 0dB or 0.5dB; 0.25dB center-off is calculated from 0dB _and_ 0.5dB not being engaged)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_B),  GPX_PULLUP2, 0b11000000U);      // Set pull-up resistors bit 14, 15 (Mode switch front panel, FB_CLEAN and channel MODE momentary; Feedforward Bloom is calculated from FB_CLEAN not engaged)
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_C),  GPX_PULLUP2, 0b10000000U);      // Set pull-up resistors bit 14, 15 (Mode switch front panel, FB_CLEAN and channel MODE momentary; Feedforward Bloom is calculated from FB_CLEAN not engaged)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_B),  GPX_OUTPUT1, 0b00000000U);      // Set output register GPX_OUTPUT1 to all bits off (zeros)
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_B),  GPX_OUTPUT2, 0b00000000U);      // Set output register GPX_OUTPUT2 to all bits off (zeros)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_B),  GPX_DIR1, 0b00000000U);      // Set GPX_DIR1 register to define as outputs
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_B),  GPX_DIR2, 0b11000000U);      // Set GPX_DIR2 register to define outputs (and bit 7,8 for schematic/chip port 14,15 as input)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_C),  GPX_OUTPUT1, 0b00000000U);      // Set output register GPX_OUTPUT1 to off
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_C),  GPX_OUTPUT2, 0b00000000U);      // Set output register GPX_OUTPUT2 to off

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_C),  GPX_DIR1, 0b00000000U);      // Set GPX_DIR1 register to define as output
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_C),  GPX_DIR2, 0b10000000U);      // Set GPX_DIR2 register to define as output (and bit 8 for schematic/chip port 15 as input; input reads state of Bypass switch for this channel)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_D),  GPX_OUTPUT1, 0b00000000U);      // Set output register GPX_OUTPUT1 to off
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_D),  GPX_OUTPUT2, 0b11111111U);      // Set output register GPX_OUTPUT2 to all ones for LED display to be off (display segments are active low)

        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_D),  GPX_DIR1, 0b00000000U);      // Set GPX_DIR1 register to define as outputs
        writeSPI_GPIO_EXP(CALCULATE_CS(channel, CS_D),  GPX_DIR2, 0b00000000U);      // Set GPX_DIR2 register to define as outputs

    }
}

//reads all 8 bytes of GPIO expansion chip values for each channel and stores in bouncy GPIO values for later bulk-debounce
void read_All_GPIO_Bytes(void)
{
    //update the blink counter here as this function reads all GPIO bytes whether in autostandby or in the main while(1) loop
    blink_counter++;
    if(blink_counter > 6) blink_counter = 0;  //increment for any LEDs we wish to blink

    for(uchar_t channel = LEFT; channel <= RIGHT; channel++)  //iterate over both stereo channels/channels
    {
        for (uchar_t byte_group = 0; byte_group < GPIO_BYTE_GROUPS_T_SZ; byte_group++) // iterate through the byte_groups, to set raw GPIO data into the master GPIO map struct (where casted)
        {
            //determine if we need to address low half of the GPIO expansion chip, P0-P7 or high half, P8-P15 
            //(16-bits = 2 byte groups for each chip), yet our SPI is 8-bit wide data; even numbered byte
            uchar_t even;
            
            uchar_t chip_select;  // calculated chip select (they're in a particular pattern / order laid out in hardware)
            uchar_t GSR_command;  // calculated: will be either GPX_STATE1 or GPX_STATE2 depending upon even or odd calculated above...
            
            even = (uchar_t)(byte_group & 0b00000001);  // check LSB for even-or-odd in the byte_group number for chip select to see if odd or even for low port or high port on each GPIO expansion chip
            
            //GSR command is what sets the which part of the chip to read by getting the GPIO state registers in the GPIO expansion chip
            GSR_command = (uchar_t) (GPX_STATE1 + even);  // if even, then GPX_STATE1 becomes GPX_STATE2 (always port 1 register + 1) for high port
            
//            chip_select = (uchar) ((byte_group / 2) + (channel * 4));   //use truncating integer division to figure out the chip select
            chip_select = (uchar_t) ((byte_group >> 1) + (channel << 2));   //use truncating integer division to figure out the chip select; use bit shifting instead of C division library 

            //assign raw (un-debounced) byte into the byte group union that maps to individual line amp controls or relays
            bouncyGPIO_Values[channel].byte_groups[byte_group] = readSPI_GPIO_EXP(chip_select, GSR_command); 
        }
    }
        
}

//writes all 8 bytes of GPIO expansion chip values for each channel using debounced GPIO values 
void write_All_GPIO_Bytes(void)
{
    for(uchar_t channel = LEFT; channel <= RIGHT; channel++)  //iterate over both stereo channels/channels
    {
        // use GPIO_BYTE_GROUPS_T_SZ -1 to prevent writing to the display at the very end of the GPIO memory bytes; display writes (for debug) happen ad hoc elsewhere at the time of the display write function call
        for (uchar_t byte_group = 0; byte_group < GPIO_BYTE_GROUPS_T_SZ - 1; byte_group++) // iterate through the byte_groups, to set raw GPIO data into the master GPIO map struct (where casted)
        {   //note: writing output bits/bytes to input configured pins has no effect on either the XRA1405 or the MCP23S18 (only the output latch is written)

            //(16-bits = 2 byte groups for each chip), yet our SPI is 8-bit wide data; even numbered byte
            uchar_t even;         //determine if we need to address low half of the GPIO expansion chip, P0-P7 or high half, P8-P15 

            
            uchar_t chip_select;  // calculated chip select (they're in a particular pattern / order laid out in hardware)
            uchar_t OCR_command;  // calculated: will be either GPX_OUTPUT1 or GPX_OUTPUT2 depending upon even or odd calculated above...
           
            //check the LSb, if it's a one then it is odd... otherwise it's even
            even = (uchar_t)(byte_group & (uchar_t) 0b00000001);  // check the byte_group number for chip select to see if odd or even for low port or high port on each GPIO expansion chip
            
            //OCR command selects which half of the chip to write (output latch registers)
            OCR_command = (uchar_t) (GPX_OUTPUT1 + even);  // if even, then GPX_OUTPUT1 becomes GPX_OUTPUT2 (always port 1 register + 1) for high port within this single 16-bit (2 byte) worth of GPIO expansion chip
            
            //formula for the below: byte_group / 2 + channel * 4;  integer divide by 2 (or single bit shift) truncates, which is desirable here. This gets the appropriate group of 2 bytes for each 16-bit GPIO expansion chip
            chip_select = (uchar_t) ((byte_group >> 1) + (channel << 2));   //shift single bit(s) for divide or multiply by factors of 2; every shift is either a multiply or divide by 2 (depending upon shift direction)

            //assign final debounced byte into the byte group union that maps to individual line amp controls or relays
            writeSPI_GPIO_EXP(chip_select, OCR_command, debouncedGPIO_Values[channel].byte_groups[byte_group]);
        }
    }
       
}
