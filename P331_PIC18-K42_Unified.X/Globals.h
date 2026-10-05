#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define YES 1
#define NO  0

///////////////////////////////////////
#define AM_I_EVL        YES  /* use NO here for original P331 and YES for EVL; Metallica eat yer heart out.... */
///////////////////////////////////////

#define SN_GREATER_THAN_133 /*for serial number greater than 133, ORIGINAL only, we need another fixed 0.5 dB or so decrease (it's too hot with this new run of boards)*/

#if AM_I_EVL == YES
#define I_AM_EVL
#define EVL
#undef P331_ORIGINAL
#else
#define I_AM_NOT_EVL
#undef EVL
#define P331_ORIGINAL
#endif

#define LEFT 0U
#define RIGHT 1U

#define IN_STEREO 2

#define FALSE 0
#define TRUE !FALSE

#define LED_DEFAULT_BIAS 200

typedef unsigned char uchar_t;

typedef unsigned int  uint16_t;
typedef signed   int  int16_t;

enum enumChannelModes           //* enumeration for channel modes actuated by momentary Mode switch on loading knob controls...
{
    PadMatchTube,               //* "Pre Mode":  Input pad attenuation matches tube gain (loading knob)
    MasterVolumeMode,           //* "Post MOde": Output Level CUTs and tracks Tube Gain (both operated by loading knob).  <BR> If user manually CUTs using Output Level knob, then Output Level is CUT further until running out of relay ladder positions, at which point Input Pad will begin to provide additional "CUTing"
    TubeGainNoPadControl,       //* "Open Mode": Input pad disabled (0dB constant) and does not track loading knob or Tube gain (loading knob)
    _enum_max_SetLEDBrightness            //* Used to enter LED brightnes adjust mode; should adjust both channel's LEDs simultaneously
            
};
uchar_t ChannelMode[IN_STEREO];    //* Overall channel mode, "pre", "post", "open" e.g., <BR> Input Pad matched Tube Gain (Loading Knob), <BR> Master Volume Mode, <BR> separate Tube Gain and Output Level control without Input Pad (Pad disabled) <BR><BR> also contains the channel mode for LED brightness adjustment
uchar_t PrevChannelMode[IN_STEREO];    //*use for autoCUT()... Overall channel mode, "pre", "post", "open" e.g., <BR> Input Pad matched Tube Gain (Loading Knob), <BR> Master Volume Mode, <BR> separate Tube Gain and Output Level control without Input Pad (Pad disabled) <BR><BR> also contains the channel mode for LED brightness adjustment

uchar_t gLinkChannels;             //* determines if channels are linked.  Left channel control settings become the master for both channel settings.

uchar_t autoCut_mode[IN_STEREO];

uchar_t blink_counter = 0; // 256 with rollover for LED blinking (but we'll probably only need to go up to 12 or so and force rollover)

#endif
