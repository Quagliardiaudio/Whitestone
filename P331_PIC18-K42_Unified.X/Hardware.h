#ifndef _HARDWARE_H_
#define _HARDWARE_H_

 // Definitions
#define _XTAL_FREQ  16000000        /* this is used by the __delay_ms(xx) and __delay_us(xx) functions  */

#define RST_GPIO_outhi     LATCbits.LC0  /*trigger hard reset of GPIO expansion chips*/
#define PS_OPERATE_outlo   PORTCbits.RC1 /*Output to solid state relays on Power Supply to bypass inrush current limiting devices*/
#define HT_READY           PORTCbits.RC2 /*Output to solid state relays on Power Supply to bypass inrush current limiting devices*/




#endif


