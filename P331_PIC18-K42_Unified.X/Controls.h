 /* File:   Controls.h
  * Author: _
  *
  * Created on November 12, 2017, 2:27 PM
  */

#ifndef CONTROLS_H
#define	CONTROLS_H

#include "Globals.h"  //includes typedef for uchar

//user configurable defines
#define MODE_COUNTER_PRESSHOLD_THRESHOLD 50  /*amount of times function is called for press/hold of System Mode momentary switch to enter LED dimming and additional/general mode switch debounce*/
#define MODE_COUNTER_INCREMENT 1

//set the debounce threshold to number of times we loop and increment counters for each bin in the GPIO controls field
#define DEBOUNCE_THRESHOLD 2  /* debounced bit must remain high for <DEBOUNCE_TRHESHOLD> cycles through the (truncated/fast loop due to keep-reading-until-change) main loop */

//----------------------------------

signed char ModeSW_Timer_Counter[IN_STEREO];  //signed so that we can keep track of press/hold of mode momentary switch using signed numbers
uchar_t ModeSW_increment[IN_STEREO];

 // function prototypes
void controls_INIT(void);
void read_All_GPIO_Bytes(void);
void write_All_GPIO_Bytes(void);

char debounce_Changed(void);  //returns true for any controls that changed after debouncing... else false

void interpret_Loading_Controls(uchar_t channel);
void interpret_Xformer_Control(uchar_t channel);
void interpret_OutputLevel_Controls(uchar_t channel);
void interpret_FilterLift_Control(uchar_t channel);
void interpret_Channel_Mode(uchar_t channel);

void calculate_Channel_Relay_Outputs(void);

void set_Tube_Gain(uchar_t channel, uchar_t gain_level, uchar_t FF_Mode);
//void set_Output_Gain(uchar_t channel, uchar_t gain_level, uchar_t pBOOST, uchar_t enable);
//void set_Output_Gain(uchar_t channel, uchar_t gain_level, uchar_t boost, uchar_t enable, uchar_t cut, uchar_t resolution); 
//void set_Output_Gain(uchar_t channel, uchar_t outlevel_knob, uchar_t outlevel_override, uchar_t boost, uchar_t cut, uchar_t resolution) //channel, overall_gain_less_pad, (overall_gain_less_pad < new_gain), (overall_gain_less_pad-new_gain != 0))
//void set_Output_Gain(uchar_t channel, uchar_t outlevel_knob, uchar_t outlevel_override, uchar_t boost, uchar_t cut, uchar_t resolution); 
//void set_Output_Gain(uchar_t channel, uchar_t outlevel_knob, uchar_t outlevel_override, uchar_t boost, uchar_t cut, uchar_t resolution, uchar_t boost_d,  uchar_t enable_d);
void set_Output_Gain(uchar_t channel, uchar_t outlevel_knob, signed char outlevel_override, uchar_t boost, uchar_t cut, signed char resolution, signed char outlevel_fractional_override, signed char sign);
void set_Xformer(uchar_t channel, uchar_t xformer_knob_value);
void set_FilterLift(uchar_t channel, uchar_t lift_knob_value);


void Display_Write(uchar_t Side, uchar_t idx_char);
void set_Display_decimal_point(uchar_t channel, uchar_t on);

void set_Pad_Attenuation(uchar_t side, uchar_t switch_pos);

void setStatic_Display(uchar_t channel, uchar_t idx_char);
void updateDebugStatic_7segmentDisplay(void);
uchar_t getStatic_Display(uchar_t channel); //gets last value written to memory for the display to show...

void _onChange(uchar_t channel, uchar_t current_value, uchar_t *previous, uchar_t *previous_display);

