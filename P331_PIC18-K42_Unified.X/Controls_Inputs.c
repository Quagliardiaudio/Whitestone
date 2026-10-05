#include "Controls.h"
#include "GPIO.h"
#include "SPI.h"
#include "Hardware.h"  //needed for __delay(x)
#include <xc.h> // include standard header file
#include "Globals.h"
#include "EEPROM.h"

//init the controls such that they start at some known state...
void controls_INIT(void)
{
    ModeSW_Timer_Counter[LEFT] = 0;     //init the timer/counter for mode switch debounce/pres and hold functionality
    ModeSW_Timer_Counter[RIGHT] = 0;    //init the timer/counter for mode switch debounce/pres and hold functionality

    ModeSW_increment[LEFT]  = 1; //allow incrementing of Mode when we press the Mode (momentary) switch
    ModeSW_increment[RIGHT] = 1; //allow incrementing of Mode when we press the Mode (momentary) switch
    
    ChannelMode[LEFT] = PadMatchTube;  //global system mode initialized to original function of the pad matching tube gain
    ChannelMode[RIGHT] = PadMatchTube;  //global system mode initialized to original function of the pad matching tube gain
    
    PrevChannelMode[LEFT] = ChannelMode[LEFT];
    PrevChannelMode[RIGHT] = ChannelMode[RIGHT];

    //set global/all LED brightness to be very dim
   // now done in main() ... 24 is too low of a value = too bright (255 is off)... set_LED_bias(24);  //initialized to "something"... TBD... hardware reset already sets it to 0-ish...
    
    // init the debouncers
    for (uchar_t channel = LEFT; channel <= RIGHT; channel++)
    {
        for(uchar_t which_byte=0; which_byte < sizeof(debounce_counters_t); which_byte++) {
            debounce_low[channel].debounce_counters_bytes[which_byte] = 0x00;
            debounce_hi[channel].debounce_counters_bytes[which_byte] = 0x00;
        }
        previous_control_value[channel].loading_knob=1;  
        previous_control_value[channel].output_level_knob=1;
        previous_control_value[channel].xformer_knob=1;
        previous_control_value[channel].lift_knob=1;
       
        for(uchar_t control_byte = 0; control_byte < sizeof(GPIO_ChannelFrame_t); control_byte++)
        {   //debouncedGPIO_Values is a GPIO_ChannelFrame struct/union
            debouncedGPIO_Values[channel].byte_groups[control_byte]=0x00;
        }
    }
}

//process the loading knob value and the tube opamp gain mode (feedforward or feedback) from the debounced mode switch
void interpret_Loading_Controls(uchar_t channel)
{
    uchar_t FF_Mode = 0x00;

    uchar_t loading_knob_value;

    loading_knob_value = (debouncedGPIO_Values[channel].byte_groups[BYTE_INDEX_LOADING_KNOB] & MASK_LOADING_KNOB) >> SHIFT_LOADING_KNOB; // get current value of knob (debounced)

    //The lookup table for vacuum tube opamp loading relays also includes the feed forward (FF) mode switch (clean feedforward loading or feedback method of gain reduction)
    FF_Mode |=  debouncedGPIO_Values[channel].Bits.pFB_CLEAN & (0b00000001U); //Note fb means "feedback" also "clean" mode (active low, but lookup table is already coded this way)

    //set the current control values in the data struct, to be accessed by further controls algorithms
    current_control_value[channel].loading_knob = loading_knob_value;
    current_control_value[channel].loading_mode_switch = FF_Mode;

    _onChange(channel, loading_knob_value, &previous_control_value[channel].loading_knob, &previous_display_value[channel].loading_knob);
}

//set the output gain relay ladder in the output gain level (or pCUT)....
//being mindful of xformer mode 2 as gain compensation is temporarily needed
void interpret_OutputLevel_Controls(uchar_t channel)
{
    //TODO: why are these signed?
    uchar_t output_level_pos; //initialized below from function call..

    //read the output level knob switch position setting
    output_level_pos = (debouncedGPIO_Values[channel].byte_groups[BYTE_INDEX_OUTLEVEL_KNOB] & MASK_OUTLEVEL_KNOB) >> SHIFT_OUTLEVEL_KNOB; // get current value of knob (debounced)
    
//    //tmp debug broken cut/boost switch on open faced sandwich 17SEP2021
//    debouncedGPIO_Values[channel].Bits.pBOOST = 1;
//    debouncedGPIO_Values[channel].Bits.pCUT =1;
//    
    
    
    //get the settings from the front panel switch (will be overridden if xformer mode 2)
    current_control_value[channel]._output_gain_boost_mode = (uchar_t) ~debouncedGPIO_Values[channel].Bits.pBOOST  & (0b00000001U);  //active low
    current_control_value[channel]._output_gain_enable = (uchar_t) (~debouncedGPIO_Values[channel].Bits.pBOOST | ~debouncedGPIO_Values[channel].Bits.pCUT)  & (0b00000001U); //both pBOOST/pCUT are active low...
    current_control_value[channel].output_level_knob = output_level_pos;
    current_control_value[channel]._output_gain_cut_mode = ~debouncedGPIO_Values[channel].Bits.pCUT & 0x01 ;
    current_control_value[channel].output_resolution_switch = (uchar_t) (debouncedGPIO_Values[channel].Bits.pFRAC_0DB + (~debouncedGPIO_Values[channel].Bits.pFRAC_0P5DB & 0x01U));

    //_onChange(channel, output_level_pos, &previous_control_value[channel].output_level_knob, &previous_display_value[channel].output_level_knob);

}

