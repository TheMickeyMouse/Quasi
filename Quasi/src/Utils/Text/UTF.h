#pragma once
#include "Utils/Numeric.h"
#include "Utils/Option.h"
#include "Utils/String.h"

namespace Quasi {
    struct String;
}

namespace Quasi::Text {
    /// A UTF-32 character.
    using Utf32 = u32;
    /// A UTF-16 character.
    using Utf16 = u16;
    /// A UTF-8 character.
    using Utf8  = u8;
    // A Latin-1 / ISO/IEC 8859-1 character.
    enum Latin1 : u8 {};

    /// Determines the code point length given the first character in a sequence.
    /// @param firstByte the first byte of a code point sequence
    /// @return The number of bytes that the code point contains (including the first)
    u32 Utf8CodeLen(Utf8 firstByte);

    /// Determines a UTF-32 character's code point length if it were converted to UTF-8.
    /// @param x the UTF-32 character to test
    /// @return The number of bytes of the code point that character will be encoded as
    u32 Utf32CodeLen(Utf32 x);

    /// Determines whether a UTF-16 character is the 'high surrogate' code unit in a surrogate pair.
    bool IsHighSurrogateUtf16(Utf16 x);

    /// Converts and returns the first UTF-32 character in a UTF-8 string
    /// @param[in] chars The UTF-8 encoded string.
    /// @param[in] size The amount one can safely read into the input string. (use 4 if you are *sure* reading is safe)
    /// @param[out] x The output UTF-32 character.
    /// @param[out] n The amount of bytes consumed.
    /// @return true if success, false if an ill-formed UTF-8 string occurred or couldn't read.
    bool TryUtf8CharTo32(const Utf8* chars, u32 size, Utf32& x, u32& n);

    /// Converts and writes a UTF-32 character into a UTF-8 code point
    /// @param[in] c The input UTF-32 character to be converted
    /// @param[out] x The destination UTF-8 string
    /// @param[in] size The amount of bytes that is safe to write to the destination string (use 4 if you are *sure* writing is safe)
    /// @param[out] n The amount of bytes written
    /// @return true if success, false if an ill-formed UTF-32 character occurred or couldn't write.
    bool TryUtf32CharTo8(Utf32 c, Utf8* x, u32 size, u32& n);

    /// Converts and returns the first UTF-32 character in a UTF-16 string
    /// @param[in] chars The UTF-16 encoded string.
    /// @param[in] size The amount one can safely read into the input string. (use 2 if you are *sure* reading is safe)
    /// @param[out] x The output UTF-32 character.
    /// @param[out] n The amount of bytes consumed.
    /// @return true if success, false if an ill-formed UTF-16 string occurred or couldn't read.
    bool TryUtf16CharTo32(const Utf16* chars, u32 size, Utf32& x, u32& n);

    /// Converts and writes a UTF-32 character into a UTF-16 code point
    /// @param[in] c The input UTF-32 character to be converted
    /// @param[out] x The destination of the UTF-16 string
    /// @param[in] size The amount of bytes that is safe to write to the destination string (use 2 if you are *sure* writing is safe)
    /// @param[out] n The amount of bytes written
    /// @return true if success, false if an ill-formed UTF-32 character occurred or couldn't write.
    bool TryUtf32CharTo16(Utf32 c, Utf16* x, u32 size, u32& n);

    /// Converts and writes the first UTF-8 code point in a UTF-16 string
    /// @param[in]  in      The input UTF-16 encoded string.
    /// @param[in]  inSize  The amount one can safely read into the input string. (use 2 if you are *sure* reading is safe)
    /// @param[out] inNum   The amount of bytes consumed from the input.
    /// @param[out] out     The destination for the UTF-8 code point.
    /// @param[in]  outSize The amount of bytes that is safe to write to the destination string (use 4 if you are *sure* writing is safe)
    /// @param[out] outNum  The amount of bytes written to the output.
    /// @return true if success, false if an ill-formed UTF-16 string occurred or couldn't read or write
    bool TryUtf16CharTo8(const Utf16* in, u32 inSize, u32& inNum, Utf8* out, u32 outSize, u32& outNum);

    /// Converts and writes the first UTF-16 code point in a UTF-8 string
    /// @param[in]  in      The input UTF-8 encoded string.
    /// @param[in]  inSize  The amount one can safely read into the input string. (use 4 if you are *sure* reading is safe)
    /// @param[out] inNum   The amount of bytes consumed from the input.
    /// @param[out] out     The destination for the UTF-16 code point.
    /// @param[in]  outSize The amount of bytes that is safe to write to the destination string (use 2 if you are *sure* writing is safe)
    /// @param[out] outNum  The amount of bytes written to the output.
    /// @return true if success, false if an ill-formed UTF-8 string occurred or couldn't read or write
    bool TryUtf8CharTo16(const Utf8* in, u32 inSize, u32& inNum, Utf16* out, u32 outSize, u32& outNum);

    /// Converts and writes a Latin-1 character into a UTF-8 code point
    /// @param[in] c The input Latin-1 character to be converted
    /// @param[out] x The destination UTF-8 string
    /// @param[in] size The amount of bytes that is safe to write to the destination string (use 2 if you are *sure* writing is safe)
    /// @param[out] n The amount of bytes written
    /// @return true if success, false if an ill-formed Latin-1 character occurred or couldn't write.
    bool TryLatin1CharToUtf8(Latin1 c, Utf8* x, u32 size, u32& n);

    /// Converts a UTF-32 string into a UTF-8 string.
    /// @param s32 a UTF-32 encoded string
    /// @return a re-encoded UTF-8 string
    Option<String> Utf32To8(Span<const Utf32> s32);

    /// Converts a UTF-8 string into a UTF-32 string.
    /// @param s8 a UTF-8 encoded string
    /// @return a re-encoded UTF-32 string
    Option<Vec<Utf32>> Utf8To32(Str s8);

    /// Converts a UTF-16 string into a UTF-8 string.
    /// @param s16 a UTF-16 encoded string
    /// @return a re-encoded UTF-8 string
    Option<String> Utf16To8(Span<const Utf16> s16);

    /// Converts a UTF-8 string into a UTF-16 string.
    /// @param s8 a UTF-8 encoded string
    /// @return a re-encoded UTF-16 string
    Option<Vec<Utf16>> Utf8To16(Str s8);

    /// Converts a UTF-16 string into a UTF-32 string.
    /// @param s16 a UTF-16 encoded string
    /// @return a re-encoded UTF-32 string
    Option<Vec<Utf32>> Utf16To32(Span<const Utf16> s16);

    /// Converts a UTF-32 string into a UTF-16 string.
    /// @param s32 a UTF-32 encoded string
    /// @return a re-encoded UTF-16 string
    Vec<Utf16> Utf32To16(Span<const Utf32> s32);

    String Latin1ToUtf8(Span<const Latin1> sLat);
}
