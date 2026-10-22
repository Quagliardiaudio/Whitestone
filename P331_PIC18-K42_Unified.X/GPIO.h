#ifndef _GPIO_H_
#define _GPIO_H_

#include "Globals.h"
#include "Controls.h"  //includes typedef for uchar
#include "Hardware.h" //GPIO_EXPANDER selection

//Note: the GPIO expansion chip is selected with GPIO_EXPANDER in Hardware.h: either the original Exar/MaxLinear XRA1405 (abbreviated XR)
//      or its replacement, the Microchip MCP23S18 (abbreviated MCP). Application code uses the chip-neutral GPX_ register names defined below.

//prototypes
void GPIO_INIT(void);        //Initialize IO chips
void read_All_GPIO_Bytes(void);
void write_All_GPIO_Bytes(void);

enum {
    CS_A   = 0U,  // offset by 0 for chip select A GPIO expansion chip for either channel
    CS_B   = 1U,  // offset by 1 for chip select B GPIO expansion chip for either channel
    CS_C   = 2U,  // offset by 2 for chip select C GPIO expansion chip for either channel
    CS_D   = 3U  // offset by 3 for chip select D GPIO expansion chip for either channel
};
//* this define creates the equivalent of Left CS_x or Right CS_x from the calculation depending upon which channel using #define LEFT 0 or #define RIGHT 1 channels 
#define CALCULATE_CS(zero_based_channel_LEFTorRIGHT_hash_defined, individual_CS_A_thru_D_for_this_channel) (uchar_t)((zero_based_channel_LEFTorRIGHT_hash_defined << 2) + (individual_CS_A_thru_D_for_this_channel))  /*(note bit shifting to left by 1 effectively multiplies by 2; shifting by 2 multiplies by 4 (2*2))*/

#if GPIO_EXPANDER == GPIO_EXP_XRA1405
//handy enums for GPIO expansion chips (MFG: Exar)
enum {
    GSR1 	=	0x00U,	//	GSR1 - GPIO State for P0-P7                                         Read-Only	Default: 0xXX	
    GSR2 	=	0x01U,	//	GSR2 - GPIO State for P8-P15                                        Read-Only	Default: 0xXX	
    OCR1 	=	0x02U,	//	OCR1 - Output Control for P0-P7                                     Read/Write	Default: 0xFF	
    OCR2 	=	0x03U,	//	OCR2 - Output Control for P8-P15                                    Read/Write	Default: 0xFF	
    XR_PIR1 =   0x04U,	//	PIR1 - Input Polarity Inversion for P0-P7  (prepend XR_ conflict with PIC *.h file definitions) Read/Write	Default: 0x00	
    XR_PIR2 =	0x05U,	//	PIR2 - Input Polarity Inversion for P8-P15 (prepend XR_ conflict with PIC *.h file definitions) Read/Write	Default: 0x00	
    GCR1 	=	0x06U,	//	GCR1 - GPIO Configuration for P0-P7                                 Read/Write	Default: 0xFF	
    GCR2 	=	0x07U,	//	GCR2 - GPIO Configuration for P8-P15                                Read/Write	Default: 0xFF	
    PUR1 	=	0x08U,	//	PUR1 - Input Internal Pull-up Resistor Enable/Disable for P0-P7     Read/Write	Default: 0x00	
    PUR2 	=	0x09U,	//	PUR2 - Input Internal Pull-up Resistor Enable/Disable for P8-P15	Read/Write	Default: 0x00	
    IER1 	=	0x0AU,	//	IER1 - Input Interrupt Enable for P0-P7                             Read/Write	Default: 0x00	
    IER2 	=	0x0BU,	//	IER2 - Input Interrupt Enable for P8-P15                            Read/Write	Default: 0x00	
    TSCR1 	=	0x0CU,	//	TSCR1 - Output Three-State Control for P0-P7                        Read/Write	Default: 0x00	
    TSCR2 	=	0x0DU,	//	TSCR2 - Output Three-State Control for P8-P15                       Read/Write	Default: 0x00	
    ISR1 	=	0x0EU,	//	ISR1 - Input Interrupt Status for P0-P7                             Read        Default: 0x00	
    ISR2 	=	0x0FU,	//	ISR2 - Input Interrupt Status for P8-P15                            Read        Default: 0x00	
    REIR1 	=	0x10U,	//	REIR1 - Input Rising Edge Interrupt Enable for P0-P7                Read/Write	Default: 0x00	
    REIR2 	=	0x11U,	//	REIR2 - Input Rising Edge Interrupt Enable for P8-P15               Read/Write	Default: 0x00	
    FEIR1 	=	0x12U,	//	FEIR1 - Input Falling Edge Interrupt Enable for P0-P7               Read/Write	Default: 0x00	
    FEIR2 	=	0x13U,	//	FEIR2 - Input Falling Edge Interrupt Enable for P8-P15              Read/Write	Default: 0x00	
    IFR1 	=	0x14U,	//	IFR1 - Input Filter Enable/Disable for P0-P7                        Read/Write	Default: 0xFF	
    IFR2 	=	0x15U	//	IFR2 - Input Filter Enable/Disable for P8-P15                       Read/Write	Default: 0xFF	
};