//Loading Relay bits per switch position, based upon whether FF is 0 or 1
//Note: There is no K01 or K12 as these are handled by circular relay logic 
//      implemented in hardware (for relay count reduction in BOM), 
//      aided with encodings for K_FF and K_FB in lookup table below 
//      to complete the circular seam implemented in logic.
uint16_t bmpLoading_FF_Relay_LUT[2][12] =   //2D array lookup table (LUT): [12] is for switch positions, [2] for FF=0,FF=1
{
    {
        //FF = [0]   feedback mode (clean mode)
        //      Read Relay K Na|mes Downward
        //|    KKKKKKKKKKKK    |   K_FF = K_Bloom on schematic
        //|    F1100000000F <--|   K_FB = K_Clean on schematic                   
        //|xxxxF1098765432B    V   For this mode, K_FB is always 0, K_FF is always 1
         0b0000100000000001,      //SWPOS:1  [0]                 0b0000100000000000,      //SWPOS:1  [0]        
         0b0000100000000010,      //SWPOS:2  [1]        
         0b0000100000000100,      //SWPOS:3  [2]       
         0b0000100000001000,      //SWPOS:4  [3]      
         0b0000100000010000,      //SWPOS:5  [4]     
         0b0000100000100000,      //SWPOS:6  [5]    
         0b0000100001000000,      //SWPOS:7  [6]   
         0b0000100010000000,      //SWPOS:8  [7]  
         0b0000100100000000,      //SWPOS:9  [8] 
         0b0000101000000000,      //SWPOS:10 [9]
         0b0000110000000000,      //SWPOS:11 [10]
         0b0000100000000000       //SWPOS:12 [11]             0b0000100000000001       //SWPOS:12 [11]    //TODO: is this right KFF?

    },
    {   
        //FF = [1]  Feed forward mode (bloom mode)
        //      Read Relay K Na|mes Downward
        //|    KKKKKKKKKKKK    |   K_FF = K_Bloom on schematic
        //|    F1100000000F <--|   K_FB = K_Clean on schematic                   
        //|xxxxF1098765432B    V   For this mode, K_FB is always 1, K_FF is always 0
         0b0000100000000001,      //SWPOS:1  [0]                 0b0000000000000001,      //SWPOS:1  [0]        
         0b0000000000000011,      //SWPOS:2  [1]        
         0b0000000000000101,      //SWPOS:3  [2]       
         0b0000000000001001,      //SWPOS:4  [3]      
         0b0000000000010001,      //SWPOS:5  [4]     
         0b0000000000100001,      //SWPOS:6  [5]    
         0b0000000001000001,      //SWPOS:7  [6]   
         0b0000000010000001,      //SWPOS:8  [7]  
         0b0000000100000001,      //SWPOS:9  [8] 
         0b0000001000000001,      //SWPOS:10 [9]
         0b0000010000000001,      //SWPOS:11 [10]
         0b0000000000000001       //SWPOS:12 [11]           0b0000100000000001       //SWPOS:12 [11]  
    }
};
/* 7-segment display maps per LED section (a, b, c, d, etc.)*/
uchar_t bmpDisplay_Value[18] = {  // 7-segment LED display available for debug
     // Pgfedcba   Segments (active Low). Decimal point ("P") is off (=1)        
      0b11000000,  //0
      0b11111001,  //1
      0b10100100,  //2
      0b10110000,  //3
      0b10011001,  //4
      0b10010010,  //5
      0b10000010,  //6
      0b11111000,  //7
      0b10000000,  //8
      0b10011000,  //9
      0b10001000,  //a
      0b10000011,  //b
      0b10100111,  //C
      0b10100001,  //d
      0b10000110,  //E
      0b10001110,  //F
      0b01111111,   //ALL SEGMENTS OFF
      0b10101010  //special character for special test mode
    };

#define SPECIAL_TEST_MODE_CHAR 17

//is this LUT only used for EVL????
/* Pad relays and output level gain relays are 7 relays decoded from 12 position switch*/
signed char bmp7_Lift_Mode_Compesation[12] = { 
      0,
      1,
      1,
      1,
      1,
      2,
      2,
      2,
      3,
      3,
      4,
      4
    };
#define SIGN 0
#define FRACTIONAL_LOOKUP 2
#define WHOLE_LOOKUP 1
#define PAD_LOOKUP 3
#define dB0P0  0
#define dB0P25 1
#define dB0P5  2
#define dB0P75 3
#define NEG 0b10000000
#define POS 0b00000000

/* Pad relays and output level gain relays are 7 relays decoded from 12 position switch*/
//note: first column is whole dB, second column is fractional dB
//note: fractional is the second column in these array lookup tables... as such the negative sign is only valid for the whole dB; do not use negative signs in the fractionals

#ifdef EVL
//golden standard
signed char bmp7_Compesation_PRE_CLEAN_MODE[12][3] = { 
    {NEG, 1, dB0P75},       //0
    {NEG, 1, dB0P75},       //1
    {NEG, 1, dB0P75},       //2
    {NEG, 1, dB0P5},        //3
    {NEG, 1, dB0P5},        //4
    {NEG, 1, dB0P5},        //5
    {NEG, 1, dB0P5},        //6
    {NEG, 1, dB0P5},        //7
    {NEG, 1, dB0P25},       //8
    {NEG, 1, dB0P25},       //9
    {NEG, 1, dB0P25},       //10
    {NEG, 1, dB0P25},       //11
    
    
    
};
signed char bmp7_Compesation_POST_CLEAN_MODE[12][3] = { 
    {NEG, 1, dB0P75},       //0
    {NEG, 2, dB0P0},        //1
    {NEG, 1, dB0P25},       //2
    {NEG, 0, dB0P75},       //3
    {NEG, 0, dB0P25},       //4
    {NEG, 0, dB0P75},       //5
    {NEG, 0, dB0P25},        //6
    {POS, 0, dB0P0},        //7
    {NEG, 0, dB0P25},       //8
    {POS, 0, dB0P0},        //9
    {NEG, 0, dB0P5},        //10
    {POS, 0, dB0P25},        //11
    
    
    
    
};

