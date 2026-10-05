#include "mcc_generated_files/memory.h"
#include "EEPROM.h"
#include "GPIO.h"
#include "Globals.h"

uint16_t EE_head;
uint16_t EE_tail;
uint16_t max_EEPROM_index;

void _init_EEPROM_firsttime()
{
    uchar_t flip_bit = 0b10000000; // note: this will be flipped below (again) first before writing
    //EEPROM never used here...
    _EEwrite(EE_tail, 0); //double write the first location to prevent weirdness
    
    for(uint16_t init_index = EE_tail; init_index < max_EEPROM_index; init_index += USER_SETTINGS_T_SZ)
    {   
        flip_bit = (~flip_bit) & 0b10000000;
        _EEwrite(init_index, flip_bit); // notice write_flip_bit flip here starts with 0x00 due to double flip from normal EEPROM 0xFF init state....
    }

}
//NOTE: number of EEPROM writable buckets MUST BE ODD!!!!!
void _find_EEPROM_head_tail() // during testing if we are using the fake EEPROM (RAM actually) then it should have been initialized to 0 in main() before we start using the "EEPROM"
{
    max_EEPROM_index = MAX_EEPROM_INDEX;
    
    for (uint16_t index = 0; index < max_EEPROM_index; index += USER_SETTINGS_T_SZ)
    {
        EE_tail = index;
        EE_head = _EEPROM_next_index(index); //  includes one wrap-around to the 0 index (poor man's modulus) if the end of the index and zero-wrap-around are our actual location
        
        uchar_t byte0[2];
        
        byte0[0] = _EEread(EE_tail);
        byte0[1] = _EEread(EE_head);
        
        if((byte0[0] & 0b10000000) == ( byte0[1] & 0b10000000)) // the first bit in the bucket is a write toggle bit... we look for them to be the same which tells us where we left off last time in this circular queue, assuming number of buckets MUST be odd of course... (as defined in #define MAX_EEPROM_INDEX math)
        {
            if(byte0[1] == 0xFF)
            {
                _init_EEPROM_firsttime();
            }
            return;
        }
    }
        
}

uint16_t _EEPROM_next_index(uint16_t index)  //gets the next bucket with MOD wrap around circular buffer behaviour
{
    index+=USER_SETTINGS_T_SZ;
    
    if(index >= max_EEPROM_index) index = 0; 
    return index;
}

void Recall_EEPROM_settings() // only happens on boot up, once..., but could be called multiple times during intial testing when triggered by the developer
{
    shadowed_user_settings.bytes[0] = 0;

    _find_EEPROM_head_tail();
    read_User_Settings_EEPROM();   //shadows EEprom into RAM   (sets shadowed_user_settings which gets the write_flip_bit)
    apply_User_Settings_shadowEE();  //applies RAM/EEPROM user settings
    current_user_settings.bytes[0] = shadowed_user_settings.bytes[0];

    //remember what we need to prevent an erroneous trigger an EEPROM write 
    user_settings_write_clock = 0;

}


// -------------------------------------------------------------------------------------

void _EEwrite(uint16_t addr, uchar_t data) 
{
    DATAEE_WriteByte(addr, data); 
}
uchar_t _EEread(uint16_t addr) 
{
    return DATAEE_ReadByte(addr);   
}

#ifdef OLD_P331_PIC18_K20
void _EEwrite(uint16_t addr, uchar_t data) 
{
    uchar_t GIEBitValue = INTCONbits.GIE;

    EEADRH = (addr >> 8) & 0x03;
    EEADR  = (addr &0xFF);
    EEDATA = data;
    EECON1bits.EEPGD=0;     // Point to Data EEPROM Memory (not program EEPROM data!!!!)
    EECON1bits.CFGS=0;      // Access EEPROM
    EECON1bits.WREN=1;      //Allow write operations to EEPROM
    INTCONbits.GIE = 0;     // disattiva interrupt globali... e USB?
    EECON2 = 0x55;          // Write 55h
    EECON2 = 0xAA;          // Write AAh
    EECON1bits.WR=1;        // write.
    do 
    {
        ClrWdt();
    } while(EECON1bits.WR); // occupato ? 

    EECON1bits.WREN=0;      // disabilita write.
    INTCONbits.GIE = GIEBitValue;     // attiva interrupt globali

    uchar_t readback;
    readback = _EEread(addr);
    if(readback != data)
        fake_EEPROM.bytes[addr] = 0b01010101;
    else
        fake_EEPROM.bytes[addr] = readback;
}
  