// chip-neutral register names; port 1 = P0-P7, port 2 = P8-P15 (port 2 register is always port 1 register + 1)
#define GPX_STATE1   GSR1   /* read pin states   */
#define GPX_STATE2   GSR2
#define GPX_OUTPUT1  OCR1   /* output latch      */
#define GPX_OUTPUT2  OCR2
#define GPX_DIR1     GCR1   /* 1 = input, 0 = output */
#define GPX_DIR2     GCR2
#define GPX_PULLUP1  PUR1   /* 1 = pull-up enabled */
#define GPX_PULLUP2  PUR2

#elif GPIO_EXP_IS_MCP23SXX
//handy enums for GPIO expansion chips (MFG: Microchip MCP23S17 / MCP23S18), register addresses with IOCON.BANK = 0 (power-up default)
//port A (GPA0-GPA7) replaces XRA1405 P0-P7, port B (GPB0-GPB7) replaces P8-P15
enum {
    MCP_IODIRA   =  0x00U,  //  IODIRA   - I/O Direction for GPA0-7 (1 = input)                    Read/Write  Default: 0xFF
    MCP_IODIRB   =  0x01U,  //  IODIRB   - I/O Direction for GPB0-7 (1 = input)                    Read/Write  Default: 0xFF
    MCP_IPOLA    =  0x02U,  //  IPOLA    - Input Polarity for GPA0-7                               Read/Write  Default: 0x00
    MCP_IPOLB    =  0x03U,  //  IPOLB    - Input Polarity for GPB0-7                               Read/Write  Default: 0x00
    MCP_GPINTENA =  0x04U,  //  GPINTENA - Interrupt-on-change Enable for GPA0-7                   Read/Write  Default: 0x00
    MCP_GPINTENB =  0x05U,  //  GPINTENB - Interrupt-on-change Enable for GPB0-7                   Read/Write  Default: 0x00
    MCP_DEFVALA  =  0x06U,  //  DEFVALA  - Default Compare Value for GPA0-7                        Read/Write  Default: 0x00
    MCP_DEFVALB  =  0x07U,  //  DEFVALB  - Default Compare Value for GPB0-7                        Read/Write  Default: 0x00
    MCP_INTCONA  =  0x08U,  //  INTCONA  - Interrupt Control for GPA0-7                            Read/Write  Default: 0x00
    MCP_INTCONB  =  0x09U,  //  INTCONB  - Interrupt Control for GPB0-7                            Read/Write  Default: 0x00
    MCP_IOCON    =  0x0AU,  //  IOCON    - Configuration (also mirrored at 0x0B)                   Read/Write  Default: 0x00
    MCP_GPPUA    =  0x0CU,  //  GPPUA    - Input Pull-up (~100k) Enable for GPA0-7                 Read/Write  Default: 0x00
    MCP_GPPUB    =  0x0DU,  //  GPPUB    - Input Pull-up (~100k) Enable for GPB0-7                 Read/Write  Default: 0x00
    MCP_INTFA    =  0x0EU,  //  INTFA    - Interrupt Flag for GPA0-7                               Read        Default: 0x00
    MCP_INTFB    =  0x0FU,  //  INTFB    - Interrupt Flag for GPB0-7                               Read        Default: 0x00
    MCP_INTCAPA  =  0x10U,  //  INTCAPA  - Interrupt Capture for GPA0-7                            Read        Default: 0x00
    MCP_INTCAPB  =  0x11U,  //  INTCAPB  - Interrupt Capture for GPB0-7                            Read        Default: 0x00
    MCP_GPIOA    =  0x12U,  //  GPIOA    - Port state for GPA0-7                                   Read/Write  Default: 0x00
    MCP_GPIOB    =  0x13U,  //  GPIOB    - Port state for GPB0-7                                   Read/Write  Default: 0x00
    MCP_OLATA    =  0x14U,  //  OLATA    - Output Latch for GPA0-7                                 Read/Write  Default: 0x00
    MCP_OLATB    =  0x15U   //  OLATB    - Output Latch for GPB0-7                                 Read/Write  Default: 0x00
};

