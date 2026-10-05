/*
 * Project: P331 (version 3)
 * @version     %I%, %G%
 * @since       1.0
 * @author       David Rettagliata
 * @author       C Mack
 * 
 * ****************************
 * NOTE: this firmware is universal and unified for both EVL and Original P331
 *       Use the #define macro in Globals.h to set which hex file to compile.
 * EVL answers YES to "AM_I_EVL"
 * Original P331 answers NO to "AM_I_EVL"
 * 
 * ****************************
 * Created on September 24, 2017, 8:33 AM
 * 
 * Written with MPLAB X IDE 4.01
 * Compiler XC8 ???
 * 
 * 
 * BETA released on 10/9/17 V1.0
 * BETA released on 6/30/2018  V3.0
 * 
 */


//// CONFIG1H
//#pragma config FOSC = INTIO67   // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
//#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
//#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)
//
//// CONFIG2L
//#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
//#pragma config BOREN = ON       // Brown-out Reset Enable bits (Brown-out Reset enabled and controlled by software (SBOREN is enabled))
//#pragma config BORV = 27        // Brown Out Reset Voltage bits (VBOR set to 2.7 V nominal)
//
//// CONFIG2H
//#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
//#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)
//
//// CONFIG3H
//#pragma config CCP2MX = PORTBE  // CCP2 MUX bit (CCP2 input/output is multiplexed with RB3)
//#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
//#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
//#pragma config HFOFST = OFF     // HFINTOSC Fast Start-up (The system clock is held off until the HFINTOSC is stable.)
//#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
//
//// CONFIG4L
//#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
////#pragma config LVP = ON         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
//#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
//#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))
//
//// CONFIG5L
//#pragma config CP0 = OFF        // Code Protection Block 0 (Block 0 (000800-003FFFh) not code-protected)
//#pragma config CP1 = OFF        // Code Protection Block 1 (Block 1 (004000-007FFFh) not code-protected)
//#pragma config CP2 = OFF        // Code Protection Block 2 (Block 2 (008000-00BFFFh) not code-protected)
//#pragma config CP3 = OFF        // Code Protection Block 3 (Block 3 (00C000-00FFFFh) not code-protected)
//
//// CONFIG5H
//#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
//#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)
//
//// CONFIG6L
//#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-003FFFh) not write-protected)
//#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (004000-007FFFh) not write-protected)
//#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (008000-00BFFFh) not write-protected)
//#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (00C000h-00FFFFh) not write-protected)
//
//// CONFIG6H
//#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
//#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
//#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
//
//// CONFIG7L
//#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
//#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
//#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
//#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)
//
//// CONFIG7H
//#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h> // include standard header file
#include "mcc_generated_files/mcc.h"
#include "SPI.h"
#include <string.h>                                            //
#include <stdlib.h>
#include <stdio.h>
#include "Hardware.h"
#include "GPIO.h"
#include "Globals.h"
#include "Controls.h"
#include "DAC_LEDs.h"
#include "EEPROM.h"

//// set Config bits
//// CONFIG1H
//#pragma config FOSC = INTIO67   // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
//#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
//#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)
//
//// CONFIG2L
//#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
//#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
//#pragma config BORV = 27        // Brown Out Reset Voltage bits (VBOR set to 2.7 V nominal)
//
//// CONFIG2H
//#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
//#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)
//
//// CONFIG3H
//#pragma config CCP2MX = PORTBE  // CCP2 MUX bit (CCP2 input/output is multiplexed with RB3)
//#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
//#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
//#pragma config HFOFST = ON      // HFINTOSC Fast Start-up (HFINTOSC starts clocking the CPU without waiting for the oscillator to stablize.)
//#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
//
//// CONFIG4L
//#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
//#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
//#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))
//
//// CONFIG5L
//#pragma config CP0 = OFF        // Code Protection Block 0 (Block 0 (000800-003FFFh) not code-protected)
//#pragma config CP1 = OFF        // Code Protection Block 1 (Block 1 (004000-007FFFh) not code-protected)
//#pragma config CP2 = OFF        // Code Protection Block 2 (Block 2 (008000-00BFFFh) not code-protected)
//#pragma config CP3 = OFF        // Code Protection Block 3 (Block 3 (00C000-00FFFFh) not code-protected)
//
//// CONFIG5H
//#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
//#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)
//
//// CONFIG6L
//#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-003FFFh) not write-protected)
//#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (004000-007FFFh) not write-protected)
//#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (008000-00BFFFh) not write-protected)
//#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (00C000h-00FFFFh) not write-protected)
//
//// CONFIG6H
//#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
//#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
//#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
//
//// CONFIG7L
//#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
//#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
//#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
//#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)
//
//// CONFIG7H
//#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)


