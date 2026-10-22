#ifndef _HARDWARE_H_
#define _HARDWARE_H_

 // Definitions
#define _XTAL_FREQ  16000000        /* this is used by the __delay_ms(xx) and __delay_us(xx) functions  */

#define RST_GPIO_outhi     LATCbits.LC0  /*trigger hard reset of GPIO expansion chips*/
#define PS_OPERATE_outlo   PORTCbits.RC1 /*Output to solid state relays on Power Supply to bypass inrush current limiting devices*/
#define HT_READY           PORTCbits.RC2 /*Output to solid state relays on Power Supply to bypass inrush current limiting devices*/


// GPIO expansion chip fitted on the PCB (8 chips: CS_A..CS_D for each channel)
#define GPIO_EXP_XRA1405   1   /* MaxLinear (Exar) XRA1405IL24-F, original boards */
#define GPIO_EXP_MCP23S18  2   /* Microchip MCP23S18-E/MJ (24-QFN, open-drain outputs) */
#define GPIO_EXP_MCP23S17  3   /* Microchip MCP23S17T-E/ML (28-QFN, push-pull outputs), fitted on P331_Rear_IO vx3 */

#define GPIO_EXPANDER      GPIO_EXP_MCP23S17   /* select the chip fitted on the board being programmed */

// MCP23S17 and MCP23S18 share the same register map and SPI frame, so they use the same driver code
#define GPIO_EXP_IS_MCP23SXX  ((GPIO_EXPANDER == GPIO_EXP_MCP23S17) || (GPIO_EXPANDER == GPIO_EXP_MCP23S18))


#endif


