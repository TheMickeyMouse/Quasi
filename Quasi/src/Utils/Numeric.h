#pragma once
#include <cstdint>
#include <cfloat>
#include <cmath>

#undef NAN
#undef INFINITY

namespace Quasi {
    using i8   = std::int8_t;
    using i16  = std::int16_t;
    using i32  = std::int32_t;
    using i64  = std::int64_t;
    using u8   = std::uint8_t;
    using u16  = std::uint16_t;
    using u32  = std::uint32_t;
    using u64  = std::uint64_t;
    using f32  = float;
    using f64  = double;
    using ushort = u16;
    using ulong  = u64;
    using usize  = std::size_t;
    using isize  = std::intptr_t;

    using uchar = unsigned char;
    using byte  = unsigned char;
    using sbyte = signed char;
    using uint  = unsigned int;

    namespace Math {
        // a fractional approximation of log_2(10). estimated by the continous fraction:
        // [3, 3, 9, 2, 2, 4, 6, 2, 1, 1, 3, 1, 18, 1, 6, 1, 2, 1, 1, 4, 1, 42, 6, 1, 4, 2, 3, 1]
        // or around 3.321928094887362.
        static constexpr usize LOG10BASE2_NUMERATOR   = 49007499804991;
        static constexpr usize LOG10BASE2_DENOMINATOR = 14752727453799;
        static constexpr float LOG10BASE2 = 3.321928094887362f;
    }

#define QUASI_DEFINE_INTEGER(INT) \
    namespace INT##s { \
        static constexpr bool  IS_SIGNED = ((INT)-1) < 0; \
        static constexpr INT   MAX       = ~(IS_SIGNED ? 1 << (sizeof(INT) * 8 - 1) : 0); \
        static constexpr INT   MIN       = ~MAX; \
        static constexpr usize BITS      = sizeof(INT) * 8; \
        static constexpr usize DIGITS    = 1 + (BITS - IS_SIGNED) * Math::LOG10BASE2_DENOMINATOR / Math::LOG10BASE2_NUMERATOR; \
    }

    QUASI_DEFINE_INTEGER(i8);
    QUASI_DEFINE_INTEGER(u8);
    QUASI_DEFINE_INTEGER(i16);
    QUASI_DEFINE_INTEGER(u16);
    QUASI_DEFINE_INTEGER(i32);
    QUASI_DEFINE_INTEGER(u32);
    QUASI_DEFINE_INTEGER(i64);
    QUASI_DEFINE_INTEGER(u64);
    QUASI_DEFINE_INTEGER(isize);
    QUASI_DEFINE_INTEGER(usize);

#undef QUASI_DEFINE_INTEGER

#define QUASI_DEFINE_FLOATING(FLOAT, CCODENAME) \
    namespace FLOAT##s { \
        static constexpr FLOAT INFINITY     =  1 / (FLOAT)0; \
        static constexpr FLOAT NEG_INFINITY = -1 / (FLOAT)0; \
        static constexpr FLOAT MAX          =  CCODENAME##_MAX; \
        static constexpr FLOAT MIN          = -CCODENAME##_MAX; \
        static constexpr FLOAT EPSILON      =  CCODENAME##_MIN; \
        static constexpr FLOAT TRUE_EPSILON =  CCODENAME##_TRUE_MIN; \
        static constexpr FLOAT DELTA        =  CCODENAME##_EPSILON; \
        static constexpr FLOAT NAN          =  INFINITY - INFINITY; \
        static constexpr int   MAX_EXP      =  CCODENAME##_MAX_EXP; \
        static constexpr int   MIN_EXP      =  CCODENAME##_MIN_EXP; \
        static constexpr int   MAX_EXP10    =  CCODENAME##_MAX_10_EXP; \
        static constexpr int   MIN_EXP10    =  CCODENAME##_MIN_10_EXP; \
        static constexpr usize DIGITS       =  CCODENAME##_MANT_DIG; \
        \
        static constexpr usize BITS          = sizeof(FLOAT) * 8; \
        static constexpr usize EXPONENT_BITS = 5 + sizeof(FLOAT) / 4 * 3; \
        static constexpr usize MANTISSA_BITS = BITS - EXPONENT_BITS - 1; \
        \
    }

    QUASI_DEFINE_FLOATING(f32, FLT);
    QUASI_DEFINE_FLOATING(f64, DBL);

#undef QUASI_DEFINE_FLOATING

    constexpr struct NaNType {
        constexpr operator f32() const { return f32s::NAN; }
        constexpr operator f64() const { return f64s::NAN; }
        constexpr bool operator==(const NaNType&) const = default;
        constexpr bool operator==(f32 f) const { return std::isnan(f); }
        constexpr bool operator==(f64 f) const { return std::isnan(f); }
    } NaN;
}