void main(void) {
    SYSTEM_Initialize();
    
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts
    // Use the following macros to:
    
    // Enable the Global Interrupts
    INTCON0bits.GIE = 1;
    PIE2bits.SPI1IE = 1; // enable SPI interrupts for exchange byte

    //INTERRUPT_GlobalInterruptEnable();
    
    //    // set up oscillator control register
    //    OSCCONbits.IRCF=0x07;       //set OSCCON IRCF bits to select OSC frequency=16Mhz
    //    OSCCONbits.SCS=0x02;        //set the SCS bits to select internal oscillator block
    //    
    //    // PORT Assignments
    //    TRISAbits.RA0 = 0;	//Chip selects
    //    TRISAbits.RA1 = 0;	//Chip selects
    //    TRISAbits.RA2 = 0;	//Chip selects
    //    TRISAbits.RA3 = 0;  //Chip selects
    //    TRISAbits.RA4 = 0;	//Chip selects
    //    TRISAbits.RA5 = 0;  //Chip selects
    //    TRISAbits.RA6 = 0;	//Chip selects
    //    TRISAbits.RA7 = 0;  //Chip selects
    //    
    //    TRISBbits.RB0 = 0;  //Output for LD pin on DAC (load / latch, e.g., like a chip select on SPI bus for DAC that controls all LEDs)
    //    TRISBbits.RB1 = 0;
    //    TRISBbits.RB2 = 0;
    //    TRISBbits.RB3 = 1;  //tmp debug input for channel_link and other sundries; implementation for USART GPIO add-on PCB (tube PCB 6-pin header via 30-conductor ribbon for tube gain relays)
    //    TRISBbits.RB4 = 0;  //Optional implementation for USART GPIO add-on PCB (tube PCB 6-pin header via 30-conductor ribbon for tube gain relays)
    //    TRISBbits.RB5 = 0;
    //    TRISBbits.RB6 = 0;
    //    TRISBbits.RB7 = 0;
    //    
    //    //TRISB = 0xF8;
    //    ANSEL = 0x00;
    //    ANSELH = 0x00;
    //    WPUB = 0b00000100; // set weak pull up high for bit 3 of PORT B (generic GPIO above)
    //    RBPU  = 0;  //active low enable pullups
    //    INTCON2bits.nRBPU = 1;
    //
    //    
    //    TRISCbits.RC0 = 0;  //RST_GPIO output (active high)
    //    TRISCbits.RC1 = 0;  //PS_OPERATE output (active low)
    //    TRISCbits.RC2 = 1;  //HT_READY input (active low)
    //    TRISCbits.RC4 = 1;  //SDI as input
    //    TRISCbits.RC3 = 0;  //SCK as output
    //    TRISCbits.RC5 = 0;  //SDO as output
    //    TRISCbits.RC6 = 0;  //Optional implementation for USART TX add-on PCB (tube PCB 6-pin header via 30-conductor ribbon for tube gain relays)
    //    TRISCbits.RC7 = 1;  //Optional implementation for USART RX add-on PCB (tube PCB 6-pin header via 30-conductor ribbon for tube gain relays)
    //    
    //    LATBbits.LB0 = 1;  //keep this pin high as it is now a chip select for the LED(s) DAC(s)
    //
    //    // Set unused pins low
    //   
    //    LATBbits.LB1 = 0;
    //    LATBbits.LB2 = 0;
    ////    LATBbits.LB3 = 0;
    //    LATBbits.LB4 = 0;
    //    LATBbits.LB5 = 0;
    //    LATBbits.LB6 = 0;
    //    LATBbits.LB7 = 0;
    //         
    //    // Set chip selects  (active low)
    //    LATA = 0xFF;  //set to all ones....
    //    //RST_GPIO_outhi = 1;
    
    Init_SPI_Bus();       //Initialize SPI bus
    GPIO_INIT();        //Initialize IO chips
    controls_INIT();    //initialize any control objects (init debouncers)
    
    RST_GPIO_outhi = 1;
    set_LED_bias(200); // turn LEDs off; 255 = off, 0 = full bias on; should never be 0 -- maybe 128 or so
    
    __delay_ms(20);
    // read and purge any random garbage in the GPIO chips (we do nothing with the data read here)
    read_All_GPIO_Bytes(); //read the entire field of GPIO bytes
    __delay_ms(20);
    BRIGHTNESS_VU_LEFT = BRIGHTNESS_VU_DEFAULT;   
    BRIGHTNESS_VU_RIGHT = BRIGHTNESS_VU_DEFAULT;
    writeSPI_DAC(VU_LEFT, BRIGHTNESS_VU_LEFT);
    writeSPI_DAC(VU_RIGHT, BRIGHTNESS_VU_LEFT);
    
    set_Pilot_Brightness(BRIGHTNESS_PILOT_DEFAULT);
    set_LED_bias(LED_DEFAULT_BIAS); // turn LEDs off; 255 = off, 0 = full bias on; should never be 0 -- maybe 128 or so
    gLinkChannels =0;
    //                _init_EEPROM_firsttime(); //handled in EEPROM now, left commented out here for development use only
    
    //init fake EEPROM for initial development testing
    for (uint16_t fake_index = 0; fake_index < MAX_EEPROM_INDEX; fake_index ++)
    {
        fake_EEPROM.bytes[fake_index] = _EEread(fake_index);
    }
    
    
    //EERPOM for user settings
    Recall_EEPROM_settings(); //this should be called after controls_INIT() and gLinkChannels =0 above
    //exercise_EEPROM_for_wraparound();  //debug, not for production
    
    
    blink_counter = 0; // 256 with rollover
    
    //breathe autostandby also senses control GPIO input if we wish to easter egg an autostandby bypass...
    breathe_autostandby();  //wait for high tension (HT) power supply to come up and change the state of HT_READY; in the meantime Pilot LED "breathes"      
    
    set_Pilot_Brightness(BRIGHTNESS_PILOT_DEFAULT);
    
#ifdef DAC_EXPERIMENT_PLAYGROUND_DEBUG
        writeSPI_DAC(10,0);
        writeSPI_DAC(11,0);
        writeSPI_DAC(12,0);
    while(1)
    {
        ClrWdt();
        //        set_Mode_LED_Color(LEFT, 128, 20, 20);
        //        __delay_ms(10);
        //        for(uchar_t R=255; R > 200 ; R--)
        //        {
        //            //set_Mode_LED_Color(LEFT, R, 20, 20);
        //            set_LED_bias(R);
        //            __delay_ms(1);
        //            ClrWdt();  // clear the watchdog timer
        //        }
//        set_LED_bias(200); 
//        set_Mode_LED_Color(LEFT, 200, 200, 120);
//        set_Mode_LED_Color(RIGHT, 200, 200, 120);
//        
        writeSPI_DAC(10,120);
        __delay_ms(10);
//        writeSPI_DAC(11,0);
//        writeSPI_DAC(12,0);
//
//        writeSPI_DAC(10,0);
//        writeSPI_DAC(11,120);
//        writeSPI_DAC(12,0);
        
    }
#endif
    
    LATCbits.LC1 = 0;  //PS_Operate... Active Low
    // Initialize the device
    
    // Main loop
    while (1) {
        signed char controls_changed;
        
        ClrWdt();  // clear the watchdog timer
        
        read_All_GPIO_Bytes(); //read the entire field of GPIO bytes (the front panel mechanical knob/switch conrtols etc.)
        
        controls_changed = debounce_Changed();  //debounce determines if controls changed since last time through the loop
        if (controls_changed)  
        {
            calculate_Channel_Relay_Outputs();  // only calculates what the relay/LED outputs should be based upon switches/controls inputs and modes of operation; does _not_ actually write out to GPIO chips
            
            //updateDebugStatic_7segmentDisplay();  //debug, last knob/control operated will show up here...  Remains lit statically until next control is operated by the human
            write_All_GPIO_Bytes(); //write out the entire set of shadowed GPIO bytes calculated above
            
            //TODO: add writing out LED DAC levels (if they have changed?)
        }
    } //main loop
    
}