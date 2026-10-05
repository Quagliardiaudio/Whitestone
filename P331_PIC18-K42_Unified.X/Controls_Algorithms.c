#include "Controls.h"
#include "GPIO.h"
#include "SPI.h"
#include "Hardware.h"  //needed for __delay(x)
#include <xc.h> // include standard header file
#include "Globals.h"
#include "DAC_LEDs.h"
#include "EEPROM.h"
#include "fixed_point.h"

//* Read, process channel controls, and calculate output relay combinations for digitally controlled analog paramaters within line amp. <BR> This is the main do function for reading and processing channels... Called by main()
void calculate_Channel_Relay_Outputs() // This is the main hub of activity for taking control input, calculating relay states, and setting relay states (and LEDs) in response to inputs
{
    uchar_t control_channel; // use this intermediary variable to override control channel (LEFT or RIGHT channel) for link mode
    
    //variables for Post mode to calculate overidden gains
    signed char outlevel_gain_desired;
    signed char outlevel_override;
    signed char outlevel_fractional_override;
    
    signed char pad_override;
    
    for(uchar_t channel = LEFT; channel <= RIGHT; ++channel) {
        
        outlevel_fractional_override = 0; // init here
        outlevel_override = 0;
        
        //for link, to use the current control channel's knob (could be just LEFT when linked, or LEFT/RIGHT for when not linked) each time through this loop; yet using just "channel" will always be LEFT/RIGHT per this containing for loop            
        control_channel = gLinkChannels ? LEFT : channel;  // if channels are linked we only use the left channel input control values; otherwise, control_channel would contain LEFT/RIGHT each time through this containing for loop
        
        ////////////////////
        //        channel = RIGHT; // TMP DEBUG   
        //        control_channel = RIGHT; // TMP DEBUG
        //////////////////////
        
        //read or process channels control values into current_control_value[channel]
        interpret_Channel_Mode(channel);           //check mode switch (momentary) for overall channel mode selection; things like linking channels and led brightness press/hold modes
        interpret_Xformer_Control(channel);        //read xformer knob
        interpret_Loading_Controls(channel);       //read the loading knob and FF mode (or tube mode 3?)
        interpret_OutputLevel_Controls(channel);   //read output level knob and fractional gains
        interpret_FilterLift_Control(channel);     //process filer knob
        
        if (gLinkChannels)
        {
            autoCut_mode[RIGHT] = autoCut_mode[LEFT];
        }
        
        OUTPUT_GPIO[channel].Bits.dK_Lineamp_IN = 1; //debouncedGPIO_Values[channel].Bits.bLINEAMP_ENGAGE;  //enable or bypass (hardwire) for each channel
        
        //Note: CCV is used for link, to use the current control channel's knob (could be just LEFT when linked, or LEFT/RIGHT for when not linked) each time through this loop; yet using just "channel" will always be LEFT/RIGHT per this containing for loop        
#define CCV (current_control_value[control_channel])  /*alias to shorten code*/
        
        uchar_t channelmode = ChannelMode[control_channel];
        
        //local variables of similar to the global structs, but these may look at only the LEFT channel only when channels are linked...  (see define for CCV above using control_channel instead of channel especially during a channel link mode)
        uchar_t _output_gain_enable; // = CCV._output_gain_enable;
        uchar_t output_level_knob; // = CCV.output_level_knob;
        uchar_t _output_gain_boost_mode; // = CCV._output_gain_boost_mode;
        uchar_t loading_knob; // = CCV.loading_knob;
        uchar_t load_position;
        
        /****************************************************************************/        
#ifdef NOT_USED
        uchar_t output_level_boost_temporary;
        //temporary for AES October 2018
        if (debouncedGPIO_Values[channel].Bits.dFILT_EN == 1)     //filter enable is set 
        {
            output_level_boost_temporary = 6; //add X dB to output boost to overcome filter insertion loss
        }
        else
        {
            output_level_boost_temporary = 0; //not lifting so 0 dB offset
        }
#endif 
        /****************************************************************************/
        
        //set some local vairables to do gain offsets for the modes below
        _output_gain_enable = CCV._output_gain_enable;
        output_level_knob = CCV.output_level_knob;
        _output_gain_boost_mode = CCV._output_gain_boost_mode;
        loading_knob = (uchar_t) (11 - CCV.loading_knob);  //change the direction of the loading knob
        load_position =  current_control_value[control_channel].loading_knob;
        signed char direction;
        signed char sign;
        
        switch(channelmode)
        {   
            case _enum_max_SetLEDBrightness:
                
                direction = CCV._output_gain_boost_mode ? -1:0;
                direction = CCV._output_gain_cut_mode ? 1 : direction;
                
                if(channel == LEFT)
                    BRIGHTNESS_VU_LEFT = BRIGHTNESS_VU_DEFAULT + ((output_level_knob + 1) << 1) * direction;
                if(channel == RIGHT)
                    BRIGHTNESS_VU_RIGHT = BRIGHTNESS_VU_DEFAULT + ((output_level_knob + 1) << 1) * direction;
                
                writeSPI_DAC(VU_LEFT, BRIGHTNESS_VU_LEFT);
                writeSPI_DAC(VU_RIGHT, BRIGHTNESS_VU_RIGHT);
                
                //blink alternately the CUT and MODE LED like a school bus...
                if(blink_counter > 3 )  //divide down counter rate
                {
                    set_Pilot_Brightness(previous_pilot_level);  //Pilot LED on for Mode led
                }
                else
                { 
                    previous_pilot_level = BRIGHTNESS_PILOT;
                    set_Pilot_Brightness(0);  //Mode LED off (do not set to zero as hardware has capacitors that take time; will never see the LED illuminate if going all the way to 0)
                }
                
                break;
                
                //Note that current_control_value[channel] is stuffed with current control knob/switch/relay setting levels (for input pad) from the interpret_<Control> function calls above in this function
            case PadMatchTube:  // Original PRE  Mode Front panel artwork on Mode switch for Loading knob: original (Pre) mode
                autoCut_mode[channel] = 0; // AutoCUT does not apply in this mode; this is here to allow cycling through the different modes and aborting autocut in any mode other than BLUE below
                
                set_Pad_Attenuation(channel,(uchar_t) (11 - loading_knob));  //set pad first for best practices
                set_Tube_Gain(channel, loading_knob, CCV.loading_mode_switch);
                
                
                //override output level gain relays with calculated gain or attenuation
                outlevel_override =  ( (_output_gain_boost_mode ? output_level_knob : -output_level_knob) * _output_gain_enable );
                //offset output gain is executed below outside of this switch statement...
                
 #ifdef EVL
                if(CCV.loading_mode_switch == LOADING_SWITCH_CLEAN_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    sign = bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][WHOLE_LOOKUP] : bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][FRACTIONAL_LOOKUP];
                }

                if(CCV.loading_mode_switch == LOADING_SWITCH_BLOOM_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    sign = bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][WHOLE_LOOKUP] : bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][FRACTIONAL_LOOKUP];
                }