signed char bmp7_Compesation_PRE_BLOOM_MODE[12][3] = { 
    {NEG, 1, dB0P0},        //0
    {NEG, 0, dB0P75},       //1
    {NEG, 0, dB0P75},       //2
    {NEG, 0, dB0P75},        //3
    {NEG, 1, dB0P0},        //4
    {NEG, 1, dB0P0},        //5
    {NEG, 1, dB0P0},       //6
    {NEG, 1, dB0P0},       //7
    {NEG, 1, dB0P0},       //8
    {NEG, 1, dB0P0},        //9
    {NEG, 1, dB0P0},        //10
    {NEG, 1, dB0P0},       //11
    
};

signed char bmp7_Compesation_POST_BLOOM_MODE[12][3] = { 
    {NEG, 1, dB0P0},        //0
    {NEG, 1, dB0P0},        //1
    {NEG, 0, dB0P5},       //2
    {NEG, 0, dB0P75},       //3
    {NEG, 0, dB0P25},       //4
    {NEG, 0, dB0P75},        //5
    {NEG, 0, dB0P5},       //6
    {NEG, 0, dB0P0},       //7
    {NEG, 0, dB0P5},       //8
    {NEG, 0, dB0P0},       //9
    {NEG, 0, dB0P5},        //10
    {POS, 0, dB0P25},        //11
    
    
};


signed char bmp7_Compesation_OPEN_CLEAN_MODE[12][4] = { 
    /*sign, integer dB, fractional dB, PAD value 0~11*/
    {POS, 1, dB0P0, 11},    //0
    {POS, 1, dB0P25, 8},    //1
    {POS, 1, dB0P25, 5},    //2
    {POS, 1, dB0P75, 4},    //3
    {POS, 2, dB0P0, 3},     //4
    {POS, 2, dB0P0, 2},     //5
    {POS, 1, dB0P5, 0},     //6
    {POS, 1, dB0P75, 0},    //7
    {POS, 2, dB0P5, 0},     //8
    {POS, 2, dB0P75, 0},    //9
    {POS, 3, dB0P25, 0},    //10
    {POS, 4, dB0P0, 0},     //11
    
};

signed char bmp7_Compesation_OPEN_BLOOM_MODE[12][4] = { 
    /*sign, integer dB, fractional dB, PAD value 0~11*/
    {POS, 1, dB0P25, 6},    //0
    {POS, 0, dB0P75, 2},    //1
    {POS, 1, dB0P0, 2},     //2
    {POS, 1, dB0P25, 2},    //3
    {POS, 1, dB0P75, 2},    //4
    {POS, 1, dB0P5, 1},     //5
    {POS, 1, dB0P5, 1},     //6
    {POS, 1, dB0P75, 0},    //7
    {POS, 2, dB0P25, 0},    //8
    {POS, 2, dB0P75, 0},    //9
    {POS, 3, dB0P25, 0},    //10
    {POS, 4, dB0P0, 0},     //11
    
};
#endif

#ifdef P331_ORIGINAL
//**********
// Note that as of SN: 133 we need to decrease gain by a fixed 0.5dB or so.... 
//**********
#ifdef SN_GREATER_THAN_133 /*this define is set in globals.h */
signed char bmp7_Compesation_PRE_CLEAN_MODE[12][3] = { 
    {NEG, 0, dB0P5},        //0
    {NEG, 0, dB0P5},        //1
    {NEG, 0, dB0P5},        //2
    {NEG, 0, dB0P5},        //3
    {NEG, 0, dB0P5},        //4
    {NEG, 0, dB0P5},        //5
    {NEG, 0, dB0P5},        //6
    {NEG, 0, dB0P5},        //7
    {NEG, 0, dB0P5},        //8
    {NEG, 0, dB0P5},        //9
    {NEG, 0, dB0P5},        //10
    {NEG, 0, dB0P5},        //11
    
    
    
};
signed char bmp7_Compesation_POST_CLEAN_MODE[12][3] = { 
    {NEG, 0, dB0P5},        //0
    {POS, 0, dB0P0},        //1
    {POS, 0, dB0P0},        //2
    {POS, 0, dB0P0},        //3
    {POS, 0, dB0P25},        //4
    {POS, 0, dB0P5},        //5
    {POS, 0, dB0P25},        //6
    {POS, 0, dB0P0},        //7
    {POS, 0, dB0P5},        //8
    {POS, 0, dB0P25},       //9
    {POS, 0, dB0P25},       //10
    {POS, 0, dB0P5},        //11
    
};

