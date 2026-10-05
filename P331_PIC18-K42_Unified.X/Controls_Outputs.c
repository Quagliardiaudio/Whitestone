
#include "Controls.h"
#include "GPIO.h"
#include "SPI.h"
#include "Hardware.h"  //needed for __delay(x)
#include <xc.h> // include standard header file
#include "Globals.h"
#include "DAC_LEDs.h"

//sets the pad relays... for guitar amplifier preamp gain and master volume methodologies, this could change to update the output gain/attenuation versus using the input pads (and vice versa)
void set_Pad_Attenuation(uchar_t channel, uchar_t attenuation_value_as_nonnegative)
{
    //clear bits associated with loading knob relays
    OUTPUT_GPIO[channel].byte_groups[K_PAD_BYTE_GROUP]  &= MASK_PAD_RELAYS; //ignore the top most bit unrelated to Pad relays in the struct
    
    //tmp for AES 2018
    //if(attenuation_value_as_nonnegative == 0) attenuation_value_as_nonnegative = 1;
    
    //set bits
    current_control_value[channel].input_pad_value = attenuation_value_as_nonnegative; //bmp7_Relays_from_12postions[attenuation_value_as_nonnegative]; 
    OUTPUT_GPIO[channel].byte_groups[K_PAD_BYTE_GROUP]  |=  (uchar_t) bmp7_Relays_from_12postions[attenuation_value_as_nonnegative] & 0b01111111U; //LUT has a 0 in the MSb so this should be OK...
    
}

//set the tube PCB relays and feedback/feedforward mode (relays) using bitmap lookup table for each loading knob switch position versus relay states (bits in the lookup bitmap table)
void set_Tube_Gain(uchar_t channel, uchar_t gain_level, uchar_t FF_Mode)
{
    uint16_t bmpLoadingRelay;  //temp variable to hold lookup table bitmap ("bmp" prefix) for relay patterns over 12 bits/position switch range implemented into relays on tube board (including FF and FB modes implemented in 12 relays)

    //get the lookup table value word (2 bytes) which contains the 12 bits to set the vacuum tube loading relays
    bmpLoadingRelay = bmpLoading_FF_Relay_LUT[FF_Mode][gain_level];  

    //clear bits associated with loading knob relays
    OUTPUT_GPIO[channel].byte_groups[K_LOADING_BYTE_GROUP]   &= ~MASK_LOADING_RELAYS_0;
    OUTPUT_GPIO[channel].byte_groups[K_LOADING_BYTE_GROUP+1] &= ~MASK_LOADING_RELAYS_1;  //ignore and mask for other bits in the struct unrelated to loading knob relays

    //define a byte pointer into the two-byte lookup table that we can index into...
    uchar_t *p_bmpLoadingRelayLUTValue;  // treat this 2-byte integer in the lookup table for the loading knob (2-bytes for 12 relays) as two bytes of uchar for byte-wise operations below
    p_bmpLoadingRelayLUTValue = (uchar_t *)&bmpLoadingRelay;  //treat it as a byte pointer, that we can index[] into memory by hand below to get the upper few bits from the lookup table...

    //set bits using bits from lookup table (bitmap "bmp...")
    OUTPUT_GPIO[channel].byte_groups[K_LOADING_BYTE_GROUP]     |= (uchar_t) (p_bmpLoadingRelayLUTValue[0]);   //lower byte
    OUTPUT_GPIO[channel].byte_groups[K_LOADING_BYTE_GROUP + 1] |= (uchar_t) (p_bmpLoadingRelayLUTValue[1]);   //higher byte  (mask out higher bits unrelated to loading knob relays)

    //debug display: check to see if there is any change from previous value
    //_onChange(channel, gain_level, &(previous_control_value[channel].loading_knob), &(previous_display_value[channel].loading_knob));

}

