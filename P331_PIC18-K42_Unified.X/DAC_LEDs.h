/* 
 * File:   DAC_LEDs.h
 * Author: _
 *
 * Created on June 1, 2018, 8:21 PM
 */

#ifndef DAC_LEDS_H
#define	DAC_LEDS_H

#ifdef	__cplusplus
extern "C" {
#endif
    //global brightness variables for VU meter brightness
    uchar_t	    BRIGHTNESS_VU_LEFT;
    uchar_t	    BRIGHTNESS_VU_RIGHT;
    uchar_t	    BRIGHTNESS_PILOT;
    uchar_t previous_pilot_level;      

#define BRIGHTNESS_VU_DEFAULT 100
#define BRIGHTNESS_PILOT_DEFAULT 200
    
//prototypes
uchar_t writeSPI_DAC(uchar_t DACchannel, uchar_t data); // this is a WOM (write-only memory)
void set_Pilot_Brightness(uchar_t level);
void set_LED_bias(uchar_t level);
void breathe_autostandby(void);
void set_Mode_LED_by_Color_Map(uchar_t channel, uchar_t *RGB);
void set_Mode_LED_Color(uchar_t channel, uchar_t R, uchar_t G, uchar_t B);

// RGB LED colour definitions (nominal factory)
#define RGB_LED_YELLOW   0,   80,  128
#define RGB_LED_ORANGE 200,  200,  120
#define RGB_LED_RED    128,    0,    0
#define RGB_LED_GREEN    0,  218,    0
#define RGB_LED_BLUE    10,   10,  200
#define RGB_LED_WHITE  180,  140,  180
#define RGB_LED_OFF     10,   10,   95

#define COLOR_SPACE 3

uchar_t color_map[][COLOR_SPACE] = 
{
    RGB_LED_YELLOW ,
    RGB_LED_ORANGE ,
    RGB_LED_RED    ,
    RGB_LED_GREEN  ,
    RGB_LED_BLUE   ,
    RGB_LED_OFF    
};

#define RGB_LED(x) color_map[(x)]

enum rgb_colors
{
    rgb_led_yellow ,
    rgb_led_orange ,
    rgb_led_red    ,
    rgb_led_green  ,
    rgb_led_blue   ,
    rgb_led_off    
}rgb_colors;

enum //do not use these unless in explicit low level code
{
    Pilot = 1,
    VU_LEFT = 2,
    VU_RIGHT = 3,
    _bias =4,
    OutLevelGainMode_RIGHT =5,  //CUT LED but generic name here in case of repurposing or front panel silk screen change (or firmware repurposing)
    Mode_R_RIGHT =6,
    Mode_G_RIGHT =7,
    Mode_B_RIGHT =8,
    OutLevelGainMode_LEFT =9,  //CUT LED but generic name here in case of repurposing or front panel silk screen change (or firmware repurposing)
    Mode_R_LEFT =10,
    Mode_G_LEFT =11,
    Mode_B_LEFT =12
};

enum //LED_OFFSET_enum for channels
{
    LED_Pilot               = 1, //* Pilot LED 
    LED_VU                  = 2, //* VU LEDs, use GET_LED_OFFSET(...): <BR> uses proper ternary macro to calculate correct offset as channel offset is not swapped like other LEDs, (where channel #defines are LEFT = 0 , RIGHT = 1)
                                 //Note: we skip 2 because of LED_VU plus channel offset for RIGHT channel (VU LEDs are not channel swapped in the enum or schematics like the other LEDs are)
    _LED_bias               = 4, //* LED bias voltage (hardware global): <BR> changes global LED opamp driver bias, e.g., for dimming globally...  use GET_LED_OFFSET(...) which uses proper ternary macro to calculate correct offset to address this singular bias voltage
    LED_OUTPUT_LEVEL_MODE   = 5, //* Output Level Mode (CUT) LED: <BR> Note these LEDs are for RIGHT channel, LEFT channel is offset by another 4, as is calculated by GET_LED_OFFSET(...) and the ternary macros
    LED_RED                 = 6, //* RED for RGB Mode LED: <BR> Note these LEDs are for RIGHT channel, LEFT channel is offset by another 4, as is calculated by GET_LED_OFFSET(...) and the ternary macros
    LED_GREEN               = 7, //* GREEN for RGB Mode LED: <BR> Note these LEDs are for RIGHT channel, LEFT channel is offset by another 4, as is calculated by GET_LED_OFFSET(...) and the ternary macros
    LED_BLUE                = 8  //* BLUE for RGB Mode LED: <BR> Note these LEDs are for RIGHT channel, LEFT channel is offset by another 4, as is calculated by GET_LED_OFFSET(...) and the ternary macros
}ChannelLEDs;


////use ChannelLEDs enum:  Note because the schematic has incremental left and right LEDs swapped and are one 4-channel bank higher, we need to swap channel argument with subtraction and add offsets; VU LEDs are not swapped, thus simple addition of channel for offset
//#define GET_LED_OFFSET(channel, LED_OFFSET_enum)  ( ( (LED_OFFSET_enum) > _LED_bias) ? (1-(channel) << 2) + (LED_OFFSET_enum)  :                          \
//                                                    ( (LED_OFFSET_enum) == LED_VU)   ? (channel)          + (LED_OFFSET_enum)  : (LED_OFFSET_enum) )
// optimized the above #define into the below #define

//use ChannelLEDs enum:  Note because the schematic has incremental left and right LEDs swapped and are one 4-channel bank higher, we need to swap channel argument with subtraction and add offsets; VU LEDs are not swapped, thus simple addition of channel for offset
#define GET_LED_OFFSET(channel, LED_OFFSET_enum)  ((uchar_t)( (LED_OFFSET_enum) + ( ( (LED_OFFSET_enum) > _LED_bias ) ? ((1-(channel)) << 2) :                          \
                                                                          ( (LED_OFFSET_enum) == LED_VU   ) ?     (channel)        : 0 ) ))