signed char bmp7_Compesation_PRE_BLOOM_MODE[12][3] = { 
    {NEG, 0, dB0P5},       //0
    {POS, 0, dB0P0},        //1
    {POS, 0, dB0P0},        //2
    {NEG, 0, dB0P75},       //3
    {NEG, 0, dB0P5},        //4
    {NEG, 0, dB0P75},        //5
    {NEG, 0, dB0P5},        //6
    {NEG, 0, dB0P5},        //7
    {NEG, 0, dB0P5},        //8
    {NEG, 0, dB0P5},        //9
    {NEG, 0, dB0P5},        //10
    {NEG, 0, dB0P5},        //11
    
    
    
};

signed char bmp7_Compesation_POST_BLOOM_MODE[12][3] = { 
    {NEG, 0, dB0P5},        //0
    {POS, 0, dB0P25},        //1
    {NEG, 0, dB0P75},       //2
    {NEG, 1, dB0P0},        //3
    {NEG, 1, dB0P0},        //4
    {NEG, 1, dB0P0},        //5
    {NEG, 1, dB0P25},        //6
    {NEG, 1, dB0P0},        //7
    {NEG, 0, dB0P5},        //8
    {NEG, 0, dB0P25},        //9
    {POS, 0, dB0P25},        //10
    {POS, 0, dB0P5},        //11
    
    
};


signed char bmp7_Compesation_OPEN_CLEAN_MODE[12][4] = { 
    /*sign, integer dB, fractional dB, PAD value 0~11*/
    {NEG, 0, dB0P5, 0},     //0
    {NEG, 0, dB0P5, 0},     //1
    {NEG, 0, dB0P5, 0},     //2
    {NEG, 0, dB0P5, 0},     //3
    {NEG, 0, dB0P5, 0},     //4
    {NEG, 0, dB0P5, 0},     //5
    {NEG, 0, dB0P5, 0},     //6
    {NEG, 0, dB0P5, 0},     //7
    {NEG, 0, dB0P5, 0},     //8
    {NEG, 0, dB0P5, 0},     //9 Note, feathering back to 0 offset
    {NEG, 0, dB0P25, 0},     //10 Note, feathering back to 0 offset
    {POS, 0, dB0P0, 0},     //11 Note, feathering back to 0 offset
    
};

signed char bmp7_Compesation_OPEN_BLOOM_MODE[12][4] = { 
    /*sign, integer dB, fractional dB, PAD value 0~11*/
    {NEG, 0, dB0P5, 0},     //0
    {NEG, 0, dB0P5, 0},     //1
    {NEG, 0, dB0P5, 0},     //2
    {NEG, 0, dB0P5, 0},     //3
    {NEG, 0, dB0P5, 0},     //4
    {NEG, 0, dB0P5, 0},     //5
    {NEG, 0, dB0P5, 0},     //6
    {NEG, 0, dB0P5, 0},     //7
    {NEG, 0, dB0P5, 0},     //8
    {NEG, 0, dB0P5, 0},     //9  Note, feathering back to 0 offset
    {NEG, 0, dB0P25, 0},     //10 Note, feathering back to 0 offset
    {POS, 0, dB0P0, 0},     //11 Note, feathering back to 0 offset
    
};

///new 08OCT2022 LIFT mode compensation for P331 original (do we need these for EVL too?)
//these LUTs are in combination with the old integer (whole dB) LUT used for LIFT mode compensation
//consider these LUTs the fine tuning (and are deviations from) of the the normal non-LIFT mode compensation LUTs in POST only mode....
//that is, these LUTs below were direct copies of the normal non-LIFT compensation for POST only mode, directly copied from above, and then tweaked small amounts (fine adjustment), given that the integer LUT for LIFT compensation is also applied in the controls_algorithm.c file
//procedure: copy the post clean/bloom tables from above to these locations, then fine adjust from there for these below...
signed char bmp7_Compesation_LIFT_POST_CLEAN_MODE[12][3] = { 
    {NEG, 0, dB0P5},        //0
    {NEG, 1, dB0P0},        //1
    {NEG, 0, dB0P5},        //2
    {POS, 0, dB0P0},        //3
    {POS, 0, dB0P5},        //4
    {POS, 0, dB0P0},        //5
    {POS, 0, dB0P25},        //6
    {POS, 0, dB0P5},        //7
    {POS, 0, dB0P25},        //8
    {POS, 0, dB0P25},       //9
    {NEG, 0, dB0P5},       //10
    {POS, 0, dB0P0},        //11

};

signed char bmp7_Compesation_LIFT_POST_BLOOM_MODE[12][3] = { 
    {NEG, 0, dB0P75},        //0
    {NEG, 0, dB0P75},        //1
    {NEG, 0, dB0P5},       //2
    {NEG, 0, dB0P75},        //3
    {NEG, 0, dB0P25},        //4
    {NEG, 0, dB0P75},        //5
    {NEG, 0, dB0P75},        //6
    {NEG, 0, dB0P25},        //7
    {NEG, 0, dB0P75},        //8
    {NEG, 0, dB0P25},        //9
    {NEG, 0, dB0P5},        //10
    {POS, 0, dB0P0},        //11
    
};




