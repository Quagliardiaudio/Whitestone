
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef _SPI_H_
#define _SPI_H_

#include "Globals.h"  //includes typedef for uchar

void Init_SPI_Bus(void);    
uchar_t get_SPI_Data(void);
uchar_t put_SPI_Shift(uint16_t);
uchar_t put_SPI(uchar_t);

uchar_t writeSPI_XR_GPIO(uchar_t chip_select, uchar_t command, uchar_t data);
extern uchar_t readSPI_XR_GPIO(uchar_t chip_select, uchar_t command);

#endif
    


