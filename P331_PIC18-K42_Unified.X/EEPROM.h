/* 
 * File:   EEPROM.h
 * Author: _
 *
 * Created on July 21, 2018, 9:23 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

//user configurable macros
#define DISABLE_ISRs_DURING_EEPROM
#undef ENABLE_ISRs_DURING_EEPROM
#undef LIVE_USE_EEPROM  /* when undef'ed uses fake EERPOM (RAM really) for debug */
#define EEPROM_SIZE 1024U  /*1024 bytes, but we should skip the first 0 address (and user settings struct size block)*/

//includes
#include <limits.h>
#include "Globals.h"
#include <xc.h>  //neded for ClrWdt() during EEPROM write and wait for finish
#include "DAC_LEDs.h"

#define MAX_WRITE_CAPABILITY UINT_MAX /* yes, this may be only 65k writes for a chip that boasts 100k writes (depending upon temperature etc.), but conservative with 256 4-byte buckets (=1024 bytes) and 65k writes to each bucket yields 16.64 MILLION writes of user settings to the wear-leveled EEPROM here...*/

#ifdef	__cplusplus
extern "C" {
#endif

//prototypes
void _find_EEPROM_head_tail(); // during testing if we are using the fake EEPROM (RAM actually) then it should have been initialized to 0 in main() before we start using the "EEPROM"
uint16_t _EEPROM_next_index(uint16_t index);  //gets the next bucket with MOD wrap around circular buffer behaviour
void Recall_EEPROM_settings(); // only happens on boot up, once..., but could be called multiple times during intial testing when triggered by the developer
void _EEwrite(uint16_t addr, uchar_t data); 
uchar_t _EEread(uint16_t addr); 
void write_User_Settings_EEPROM();
void read_User_Settings_EEPROM();
void apply_User_Settings_shadowEE();
void _init_EEPROM_firsttime();

void exercise_EEPROM_for_wraparound();

//bucket of user settings to be stored in EEPROM (or fake RAM emulation thereof)
//typedef union user_settings_t
//{
//
//    struct user_settings {
//        uint16_t num_writes_to_this_bucket; //for wear leveling...
//        uchar_t brightness_level;
//        uchar_t VU_L_brightness;
//        uchar_t VU_R_brightness;
//        uchar_t link_mode     :1;
//        uchar_t channel_mode_LEFT  :2;
//        uchar_t channel_mode_RIGHT :2;
//    }user_settings;
//
//#define USER_SETTINGS_T_SZ sizeof(struct user_settings)
//
//    uchar_t settings_as_array[USER_SETTINGS_T_SZ];
//}user_settings_t;

typedef union user_settings_t
{

    struct user_settings {
        uchar_t channel_mode_LEFT   :2;
        uchar_t channel_mode_RIGHT  :2;
        uchar_t link_mode           :1;
        uchar_t autocut_mode        :2;
        uchar_t _write_flip          :1;
        uchar_t brightness_level;
        uchar_t VU_L_brightness;
        uchar_t VU_R_brightness;
        uint16_t checksum;

    }bucket;

#define USER_SETTINGS_T_SZ sizeof(struct user_settings)

    uchar_t bytes[USER_SETTINGS_T_SZ];
}user_settings_t;

user_settings_t shadowed_user_settings;
user_settings_t current_user_settings;

uchar_t user_settings_write_clock;

#define EEPROM_WRITE_CLOCK_COUNT_TRHESHOLD 50

#define MAX_BUCKETS ((EEPROM_SIZE) / USER_SETTINGS_T_SZ)  
#define ACTUAL_BUCKETS_ODD ((MAX_BUCKETS - 1) + (MAX_BUCKETS % 2)) 
#define MAX_EEPROM_INDEX ((ACTUAL_BUCKETS_ODD * USER_SETTINGS_T_SZ) )   /*must contain an odd number of buckets for 1-bit head-tail ascertainment*/

union fake_EEPROM{
    user_settings_t buckets[ACTUAL_BUCKETS_ODD];
    uchar_t bytes[EEPROM_SIZE];
} fake_EEPROM;

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