//set the output level gain/attenuation including fractional gain (fractional gain read from debouncedGPIO_Values)  -- TODO: would fractional gain work if whole dB gain is overridden by some other mode of operation?
//void set_Output_Gain(uchar_t channel, uchar_t gain_level, uchar_t boost, uchar_t enable, uchar_t cut, uchar_t resolution)
void set_Output_Gain(uchar_t channel, uchar_t outlevel_knob, signed char outlevel_override, uchar_t boost, uchar_t cut, signed char resolution, signed char outlevel_fractional_override, signed char sign) //channel, overall_gain_less_pad, (overall_gain_less_pad < new_gain), (overall_gain_less_pad-new_gain != 0))
{
    //this variable is needed below, as resolution relay does not act intuitively for 0dB output level knob position in CUT mode; 
    //we use this to recalculate the boost/cut relay mode as the default hardware implementation assumes boost by default and prior code was lazy evaluation hence non-intuitive to use case...
    uchar_t dboost; //bit calculation for boost relay output
    uchar_t denable; //default: FOR denable above!!! = 0: fractional on CUT, whole dB on BOOST... 1: factional on BOOST, whole dB on CUT

    //intermediate calculations for the two resolution relays: they should disengage when in "out" mode
    uchar_t K_0P25dB;  //0.25dB (center-off on switch, other bits signals pulled high, e.g., 0db and 0.5db -- they're both active low otherwise)
    uchar_t K_0P5dB;   //0dB (left switch position, pulled low, 0.5dB right switched pulled high, and not used in this calculation)

    //keep track of which direction we need to have the resolution step (+1 or -1) which is calcaulted and solely dependent upon the outlevel_knob position
    signed char resolution_direction;
    
    //calculate if we are boosting or enabled based upon the new calculated desired gain
    
//do not do this here...   outlevel_override = sign ? -outlevel_override : outlevel_override; 
    
    dboost = ((outlevel_override > 0)  ? 1 : 0 ) ;  
    // dboost == 1 means fractional on CUT; whole dB on BOOST
    // dboost == 0 means fractional on BOOST; whole dB on CUT

    denable = (outlevel_override != 0 || resolution !=0) ? 1 : 0;
    //calculate boost and enable above, BEFORE we recondition outlevel_override below


    if (!denable)
    {
        outlevel_override = 0; //override gain level to 0 (all gain relays off) if we are not enabled
        resolution = 0; // disable the resolution too...
    }
#ifdef DEAD_C0DE    
    resolution_direction = boost ? -1 : 1;
    if((cut || boost) && outlevel_knob == 0)
    {
        resolution_direction = (signed char) (~(unsigned char)resolution_direction) | 0x01U;  //fancy bitwise operations to change -1 to 1 or 1 to -1 (signed) without using multiplication methods (e.g., without ....*= -1); it's opposite different for the zero position on the output level knob    
    }   
    
    // dboost == 1 means fractional on CUT; whole dB on BOOST
    // dboost == 0 means fractional on BOOST; whole dB on CUT
    if(
           resolution > 0
       && (cut || boost) 
       && (resolution_direction > 0 && dboost)  //factional is on CUT, but we need to increase by fractional!!!
      )
    {
        outlevel_override += resolution_direction; // abs value (actual virtual knob position) turns knob to the left just like expected; reduced by 1 dB (cutting or boosting as overridden by LUT) only to be added to, by fractionals on opposite side of opamp
        resolution = 4 - resolution; //note: now includes an internal notion of 0.75dB resolution being an internal resolution = 3
    }

    //very special case to get rid off 0 knob cutting resolution actually boosting instead... because erroneously, fractional is on the boost side for this one special condition when it should be on the cut side (dboost needs augmentation)
    if(
          resolution > 0
       && outlevel_override == 0
       && outlevel_knob == 0
       && cut
       && !dboost  //factional is on CUT, but we need to increase by fractional!!!
       )
    {
        dboost = 1; //flips the fractional relays to the cut side instead of the __default__ relay-at-rest BOOST side
    }

#endif /*DEAD_C0DE*/
    
    //convert to a fixed point scheme: format of XXXXXX.YY where X is whole dB and Y is fractional...
    signed char desired_gain_fxp;  // fixed point representation of final overall desired gain, including fractional... shifting the whole dB representation 2-bits and using the lower 2-bits for the fractional dB (0 = 0, 1 = 0.25, 2 = 0.5, 3 = 0.75)
    uchar_t desired_gain_fxp_sign; // keep this as unsigned.. no math is used, only bit awareness when shifting to keep bits as-is when shifting
    
    desired_gain_fxp_sign = sign; //outlevel_override & 0b10000000;
    
    outlevel_fractional_override &= 0b00000011;
    
    signed char frac;
    signed char whole;
    
    desired_gain_fxp =
			(
            (frac = (sign ? -outlevel_fractional_override : outlevel_fractional_override)) // consider the lowest bits first to keep sign bit proper for fractions less than +/- 1
            + (whole = (outlevel_override << 2)) //eats the sign bit, but shifts the value into the upper 6 bits (value is 0 through 11, only uses a few bits anyway)
            ); // the above marries the lookup table's whole and fractional dB columns into a fixed point integer number (whole(6).fractional(2)), while preserving sign from the whole dB column in the lookup tables
    
    //add resolution switch setting to the new desired gain
    if (cut)
    {
        if (outlevel_knob == 0)
        {
            desired_gain_fxp -= resolution;
        }
        else
        {
            desired_gain_fxp += resolution;
        }
    }
    if (boost)
    {
        if (outlevel_knob == 0)
        {
            desired_gain_fxp += resolution;
        }
        else
        {
            desired_gain_fxp -= resolution; 
        }
    }
    
    //if we are above > 1 (in our fixed point scheme it is 1<<2) then we should increment and flip resoltion direction around for 
    if (desired_gain_fxp > (1<<2) && (desired_gain_fxp & 0b00000011) != 0)
    {
        desired_gain_fxp += (1 << 2);
        desired_gain_fxp = desired_gain_fxp & 0b11111100 | (( 4- desired_gain_fxp) & 0b00000011); //flip resolution direction around for > 1
    }
    
    
    //recombine and reconstitute resolution and outlevel_override from the fixed point math manipulation above
    desired_gain_fxp_sign = desired_gain_fxp & 0b10000000;
    resolution = desired_gain_fxp & 0b00000011; // note because of two's complement, we do not need to do "4 - resolution" etc... 
    outlevel_override = desired_gain_fxp >> 2;  // shift back to base whole dB
    outlevel_override |=
			desired_gain_fxp_sign
			| desired_gain_fxp_sign >> 1;			// then restore the sign bit and shift in a 1 (if the number was negative) into bit6 otherwise, it will shift a 0 if positive (if desired_gain_sign is 0)
    
    
    dboost = 0;
    if (outlevel_override >= 1) //&& (resolution > dB0P0)) // if we are greater or equal to 1.0 dB, using the lower 2 bits for the fractional, thus the 1 shifted 2 bits
    {
        dboost = 1;
    }
    if (desired_gain_fxp_sign && outlevel_override == 0 && (resolution > dB0P0)) 
    {
        dboost = 1;
    }
    
    //end new code for fixed-point resolution and whole dB offsets with lookup tables
    
    //apply absolute values, limits and ceilings
    outlevel_override = -11 > outlevel_override ? -11 : outlevel_override;  // take the floor of maximal capability of the relays (0 through 11 for 12 relay "positions" on the resistor ladders)
    outlevel_override = 11 < outlevel_override ? 11 : outlevel_override;  // take the ceiling of maximal capability of the relays (0 through 11 for 12 relay "positions" on the resistor ladders)
    outlevel_override = outlevel_override <0 ? -outlevel_override : outlevel_override;  //absolute value via ternary macro
    
    //clear bits associated with loading knob relays
    OUTPUT_GPIO[channel].byte_groups[K_OUTLEVEL_BYTE_GROUP]     &= MASK_OUTLEVEL_RELAYS_0; // the entire byte is used for output level and will be modified in this function call
    OUTPUT_GPIO[channel].byte_groups[K_OUTLEVEL_BYTE_GROUP + 1] &= MASK_OUTLEVEL_RELAYS_1; // the lower bits pertain to the output level gain, the mask ignores the upper bits


    //set bits for relays based upon gain level in output_level_position...
    OUTPUT_GPIO[channel].byte_groups[K_OUTLEVEL_BYTE_GROUP]  |=  bmp7_Relays_from_12postions[(uchar_t)outlevel_override];  //LUT has a 0 in the MSb so this should be OK...

    
    // set the fractional gain relays to the fractional gain (resolution) switch on the front panel
    //if neither 0dB or 0.5dB are (active low) set, then we must be commanding for 0.25 dB
//    K_0P25dB  = (resolution  == 1) ? 1 : 0;
//    K_0P5dB   = (resolution  == 2) ? 1 : 0;
//    if(resolution == 3) //added resolution = 3 for both relays (0.25 and 0.50 dB) when both engaged will be about 0.75dB; better if 2MOhm is paralleled when both are in....
//    {
//        K_0P25dB = 1;
//        K_0P5dB = 1;
//    }

    K_0P25dB = desired_gain_fxp & 0b00000001;
    K_0P5dB = (desired_gain_fxp & 0b00000010) >> 1;
    
    if(!(cut || boost) || autoCut_mode[channel]){
        K_0P25dB = desired_gain_fxp & 0b00000001;
        K_0P5dB = (desired_gain_fxp & 0b00000010) >> 1;
    } //turn off resolution too, when we are not in cut or boost ("out") mode
 
    
    OUTPUT_GPIO[channel].Bits.dOUTLEVEL_0_25   = K_0P25dB;
    //set 0.5 dB if 0.5dB input bit (active low) is set   
    OUTPUT_GPIO[channel].Bits.dOUTLEVEL_0_5    = K_0P5dB;
    //set pBOOST status... 0 = pCUT (only boosts or cuts if enabled below)
//    OUTPUT_GPIO[channel].Bits.dOUTLEVEL_BOOST  = (uchar_t) (dboost & enable);  
    OUTPUT_GPIO[channel].Bits.dOUTLEVEL_BOOST  = dboost;  // swaps fractional and whole (1=whole db on boost side of opamp, 0 = whole db on cut side of opamp); fractional relays will be on opposite side of opamp (cut side or boost side) opposite of whole db relays

//    if (channel == LEFT) Display_Write(LEFT, current_control_value[LEFT].input_pad_value);
//    if (channel == LEFT) Display_Write(RIGHT, outlevel_override);
//    if (channel == LEFT) set_Display_decimal_point(LEFT, (denable || (K_0P25dB || K_0P5dB) )? 1: 0);
//    if (channel == LEFT) set_Display_decimal_point(RIGHT, dboost);
    
    //CUT LED only illuminates if manual CUT (or if linked and CUTting)
    if(!autoCut_mode[channel])  //added auto cut mode for OPEN mode, which will be taken over by OPEN mode in controls_algorithms.c switch/case statement)
    { //if we were in autocut mode, we shouldn't even touch the CUT LED here... it's handled elsewhere to blink and all that...
    
        if( cut )  //current_control_value[channel]._output_gain_boost_mode)
        {
            writeSPI_DAC(GET_LED_OFFSET(channel, LED_OUTPUT_LEVEL_MODE), 138);
        }
        else
        {
            writeSPI_DAC(GET_LED_OFFSET(channel, LED_OUTPUT_LEVEL_MODE), 0);

        }
    }

    //update the debug display, sets debounce functionality, and check to see if there is any change from previous value
    //_onChange(channel, (uchar_t) gain_level, &previous_control_value[channel].output_level_knob, &previous_display_value[channel].output_level_knob);

}