// HOWEVER!.... These LUTs are for the PRE (Green LED) MODE, as such we only historically had whole dB (integer) steps available and these LUTs for PRE now have the capability to do fractional compensations
// (previously the PRE was off by fractionals, but we didn't have fractional capability in the past)
// procedure:  copy the non-LIFT LUTs from above, and then fine tweak from there...
signed char bmp7_Compesation_LIFT_PRE_CLEAN_MODE[12][3] = { 
    {NEG, 0, dB0P75},        //0
    {NEG, 0, dB0P75},       //1
    {NEG, 0, dB0P75},       //2
    {NEG, 0, dB0P75},       //3
    {NEG, 0, dB0P75},       //4
    {NEG, 0, dB0P75},       //5
    {NEG, 0, dB0P75},       //6
    {NEG, 0, dB0P75},       //7
    {NEG, 0, dB0P75},       //8
    {NEG, 0, dB0P75},       //9
    {NEG, 0, dB0P75},       //10
    {NEG, 0, dB0P75},       //11

};

signed char bmp7_Compesation_LIFT_PRE_BLOOM_MODE[12][3] = { 
    {NEG, 0, dB0P75},      //0
    {NEG, 0, dB0P5},       //1
    {NEG, 0, dB0P5},       //2
    {NEG, 0, dB0P5},       //3
    {NEG, 0, dB0P75},       //4
    {NEG, 0, dB0P75},        //5
    {NEG, 0, dB0P75},       //6
    {NEG, 0, dB0P75},       //7
    {NEG, 0, dB0P75},       //8
    {NEG, 0, dB0P75},       //9
    {NEG, 0, dB0P75},       //10
    {NEG, 0, dB0P75},       //11
    
};
#else /* end of P331 above (greater than) serial number 133 */

//ORIGINAL P331 before SN:133 

signed char bmp7_Compesation_PRE_CLEAN_MODE[12][3] = { 
    {POS, 0, dB0P0},        //0
    {POS, 0, dB0P0},        //1
    {POS, 0, dB0P0},        //2
    {POS, 0, dB0P0},        //3
    {POS, 0, dB0P0},        //4
    {POS, 0, dB0P0},        //5
    {POS, 0, dB0P0},        //6
    {POS, 0, dB0P0},        //7
    {POS, 0, dB0P0},        //8
    {POS, 0, dB0P0},        //9
    {POS, 0, dB0P0},        //10
    {POS, 0, dB0P0},        //11
    
    
    
};
signed char bmp7_Compesation_POST_CLEAN_MODE[12][3] = { 
    {POS, 0, dB0P0},        //0
    {POS, 0, dB0P5},        //1
    {POS, 0, dB0P5},        //2
    {POS, 0, dB0P5},        //3
    {POS, 0, dB0P75},        //4
    {POS, 1, dB0P0},        //5
    {POS, 0, dB0P75},        //6
    {POS, 0, dB0P5},        //7
    {POS, 1, dB0P0},        //8
    {POS, 0, dB0P75},       //9
    {POS, 0, dB0P75},       //10
    {POS, 1, dB0P0},        //11
    
};

signed char bmp7_Compesation_PRE_BLOOM_MODE[12][3] = { 
    {POS, 0, dB0P0},       //0
    {POS, 0, dB0P5},        //1
    {POS, 0, dB0P5},        //2
    {POS, 0, dB0P25},       //3
    {POS, 0, dB0P0},        //4
    {POS, 0, dB0P25},        //5
    {POS, 0, dB0P0},        //6
    {POS, 0, dB0P0},        //7
    {POS, 0, dB0P0},        //8
    {POS, 0, dB0P0},        //9
    {POS, 0, dB0P0},        //10
    {POS, 0, dB0P0},        //11
    
    
    
};

signed char bmp7_Compesation_POST_BLOOM_MODE[12][3] = { 
    {POS, 0, dB0P0},        //0
    {POS, 0, dB0P75},        //1
    {POS, 0, dB0P25},       //2
    {NEG, 0, dB0P5},        //3
    {NEG, 0, dB0P5},        //4
    {NEG, 0, dB0P5},        //5
    {NEG, 0, dB0P75},        //6
    {NEG, 0, dB0P5},        //7
    {POS, 0, dB0P0},        //8
    {POS, 0, dB0P25},        //9
    {POS, 0, dB0P75},        //10
    {POS, 1, dB0P0},        //11
    
    
};


signed char bmp7_Compesation_OPEN_CLEAN_MODE[12][4] = { 
    /*sign, integer dB, fractional dB, PAD value 0~11*/
    {POS, 0, dB0P0, 0},     //0
    {POS, 0, dB0P0, 0},     //1
    {POS, 0, dB0P0, 0},     //2
    {POS, 0, dB0P0, 0},     //3
    {POS, 0, dB0P0, 0},     //4
    {POS, 0, dB0P0, 0},     //5
    {POS, 0, dB0P0, 0},     //6
    {POS, 0, dB0P0, 0},     //7
    {POS, 0, dB0P0, 0},     //8
    {POS, 0, dB0P0, 0},     //9
    {POS, 0, dB0P0, 0},     //10
    {POS, 0, dB0P0, 0},     //11
    
};