#else               
                if(CCV.loading_mode_switch == LOADING_SWITCH_CLEAN_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    if(current_control_value[control_channel].lift_knob == 0)
                    {
                    sign = bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][WHOLE_LOOKUP] : bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_PRE_CLEAN_MODE[CCV.loading_knob][FRACTIONAL_LOOKUP];
                    }
                    else
                    {
                    sign = bmp7_Compesation_LIFT_PRE_CLEAN_MODE[CCV.loading_knob][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_LIFT_PRE_CLEAN_MODE[CCV.loading_knob][WHOLE_LOOKUP] : bmp7_Compesation_LIFT_PRE_CLEAN_MODE[CCV.loading_knob][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_LIFT_PRE_CLEAN_MODE[CCV.loading_knob][FRACTIONAL_LOOKUP];
                        
                    }   
                }
              
                if(CCV.loading_mode_switch == LOADING_SWITCH_BLOOM_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    if(current_control_value[control_channel].lift_knob == 0)
                    {
                        sign = bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][SIGN];
                        outlevel_override += sign ? -bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][WHOLE_LOOKUP] : bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][WHOLE_LOOKUP];
                        outlevel_fractional_override += bmp7_Compesation_PRE_BLOOM_MODE[CCV.loading_knob][FRACTIONAL_LOOKUP];
                    }
                    else
                    {
                        sign = bmp7_Compesation_LIFT_PRE_BLOOM_MODE[CCV.loading_knob][SIGN];
                        outlevel_override += sign ? -bmp7_Compesation_LIFT_PRE_BLOOM_MODE[CCV.loading_knob][WHOLE_LOOKUP] : bmp7_Compesation_LIFT_PRE_BLOOM_MODE[CCV.loading_knob][WHOLE_LOOKUP];
                        outlevel_fractional_override += bmp7_Compesation_LIFT_PRE_BLOOM_MODE[CCV.loading_knob][FRACTIONAL_LOOKUP];
                    }   
                }