void set_Xformer(uchar_t channel, uchar_t xformer_knob_value)
{
    //mode 1 P11 on schematic
    debouncedGPIO_Values[channel].Bits.dXFORMER_MODE2   = (uchar_t) ((xformer_knob_value > 1) ? 1 : 0); 
    
    //set bits for the Xformer relays (see the schematics to see how these work); NOTE: 10MAR2021, xformer modes are inverted logic
    debouncedGPIO_Values[channel].Bits.dXFORMER_PRI     = (uchar_t) ((xformer_knob_value > 0) ? 0 : 1);//inverted logic, fixed 10MAR2021
    debouncedGPIO_Values[channel].Bits.dXFORMER_SEC     = (uchar_t) ((xformer_knob_value > 0) ? 0 : 1);//inverted logic, fixed 10MAR2021
    
    //turn on the relay to engage transformer mode gains tuning for either mode 1 or more 2 (xformer knob is not in 0 position or "out")
    debouncedGPIO_Values[channel].Bits.dMIX_XFORMER_DB_TUNE = ((xformer_knob_value > 0) ? 1 : 0); // fixed 10MAR2021 debouncedGPIO_Values[channel].Bits.dXFORMER_PRI;
    //TODO: provide delay asymetry for mode based upon previousGPIO_Values if needed
    
}

