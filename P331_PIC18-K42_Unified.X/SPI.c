
#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "Hardware.h"
#include "Globals.h"
#include "GPIO.h"
#include "SPI.h"
//*************************************************************************************

void Init_SPI_Bus(void)
{
    // this should already be done by the MCC in MPLAB X
}

uchar_t set_cs(uchar_t chip_select_idx)
{
    uchar_t cs = 0x01U; // chip select (high for bit shifting until inversion below)
    
    LATA = (uchar_t) ~(cs << chip_select_idx) ;     //set chip select  (active low, hence the ~)
  
    __delay_us(100);
    return LATA;
    
}

uchar_t put_SPI(uchar_t SPIData)
{
  return SPI1_ExchangeByte(SPIData);
}

uchar_t writeSPI_XR_GPIO(uchar_t chip_select, uchar_t command, uchar_t data)
{
    uchar_t return_data = 0x00;
   set_cs(chip_select);
    put_SPI((uchar_t)(command << 1) & 0b01111111U);     //force read/write bit to 0 (write) after shifting command by 1 bit per SPI address format in Exar chip
   __delay_us(100);
    return_data = put_SPI(data);
    LATA = 0xFFU; //unset chip selects  (all ones; active low)
    __delay_us(100); 
    
    return ((uchar_t) return_data);
    
}
uchar_t readSPI_XR_GPIO(uchar_t chip_select, uchar_t command)
{
    uchar_t return_data = 0x00;
      set_cs(chip_select);
    put_SPI((uchar_t)(command << 1) | 0b10000000U);     //also set the read bit, and shift by 1 for address format of registers within Exar chip SPI
    return_data = put_SPI(0x00);                            //bogus data to start SPI clocking of 8 bits
    LATA = 0xFF; //unset chip selects (all ones; active low)
    __delay_us(100); 
    return (return_data);  
}

#ifdef OLD_PIC_P331_DO_NOT_USE_FOR_NEW_PIC18F26K42   
    
void Init_SPI_Bus(void){
    
    // SPI bus init
    SSPCON1bits.SSPEN = 0;  //Turn off SPI bus
   
    //0010 = SPI Master mode, clock = FOSC/64
    //0001 = SPI Master mode, clock = FOSC/16
    //0000 = SPI Master mode, clock = FOSC/4
    SSPCON1bits.SSPM = 1; // see above comments
    
    SSPCON1bits.CKP = 0;  // Clock idle low
    SSPSTATbits.CKE = 1;
    SSPSTATbits.SMP = 0;
    SSPCON1bits.SSPEN = 1;  //Turn on SPI bus
     
    return;
}

uchar_t set_cs(uchar_t chip_select_idx)
{
    uchar_t cs = 0x01U; // chip select (high for bit shifting until inversion below)
    
    LATA = (uchar_t) ~(cs << chip_select_idx) ;     //set chip select  (active low, hence the ~)
  
    __delay_us(100);
    return LATA;
    
}
uchar_t put_SPI(uchar_t SPIData)
{
      
        SSPBUF  = SPIData;
        while(!SSPSTATbits.BF); // wait until bus cycle complete
        
        return (SSPBUF);
}

uchar_t writeSPI_XR_GPIO(uchar_t chip_select, uchar_t command, uchar_t data)
{
    
    set_cs(chip_select);
    put_SPI((uchar_t)(command << 1) & 0b01111111U);     //force read/write bit to 0 (write) after shifting command by 1 bit per SPI address format in Exar chip
   __delay_us(100);
    put_SPI(data);
    LATA = 0xFFU; //unset chip selects  (all ones; active low)
    __delay_us(100); 
    
    return ((uchar_t) SSPBUF);
}

uchar_t readSPI_XR_GPIO(uchar_t chip_select, uchar_t command)
{
    set_cs(chip_select);
    put_SPI((uchar_t)(command << 1) | 0b10000000U);     //also set the read bit, and shift by 1 for address format of registers within Exar chip SPI
    put_SPI(0x00);                            //bogus data to start SPI clocking of 8 bits
    LATA = 0xFF; //unset chip selects (all ones; active low)
    __delay_us(100); 
    return (SSPBUF);
}

#endif