void interpret_Xformer_Control(uchar_t channel)
{
    //read the Xformer switch position
    uchar_t xformer_mode;

    //read the value of the xformer mode rotary switch
    xformer_mode = ((uchar_t) debouncedGPIO_Values[channel].byte_groups[BYTE_INDEX_XFORMER_KNOB] & MASK_XFORMER_KNOB) >> SHIFT_XFORMER_KNOB; // get current value of knob (debounced)

    xformer_mode = (uchar_t) (3 - xformer_mode); //active low, with 0-2 positions (still 2bits, 3 total, but only 0-2 are used) 
    
    current_control_value[channel].xformer_knob = xformer_mode;

    //TODO: provide delay asymetry for mode based upon previousGPIO_Values

    //check to see if there is any change from previous value
    //_onChange(channel, xformer_mode, &previous_control_value[channel].xformer_knob, &previous_display_value[channel].xformer_knob);
}

void interpret_FilterLift_Control(uchar_t channel)
{
    //Note: this hardware cannot use SW_POS_BASE as all 4 states are used on 2-bit encoding
    
    //read the Xformer switch position
    uchar_t lift_mode;
    //read the value of the xformer mode rotary switch
    lift_mode = ((uchar_t) debouncedGPIO_Values[channel].byte_groups[BYTE_INDEX_FILTER_KNOB] & MASK_FILTER_KNOB) >> SHIFT_FILTER_KNOB; // get current value of knob (debounced)

    lift_mode = (uchar_t)(3 - lift_mode); //active low, with 0-3 positions (4 total = 2bits) 

    current_control_value[channel].lift_knob = lift_mode;

    //check to see if there is any change from previous value
    //_onChange(channel, lift_mode, &previous_control_value[channel].lift_knob, &previous_display_value[channel].lift_knob);
}