#endif 
#ifdef P331_ORIGINAL
                if(current_control_value[control_channel].xformer_knob > 0)
                {
                    if(current_control_value[control_channel].xformer_knob == 2)
                        outlevel_override += 1; //compensate for transformer tweaky pots not having enough range for now...
                    
                    if(current_control_value[control_channel].xformer_knob == 1)
                        outlevel_override += 2; //compensate for transformer tweaky pots not having enough range for now...
                    
                }
#endif
                //set_Mode_LED_by_Color_Map(channel, RGB_LED(rgb_led_off));
                set_Mode_LED_Color(channel, RGB_LED_GREEN);  //GREEN
                //if (gLinkChannels && channel == RIGHT) {} else 
                //Display_Write(channel, current_control_value[channel].loading_knob);
                
                break;
                
                
            case MasterVolumeMode:  // POST Mode Front panel artwork on Mode switch for Loading knob: Post mode
                autoCut_mode[channel] = 0; // AutoCUT does not apply in this mode; this is here to allow cycling through the different modes and aborting autocut in any mode other than BLUE below
                
                //override output level gain relays with calculated gain or attenuation
                outlevel_gain_desired = -( 11 - loading_knob) + ( (_output_gain_boost_mode ? output_level_knob : -output_level_knob) * _output_gain_enable );
                outlevel_override = -11 > outlevel_gain_desired ? -11 : outlevel_gain_desired;  //needed here... to provide pre-calcuation for pad_override on the next line below (take the floor of maximal capability of the relays (0 through 11 for 12 relay "positions" on the resistor ladders)
                pad_override = outlevel_override - outlevel_gain_desired;  //calculate this before taking the absolute value below (with ternary macro); may not be efficient use of values in register under the hood?
                