void set_FilterLift(uchar_t channel, uchar_t lift_knob_value)
{
    //set bits  for the lift relays (see schematics for details) 

#ifdef EVL
#pragma message "EVL turns the filter enable relay on here!  Regular P331 doesn't want this"
    //EVL usage... this relay is always on....
    debouncedGPIO_Values[channel].Bits.dFILT_EN = 1; //For EVL, we want this always on... gains are fixed with different resistor values 2JUL2021 dcm 
#else
    debouncedGPIO_Values[channel].Bits.dFILT_EN = (uchar_t)(lift_knob_value > 0);     //enable is set for any mode other than "out" (lift_mode = 0)
#endif    
    debouncedGPIO_Values[channel].Bits.dFILT_HP_EN  = (uchar_t)(lift_knob_value & 0x01);  //odd (LSb is set, e.g. lift_mode = 1 or 3) is for hi filter enable
    debouncedGPIO_Values[channel].Bits.dFILT_LP_EN = (uchar_t)(lift_knob_value > 1);     //this captures mode 2 (filter for low) and duo (both)
    
}


void Display_Write(uchar_t channel, uchar_t idx_char)
{
    //shadow the LED display bits, such that we could independently turn on the decimal point (function below)
    debouncedGPIO_Values[channel].byte_groups[DISPLAY_BYTE_GROUP] = bmpDisplay_Value[idx_char];
    __delay_ms(2);
    
    writeSPI_XR_GPIO(CALCULATE_CS(channel, CS_D), OCR2, debouncedGPIO_Values[channel].byte_groups[DISPLAY_BYTE_GROUP]);   //OCR2 is upper byte (P8-P15) of GPIO expansion chip
    
    return;
}