signed char bmp7_Compesation_OPEN_BLOOM_MODE[12][4] = { 
    /*sign, integer dB, fractional dB, PAD value 0~11*/
    {POS, 0, dB0P0, 0},     //0
    {POS, 0, dB0P0, 0},     //1
    {POS, 0, dB0P0, 0},     //2
    {POS, 0, dB0P0, 0},     //3
    {POS, 0, dB0P0, 0},     //4
    {POS, 0, dB0P0, 0},     //5
    {POS, 0, dB0P0, 0},     //6
    {POS, 0, dB0P0, 0},     //7
    {POS, 0, dB0P0, 0},     //8
    {POS, 0, dB0P0, 0},     //9
    {POS, 0, dB0P0, 0},     //10
    {POS, 0, dB0P0, 0},     //11
    
};

///new 08OCT2022 LIFT mode compensation for P331 original (do we need these for EVL too?)
//these LUTs are in combination with the old integer (whole dB) LUT used for LIFT mode compensation
//consider these LUTs the fine tuning (and are deviations from) of the the normal non-LIFT mode compensation LUTs in POST only mode....
//that is, these LUTs below were direct copies of the normal non-LIFT compensation for POST only mode, directly copied from above, and then tweaked small amounts (fine adjustment), given that the integer LUT for LIFT compensation is also applied in the controls_algorithm.c file
//procedure: copy the post clean/bloom tables from above to these locations, then fine adjust from there for these below...
signed char bmp7_Compesation_LIFT_POST_CLEAN_MODE[12][3] = { 
    {POS, 0, dB0P0},        //0
    {NEG, 0, dB0P5},        //1
    {POS, 0, dB0P0},        //2
    {POS, 0, dB0P5},        //3
    {POS, 1, dB0P0},        //4
    {POS, 0, dB0P5},        //5
    {POS, 0, dB0P75},        //6
    {POS, 1, dB0P0},        //7
    {POS, 0, dB0P75},        //8
    {POS, 0, dB0P75},       //9
    {POS, 0, dB0P0},       //10
    {POS, 0, dB0P5},        //11

};

signed char bmp7_Compesation_LIFT_POST_BLOOM_MODE[12][3] = { 
    {NEG, 0, dB0P25},        //0
    {NEG, 0, dB0P25},        //1
    {POS, 0, dB0P0},       //2
    {NEG, 0, dB0P25},        //3
    {POS, 0, dB0P25},        //4
    {NEG, 0, dB0P25},        //5
    {NEG, 0, dB0P25},        //6
    {POS, 0, dB0P25},        //7
    {NEG, 0, dB0P25},        //8
    {POS, 0, dB0P25},        //9
    {POS, 0, dB0P0},        //10
    {POS, 0, dB0P5},        //11
    
};




// HOWEVER!.... These LUTs are for the PRE (Green LED) MODE, as such we only historically had whole dB (integer) steps available and these LUTs for PRE now have the capability to do fractional compensations
// (previously the PRE was off by fractionals, but we didn't have fractional capability in the past)
// procedure:  copy the non-LIFT LUTs from above, and then fine tweak from there...
signed char bmp7_Compesation_LIFT_PRE_CLEAN_MODE[12][3] = { 
    {NEG, 0, dB0P25},        //0
    {NEG, 0, dB0P25},       //1
    {NEG, 0, dB0P25},       //2
    {NEG, 0, dB0P25},       //3
    {NEG, 0, dB0P25},       //4
    {NEG, 0, dB0P25},       //5
    {NEG, 0, dB0P25},       //6
    {NEG, 0, dB0P25},       //7
    {NEG, 0, dB0P25},       //8
    {NEG, 0, dB0P25},       //9
    {NEG, 0, dB0P25},       //10
    {NEG, 0, dB0P25},       //11

};

signed char bmp7_Compesation_LIFT_PRE_BLOOM_MODE[12][3] = { 
    {NEG, 0, dB0P25},      //0
    {POS, 0, dB0P0},       //1
    {POS, 0, dB0P0},       //2
    {POS, 0, dB0P0},       //3
    {NEG, 0, dB0P25},       //4
    {NEG, 0, dB0P25},        //5
    {NEG, 0, dB0P25},       //6
    {NEG, 0, dB0P25},       //7
    {NEG, 0, dB0P25},       //8
    {NEG, 0, dB0P25},       //9
    {NEG, 0, dB0P25},       //10
    {NEG, 0, dB0P25},       //11
    
};

#endif /*ORIGINAL P331 before serial number 133*/
#endif /*ifdef ORIGINAL P331*/

