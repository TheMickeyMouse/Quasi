#pragma once
#include "Utils/Numeric.h"
#include "Formatting.h"
#include "Parsing.h"

namespace Quasi::Text {
    struct IntParseOptions;
    struct IntFormatOptions;
    struct FloatFormatOptions;

    namespace NumberConversion {
        // different from plain addition, bytes do not overflow to other bytes
        static u32 Add4Bytes(u32 a, u32 b);

        // checks if all 8 bytes in digits is a valid digit character (0-9)
        static bool AreAllInCharRange8(u64 digits, uchar min, uchar max);
        static bool AreAllInCharRange4(u32 digits, uchar min, uchar max);
        static bool AreAllDigits4(u32 digits);
        static bool AreAllDigitsRadix4(u32 digits, u32 radix);
        static bool AreAllDigits8(u64 digits);
        static bool AreAllHexDigits4(u32 digits);
        static bool AreAllHexDigitsRadix4(u32 digits, u32 radix);

        static u32 ConvertDigits4(u32 chars);
        static u64 ConvertDigits8(u64 chars);
        static u32 ConvertHexDigits4(u32 chars);
        static u64 ParseDigits8(u64 digits);
        static u32 ParseDigits4(u32 digits);
        static u32 ParseHexDigits4(u32 xdigits);

        // base 2-4, 4 digits can fit in 1 byte
        static u32 ParseDigitsTinyRadix4(u32 dig, u32 radix);
        // base 5-16, 4 digits can fit in 2 bytes
        static u32 ParseDigitsSmallRadix4(u32 dig, u32 radix);
        // base 17+, 4 digits can fit in a u32
        static u32 ParseDigitsLargeRadix4(u32 dig, u32 radix);

        template <class N, usize ByteParallelCount>
        OptionUsize ParseIntBy(Str string, Out<N&> out, bool neg, const Fn<bool, N&, const char*> auto& acc, u32 maxDigs, bool exactDigs, u32 radix) {
            string = string.TrimStart('0');

            if (string.Length() > maxDigs) return nullptr;
            N num = 0;
            if (!exactDigs && string.Length() == maxDigs) {
                if (const auto i = ParseIntBy<N, ByteParallelCount>(string.Init(), num, neg, acc, maxDigs - 1, true, radix); i == string.Length() - 1) {
                    if (const Option d = Chr::TryToDigitRadix(string.Last(), radix))
                        if (NumInfo<N>::MulOverflow(num, radix, num) &&
                            (neg ? NumInfo<N>::SubOverflow(num, *d, num) : NumInfo<N>::AddOverflow(num, *d, num))) {
                            out = num;
                            return string.Length();
                        } else return nullptr;
                    else return i;
                } else return i;
            }

            if (!neg) {
                usize i = 0;
                for (; i < string.Length() & -ByteParallelCount; i += ByteParallelCount) {
                    if (!acc(num, string.Data() + i)) break;
                }
                for (; i < string.Length(); ++i) {
                    if (const Option d = Chr::TryToDigitRadix(string[i], radix)) {
                        num = num * radix + *d;
                    } else { out = num; return i; }
                }
            } else {
                usize i = 0;
                for (; i < string.Length() & -ByteParallelCount; i += ByteParallelCount) {
                    if (!acc(num, string.Data() + i)) break;
                }
                num = -num;
                for (; i < string.Length(); ++i) {
                    if (const Option d = Chr::TryToDigitRadix(string[i], radix)) {
                        num = num * radix - *d;
                    } else { out = num; return i; }
                }
            }
            out = num;
            return string.Length();
        }

        template <Integer I>
        OptionUsize ParseBinaryInt(Str string, Out<I&> out, bool neg);
        template <Integer I>
        OptionUsize ParseDecimalInt(Str string, Out<I&> out, bool neg);
        template <Integer I>
        OptionUsize ParseHexInt(Str string, Out<I&> out, bool neg);
        template <Integer I>
        OptionUsize ParseTinyRadixInt(Str string, Out<I&> out, bool neg, u32 radix);
        template <Integer I>
        OptionUsize ParseSmallRadixInt(Str string, Out<I&> out, bool neg, u32 radix);
        template <Integer I>
        OptionUsize ParseAsciiRadixInt(Str string, Out<I&> out, bool neg, u32 radix);
        template <Integer I>
        OptionUsize ParseLargeRadixInt(Str string, Out<I&> out, bool neg, u32 radix);
        template <Integer I>
        OptionUsize ParseInt(Str string, Out<I&> out, IntParseOptions options);

        template <Floating F>
        OptionUsize ParseNanOrInf(Str string, Out<F&> out);

        // formatting functions
        u64 U64ToBCD2(u64 x);
        u64 U64ToBCD4(u64 x);
        u64 U64ToBCD8(u64 x);

        u32 SmallU64ToString(u64 x, char* out);
		u32 U64ToString     (u64 x, char* out);
		u32 U64FullToString (u64 x, char* out);
        usize FormatU64(StringWriter sw, u64 num, const IntFormatOptions& options, char sign = '\0');
        usize FormatI64(StringWriter sw, i64 num, const IntFormatOptions& options);

        void WriteU64Decimal(StringWriter sw, u64 num);
        void WriteU64Binary (StringWriter sw, u64 num, u32 bitwidth);
        void WriteU64Octal  (StringWriter sw, u64 num, u32 octalDigits);
        void WriteU64Hex    (StringWriter sw, u64 num, u32 hexDigits, bool upperCase);