void set_Display_decimal_point(uchar_t channel, uchar_t on)
{
    uchar_t LED_Display_byte;
    LED_Display_byte = debouncedGPIO_Values[channel].byte_groups[DISPLAY_BYTE_GROUP];
    __delay_ms(2);

    // note that LEDs are active low... decimal point is also active low hence 0b0... for "on"
    writeSPI_XR_GPIO(CALCULATE_CS(channel, CS_D), OCR2, (uchar_t) on ? 0b01111111U & LED_Display_byte : 0b10000000U | LED_Display_byte);   //OCR2 is upper byte (P8-P15) of GPIO expansion chip
    
    return;
}


#ifdef static_update
uchar_t g_idx_char[IN_STEREO];
void setStatic_Display(uchar_t channel, uchar_t idx_char)
{
    g_idx_char[channel] = idx_char;
}
uchar_t getStatic_Display(uchar_t channel)
{
    return g_idx_char[channel];
}


void updateDebugStatic_7segmentDisplay(void)
{
    for(uchar_t channel = LEFT; channel < RIGHT; channel++) 
    {
        Display_Write(channel, getStatic_Display(channel));
        OUTPUT_GPIO[channel].byte_groups[DISPLAY_BYTE_GROUP] = bmpDisplay_Value[getStatic_Display(channel)];
    }
}

#endif