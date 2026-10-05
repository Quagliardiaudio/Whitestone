#include "fixed_point.h"

signed char to_fp(uchar_t sign, signed char whole, uchar_t frac) /// frac must be positive 0, 1, 2, or 3 (per our fractional dB scheme, where 1 means 0.25 dB, 2 means 0.5 dB etc.)... the sign will keep things negative for whole or frac...
{
    signed char fxp;
    fxp =
            (
              (sign ? -frac : frac) // consider the lowest bits first to keep sign bit proper for fractions less than/greater than +/- 1 around either side of 0
            + (whole << 2) //eats the sign bit of this SIGNED _whole_ number, but shifts the value into the upper 6 bits (value is 0 through 11, only uses a few bits anyway)
            ); // the above marries the lookup table's whole and fractional dB columns into a fixed point integer number (sign(1).whole(5).fractional(2)), while preserving sign from the sign column in the lookup tables
    
            
    return fxp;
    
}

void from_fp(signed char fxp /**input */, fp_t* fp /**output*/)
{
    
    //convert back to separate variables for whole and fractional after we have added the new fractional resolution from the LUT (which handles carry forward of bits during the addition in the whole bit field range in the desired_gain_fxp variable naturally and organically)
    fp->sign = fxp & 0b10000000;
    fp->frac = fxp & 0b00000011;    // get the fractional part
    fp->whole = fxp >> 2;           // shift back to base whole dB
    fp->whole |=
            fp->sign
          | fp->sign >> 1;			// then restore the sign bit and shift in a 1 (if the number was negative) into bit6 otherwise, it will shift a 0 if positive (if desired_gain_sign is 0)

}
