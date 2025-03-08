#pragma once
#include <cstdint>
#include <cfloat>
#include <cmath>
#include <bit>

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

    template <class T> concept Integer   = std::is_integral_v<T>;
    template <class T> concept Signed    = std::is_signed_v<T>;
    template <class T> concept Unsigned  = std::is_unsigned_v<T>;
    template <class T> concept Floating  = std::is_floating_point_v<T>;
    template <class T> concept Numeric   = std::is_arithmetic_v<T>;

    template <class T> using IntoSigned   = std::make_signed<T>;
    template <class T> using IntoUnsigned = std::make_unsigned<T>;

    template <class T> struct NumInfo {};

    namespace Math {
        static constexpr u32 LOG2_LOOKUP[37] = {
            0,      0,      65536,  103872, 131072, 152169,
            169408, 183982, 196608, 207744, 217705, 226717,
            234944, 242512, 249518, 256041, 262144, 267875,
            273280, 278392, 283241, 287854, 292253, 296456,
            300480, 304339, 308048, 311616, 315054, 318372,
            321577, 324678, 327680, 330589, 333411, 336152, 338816
        };
        static constexpr u32 INV_LOG2_LOOKUP[37] = {
            ~0u,   ~0u,   65536, 41348, 32768, 28224,
            25352, 23344, 21845, 20674, 19728, 18944,
            18280, 17710, 17212, 16774, 16384, 16033,
            15716, 15427, 15163, 14920, 14696, 14487,
            14293, 14112, 13942, 13782, 13632, 13490,
            13355, 13228, 13107, 12991, 12881, 12776, 12676
        };
        static constexpr usize POWERS_OF_10[20] = {
            1,
            10,
            100,
            1'000,
            10'000,
            100'000,
            1'000'000,
            10'000'000,
            100'000'000,
            1'000'000'000,
            10'000'000'000,
            100'000'000'000,
            1'000'000'000'000,
            10'000'000'000'000,
            100'000'000'000'000,
            1'000'000'000'000'000,
            10'000'000'000'000'000,
            100'000'000'000'000'000,
            1'000'000'000'000'000'000,
            10'000'000'000'000'000'000,
        };

        // a fractional approximation of log_2(10). = 217706 / 2^16
        static constexpr usize LOG10_2_MUL     = LOG2_LOOKUP[10];
        // a fractional approximation of 1 / log_2(10). = 19728 / 2^16
        static constexpr usize INV_LOG10_2_MUL = INV_LOG2_LOOKUP[10];
        static constexpr f64 LOG10_2 = 3.321928094887362f;
    }

#define QUASI_DEFINE_INTEGER(INT, UINT) \
    namespace INT##s { \
        static constexpr bool  IS_SIGNED = ((INT)-1) < 0; \
        static constexpr INT   MAX       = ~(IS_SIGNED ? 1 << (sizeof(INT) * 8 - 1) : 0); \
        static constexpr INT   MIN       = ~MAX; \
        static constexpr usize BITS      = sizeof(INT) * 8; \
        static constexpr usize DIGITS    = 1 + ((BITS - IS_SIGNED) * Math::INV_LOG10_2_MUL >> 16); \
        \
        bool AddOverflow(INT a, INT b, INT& out) { return __builtin_add_overflow(a, b, &out); } \
        bool SubOverflow(INT a, INT b, INT& out) { return __builtin_sub_overflow(a, b, &out); } \
        bool MulOverflow(INT a, INT b, INT& out) { return __builtin_mul_overflow(a, b, &out); } \
        \
        u32  CountOnes (INT x) { return std::popcount((UINT)x); } \
        u32  CountZeros(INT x) { return std::popcount((UINT)~(UINT)x); } \
        u32  CountLeftZeros (INT x) { return std::countl_zero((UINT)x); } \
        u32  CountLeftOnes  (INT x) { return std::countl_one ((UINT)x); } \
        u32  CountRightZeros(INT x) { return std::countr_zero((UINT)x); } \
        u32  CountRightOnes (INT x) { return std::countr_one ((UINT)x); } \
        \
        bool IsPow2    (INT x) { return x && !(x & (x - 1)); } \
        u32  BitWidth  (INT x) { return std::bit_width((UINT)x); } \
        u32  Log2      (INT x) { return BitWidth(x) - 1; } \
        u32  Log10     (INT x) { \
            u32 approx10 = Log2(x) * Math::INV_LOG10_2_MUL >> 16; \
            return approx10 - (x < Math::POWERS_OF_10[approx10]); \
        } \
    }

    QUASI_DEFINE_INTEGER(i8,    u8);
    QUASI_DEFINE_INTEGER(u8,    u8);
    QUASI_DEFINE_INTEGER(i16,   u16);
    QUASI_DEFINE_INTEGER(u16,   u16);
    QUASI_DEFINE_INTEGER(i32,   u32);
    QUASI_DEFINE_INTEGER(u32,   u32);
    QUASI_DEFINE_INTEGER(i64,   u64);
    QUASI_DEFINE_INTEGER(u64,   u64);
    QUASI_DEFINE_INTEGER(isize, usize);
    QUASI_DEFINE_INTEGER(usize, usize);