#define MCP_OPCODE_WRITE  0x40U  /* SPI opcode 0100 A2A1A0 R/W with address 000: MCP23S18 has no address pins; MCP23S17 ignores A2-A0 while IOCON.HAEN = 0 */
#define MCP_OPCODE_READ   0x41U

// IOCON bits: BANK(7) MIRROR(6) SEQOP(5) DISSLW(4, S17 only) HAEN(3, S17 only) ODR(2) INTPOL(1) INTCC(0)
// BANK = 0   : A/B registers paired at consecutive addresses (required by the port 1 + 1 = port 2 math in GPIO.c)
// SEQOP = 1  : no address auto-increment; every SPI frame accesses exactly one register
// ODR = 1    : INTA/INTB open-drain, like the XRA1405 IRQ# output they replace
// HAEN = 0   : (MCP23S17) hardware address pins ignored; each chip is reached through its own chip select only
#define MCP_IOCON_INIT    0b00100100U

// chip-neutral register names; port 1 = GPA0-7 (was P0-P7), port 2 = GPB0-7 (was P8-P15) (port 2 register is always port 1 register + 1)
#define GPX_STATE1   MCP_GPIOA   /* read pin states   */
#define GPX_STATE2   MCP_GPIOB
#define GPX_OUTPUT1  MCP_OLATA   /* output latch      */
#define GPX_OUTPUT2  MCP_OLATB
#define GPX_DIR1     MCP_IODIRA  /* 1 = input, 0 = output */
#define GPX_DIR2     MCP_IODIRB
#define GPX_PULLUP1  MCP_GPPUA   /* 1 = pull-up enabled */
#define GPX_PULLUP2  MCP_GPPUB

#else
#error "GPIO_EXPANDER in Hardware.h must be GPIO_EXP_XRA1405, GPIO_EXP_MCP23S17 or GPIO_EXP_MCP23S18"
#endif

// GPIO channel frame; bits directly from GPIO expansion chip schematics (plural) for all signals on each of the GPIO expansion chips
// Note convention for inputs and outputs with active high or active low configurations for variable (bits) prefixes:
// Variable bits names will begin with a lowercase letter p, b, q, d -- the convention is: look at the _tail_ of the letter prefix, 
//
//               if tail points up, then active high (b or d)
//               if tail points down, then active low (p or q)
//               for tail on left side of the letter body (p and b), then input
//               for tail on right side of the letter body (q and d), then output
//               in other words, the circle-loop of the letter is the "thing" (bit/variable) being input or output, and the tail denotes the polarity of active state (high or low)
//