#ifdef EVL
                outlevel_override += bmp7_Lift_Mode_Compesation[load_position];
                if(CCV.loading_mode_switch == LOADING_SWITCH_CLEAN_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    //EVL: clean mode needs a slightly modified lookup table
                    sign = bmp7_Compesation_POST_CLEAN_MODE[load_position][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_POST_CLEAN_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_POST_CLEAN_MODE[load_position][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_POST_CLEAN_MODE[load_position][FRACTIONAL_LOOKUP];
                }

                if(CCV.loading_mode_switch == LOADING_SWITCH_BLOOM_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    //EVL: bloom mode needs a slightly modified lookup table
                    sign = bmp7_Compesation_POST_BLOOM_MODE[load_position][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_POST_BLOOM_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_POST_BLOOM_MODE[load_position][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_POST_BLOOM_MODE[load_position][FRACTIONAL_LOOKUP];
                    
                }

#endif

#ifdef P331_ORIGINAL
                //***                //this LIFT mode compensation may also be needed in EVL?????
                if(current_control_value[control_channel].lift_knob > 0)
                {
                    outlevel_override += bmp7_Lift_Mode_Compesation[load_position];
                }
                
                
                if(current_control_value[control_channel].xformer_knob > 0)
                {
                    if(current_control_value[control_channel].xformer_knob == 2)
                        outlevel_override += 1; //compensate for transformer tweaky pots not having enough range for now...
                    
                    if(current_control_value[control_channel].xformer_knob == 1)
                        outlevel_override += 2; //compensate for transformer tweaky pots not having enough range for now...
                    
                }
                
                
                if(CCV.loading_mode_switch == LOADING_SWITCH_CLEAN_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    if(current_control_value[control_channel].lift_knob == 0)
                    {
                        //clean mode needs a slightly modified lookup table
                        sign = bmp7_Compesation_POST_CLEAN_MODE[load_position][SIGN];
                        outlevel_override += sign ? -bmp7_Compesation_POST_CLEAN_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_POST_CLEAN_MODE[load_position][WHOLE_LOOKUP];
                        outlevel_fractional_override += bmp7_Compesation_POST_CLEAN_MODE[load_position][FRACTIONAL_LOOKUP];
                    }
                    else
                    {
                        sign = bmp7_Compesation_LIFT_POST_CLEAN_MODE[load_position][SIGN];
                        outlevel_override += sign ? -bmp7_Compesation_LIFT_POST_CLEAN_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_LIFT_POST_CLEAN_MODE[load_position][WHOLE_LOOKUP];
                        outlevel_fractional_override += bmp7_Compesation_LIFT_POST_CLEAN_MODE[load_position][FRACTIONAL_LOOKUP];
                    }
                    
                    
                }
                
                if(CCV.loading_mode_switch == LOADING_SWITCH_BLOOM_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    if(current_control_value[control_channel].lift_knob == 0)
                    {
                        //bloom mode needs a slightly modified lookup table
                        sign = bmp7_Compesation_POST_BLOOM_MODE[load_position][SIGN];
                        outlevel_override += sign ? -bmp7_Compesation_POST_BLOOM_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_POST_BLOOM_MODE[load_position][WHOLE_LOOKUP];
                        outlevel_fractional_override += bmp7_Compesation_POST_BLOOM_MODE[load_position][FRACTIONAL_LOOKUP];
                    }
                    else
                    {
                        sign = bmp7_Compesation_LIFT_POST_BLOOM_MODE[load_position][SIGN];
                        outlevel_override += sign ? -bmp7_Compesation_LIFT_POST_BLOOM_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_LIFT_POST_BLOOM_MODE[load_position][WHOLE_LOOKUP];
                        outlevel_fractional_override += bmp7_Compesation_LIFT_POST_BLOOM_MODE[load_position][FRACTIONAL_LOOKUP];
                    }
                    
                }
#endif
                //absolute value of pad_override (shouldn't be necessary, but just in case)
                pad_override = 0 > pad_override ? -pad_override: pad_override;  // absolute value
                
                //not_used outlevel_gain_desired += output_level_boost_temporary;
                
                //override input pad if user is operating Output Level controls in a manner that desires more CUTting beyond the capabilities of the Output Level Relay Ladders
                set_Pad_Attenuation(channel, (uchar_t) pad_override);
                
                set_Tube_Gain(channel, loading_knob, CCV.loading_mode_switch); //use (11 - loading knob) to make it operate in the opposite direction
                
                //set_Mode_LED_by_Color_Map(channel, RGB_LED(rgb_led_orange));
                set_Mode_LED_Color(channel, RGB_LED_ORANGE);  //ORANGE
                
                // Display_Write(channel, pad_override);
                
                break;
                //EVL may use pad now due to compressed gain ranges max - min != 11 dB anymore due to cathode resistor value)... so the "no pad control" in the TubeGainNoPadControl enum constant-value might be misleading until we refactor it
            case TubeGainNoPadControl: // OPEN mode Front panel artwork on Mode switch for Loading knob: Open mode
                
                //autoCUT:  use memory of prior mode to make this a one shot autoCUT (until we back down the loading knob to position threshold shown below)...
                if(PrevChannelMode[control_channel] != ChannelMode[control_channel])
                {
                    //TODO: implement call to autoCUT here if we are going to jump in gain when switching into this mode!
                    autoCut_mode[channel] = 1;
                }
                
                if(CCV.loading_knob == 0) //loading knob turned back all the way to 0 (position 1)
                {
                    autoCut_mode[channel] = 0; // have to turn loading knob all the way back to "0" (position 1) to reset auto cut mode
                }
                
                if(autoCut_mode[channel] == 1)
                {  
                    loading_knob = 11;  // 11 - knob_position... override using the loading knob rather than pads.... for EVL, the passive pad is not 1dB steps, but the tube gain and LUTs get it to 0dB in autocut mode
                    load_position = 0; //  == knob_position... to get all code below that either uses loading_knob or load_position in their native context; all this forces the tube to lowest gain instead of using the (non-1db step EVL) pads to counteract tube gain to simulate a 0db in autocut
                    
                    
                    //use pad to perform cut as it is not normally used in this mode for cutting (only output level cutting mechanisms are used normally)
                    //                    set_Pad_Attenuation(channel, CCV.loading_knob);  //set pad first for best practices 
                    
                    //blink alternately the CUT and MODE LED like a school bus...
                    if(blink_counter > 3 )  //divide down counter rate
                    {
                        set_Mode_LED_Color(channel, RGB_LED_BLUE);  //BLUE LED on for Mode led
                        //                       __delay_ms(100);
                        writeSPI_DAC(GET_LED_OFFSET(channel, LED_OUTPUT_LEVEL_MODE), 138);  // CUT LED ON
                    }
                    else
                    { 
                        set_Mode_LED_Color(channel, RGB_LED_OFF);  //Mode LED off (do not set to zero as hardware has capacitors that take time; will never see the LED illuminate if going all the way to 0)
                        //                       __delay_ms(100);
                        writeSPI_DAC(GET_LED_OFFSET(channel, LED_OUTPUT_LEVEL_MODE), 138);  // CUT LED ON
                    }
                }
                else
                {   // normally in this mode without an event that would require autocut
                    set_Mode_LED_Color(channel, RGB_LED_BLUE);  //BLUE
                    
                    set_Pad_Attenuation(channel, 0);  //set pad back to nothing as this is now non-blinking blue mode (no longer in autocut mode) 
                    
                    //input pad disabled
                }
                
                
                //set the tube gain (if we are in autoCUT mode, then pad will match tube gain as we turn back to 0 to reset out of autocut mode... sort of acts like PRE mode above temporarily until reset)
                set_Tube_Gain(channel, loading_knob, CCV.loading_mode_switch);  
                
                //override output level gain relays with calculated gain or attenuation
                outlevel_override = ( (_output_gain_boost_mode ? output_level_knob : -output_level_knob) * _output_gain_enable );
                //                outlevel_override = 0;
                if(CCV.loading_mode_switch == LOADING_SWITCH_CLEAN_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    //EVL: clean mode needs a slightly modified lookup table
                    sign = bmp7_Compesation_OPEN_CLEAN_MODE[load_position][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_OPEN_CLEAN_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_OPEN_CLEAN_MODE[load_position][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_OPEN_CLEAN_MODE[load_position][FRACTIONAL_LOOKUP];
                    set_Pad_Attenuation(channel, bmp7_Compesation_OPEN_CLEAN_MODE[load_position][PAD_LOOKUP]);  //EVL might want pad lookup for limited compressed gain range
                }
                
                if(CCV.loading_mode_switch == LOADING_SWITCH_BLOOM_MODE)  // if we are in BLOOM mode, then offset (tmp debug) using the original lookup table for the interaction between the tube and lift output amp (not the lift filters which are further upstream)
                {
                    
                    sign = bmp7_Compesation_OPEN_BLOOM_MODE[load_position][SIGN];
                    outlevel_override += sign ? -bmp7_Compesation_OPEN_BLOOM_MODE[load_position][WHOLE_LOOKUP] : bmp7_Compesation_OPEN_BLOOM_MODE[load_position][WHOLE_LOOKUP];
                    outlevel_fractional_override += bmp7_Compesation_OPEN_BLOOM_MODE[load_position][FRACTIONAL_LOOKUP];
                    set_Pad_Attenuation(channel, bmp7_Compesation_OPEN_BLOOM_MODE[load_position][PAD_LOOKUP]);  //EVL might want pad lookup for limited compressed gain range
                }
#ifdef P331_ORIGINAL
                if(current_control_value[control_channel].xformer_knob > 0)
                {
                    if(current_control_value[control_channel].xformer_knob == 2)
                        outlevel_override += 1; //compensate for transformer tweaky pots not having enough range for now...
                    
                    if(current_control_value[control_channel].xformer_knob == 1)
                        outlevel_override += 2; //compensate for transformer tweaky pots not having enough range for now...
                    
                }
#endif                
                
                //                //tmp debug... don't use this in production... for quick test and characterization of the pads only; temporary! (PASSIVE pads are seemingly not 1dB steps in EVL like they are for the original P331)
                //                set_Pad_Attenuation(channel, (uchar_t)bmp7_Compesation_OPEN_MODE_FULL_CUT[1][PAD_NEEDED]);  //set pad back to nothing as this is now non-blinking blue mode (no longer in autocut mode) 
#undef PAD_COMP                
#ifdef PAD_COMP
                if(CCV._output_gain_cut_mode) //&& autoCut_mode[channel] == 0) 
                {
                    signed char desired_gain_fxp;
                    desired_gain_fxp = to_fp(sign, outlevel_override, outlevel_fractional_override);
                    
                    if(desired_gain_fxp < TO_FP_INT_WHOLE(-11)) // includes if the fraction is less than -11.0 by using int -11 
                    {
                        //calculate how much pad we need...
                        signed char pad_needed_index;
                        signed char pad_needed_index_raw = -(outlevel_override + 11 ); // e.g., -12 - -12 = 0, and -12 - -13 = 1 index which (zero base index gets us the 0th element in the LUT)
                        
                        pad_needed_index = pad_needed_index_raw;
                        
                        
                        if (pad_needed_index > 3) pad_needed_index = 3; // clip the max
                        if (pad_needed_index < 0) pad_needed_index = 0; // clip the min
                        
                        if(pad_needed_index_raw >=0)
                        {
                            sign = bmp7_Compesation_OPEN_MODE_FULL_CUT[pad_needed_index][SIGN];
                            outlevel_override += sign ? -bmp7_Compesation_OPEN_MODE_FULL_CUT[pad_needed_index][WHOLE_LOOKUP] : bmp7_Compesation_OPEN_MODE_FULL_CUT[pad_needed_index][WHOLE_LOOKUP];
                            
                            uchar_t new_fractional;
                            new_fractional = bmp7_Compesation_OPEN_MODE_FULL_CUT[pad_needed_index][FRACTIONAL_LOOKUP]; // hit the LUT
                            
                            //convert into a fixed point integer math variable with fractionals in the lowest 2 bits and the whole in the upper bits (with reverence for sign)
                            
                            
                            desired_gain_fxp = to_fp(sign, outlevel_override, new_fractional);
                            //                                (
                            //                                (frac = (sign ? -new_fractional : new_fractional)) // consider the lowest bits first to keep sign bit proper for fractions less than +/- 1
                            //                                + (whole = (outlevel_override << 2)) //eats the sign bit, but shifts the value into the upper 6 bits (value is 0 through 11, only uses a few bits anyway)
                            //                                ); // the above marries the lookup table's whole and fractional dB columns into a fixed point integer number (whole(6).fractional(2)), while preserving sign from the whole dB column in the lookup tables
                            
                            //add the combined resolution fractional
                            desired_gain_fxp += sign? -new_fractional : new_fractional;
                            
                            
                            //convert back to separate variables for whole and fractional after we have added the new fractional resolution from the LUT (which handles carry forward of bits during the addition in the whole bit field range in the desired_gain_fxp variable naturally and organically)
                            fp_t fp;
                            from_fp(desired_gain_fxp, &fp);
                            
                            sign = fp.sign;
                            outlevel_fractional_override = fp.frac;
                            outlevel_override = fp.whole;
                            
                            uchar_t foo = (uchar_t)bmp7_Compesation_OPEN_MODE_FULL_CUT[pad_needed_index][PAD_NEEDED];
                            //21MAY2022 commented out beacause we have a new cathode resistor that may not need this:  set_Pad_Attenuation(channel, foo);  //compensate using pad (note zero index is 1dB pad hence +1)
                        }
                        else
                        {
                            set_Pad_Attenuation(channel, 0);  //pad compensation not needed here go back to normal outlevel compensation
                        }
                    }
                }   
                //                else
                //                {
                //                    set_Pad_Attenuation(channel, 0);  //set pad back to nothing as this is now non-blinking blue mode (no longer in autocut mode) 
                //                }
                
#endif
                //note this is added here with new cathode resisor of 21MAY2022 as we may not need the LUT for pad compensation above....
                //Note this was taken out so that the new lookup table 5JUN2022 with new 4th data element of adding in a pad lookup offset now...
                //set_Pad_Attenuation(channel, 0);  //pad compensation not needed here go back to normal outlevel compensation
                
                //below not needed any more... 24DEC2021
                //                //Compensate for lift dB jump (by 2 or so dB at 12th position on loading knob)
                //                if(debouncedGPIO_Values[control_channel].Bits.dFILT_EN == 1)
                //                if(current_control_value[control_channel].lift_knob > 0  && !autoCut_mode[channel])  //Note: !!! Autocut is like GREEN mode where there should be no compensation for the lift filter
                //                {
                //                    outlevel_override += bmp7_Lift_Mode_Compesation[load_position];
                //                }
                
                
                
                break;
                
                
            default:  // default, subject to change?  
                
                set_Mode_LED_by_Color_Map(channel, RGB_LED(rgb_led_red));  //why not...
                
        } // end switch case for Channel Modes
        
        if(channelmode != _enum_max_SetLEDBrightness) 
        {
            set_LED_bias(LED_DEFAULT_BIAS);  //tmp reset out of set LED brightness mode...
            PrevChannelMode[channel] = channelmode; //ChannelMode[channel];  // make sure to remember that we were not in the special LED brightness setting mode; PreviousChannelMode is for analog amplifier modes only
            
            //TODO: EVL initial prototype was off by 1 dB so lets artificially offset in software for now 3JUL2021... 
            //       outlevel_override -= 2;
            //Don't need this if the hardware is modified per "conversion to EVL" document sent 9/20/2021
            
            
            //     if(channel==LEFT) Display_Write(RIGHT, bmp7_Lift_Mode_Compesation[load_position]);
            //    if(channel==LEFT) Display_Write(RIGHT, outlevel_override);
            //    if(channel==LEFT) Display_Write(LEFT, current_control_value[LEFT].input_pad_value);
            // if(channel==LEFT) set_Display_decimal_point(LEFT, dboost);
            
            //TODO: fixed point math needs to figure out the sign            sign = outlevel_override
            
            //channel, output knob position; may be ignored due to next parameter --->, output level override (actual gain used by this set routine, boost, enable, cut, resolution_switch); 
            set_Output_Gain(channel,    
                    CCV.output_level_knob, //raw output level knob position used to determine if zero position for resolution special case at 0
                    outlevel_override, //overridden gain or cut level absolute value only on unsigned char (uchar_t)
                    CCV._output_gain_boost_mode, //boost mode raw switch (not overridden)
                    CCV._output_gain_cut_mode,   //cut mode raw switch (not overridden)
                    CCV.output_resolution_switch,// 0 through 2 for off/0dB (0), 0.25dB (1), and 0.5dB (2); we artificially create (3) to resolition capabilities inside this function
                    outlevel_fractional_override,
                    sign); 
            
            set_Xformer(channel, CCV.xformer_knob);  //uses control_channel instead of channel (see the #define above) for link mode, yet we use channel in this for loop (LEFT and RIGHT iterator) to write CCV to the relays
            set_FilterLift(channel, CCV.lift_knob);
            
        } // end of actually set gain relays if we are not adjusting the brightness of LEDs in _enum_max_SetLEDBrightness mode     
    } // end of for(;;) each channel...          
    
    //EEPROM
    //remember what we need to trigger an EEPROM write if needed
    current_user_settings.bytes[0] = 0;
    current_user_settings.bucket.channel_mode_LEFT = ChannelMode[LEFT] & 0b00000011;
    current_user_settings.bucket.channel_mode_RIGHT = ChannelMode[gLinkChannels? LEFT : RIGHT] & 0b00000011;
    current_user_settings.bucket.link_mode = gLinkChannels;
    current_user_settings.bucket.autocut_mode = autoCut_mode[LEFT] | (autoCut_mode[RIGHT] << 1);
    //TODO: Also store VU meter brightness, and write user adjustable brightness routines....
    
    //EEPROM writing of new settings if dwelled 
    if((shadowed_user_settings.bytes[0] & 0b01111111) != (current_user_settings.bytes[0] & 0b01111111))
    {
        user_settings_write_clock++;
    }
    else
    {
        user_settings_write_clock=0; // if the user cycles through the channel settings and gets back to what we have in the shadow, then there is no need for re-writing it...
    }        
    
    if (user_settings_write_clock > EEPROM_WRITE_CLOCK_COUNT_TRHESHOLD)
    {
        //we are different set the EEPROM after x cycles
        write_User_Settings_EEPROM();
        user_settings_write_clock=0; // if the user cycles through the channel settings and gets back to what we have in the shadow, then there is no need for re-writing it...
        
    }
    
    // end of EEPROM write new settings if changed...
    
}

// handles updating debug static LED display of the last knob dialed to what position...  We display only the last knob turned on the right-side channel as that display is on the top side of the PCB
void _onChange(uchar_t channel, uchar_t current_value, uchar_t *previous, uchar_t *previous_display)
{
    if (current_value != *previous_display)
    {
        // setStatic_Display(channel, current_value);  //set the display to the value of the last knob or control value change on it's respective channel/side...    
        
        
        *previous = current_value; //previous gets artificially decremented in debounce knob _value_ routine below
        *previous_display = current_value; //this doesn't get decremented in any debounce method (used for determining actual change of control value for executing this debug display)
    }   
}
char debounce_Changed(void)
{
    //this function exists mainly for non-shorting switches... this function can still be used for shorting switches (yet, toggle switches should still be debounced at the individual bit level)
    //shorting switches were not available at the time of hardware construction... TODO non-shorting switches will be replaced with shorting versions
    
    for(uchar_t channel = LEFT; channel <= RIGHT; channel++)  //iterate over both stereo channels/channels
    {
        //we have the first two (hard coded as 2 in the 'for' statement below) bytes as the inputs from the front panel, and also don't forget the straggler input bits on other output laden GPIO chips(handled separately below)
        for (uchar_t input_control_byte = 0; input_control_byte < 2; input_control_byte++) // iterate through the 2 input byte_groups (neglecting the other input bits on other GPIO chips -- handled separately below), to set raw GPIO data into the master GPIO map struct (where casted)
        {
            debouncedGPIO_Values[channel].byte_groups[input_control_byte] = bouncyGPIO_Values[channel].byte_groups[input_control_byte];              
        }
        
        debouncedGPIO_Values[channel].Bits.pFB_CLEAN = bouncyGPIO_Values[channel].Bits.pFB_CLEAN;
        debouncedGPIO_Values[channel].Bits.pMODE = bouncyGPIO_Values[channel].Bits.pMODE;
        debouncedGPIO_Values[channel].Bits.bLINEAMP_ENGAGE = bouncyGPIO_Values[channel].Bits.bLINEAMP_ENGAGE;
        
    }
    
    return TRUE;
    
}

#ifdef DEBOUNCE_EXPERIMENTAL
/**debounce the controls using full uchar bit-depth and boundaries at the individual / single bit level (there's a separate byte-sized counter for each bit in the control data struct)
 note, knobs use multiple bits to represent an uint16_integer (single char byte-sized)... we only debounce individual bits here...
 additional inputs outside of the first two GPIO bytes (configured as all inputs) are debounced individually and live at the bottom of this subroutine (we all live in a yellow subroutine)
 returns true for control values changed or false for no change since last time through the loop  */
char debounce_Changed(void)
{
    //this function exists mainly for non-shorting switches... this function can still be used for shorting switches (yet, toggle switches should still be debounced at the individual bit level)
    //shorting switches were not available at the time of hardware construction... TODO non-shorting switches will be replaced with shorting versions
    char controls_changed ; //= FALSE; // keep track of whether the controls changed since the last time through the loop
    
    for(uchar_t channel = LEFT; channel <= RIGHT; channel++)  //iterate over both stereo channels/channels
    {
        //we have the first two (hard coded as 2 in the 'for' statement below) bytes as the inputs from the front panel, and also don't forget the straggler input bits on other output laden GPIO chips(handled separately below)
        for (uchar_t input_control_byte = 0; input_control_byte < 2; input_control_byte++) // iterate through the 2 input byte_groups (neglecting the other input bits on other GPIO chips -- handled separately below), to set raw GPIO data into the master GPIO map struct (where casted)
        {
            
            uchar_t control_byte;
            control_byte = bouncyGPIO_Values[channel].byte_groups[input_control_byte];  // get the current byte-wide inputs
            //debouncedGPIO_Values[channel].byte_groups[input_control_byte] = bouncyGPIO_Values[channel].byte_groups[input_control_byte];            
            if  (control_byte != debouncedGPIO_Values[channel].byte_groups[input_control_byte]) {  //control state has changed let's debounce it.'
                
                uchar_t mask = 0b0000001;
                for(uchar_t which_debounce_counter = 0; which_debounce_counter < sizeof(uchar_t); which_debounce_counter++) 
                {  // iterate until end of the 8 bits (8 input bits on each input_control_byte)
                    
                    if (control_byte & mask)
                    {  // if it's a one, then increment the hi counter....
                        debounce_hi[channel].debounce_counters_bytes[which_debounce_counter] ++;
                        debounce_low[channel].debounce_counters_bytes[which_debounce_counter] = 0; // reset the counter for the other opposite state
                    }
                    else
                    {  // if it's a zero, then increment the low counter....
                        debounce_low[channel].debounce_counters_bytes[which_debounce_counter] ++;
                        debounce_hi[channel].debounce_counters_bytes[which_debounce_counter] = 0; // reset the counter for the other opposite state
                    }
                    
                    mask <<= 1;  // shift by one bit each time through the loop to inspect each bit of the input controls
                    
                    if (debounce_hi[channel].debounce_counters_bytes[which_debounce_counter] > DEBOUNCE_THRESHOLD) 
                    { // then it must be true it's a one and debounced :)
                        debouncedGPIO_Values[channel].byte_groups[input_control_byte] |=  (uchar_t)(0b00000001 << which_debounce_counter);  // set this bit to a one
                        debounce_hi[channel].debounce_counters_bytes[which_debounce_counter] = 0; // reset the counter
                        
                        controls_changed = TRUE;
                    }
                    if (debounce_low[channel].debounce_counters_bytes[which_debounce_counter] > DEBOUNCE_THRESHOLD) 
                    { // then it must be true it's a zero and debounced :)
                        debouncedGPIO_Values[channel].byte_groups[input_control_byte] &= (uchar_t) ~(0b00000001 << which_debounce_counter);  //set this bit to a zero... note: uchar promotion for this local mask calculation
                        debounce_low[channel].debounce_counters_bytes[which_debounce_counter] = 0; // reset the counter
                        
                        controls_changed = TRUE;
                    }
                    
                } // end for(;;) iterating over each input control byte (2 of them)
            } // if previous byte does not match current byte read on input(s)
            
        } // for each of the two input front panel controls bytes   
        
        //multi-line macro that writes code for us; the names of the individual bits change to protect the innocent
        DEBOUNCE_BIT(pFB_CLEAN);
        DEBOUNCE_BIT(pMODE);    
        DEBOUNCE_BIT(bLINEAMP_ENGAGE);    
        
        
    } // end of for(;;) each channel LEFT to RIGHT...
    
    return controls_changed; // use this to determine whether or not further processing is needed on all the knobs and switches
}
#endif