#ifdef DEPRECATED
//note 1-based index for this zero-base index!!!!!
#define PAD_NEEDED 3 /*index of which pad dB value to use */
#ifdef PAD_COMP
//all entries should be positive to get the outlevel_override out of using too many cut positions....
signed char bmp7_Compesation_OPEN_MODE_FULL_CUT[4][4] = { 
    //outputlevel compensation sign, whole, frac, then pad to use as passive pad in EVL mode does not play nicely to 1dB steps...
    {POS, 0, dB0P0, 1 },   //output offset for pad offset = -1dB
    {POS, 0, dB0P0, 2 },   //output offset for pad offset = -2dB
    {POS, 0, dB0P0, 3 },   //output offset for pad offset = -3dB
    {POS, 0, dB0P0, 4 }    //output offset for pad offset = -4dB
};
#endif
#endif

/* Pad relays and output level gain relays are 7 relays decoded from 12 position switch*/
uchar_t bmp7_Relays_from_12postions[12] = { 
    //  XKKKKKKK 
    //  X6543210   (active high, K1 is inverted for power failure considerations)
      0b00000000,  //0  -- all relays off
      0b00000001,  //1
      0b00000110,  //2
      0b00000111,  //3
      0b00001010,  //4
      0b00001011,  //5
      0b00010010,  //6
      0b00010011,  //7
      0b00100010,  //8
      0b00100011,  //9
      0b01000010,  //10
      0b01000011   //11
    };

#if DEPRECATED 
typedef struct xformer_mode2_LUT {
    uchar_t pad_abs;
    uchar_t tube_gain_abs;
    uchar_t gain_offset;
    
}xformer_mode2_LUT;
#endif


//the four knobs stored as a byte value for each channel...
typedef struct channel_control_values_t {
    uchar_t loading_knob;                     //* Loading knob value as uchar, 0 through 11 in decimal (represents dB)
    uchar_t loading_mode_switch;              //* Loading mode bit / switch value as uchar: <BR> 0 = Feedforward mode (bloom) <BR> 1 = Feedback mode (clean)
    uchar_t input_pad_value;                  //* Input PAD attenuation value desired as uchar, 0 through 11 in decimal (represents dB)
    uchar_t lift_knob;                        //* Lift knob value as uchar, 0 through 3 in decimal (2-bits): <BR> 0 = lift/filter is OUT <BR> 1 = HF <BR> 2 = LF <BR> 3 = Duo
    uchar_t xformer_knob;                     //* Transformer knob value as uchar, 0 through 2 in decimal <BR> (still 2-bits; decode using 2-bit/4 positions as filter knob code does; just mechanically limited rotary switch to 3 positions): <BR> 0 = Transformer is OUT <BR> 1 = Transformer IN and Mode 1 selected <BR> 2 = Transformer IN and Mode 2 selected
    uchar_t output_level_knob;                //* Output Level gain/attenuation knob value as uchar, 0 through 11 decimal (represents dB attenuating or gaining depending upon BOOST or CUT mode and gain block is not OUT)
    uchar_t _output_gain_boost_mode;          //* calculated BOOST mode from Output Level gain/attenuation mode as uchar.<BR> 0 = CUT (if _output_gain_enable) <BR> 1 = BOOST (if _output_gain_enable)
    uchar_t _output_gain_cut_mode;            //* calculated CUT mode from Output Level gain/attenuation mode as uchar.<BR> 1 = CUT (if _output_gain_enable) <BR> 0 = BOOST (if _output_gain_enable)
    uchar_t _output_gain_enable;              //* calculated Output Level gain/attenuator enabled as uchar.  <BR> 0 = not enabled; gain is 0dB <BR> 1 = enable the output gain/attenuation setting (depending upon BOOST or CUT)
    uchar_t output_resolution_switch;         //* Resolution switch setting, whether 0dB (=0), 0.25dB (=1), or 0.5dB (=2) gain or attenuation (additive to current whole dB setting, but with opposite method of gain/attenuation)<BR>If whole dB setting is for gain, then resolution setting will be attenuative.  <BR> If whole dB setting is for attenuation, then resolution setting will be gainful.
}channel_control_values_t;

channel_control_values_t previous_control_value[IN_STEREO];
channel_control_values_t previous_display_value[IN_STEREO];

channel_control_values_t current_control_value[IN_STEREO];

#if DEPRECATED 
typedef struct output_gain {  // do we need?
    uchar_t pBOOST;
    uchar_t enable;
    uchar_t output_level_abs;
    signed char true_gain;
    
}output_gain;
#endif

