#pragma once
#include <limits>
#include "Type.h"

namespace Quasi::Math {
#define NUM_LIMITS(T, V) std::numeric_limits<T>::V()
#define CONSTANT static constexpr
#define MATH_CONSTANT(X, V) CONSTANT f32 X = (f32)(V); CONSTANT f64 X##64 = V; CONSTANT f80 X##80 = V
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

    MATH_CONSTANT(DEG2RAD, PI / 180);
    MATH_CONSTANT(RAD2DEG, 180 / PI);

    CONSTANT float INF     = NUM_LIMITS(float, infinity);
    CONSTANT float EPSILON = NUM_LIMITS(float, epsilon); // min value such that 1 + EPSILON != 1
    CONSTANT float MIN_F   = NUM_LIMITS(float, min); // smallest possible positive
    
    CONSTANT double INF64     = NUM_LIMITS(double, infinity);
    CONSTANT double EPSILON64 = NUM_LIMITS(double, epsilon); // min value such that 1 + EPSILON != 1
    CONSTANT double MIN_F64   = NUM_LIMITS(double, min); // smallest possible positive
    
    CONSTANT f80 INF80     = NUM_LIMITS(f80, infinity);
    CONSTANT f80 EPSILON80 = NUM_LIMITS(f80, epsilon); // min value such that 1 + EPSILON != 1
    CONSTANT f80 MIN_F80   = NUM_LIMITS(f80, min); // smallest possible positive

    CONSTANT char  MAX_8  = NUM_LIMITS(char,  max);
    CONSTANT char  MIN_8  = NUM_LIMITS(char,  lowest);
    CONSTANT uchar MAX_U8 = NUM_LIMITS(uchar, max);
    
    CONSTANT i16 MAX_16  = NUM_LIMITS(i16, max);
    CONSTANT i16 MIN_16  = NUM_LIMITS(i16, lowest);
    CONSTANT u16 MAX_U16 = NUM_LIMITS(u16, max);

    CONSTANT i32 MAX_32  = NUM_LIMITS(i32, max);
    CONSTANT i32 MIN_32  = NUM_LIMITS(i32, lowest);
    CONSTANT u32 MAX_U32 = NUM_LIMITS(u32, max);
    
    CONSTANT i64 MAX_64  = NUM_LIMITS(i64, max);
    CONSTANT i64 MIN_64  = NUM_LIMITS(i64, lowest);
    CONSTANT u64 MAX_U64 = NUM_LIMITS(u64, max);

    struct Angle {
        f80 radians;
        operator f32() const { return (f32)radians; }
        operator f64() const { return (f64)radians; }
        operator f80() const { return (f80)radians; }
    };

    inline Angle operator ""_deg(f80 deg) {
        return { deg / 180.0 * PI80 };
    }

    struct IPow { i64 pow; IPow operator-() const { return { -pow }; } };
    inline IPow operator ""_p(u64 p) { return { (i64)p }; }

    inline float operator^(float flt, IPow p) {
        if (p.pow == 0) return 1.0f;
        if (p.pow < 0) return 1.0f / (flt ^ -p);
        float result = 1.0f;
        while (p.pow) {
            if (p.pow & 1) result *= flt;
            p.pow = p.pow >> 1;
            flt *= flt;
        }
        return result;
    }

#undef MATH_CONSTANT
#undef CONSTANT
#undef NUM_LIMITS
}
