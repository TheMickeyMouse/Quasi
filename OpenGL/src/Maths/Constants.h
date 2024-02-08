#pragma once
#include <limits>

#include "NumTypes.h"

namespace Maths {
#define NUM_LIMITS(T, V) std::numeric_limits<T>::V()
#define CONSTANT static constexpr
#define MATH_CONSTANT(X, V) CONSTANT float X = (float)(V); CONSTANT double X##64 = V; CONSTANT quadf X##128 = V
    MATH_CONSTANT(TAU, 6.28318'53071'79586'47692'52867);
    MATH_CONSTANT(PI,  3.14159'26535'89793'23946'26433);
    MATH_CONSTANT(HALF_PI,    PI * 0.5);
    MATH_CONSTANT(QUARTER_PI, PI * 0.25);
    
    MATH_CONSTANT(E,     2.71828'18284'59045'23536'02874);
    MATH_CONSTANT(INV_E, 0.36787'94411'71442'32159'55238);
    MATH_CONSTANT(LN_2,  0.69314'71805'59945'30941'72321);

    MATH_CONSTANT(ROOT_2,      1.41421'35623'73095'04880'16887);
    MATH_CONSTANT(HALF_ROOT_2, ROOT_2 * 0.5);
    MATH_CONSTANT(INV_ROOT_2,  HALF_ROOT_2);
    
    MATH_CONSTANT(ROOT_3,      1.73205'08075'68877'29352'74463);
    MATH_CONSTANT(HALF_ROOT_3, ROOT_3 * 0.5);
    MATH_CONSTANT(INV_ROOT_3,  1 / ROOT_3);

    MATH_CONSTANT(SIN_30, HALF_ROOT_3);
    MATH_CONSTANT(COS_30, 0.5);
    MATH_CONSTANT(SIN_45, INV_ROOT_2);
    MATH_CONSTANT(COS_45, SIN_45);
    MATH_CONSTANT(SIN_60, COS_30);
    MATH_CONSTANT(COS_60, SIN_30);

    MATH_CONSTANT(ROOT_5, 2.23606'79774'99789'69640'91737);
    MATH_CONSTANT(PHI,    1.61803'39887'49894'84820'45868);
    

    CONSTANT float INF     = NUM_LIMITS(float, infinity);
    CONSTANT float EPSILON = NUM_LIMITS(float, epsilon); // min value such that 1 + EPSILON != 1
    CONSTANT float MIN_F   = NUM_LIMITS(float, min); // smallest possible positive
    
    CONSTANT double INF64     = NUM_LIMITS(double, infinity);
    CONSTANT double EPSILON64 = NUM_LIMITS(double, epsilon); // min value such that 1 + EPSILON != 1
    CONSTANT double MIN_F64   = NUM_LIMITS(double, min); // smallest possible positive
    
    CONSTANT quadf INF128     = NUM_LIMITS(quadf, infinity);
    CONSTANT quadf EPSILON128 = NUM_LIMITS(quadf, epsilon); // min value such that 1 + EPSILON != 1
    CONSTANT quadf MIN_F128   = NUM_LIMITS(quadf, min); // smallest possible positive

    CONSTANT char  MAX_8  = NUM_LIMITS(char,  max);
    CONSTANT char  MIN_8  = NUM_LIMITS(char,  lowest);
    CONSTANT uchar MAX_U8 = NUM_LIMITS(uchar, max);
    
    CONSTANT int16  MAX_16  = NUM_LIMITS(short,  max);
    CONSTANT int16  MIN_16  = NUM_LIMITS(short,  lowest);
    CONSTANT uint16 MAX_U16 = NUM_LIMITS(uint16, max);

    CONSTANT int  MAX_32  = NUM_LIMITS(int,  max);
    CONSTANT int  MIN_32  = NUM_LIMITS(int,  lowest);
    CONSTANT uint MAX_U32 = NUM_LIMITS(uint, max);
    
    CONSTANT int64  MAX_64  = NUM_LIMITS(int64,  max);
    CONSTANT int64  MIN_64  = NUM_LIMITS(int64,  lowest);
    CONSTANT uint64 MAX_U64 = NUM_LIMITS(uint64, max);

    inline float operator ""_deg(quadf deg) {
        return (float)(deg * 180.0 / PI128);
    }
#undef MATH_CONSTANT
#undef CONSTANT
#undef NUM_LIMITS
}