typedef union GPIO_ChannelFrame_t
{
    
    struct GPIO_bitfields{

#define BYTE_INDEX_LOADING_KNOB 0
#define BYTE_INDEX_XFORMER_KNOB 0
#define MASK_LOADING_KNOB 0b00111100U
#define SHIFT_LOADING_KNOB 2
#define MASK_XFORMER_KNOB  0b11000000U
#define SHIFT_XFORMER_KNOB 6 

        uchar_t 	
        pCUT                :1, 	//* 0 = Output level CUT mode <BR> 1 = Output level not CUTting <BR> Description: Gain Mode Switch (Cut), Front Panel <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P0 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: S1,  <BR> Note: Gain mode OUT is calculated when neither Cut or Boost is active, and should set Output Level seven relay ladder range and output relay 1 for 0dB
        pBOOST              :1, 	//* 0 = Output level BOOST mode <BR> 1 = Output level not BOOSTing <BR> Description: Gain Mode Switch (Boost), Front Panel <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P1 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: S1,  <BR> Note: Gain mode OUT is calculated when neither Cut or Boost is active, and should set Output Level seven relay ladder range and output relay 1 for 0dB
        bLOADING_A0         :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Loading Knob bit 0 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P2 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW3,  <BR> Note: Twelve positions is represented by these four bits (Loading Knob[3:0]) with zero base reference from 0 to 11 in binary count
        bLOADING_A1         :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Loading Knob bit 1 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P3 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW3,  <BR> Note: Twelve positions is represented by these four bits (Loading Knob[3:0]) with zero base reference from 0 to 11 in binary count
        bLOADING_A2         :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Loading Knob bit 2 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P4 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW3,  <BR> Note: Twelve positions is represented by these four bits (Loading Knob[3:0]) with zero base reference from 0 to 11 in binary count
        bLOADING_A3         :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Loading Knob bit 3 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P5 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW3,  <BR> Note: Twelve positions is represented by these four bits (Loading Knob[3:0]) with zero base reference from 0 to 11 in binary count
        pXFORMER_A0         :1, 	//* 0 = Binary group (2-bits) active <BR> 1 = Binary group (2-bits) not active <BR> Description: Transformer Mode Knob bit 0 <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P6 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW2,  <BR> Note: Transformer mode is simple binary mode 0 through 2: 0 is Transformer OUT, Modes 1 and 2 are implemented in algorithms herein
        pXFORMER_A1         :1; 	//* 0 = Binary group (2-bits) active <BR> 1 = Binary group (2-bits) not active <BR> Description: Transformer Mode Knob bit 1 <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P7 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW2,  <BR> Note: Transformer mode is simple binary mode 0 through 2: 0 is Transformer OUT, Modes 1 and 2 are implemented in algorithms herein
	
#define BYTE_INDEX_OUTLEVEL_KNOB 1	
#define BYTE_INDEX_FILTER_KNOB 1	
#define MASK_OUTLEVEL_KNOB 0b00111100U	
#define SHIFT_OUTLEVEL_KNOB 2	
#define MASK_FILTER_KNOB  0b00000011U	
#define SHIFT_FILTER_KNOB 0            	
            	
        uchar_t 	
        pFILTER_A0          :1, 	//* 0 = Binary group (2-bits) active <BR> 1 = Binary group (2-bits) not active <BR> Description: Filter Mode Knob bit 0 <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P8 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW1,  <BR> Note: Filter mode is simple binary mode 0 through 3: 0 is Filter OUT, Mode 1: Hi, Mode 2 Low, Mode 3 Duo
        pFILTER_A1          :1, 	//* 0 = Binary group (2-bits) active <BR> 1 = Binary group (2-bits) not active <BR> Description: Filter Mode Knob bit 1 <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P9 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW1,  <BR> Note: Filter mode is simple binary mode 0 through 3: 0 is Filter OUT, Mode 1: Hi, Mode 2 Low, Mode 3 Duo
        bOUTLEVEL_A0        :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Output Level Gain Knob bit 0 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P10 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW4,  <BR> Note: Twelve positions is represented by these four bits (Output Level Knob[3:0]) with zero base reference from 0 to 11 in binary count
        bOUTLEVEL_A1        :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Output Level Gain Knob bit 1 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P11 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW4,  <BR> Note: Twelve positions is represented by these four bits (Output Level Knob[3:0]) with zero base reference from 0 to 11 in binary count
        bOUTLEVEL_A2        :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Output Level Gain Knob bit 2 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P12 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW4,  <BR> Note: Twelve positions is represented by these four bits (Output Level Knob[3:0]) with zero base reference from 0 to 11 in binary count
        bOUTLEVEL_A3        :1, 	//* 0 = Binary group (4-bits) not active <BR> 1 = Binary group (4-bits) active <BR> Description: Output Level Gain Knob bit 3 <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P13 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: SW4,  <BR> Note: Twelve positions is represented by these four bits (Output Level Knob[3:0]) with zero base reference from 0 to 11 in binary count
        pFRAC_0DB           :1, 	//* 0 = Output level fractional gain/attenuation 0dB <BR> 1 = Output level fractional gain/attenuation not set for 0dB <BR> Description: Output Level Fractional gain/attenuation 0dB <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P15 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: S2,  <BR> Note: 0.25dB is center off on this switch and is calculated when neither 0.5dB or 0dB are engaged
        pFRAC_0P5DB         :1; 	//* 0 = Output level fractional gain/attenuation 0.5dB <BR> 1 = Output level fractional gain/attenuation not set for 0.5dB <BR> Description: Output Level Fractional gain/attenuation 0.5dB <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_A <BR> GPIO Port (Pin) Name:  P14 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: S2,  <BR> Note: 0.25dB is center off on this switch and is calculated when neither 0.5dB or 0dB are engaged
	    //note: it appears that pFRAC_0DB and pFRAC_0P5DB may have been swapped in hardware?  Yes, the switch sense was reversed versus the silkscreen front panel reference text... The above reflects this swap, and corrects/compensates for it... 
    	
#define K_OUTLEVEL_BYTE_GROUP 2	
#define MASK_OUTLEVEL_RELAYS_0 0b00000000	

        uchar_t 	
        dOUTLEVEL_0_RANGE   :1,     //* 0 = Relay not engaged, Odd dB values are selected <BR> 1 = Relay Engaged, Even dB levels selected <BR> Description: Seven relay ladder range (even or odd gains/attenuations): Output Level K0 (Range) even/odd dB selection  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P0 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K13, K26
        dOUTLEVEL_1         :1, 	//* 0 = Relay not engaged, 0dB or 1dB depending upon Range Relay (K0 RANGE) for even or odd dB level selection <BR> 1 = Relay engaged, 0dB or 1dB not selected <BR> Description: Seven relay ladder, relay 1 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P1 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K19, K32 <BR> Note: Relay has highest wired priority when setting gain/attenuation. <BR> Relay is opposite polarity than other relays in this ladder to account for relay priority when brown-out or power down (this relay takes precedence to drop gain closer to 0dB during power down or brown-out; mechanically / electrically rather than "softwarely")
        dOUTLEVEL_2         :1, 	//* 0 = Relay not engaged, 2dB or 3dB not selected <BR> 1 = Relay engaged, 2dB or 3dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 2 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P2 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K18, K31
        dOUTLEVEL_3         :1, 	//* 0 = Relay not engaged, 4dB or 5dB not selected <BR> 1 = Relay engaged, 4dB or 5dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 3 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P3 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K17, K30
        dOUTLEVEL_4         :1, 	//* 0 = Relay not engaged, 6dB or 7dB not selected <BR> 1 = Relay engaged, 6dB or 7dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 4 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P4 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K16, K29
        dOUTLEVEL_5         :1, 	//* 0 = Relay not engaged, 8dB or 9dB not selected <BR> 1 = Relay engaged, 8dB or 9dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 5 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P5 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K15, K28
        dOUTLEVEL_6         :1, 	//* 0 = Relay not engaged, 10dB or 11dB not selected <BR> 1 = Relay engaged, 10dB or 11dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 6 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P6 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K14, K27 <BR> Note: Relay has least wired priority
        dOUTLEVEL_0_25      :1;     //* 0 = Relay not engaged, 0.25 fractional gain/attenuation not selected <BR> 1 = Gain or attenuated by 0.25dB depending upon boost or cut status <BR> Description: Fractional gain relay 0.25dB <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P7 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K20, K33
	
#define K_XFORMER_BYTE_GROUP 3	
#define MASK_OUTLEVEL_RELAYS_1 0b11111100	

        uchar_t 	
        dOUTLEVEL_0_5       :1, 	//* 0 = Relay not engaged, 0.5 fractional gain/attenuation not selected <BR> 1 = Relay engaged, gain or attenuate by 0.5dB depending upon boost or cut status <BR> Description: Fractional gain relay 0.5dB <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P8 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K21, K34
        dOUTLEVEL_BOOST     :1, 	//* 0 = Relays not engaged, Output Level in CUT mode <BR> 1 = Relays engaged, Output Level in BOOST mode <BR> Description: Output Level boost or cut mode relay <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P9 <BR> Schematic Sheet of Interest: Output Level <BR> Reference Designators of Interest: K11, K12, K24, and K25 <BR> Note: This swaps T-network resistors to be either on the gain side of the opamp or on the attenuation side of the opamp.  Fractional T-network resistors are always on the opposite side of the main whole dB T-network resistors; when attenuating by whole dB resistor values, fractional resistor values are gain values and vice versa.
        dXFORMER_SEC        :1, 	//* 0 = Relay not engaged, Transformer not in circuit <BR> 1 =  <BR> Description:  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P10 <BR> Schematic Sheet of Interest: Transformer <BR> Reference Designators of Interest: K53, K57
        dXFORMER_MODE2      :1, 	//* 0 = Relay not engaged, Mode 1 (if Transformer is "in") <BR> 1 =  <BR> Description:  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P11 <BR> Schematic Sheet of Interest: Transformer <BR> Reference Designators of Interest: K54, K58
        dXFORMER_PRI        :1, 	//* 0 = Relay not engaged, primary of transformer not energized (other than any bleeder resistance used for anti-pop) <BR> 1 =  <BR> Description:  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P12 <BR> Schematic Sheet of Interest: Transformer <BR> Reference Designators of Interest: K55, K59
        dMIX_XFORMER_DB_TUNE:1, 	//* 0 = Relay not engaged, tuning of opamp gain to overcome Transformer insertion loss is not engaged <BR> 1 = Relay engaged, Mode 1 or Mode 2 of Transformer operation will select gain tuning resistors/potentiometers to change gain of this mixer opamp to overcome any transformer insertion loss <BR> Description:  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P13 <BR> Schematic Sheet of Interest: Output MIX <BR> Reference Designators of Interest: K22, K23
        pFB_CLEAN           :1, 	//* 0 = Mode switch set for Tube Opamp Feedback mode ("Clean Mode") <BR> 1 = Mode switch set for Tube Opamp Feedforward mode ("Bloom Mode") <BR> Description: Front Panel input for Tube Opamp mode (feedback or feedforward mode of operation, e.g., clean versus bloom modes) <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P14 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: , 
        pMODE               :1; 	//* 0 = Mode switch on momentary position, change overall system modes as programmed (e.g., pre versus post padding, or open system mode, or link channels selection) <BR> 1 = Mode switch not in momentary actuation <BR> Description: Front Panel input for Mode momentary toggle position (far right) to operate overall System mode (open, pre, post, link, LED brightness control) <BR> I/O: Input (Active Low) <BR> Chip Select Group: CS_B <BR> GPIO Port (Pin) Name:  P15 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO input sheet <BR> Reference Designators of Interest: , 
#define LOADING_SWITCH_CLEAN_MODE 0
#define LOADING_SWITCH_BLOOM_MODE 1
        
#define K_LOADING_BYTE_GROUP 4	
#define MASK_LOADING_RELAYS_0 0b11111111	

        uchar_t 	
        dTUBE_FF            :1, 	//* 0 = Relay not engaged, Feedforward (FF) not selected <BR> 1 = Relay engaged, Feedforward (FF) bloom mode switch position 12, or constant feedforward maximum gain for feedback mode (feedback reduces gain depending upon switch position) <BR> Description:  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P0 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K12, K24
        dTUBE_2DB           :1, 	//* 0 = Relay not engaged, 2dB tube gain not selected <BR> 1 = Relay engaged, 2dB of FF gain or feedback applied for 2dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 2dB feedforward gain, or feedback applied to create 2dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P1 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K11, K23
        dTUBE_3DB           :1, 	//* 0 = Relay not engaged, 3dB tube gain not selected <BR> 1 = Relay engaged, 3dB of FF gain or feedback applied for 3dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 3dB feedforward gain, or feedback applied to create 3dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P2 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K10, K22
        dTUBE_4DB           :1,     //* 0 = Relay not engaged, 4dB tube gain not selected <BR> 1 = Relay engaged, 4dB of FF gain or feedback applied for 4dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 4dB feedforward gain, or feedback applied to create 4dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P3 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K9, K21
        dTUBE_5DB           :1,     //* 0 = Relay not engaged, 5dB tube gain not selected <BR> 1 = Relay engaged, 5dB of FF gain or feedback applied for 5dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 5dB feedforward gain, or feedback applied to create 5dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P4 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K8, K20
        dTUBE_6DB           :1,     //* 0 = Relay not engaged, 6dB tube gain not selected <BR> 1 = Relay engaged, 6dB of FF gain or feedback applied for 6dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 6dB feedforward gain, or feedback applied to create 6dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P5 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K7, K19
        dTUBE_7DB           :1,     //* 0 = Relay not engaged, 7dB tube gain not selected <BR> 1 = Relay engaged, 7dB of FF gain or feedback applied for 7dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 7dB feedforward gain, or feedback applied to create 7dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P6 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K6, K18
        dTUBE_8DB           :1;     //* 0 = Relay not engaged, 8dB tube gain not selected <BR> 1 = Relay engaged, 8dB of FF gain or feedback applied for 8dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 8dB feedforward gain, or feedback applied to create 8dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P7 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K5, K17
	
#define K_FILTER_BYTE_GROUP 5	
#define MASK_LOADING_RELAYS_1 0b00001111	

        uchar_t 	
        dTUBE_9DB           :1,     //* 0 = Relay not engaged, 9dB tube gain not selected <BR> 1 = Relay engaged, 9dB of FF gain or feedback applied for 9dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 9dB feedforward gain, or feedback applied to create 9dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P8 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K4, K16
        dTUBE_10DB          :1,     //* 0 = Relay not engaged, 10dB tube gain not selected <BR> 1 = Relay engaged, 10dB of FF gain or feedback applied for 10dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 10dB feedforward gain, or feedback applied to create 10dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P9 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K3, K15
        dTUBE_11DB          :1, 	//* 0 = Relay not engaged, 11dB tube gain not selected <BR> 1 = Relay engaged, 11dB of FF gain or feedback applied for 11dB of overall gain (depending upon FF versus FB modes of operation) <BR> Description: 11dB feedforward gain, or feedback applied to create 11dB of overall gain <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P10 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K2, K14
        dTUBE_FB            :1, 	//* 0 = Relay not engaged <BR> 1 = Relay engaged, Feedback (FB) clean mode switch position 12, or constant feedback minimum (6dB) gain for feedforward mode  <BR> Description:  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P11 <BR> Schematic Sheet of Interest: Fully Differential Tube Opamp <BR> Reference Designators of Interest: K1, K13
        dFILT_EN            :1,     //* 0 = Relay not engaged, normal no filter mode <BR> 1 = Relay engaged, filter "IN" <BR> Description: Engage Input Filter (don't forget to also engage Output MIX relay) <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P12 <BR> Schematic Sheet of Interest: Input Filters <BR> Reference Designators of Interest: K2, K5
        dFILT_LP_EN         :1,     //* 0 = Relay not engaged <BR> 1 = Relay engaged, Low Pass providing LF to input filter mixing opamp <BR> Description: Low Pass filter enable <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P13 <BR> Schematic Sheet of Interest: Input Filters <BR> Reference Designators of Interest: K1, K4 <BR> Note: This is mixed locally at the input filter stage with the input filter mixing opamp
        dFILT_HP_EN         :1,     //* 0 = Relay not engaged <BR> 1 = Relay engaged, High Pass providing HF to input filter mixing opamp <BR> Description: High Pass filter enable <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P14 <BR> Schematic Sheet of Interest: Input Filters <BR> Reference Designators of Interest: K3, K6 <BR> Note: This is mixed locally at the input filter stage with the input filter mixing opamp
        bLINEAMP_ENGAGE     :1;     //* 0 = Line amp channel is bypassed <BR> 1 = Line amp channel is "IN" <BR> Description: Input to sense if bypass switch is active <BR> I/O: Input (Active High) <BR> Chip Select Group: CS_C <BR> GPIO Port (Pin) Name:  P15 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO _output_ schematic sheet as input (needed an extra pin on GPIO expansion chips, and had output chips available: note that each GPIO pin can be either I/O; see configuration of GPIO INIT code <BR> Reference Designators of Interest: S4, S5 <BR> Note: These are one-in-the-same switches but duplicated on the front panel PCB, left side or right side of the PCB such that a single front panel PCB design can be reused for both channels, just wired into different switch locations for the same singular switch function.  Same motif/paradigm for VU meter attenuation switches and VU meters, etc.)
	
#define K_PAD_BYTE_GROUP 6	
#define MASK_PAD_RELAYS 0b10000000	

        uchar_t 	
        dPAD_0_RANGE        :1, 	//* 0 = Relay not engaged, Odd dB values are selected <BR> 1 = Relay Engaged, Even dB levels selected <BR> Description: Seven relay ladder range (even or odd gains/attenuations): PAD K0 (Range) even/odd dB selection  <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P0 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K39, K46
        dPAD_1              :1, 	//* 0 = Relay not engaged, 0dB or 1dB depending upon Range Relay (K0 RANGE) for even or odd dB level selection <BR> 1 = Relay engaged, 0dB or 1dB not selected <BR> Description: Seven relay ladder, relay 1 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P1 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K45, K52 <BR> Note: Relay has highest wired priority when setting gain/attenuation. <BR> Relay is opposite polarity than other relays in this ladder to account for relay priority when brown-out or power down (this relay takes precedence to drop gain closer to 0dB during power down or brown-out; mechanically / electrically rather than "softwarely")
        dPAD_2              :1, 	//* 0 = Relay not engaged, 2dB or 3dB not selected <BR> 1 = Relay engaged, 2dB or 3dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 2 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P2 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K44, K51
        dPAD_3              :1, 	//* 0 = Relay not engaged, 4dB or 5dB not selected <BR> 1 = Relay engaged, 4dB or 5dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 3 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P3 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K43, K50
        dPAD_4              :1, 	//* 0 = Relay not engaged, 6dB or 7dB not selected <BR> 1 = Relay engaged, 6dB or 7dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 4 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P4 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K42, K49
        dPAD_5              :1, 	//* 0 = Relay not engaged, 8dB or 9dB not selected <BR> 1 = Relay engaged, 8dB or 9dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 5 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P5 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K41, K48
        dPAD_6              :1, 	//* 0 = Relay not engaged, 10dB or 11dB not selected <BR> 1 = Relay engaged, 10dB or 11dB depending upon Range relay (K0 RANGE) for even or odd dB level selection <BR> Description: Seven relay ladder, relay 6 <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P6 <BR> Schematic Sheet of Interest: PAD (Simplified) <BR> Reference Designators of Interest: K40, K47 <BR> Note: Relay has least wired priority
        dK_Lineamp_IN       :1; 	//* 0 = Actively providing ground return for channel's bypass relay, and allows the channel to come out of Bypass if so selected (powered) by the user and the front panel bypass switch <BR> 1 = Not active (Bypass relay not grounded, thus not energized; defaults channel into hardwired Bypass mode) <BR> Description: Relay driver to provide ground current sink to bypass relay <BR> I/O: Output (Active High) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P7 <BR> Schematic Sheet of Interest: Front Panel, SPI GPIO output <BR> Reference Designators of Interest: ,  <BR> Note: Engage this relay after tube warm-up (Autostandby), whence HT_PRESENT signal is good (on MCU)
	
#define DISPLAY_BYTE_GROUP 7	

        uchar_t 	
        display_a           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P8 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_b           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P9 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_c           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P10 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_d           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P11 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_e           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P12 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_f           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P13 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_g           :1,     //* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P14 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2
        display_dp          :1; 	//* 0 = Segment illuminated <BR> 1 = Segment not illuminated <BR> Description: Debug display, can also be additional GPIO for when display is not installed in production versions of Line Amp <BR> I/O: Output (Active Low, for debug display purposes) <BR> Chip Select Group: CS_D <BR> GPIO Port (Pin) Name:  P15 <BR> Schematic Sheet of Interest: SPI GPIO outputs <BR> Reference Designators of Interest: DS1, DS2

    }Bits;

#define GPIO_BYTE_GROUPS_T_SZ sizeof(struct GPIO_bitfields)

    uchar_t byte_groups[GPIO_BYTE_GROUPS_T_SZ];    //* GPIO byte groups
   
}GPIO_ChannelFrame_t;

GPIO_ChannelFrame_t bouncyGPIO_Values[IN_STEREO];         //* bouncy raw input bits (output bits not necessarily used; not memory efficient but at this time, there is plenty of memory left over and not used: 13JULY2018)
GPIO_ChannelFrame_t debouncedGPIO_Values[IN_STEREO];      //* debounced bits for inputs and storage for output bits

#define OUTPUT_GPIO  debouncedGPIO_Values   /** alias for debouncedGPIO_Values*/



#else
//externs
#endif