uchar_t _EEread(uint16_t addr) 
{

    EEADRH = (addr >> 8) & 0x03;
    EEADR  = (addr &0xFF);
    EECON1bits.CFGS=0;      // Access EEPROM
    EECON1bits.EEPGD=0;     // Point to Data Memory
    EECON1bits.RD=1;        // EE Read
    NOP();
    NOP();
    return EEDATA;          // W = EEDATA
}
#endif

void write_User_Settings_EEPROM()
{
    uchar_t EE_write_flip_bit;

    EE_write_flip_bit = shadowed_user_settings.bucket._write_flip;// _EEread(EE_head);  //TODO: should we use our flip bit in memory or whats actually there in EEPROM when we land on a cell/bucket?
//    EE_write_flip_bit >>= 7; //shadowed_user_settings.bucket.write_flip; _EEread(EE_head);  //TODO: should we use our flip bit in memory or whats actually there in EEPROM when we land on a cell/bucket?
    
    shadowed_user_settings.bucket.link_mode = gLinkChannels;
//TODO:...    shadowed_user_settings.user_settings.brightness_level = ; 
    shadowed_user_settings.bucket.channel_mode_LEFT = ChannelMode[LEFT] & 0b00000011;
    shadowed_user_settings.bucket.channel_mode_RIGHT = ChannelMode[RIGHT]  & 0b00000011;
    current_user_settings.bytes[0] = shadowed_user_settings.bytes[0];
    shadowed_user_settings.bucket._write_flip = ~EE_write_flip_bit & 0b00000001; //flip the bit

    
    //tmp debug:
    shadowed_user_settings.bucket.VU_L_brightness = BRIGHTNESS_VU_LEFT;
    shadowed_user_settings.bucket.VU_R_brightness = BRIGHTNESS_VU_RIGHT;
    shadowed_user_settings.bucket.brightness_level =0;
    shadowed_user_settings.bucket.checksum = 0;
    
    _EEwrite(EE_head, 0); // double write the first time????
    
    for (uchar_t byte_num = 0; byte_num < USER_SETTINGS_T_SZ; byte_num++)
    {
        uchar_t the_byte = shadowed_user_settings.bytes[byte_num];

        uint16_t addr = byte_num + EE_head;
        
        _EEwrite(addr, the_byte);

    }

//set ourselves to the next write bucket location (circularly)
    EE_tail = EE_head; //increment the tail first, then increment the head
    EE_head = _EEPROM_next_index(EE_tail); 



}

void read_User_Settings_EEPROM()
{
    
    for (uchar_t byte_num = 0; byte_num < USER_SETTINGS_T_SZ; byte_num++)
    {
        shadowed_user_settings.bytes[byte_num] = _EEread(byte_num + EE_tail);
    }
}

void apply_User_Settings_shadowEE()
{

    gLinkChannels = shadowed_user_settings.bucket.link_mode;
    //TODO: set_LED_bias(shadowed_user_settings.user_settings.brightness_level);
    if( shadowed_user_settings.bucket.channel_mode_LEFT < _enum_max_SetLEDBrightness )
    {
        ChannelMode[LEFT] = shadowed_user_settings.bucket.channel_mode_LEFT;
    }
    else
    {
        ChannelMode[LEFT] = PadMatchTube;  // data cell is corrupted, default to something
    }
    
    if( shadowed_user_settings.bucket.channel_mode_RIGHT < _enum_max_SetLEDBrightness )
    {
        ChannelMode[RIGHT] = shadowed_user_settings.bucket.channel_mode_RIGHT;
    }
    else
    {
        ChannelMode[RIGHT] = PadMatchTube; // data cell is corrupted, default to something
    }
    
    autoCut_mode[LEFT] = shadowed_user_settings.bucket.autocut_mode & 0b00000001U;
    autoCut_mode[RIGHT] = (shadowed_user_settings.bucket.autocut_mode & 0b00000010U) >> 1;
 
    BRIGHTNESS_VU_LEFT = shadowed_user_settings.bucket.VU_L_brightness;
    BRIGHTNESS_VU_RIGHT = shadowed_user_settings.bucket.VU_R_brightness;

    writeSPI_DAC(VU_LEFT, shadowed_user_settings.bucket.VU_L_brightness);
    writeSPI_DAC(VU_RIGHT, shadowed_user_settings.bucket.VU_R_brightness);

    
}

//for development testing only, not for production.... use breakpoints in the if statement to evaluate fake eeprom memory
void exercise_EEPROM_for_wraparound()
{
    int bucket_max = 4 * ACTUAL_BUCKETS_ODD;
    for(int bucket = 0; bucket < bucket_max; bucket++ )
    {
        write_User_Settings_EEPROM();
        Recall_EEPROM_settings();
        
        if(!(bucket % ACTUAL_BUCKETS_ODD))
        {
            //wrap around!
            int foo = 1;
            foo--;
        }
        
    }
          
}