#undef QUASI_DEFINE_INTEGER

    template <Integer N>
    struct NumInfo<N> {
        using UnsignedInt = IntoUnsigned<N>;

        static constexpr bool  IS_SIGNED = ((N)-1) < 0;
        static constexpr N     MAX       = ~(IS_SIGNED ? 1 << (sizeof(N) * 8 - 1) : 0);
        static constexpr N     MIN       = ~MAX;
        static constexpr usize BITS      = sizeof(N) * 8;
        static constexpr usize DIGITS    = 1 + ((BITS - IS_SIGNED) * Math::INV_LOG10_2_MUL >> 16);

        bool AddOverflow(N a, N b, N& out) { return __builtin_add_overflow(a, b, &out); }
        bool SubOverflow(N a, N b, N& out) { return __builtin_sub_overflow(a, b, &out); }
        bool MulOverflow(N a, N b, N& out) { return __builtin_mul_overflow(a, b, &out); }

        u32  CountOnes (N x) { return std::popcount((UnsignedInt)x); }
        u32  CountZeros(N x) { return std::popcount((UnsignedInt)~(UnsignedInt)x); }
        bool IsPow2    (N x) { return x && !(x & (x - 1)); }
        u32  BitWidth  (N x) { return std::bit_width((UnsignedInt)x); }
        u32  Log2      (N x) { return BitWidth(x) - 1; }
        u32  Log10     (N x) {
            u32 approx10 = Log2(x) * Math::INV_LOG10_2_MUL >> 16;
            return approx10 - (x < Math::POWERS_OF_10[approx10]);
        }
    };


    enum class FpClassification {
        NAN     = FP_NAN,
        INF     = FP_INFINITE,
        ZERO    = FP_ZERO,
        SUBNORM = FP_SUBNORMAL,
        NORM    = FP_NORMAL,
    };