        char* AddSign(f64 f, char* out, bool alwaysShowSign = false);
        char* WriteFltDecimal(f64 f, char* out, u32 precision = ~0);
        u32 WriteFltSci(f64 f, char* out, u32 precision = ~0, char e = 'e', int log10 = i32s::MIN);
        u32 WriteFltFxd(f64 f, char* out, u32 width = 0, u32 precision = ~0, int log10 = i32s::MIN, char pad = ' ');
        u32 WriteFltGen(f64 f, char* out, u32 width = 0, u32 precision = ~0, char e = 'e');

        usize FormatFltSci(StringWriter sw, f64 f, const FloatFormatOptions& options);
        usize FormatFltFxd(StringWriter sw, f64 f, const FloatFormatOptions& options);
        usize FormatFloating(StringWriter sw, f64 f, const FloatFormatOptions& options);
    }

#pragma region Format
    template <> struct Formatter<bool> : Formatter<Str> {
        using FormatOptions = TextFormatOptions;
        static usize FormatTo(StringWriter sw, bool input, const FormatOptions& options);
    };

    // :((?'pad'.?)(?'align'[<^>])(?'totalLen'[0-9]+)\,)?(?'showSign'\+?)(?'shouldPadZero'0?)(?'numLen'[0-9]*)(?'base'[dboxX])
    struct IntFormatOptions {
        u32 numLen = 0;
        u32 totalLength = 0;

        TextFormatOptions::Alignment alignment = TextFormatOptions::LEFT;
        char pad = ' ';
        bool showSign = false, shouldPadZero = false;
        enum Base { DECIMAL, BINARY, OCTAL, HEX, CAP_HEX } base = DECIMAL;

        static IntFormatOptions Configure(Str opt);
    };

    template <Integer N>
    struct Formatter<N> {
        using FormatOptions = IntFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return IntFormatOptions::Configure(opt); }
        static usize FormatTo(StringWriter sw, N num, const FormatOptions& options);
    };

    // ((?'pad'.?)(?'align'[<^>])(?'totalLen'[0-9]+)\,)?(?'showSign'\+?)(?'shouldPadZero'0?)(?'width'[0-9]*)\.(?'precision'[0-9]*)(?'mode'[feEgG%])
    struct FloatFormatOptions {
        u32 width = 6, precision = ~0;
        u32 totalLength = 0;

        TextFormatOptions::Alignment alignment = TextFormatOptions::LEFT;
        char pad = ' ';
        bool showSign = false, shouldPadZero = false;
        enum Mode { SCIENTIFIC, FIXED, GENERAL, SCI_CAP, GEN_CAP, PERCENTAGE } mode = FIXED;

        static FloatFormatOptions Configure(Str opt);
    };

    template <Floating N>
    struct Formatter<N> {
        using FormatOptions = FloatFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return FloatFormatOptions::Configure(opt); }
        static usize FormatTo(StringWriter sw, N num, const FormatOptions& options) {
            return NumberConversion::FormatFloating(sw, (f64)num, options);
        }
    };
#pragma endregion

#pragma region Parse
    struct BooleanParseOptions {
        enum {
            ALLOW_NUMERIC = 1, USE_WORDS = 2, GENERAL = ALLOW_NUMERIC | USE_WORDS
        } format = GENERAL;
    };

    template <> struct Parser<bool> {
        using ParseOptions = BooleanParseOptions;
        static OptionUsize ParseUntil(Str string, Out<bool&> out, ParseOptions options);
    };

    struct IntParseOptions {
        enum {
            ADAPTIVE = 0,
            BASE_N   = 0,
            BASE_2   = 2,  BINARY   = 2,
            BASE_3   = 3,  BASE_4   = 4,  BASE_5   = 5,  BASE_6   = 6,  BASE_7   = 7,
            BASE_8   = 8,  OCTAL    = 8,
            BASE_9   = 9,
            BASE_10  = 10, DECIMAL  = 10,
            BASE_11  = 11, BASE_12  = 12, BASE_13  = 13, BASE_14  = 14, BASE_15  = 15,
            BASE_16  = 16, HEX      = 16,
            BASE_17  = 17, BASE_18  = 18, BASE_19  = 19, BASE_20  = 20,
            BASE_21  = 21, BASE_22  = 22, BASE_23  = 23, BASE_24  = 24,
            BASE_25  = 25, BASE_26  = 26, BASE_27  = 27, BASE_28  = 28,
            BASE_29  = 29, BASE_30  = 30, BASE_31  = 31, BASE_32  = 32,
            BASE_33  = 33, BASE_34  = 34, BASE_35  = 35, BASE_36  = 36,
        } radix = DECIMAL;
    };

    template <Integer N>
    struct Parser<N> {
        using ParseOptions = IntParseOptions;
        static OptionUsize ParseUntil(Str string, Out<N&> out, ParseOptions options);
    };

    struct FloatParseOptions {
        enum {
            SCIENTIFIC = 1, FIXED = 2, GENERAL = SCIENTIFIC | FIXED,
        } format = GENERAL;
    };

    template <Floating N>
    struct Parser<N> {
        using ParseOptions = FloatParseOptions;
        static OptionUsize ParseUntil(Str string, Out<N&> out, ParseOptions options);
        static usize ParseInteger(Str string, Out<N&> out);
        static usize ParseDecimal(Str string, InOut<N&> out);
        static OptionUsize ParseExponent(Str string, InOut<N&> out);
    };
#pragma endregion
}