typedef union LED_DAC_values_t
{
    struct LED_DACs 
    {
        uchar_t	    Pilot;
        uchar_t	    VU_LEFT;
        uchar_t	    VU_RIGHT;
        uchar_t	    _bias;
        uchar_t	    OutLevelGainMode_RIGHT;  //CUT LED but generic name here in case of repurposing or front panel silk screen change (or firmware repurposing)
        uchar_t	    Mode_R_RIGHT;
        uchar_t	    Mode_G_RIGHT;
        uchar_t	    Mode_B_RIGHT;
        uchar_t	    OutLevelGainMode_LEFT;  //CUT LED but generic name here in case of repurposing or front panel silk screen change (or firmware repurposing)
        uchar_t	    Mode_R_LEFT;
        uchar_t	    Mode_G_LEFT;
        uchar_t	    Mode_B_LEFT;
    }LED_DACs;

#define LED_DACS_VALUES_T_SZ sizeof(struct LED_DACs)
    
    uchar_t LED_DAC_by_INDEX[LED_DACS_VALUES_T_SZ]; 
    
}LED_DAC_values_t;


//breathing quarter-sine lookup table for LED DAC output values
uchar_t breathe_LUT[128] = {
    3,
    6,
    9,
    12,
    16,
    19,
    22,
    25,
    28,
    32,
    35,
    38,
    41,
    44,
    48,
    51,
    54,
    57,
    60,
    63,
    67,
    70,
    73,
    76,
    79,
    82,
    85,
    88,
    91,
    94,
    97,
    100,
    103,
    106,
    109,
    112,
    115,
    118,
    120,
    123,
    126,
    129,
    132,
    134,
    137,
    140,
    143,
    145,
    148,
    150,
    153,
    156,
    158,
    161,
    163,
    166,
    168,
    171,
    173,
    175,
    178,
    180,
    182,
    184,
    187,
    189,
    191,
    193,
    195,
    197,
    199,
    201,
    203,
    205,
    207,
    209,
    211,
    213,
    214,
    216,
    218,
    220,
    221,
    223,
    224,
    226,
    227,
    229,
    230,
    232,
    233,
    234,
    236,
    237,
    238,
    239,
    240,
    241,
    242,
    243,
    244,
    245,
    246,
    247,
    248,
    249,
    249,
    250,
    251,
    251,
    252,
    252,
    253,
    253,
    254,
    254,
    254,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255
};



#ifdef	__cplusplus
}
#endif

#endif	/* DAC_LEDS_H */