#define QUASI_DEFINE_FLOATING(FLOAT, CCODENAME, SAME_SIZED, SAME_SIZED_SIGNED) \
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
        using EquivalentInt = SAME_SIZED; \
        using EquivalentIntSigned = SAME_SIZED_SIGNED; \
        static constexpr SAME_SIZED SIGN_MASK     = 1 << (sizeof(FLOAT) * 8 - 1); \
        static constexpr SAME_SIZED MANTISSA_MASK = (1 << MANTISSA_BITS) - 1; \
        static constexpr SAME_SIZED EXPONENT_MASK = (1 << EXPONENT_MASK) - MANTISSA_MASK - 1; \
        \
        FLOAT FromBits(SAME_SIZED x) { return __builtin_bit_cast(FLOAT, x); } \
        SAME_SIZED BitsOf(FLOAT f)   { return __builtin_bit_cast(SAME_SIZED, f); } \
        bool IsSignedNegative(FLOAT f) { return BitsOf(f) & SIGN_MASK != 0; } \
        bool IsSignedPositive(FLOAT f) { return BitsOf(f) & SIGN_MASK == 0; } \
        FLOAT Sign(FLOAT f) { return f == 0 ? f : f > 0 ? 1 : f < 0 ? -1 : NAN; } \
        \
        void  Decomp(FLOAT f, int& exp, FLOAT& mant) { mant = std::frexp(f, &exp); } \
        FLOAT Comp(int exp, FLOAT mant) { return std::ldexp(mant, exp); } \
        FLOAT Log2(FLOAT x)  { return std::log2(x); } \
        int   Log2i(FLOAT x) { return std::ilogb(x); } \
        FLOAT Log10(FLOAT x) { return std::log10(x); } \
        FLOAT Exp2(FLOAT x)  { return std::exp2(x); } \
        FLOAT Exp2i(int x)   { return Comp(x, (FLOAT)1); } \
        FLOAT Exp10(FLOAT x) { return std::exp2(x / Math::LOG10_2); } \
        void  SeparateDecimal(FLOAT x, FLOAT& integer, FLOAT& decimal) { decimal = std::modf(x, &integer); } \
        \
        /* uses bit manipulating techniques to convert floats to ints 100% accurately for values in range [0, 2^23), or [0, 2^52) for doubles */ \
        SAME_SIZED FastToIntUnsigned(FLOAT f) { return BitsOf(f + FLOAT(1 << MANTISSA_BITS)) & MANTISSA_MASK; } \
        SAME_SIZED FloorToIntUnsigned(FLOAT f) { return BitsOf(f + (FLOAT(1 << MANTISSA_BITS) - FLOAT(0.5))) & MANTISSA_MASK; } \
        SAME_SIZED CeilToIntUnsigned(FLOAT f) { return BitsOf(f + (FLOAT(1 << MANTISSA_BITS) + FLOAT(0.5))) & MANTISSA_MASK; } \
        /* uses bit manipulating techniques to convert ints to floats 100% accurately for values in range [0, 2^23), or [0, 2^52) for doubles */ \
        FLOAT FastToFloatUnsigned(SAME_SIZED x) { return FromBits(x + __builtin_bit_cast(SAME_SIZED, (FLOAT)(1 << MANTISSA_BITS))) - (FLOAT)(1 << MANTISSA_BITS); } \
        \
        FLOAT FastFloor(FLOAT x) { return FastToFloatUnsigned(FastToIntUnsigned(x)); } \
        \
        FpClassification Classify(FLOAT f) { return (FpClassification)std::fpclassify(f); } \
        bool IsNaN(FLOAT f)     { return std::isnan(f); } \
        bool IsInf(FLOAT f)     { return f == INFINITY || f == NEG_INFINITY; } \
        bool IsFinite(FLOAT f)  { return std::abs(f) < INFINITY; } \
        bool IsSubnorm(FLOAT f) { return Classify(f) == FpClassification::SUBNORM; } \
        bool IsNorm(FLOAT f)    { return Classify(f) == FpClassification::NORM; } \
        \
        FLOAT ImmediateIncr(FLOAT f) { \
            const SAME_SIZED bits = BitsOf(f); \
            if (IsNaN(f) || bits == BitsOf(INFINITY)) return f; \
            return FromBits(bits + 1); \
        } \
        FLOAT ImmediateDecr(FLOAT f) { \
            const SAME_SIZED bits = BitsOf(f); \
            if (IsNaN(f) || bits == BitsOf(NEG_INFINITY)) return f; \
            return FromBits(bits - 1); \
        } \
        \
        FLOAT Max(FLOAT a, FLOAT b) { return std::max(a, b); } \
        FLOAT Min(FLOAT a, FLOAT b) { return std::min(a, b); } \
        FLOAT Clamp(FLOAT x, FLOAT min, FLOAT max) { return x < min ? min : x > max ? max : x; } \
        /* returns the max, except if either is nan the result is nan, and positive zero is greater than negative zero */ \
        FLOAT StrictMax(FLOAT a, FLOAT b) { \
            return a >  b ? a : \
                   b >  a ? b : \
                   a == b ? IsSignedPositive(a) && IsSignedNegative(b) ? a : b : \
                   a + b; \
        } \
        /* returns the min, except if either is nan the result is nan, and positive zero is less than negative zero */ \
        FLOAT StrictMin(FLOAT a, FLOAT b) { \
            return a >  b ? b : \
                   b >  a ? a : \
                   a == b ? IsSignedNegative(a) && IsSignedPositive(b) ? a : b : \
                   a + b; \
        } \
        \
        /* handles all cases, ordered by:
            -nan < -inf < -norm < -subnorm < -0 < 0 < subnorm < norm < inf < nan
        */ \
        int CompleteCmp(FLOAT a, FLOAT b) { \
            SAME_SIZED_SIGNED ax = (SAME_SIZED_SIGNED)BitsOf(a), bx = (SAME_SIZED_SIGNED)BitsOf(b); \
            ax ^= (SAME_SIZED_SIGNED)((SAME_SIZED)(ax >> (8 * sizeof(FLOAT) - 1)) >> 1); \
            bx ^= (SAME_SIZED_SIGNED)((SAME_SIZED)(bx >> (8 * sizeof(FLOAT) - 1)) >> 1); \
            return ax - bx; \
        } \
    } \
    \
    template <> struct NumInfo<FLOAT> { \
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
        static constexpr usize DIGITS       =  CCODENAME##_DIG; \
        \
        static constexpr usize BITS          = sizeof(FLOAT) * 8; \
        static constexpr usize EXPONENT_BITS = 5 + sizeof(FLOAT) / 4 * 3; \
        static constexpr usize MANTISSA_BITS = BITS - EXPONENT_BITS - 1; \
        \
        using EquivalentInt = SAME_SIZED; \
        static constexpr SAME_SIZED SIGN_MASK     = 1 << (sizeof(FLOAT) * 8 - 1); \
        static constexpr SAME_SIZED MANTISSA_MASK = (1 << MANTISSA_BITS) - 1; \
        static constexpr SAME_SIZED EXPONENT_MASK = (1 << EXPONENT_MASK) - MANTISSA_MASK - 1; \
    };

    QUASI_DEFINE_FLOATING(f32, FLT, u32, i32);
    QUASI_DEFINE_FLOATING(f64, DBL, u64, i64);

