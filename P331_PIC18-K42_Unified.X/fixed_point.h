/* 
 * File:   fixed_point.h
 * Author: _
 *
 * Created on December 24, 2021, 7:24 PM
 */

#ifndef FIXED_POINT_H
#define	FIXED_POINT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "Globals.h"

    typedef struct fp
    {
        uchar_t sign;
        signed char whole;
        uchar_t frac;
    }fp_t;

    //prototypes
    signed char to_fp(uchar_t sign, signed char whole, uchar_t frac);
    void from_fp(signed char fxp, fp_t* fp);
    
#define TO_FP_INT_WHOLE(x) ((x)<<2)

#ifdef	__cplusplus
}
#endif

#endif	/* FIXED_POINT_H */