//* Debounce counter _bytes_ for EACH input bit in the GPIO_ChannelFrame struct/union. 
typedef union debounce_counters_t {
    struct counters 
    {
        uchar_t pCUT;            			//* Debounce bit counter for: Gain Mode Switch (Cut), Front Panel
        uchar_t pBOOST;          			//* Debounce bit counter for: Gain Mode Switch (Boost), Front Panel
        uchar_t bLOADING_A0;     			//* Debounce bit counter for: Loading Knob bit 0
        uchar_t bLOADING_A1;     			//* Debounce bit counter for: Loading Knob bit 1
        uchar_t bLOADING_A2;     			//* Debounce bit counter for: Loading Knob bit 2
        uchar_t bLOADING_A3;     			//* Debounce bit counter for: Loading Knob bit 3
        uchar_t pXFORMER_A0;     			//* Debounce bit counter for: Transformer Mode Knob bit 0
        uchar_t pXFORMER_A1;     			//* Debounce bit counter for: Transformer Mode Knob bit 1

        uchar_t pFILTER_A0;      			//* Debounce bit counter for: Filter Mode Knob bit 0
        uchar_t pFILTER_A1;      			//* Debounce bit counter for: Filter Mode Knob bit 1
        uchar_t bOUTLEVEL_A0;    			//* Debounce bit counter for: Output Level Gain Knob bit 0
        uchar_t bOUTLEVEL_A1;    			//* Debounce bit counter for: Output Level Gain Knob bit 1
        uchar_t bOUTLEVEL_A2;    			//* Debounce bit counter for: Output Level Gain Knob bit 2
        uchar_t bOUTLEVEL_A3;    			//* Debounce bit counter for: Output Level Gain Knob bit 3
        uchar_t pFRAC_0P5DB;              //* Debounce bit counter for: Output Level Fractional gain/attenuation 0.5dB
        uchar_t pFRAC_0DB;                //* Debounce bit counter for: Output Level Fractional gain/attenuation 0dB

        uchar_t pFB_CLEAN;          		//* Debounce bit counter for: Front Panel input for Tube Opamp mode (feedback or feedforward mode of operation, e.g., clean versus bloom modes)
        uchar_t pMODE;              		//* Debounce bit counter for: Front Panel input for Mode momentary toggle position (far right) to operate overall System mode (open, pre, post, link, LED brightness control)
        uchar_t bLINEAMP_ENGAGE;      	//* Debounce bit counter for: Input to sense if bypass switch is active

    }Counters;
 
    uchar_t debounce_counters_bytes[sizeof(struct counters)];  //* Debounce counters as byte index.  See also debounce_counters for individual member's names.

}debounce_counters_t;

//declare the debounce counters for both the high and low states...

debounce_counters_t debounce_low[IN_STEREO];  // debounce counters for low states (zeros) in each of the bits of the input GPIO; a byte-wide counter for each bit
debounce_counters_t debounce_hi[IN_STEREO];   // debounce counters for hi states (ones) in each of the bits of the input GPIO; a byte-wide counter for each bit

//Note: this #define is very specific to the debounce routine for debouncing single bits using a pattern of code; not for use in other locations
#define DEBOUNCE_BIT(which_bit)        /*also check x bit on Exar GPIO output chip */                                           \
        if(bouncyGPIO_Values[channel].Bits.which_bit != debouncedGPIO_Values[channel].Bits.which_bit)                           \
        {                                                                                                                       \
            /*which_bit debounce counting*/                                                                                     \
            if(bouncyGPIO_Values[channel].Bits.which_bit)                                                                       \
            {                                                                                                                   \
                debounce_hi[channel].Counters.which_bit ++;                                                                     \
                debounce_low[channel].Counters.which_bit = 0; /* reset the counter for the other opposite state */              \
            }                                                                                                                   \
            else                                                                                                                \
            {                                                                                                                   \
                debounce_low[channel].Counters.which_bit ++;                                                                    \
                debounce_hi[channel].Counters.which_bit = 0; /* reset the counter for the other opposite state */               \
            }                                                                                                                   \
            /*which_bit debounce check if greater than threshold such that we should set the new value */                       \
            if (debounce_low[channel].Counters.which_bit > DEBOUNCE_THRESHOLD)                                                  \
            { /* then it must be true it's a zero and debounced :)  */                                                          \
                debouncedGPIO_Values[channel].Bits.which_bit = 0;  /*note: uchar promotion for this local mask calculation*/    \
                debounce_low[channel].Counters.which_bit = 0; /* reset the counter */                                           \
                controls_changed = TRUE;                                                                                        \
            }                                                                                                                   \
            if (debounce_hi[channel].Counters.which_bit > DEBOUNCE_THRESHOLD)                                                   \
            { /* then it must be true it's a one and debounced :) */                                                            \
                debouncedGPIO_Values[channel].Bits.which_bit = 1;                                                               \
                debounce_hi[channel].Counters.which_bit = 0; /* reset the counter */                                            \
                controls_changed = TRUE;                                                                                        \
            }                                                                                                                   \
        }/*end of checking which_bit...   */                                                           
   

#else
#endif	/* CONTROLS_H */