#undef QUASI_DEFINE_FLOATING

    namespace Math {
        constexpr struct NaNType {
            constexpr operator f32() const { return f32s::NAN; }
            constexpr operator f64() const { return f64s::NAN; }
            constexpr bool operator==(const NaNType&) const = default;
            constexpr bool operator==(f32 f) const { return std::isnan(f); }
            constexpr bool operator==(f64 f) const { return std::isnan(f); }
        } NaN;

        struct NegInfinityType;

        constexpr struct InfinityType {
            constexpr operator f32() const { return f32s::INFINITY; }
            constexpr operator f64() const { return f64s::INFINITY; }
            constexpr NegInfinityType operator-() const;
            constexpr bool operator==(const InfinityType&) const = default;
            constexpr bool operator==(f32 f) const { return std::isnan(f); }
            constexpr bool operator==(f64 f) const { return std::isnan(f); }
        } Infinity;

        constexpr struct NegInfinityType {
            constexpr operator f32() const { return f32s::NEG_INFINITY; }
            constexpr operator f64() const { return f64s::NEG_INFINITY; }
            constexpr InfinityType operator-() const { return Infinity; }
            constexpr bool operator==(const NegInfinityType&) const = default;
            constexpr bool operator==(f32 f) const { return f == f32s::NEG_INFINITY; }
            constexpr bool operator==(f64 f) const { return f == f64s::NEG_INFINITY; }
        } NegInfinity;

        constexpr NegInfinityType InfinityType::operator-() const { return NegInfinity; }
    }
}