void interpret_Channel_Mode(uchar_t channel)
{
    uchar_t mode_switch = debouncedGPIO_Values[channel].Bits.pMODE;

    //tmp debug; the below is useful to trigger the bypass switches (in code) to test things like link, or brightness for mode switch press/hold modes
//    debouncedGPIO_Values[LEFT].Bits.bLINEAMP_ENGAGE  = 1;
//    debouncedGPIO_Values[RIGHT].Bits.bLINEAMP_ENGAGE = 1;
//    gLinkChannels = 1;

    if (mode_switch == 0)  //active low !!!
    {
        // mode switch is depressed (seek therapy)
        if(ModeSW_Timer_Counter[channel] <  MODE_COUNTER_PRESSHOLD_THRESHOLD
        && ModeSW_Timer_Counter[channel] >= 0
          )
        {
            ModeSW_Timer_Counter[channel]++;  //prevent wrap around above threshold
        } 

    }
    else
    {
        //mode switch was depressed quickly (not held for x seconds), thus increment to the next MODE
        if(ModeSW_Timer_Counter[channel] > 0 
        && ModeSW_Timer_Counter[channel] < MODE_COUNTER_PRESSHOLD_THRESHOLD )
        { 
            //normally we just increment mode, otherwise above we enter into setting of brightness mode for LED VU meters...
            //actually increment channel mode if we have reached the MODE_COUNTER_THRESHOLD threshold  above (this effectively slows down the sensitivity to fast mode changes related to SPI bus speed)
            if (ChannelMode[channel] < _enum_max_SetLEDBrightness)
            {
                ChannelMode[channel] += 1;
                ModeSW_Timer_Counter[channel] = 0; //reset our counter so that we do not fast flip through all the modes....
                
                if (ChannelMode[channel] >= _enum_max_SetLEDBrightness
 //                       && debouncedGPIO_Values[RIGHT].Bits.bLINEAMP_ENGAGE == 1  //tmp debug
 //                       && debouncedGPIO_Values[LEFT].Bits.bLINEAMP_ENGAGE == 0) 
                    )
                    ChannelMode[channel] = 0; // note magic number 0 but this is the first mode.. we needn't go into _enum brightness setting mode, that's below...
                
                return;
            }
        }
    }    
        
    //Otherwise, the mode switch was depressed (and held) for longer than x seconds for special behaviour below    
    if(ModeSW_Timer_Counter[channel] >= MODE_COUNTER_PRESSHOLD_THRESHOLD 
      )  //mode switch has been depressed (then released) for a while Marvin...
    {

#ifdef EEPROM_SAVE_USER_SETTINGS
        //this would toggle back to normal Mode for each channel, as this is the second hold-depress of the mode swtich to get back to normal (previous mode for the channel)
        if (ChannelMode[LEFT] == _enum_max_SetLEDBrightness)
        {  //get out of setting the LED brightness mode using the front panel knobs; go back to previous analog user mode
            ChannelMode[LEFT]  = PrevChannelMode[LEFT];
            ChannelMode[RIGHT] = PrevChannelMode[RIGHT];

            //save our user settings when we exit this mode
            //write_User_Settings();

            ModeSW_Timer_Counter[LEFT] = -1;  //reinitialize the counter to 0 
            ModeSW_Timer_Counter[RIGHT] = -1;  //reinitialize the counter to 0 

            return;
        }
#endif
        // (toggle) link channels (link on, link off Danielsan)
        if (channel == RIGHT 
         && debouncedGPIO_Values[LEFT].Bits.bLINEAMP_ENGAGE  == 1 
         && debouncedGPIO_Values[RIGHT].Bits.bLINEAMP_ENGAGE == 0   //link and unlink channels with all of the (the whole enchilada) logic above only iff the 2 bypass switches are in the appropriate states
           )
        {
            gLinkChannels = (uchar_t) !gLinkChannels;
            ModeSW_Timer_Counter[LEFT] = -1;  //reinitialize the counter to 0 
            ModeSW_Timer_Counter[RIGHT] = -1;  //reinitialize the counter to 0 

           // if (gLinkChannels) set_Mode_LED_Color(RIGHT, RGB_LED_WHITE);  //White  doesn't work here, gets overwritten elesewhere... however it does work in the WRITE DAC  routine, although, not the ideal location....

            return;
        }

        if (channel == LEFT //set LED brightness modes including VU meter brightnesses
         && debouncedGPIO_Values[LEFT].Bits.bLINEAMP_ENGAGE  == 0 
         && debouncedGPIO_Values[RIGHT].Bits.bLINEAMP_ENGAGE == 0  
           )
        {
            ModeSW_Timer_Counter[LEFT] = -1;  //reinitialize the counter to 0 
            ModeSW_Timer_Counter[RIGHT] = -1;  //reinitialize the counter to 0 
            if (ChannelMode[LEFT] != _enum_max_SetLEDBrightness)
            {
                PrevChannelMode[LEFT] = ChannelMode[LEFT];
                PrevChannelMode[RIGHT] = ChannelMode[RIGHT];
                
                ChannelMode[LEFT] = _enum_max_SetLEDBrightness;
                ChannelMode[RIGHT] = _enum_max_SetLEDBrightness;
                gLinkChannels = 0;
            }
            else
            {
                ChannelMode[LEFT] = PrevChannelMode[LEFT];
                ChannelMode[RIGHT] = PrevChannelMode[RIGHT];
                
                write_User_Settings_EEPROM();
                set_Pilot_Brightness(previous_pilot_level);
            }
        
            return;
        }

#ifdef LED_DIM_BRIGHTNESS
        // press-and-hold of the LEFT mode while both channels are in BYPASS, such that we can dim the LEDs (and VU lights, etc))
        if (debouncedGPIO_Values[LEFT].Bits.bLINEAMP_ENGAGE  == 0 
         && debouncedGPIO_Values[RIGHT].Bits.bLINEAMP_ENGAGE == 0 
         && channel == LEFT
         && ChannelMode[LEFT] != _enum_max_SetLEDBrightness
           )  //if both channels are bypassed and mode LEFT switch is held then we go into LED brightness setting mode
        { //
            PrevChannelMode[LEFT]  = ChannelMode[LEFT];  // remember current channel mode when we go into LED brightness mode, such that we can pop back into this current mode
            PrevChannelMode[RIGHT] = ChannelMode[RIGHT]; // remember current channel mode when we go into LED brightness mode, such that we can pop back into this current mode

            ChannelMode[LEFT]      = _enum_max_SetLEDBrightness;   //Note: we evaluate the LEFT channel momentary mode switch during this "interpret" function call for this sticky-hold mode switch depress to engage the LED brightness adjustment (for both channels))
            ChannelMode[RIGHT]     = _enum_max_SetLEDBrightness;   //Note: we evaluate the LEFT channel momentary mode switch during this "interpret" function call for this sticky-hold mode switch depress to engage the LED brightness adjustment (for both channels))
            ModeSW_Timer_Counter[LEFT] = -1;  //reinitialize the counter to 0 
            ModeSW_Timer_Counter[RIGHT] = -1;  //reinitialize the counter to 0 

            return;
        }
#endif

        // event detection is over (press and hold event), thus we can reset the counter/timer
        ModeSW_Timer_Counter[channel] = 0; //reset our counter so that we do not fast flip through all the modes....

    }
    
    if(mode_switch == 1)           //done with the depressing of the mode switch
    {
        ModeSW_Timer_Counter[channel] = 0; //reset our counter so that we do not fast flip through all the modes....
    }

}