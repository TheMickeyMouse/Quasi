#pragma once
#include "Formatting.h"
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
    /// A Latin-1 / ISO/IEC 8859-1 character.
    enum Latin1 : u8 {};

    /// A thin string view wrapper for wider UTF string characters, similar to @p Str but for
    /// different UTF encodings.
    /// @tparam C The standard conforming character type (either @p char16_t or @p char32_t)
    template <class C> struct WStr {
        using UC = IfElse<SameAs<C, char16_t>, Utf16, IfElse<SameAs<C, char32_t>, Utf32, Empty>>;
    private:
        const C* data = nullptr;
        usize length = 0;

        constexpr WStr(const C* data, usize length) : data(data), length(length) {}
    public:
        WStr(Nullptr = nullptr) {}
        WStr(const C* data)    : data(data) { while (data[length]) ++length; }
        WStr(Span<const C> s)  : data(s.Data()), length(s.Length()) {}
        WStr(const UC* data)   : WStr((const C*)data) {}
        WStr(Span<const UC> s) : data((const C*)s.Data()), length(s.Length()) {}
        WStr(const wchar_t* s) requires (sizeof(wchar_t) == sizeof(C)) : WStr((const C*)s) {}

        constexpr static WStr Slice(const C* data, usize length)  { return { data, length }; }
        constexpr static WStr Slice(const UC* data, usize length) { return { data, length }; }

        const C* Data() const { return data; }
        const UC* DataUtf() const { return (const UC*)data; }
        usize Length() const { return length; }

        const UC& operator[](usize i) const { return ((const UC*)data)[i]; }
        explicit operator bool() const { return length; }

        WStr& Advance(usize n) { data += n; length -= n; return *this; }
    };

    /// A UTF-16 encoded string view
    using U16Str = WStr<char16_t>;
    /// A UTF-32 encoded string view
    using U32Str = WStr<char32_t>;

    WStr(const wchar_t*) -> WStr<IfElse<sizeof(wchar_t) == sizeof(Utf16), char16_t, char32_t>>;

    template <class UC> using UtfStrFor = IfElse<
        SameAs<UC, char>     || SameAs<UC, Utf8>,  Str,    IfElse<
        SameAs<UC, char16_t> || SameAs<UC, Utf16>, U16Str, IfElse<
        SameAs<UC, char32_t> || SameAs<UC, Utf32>, U32Str, Empty
    >>>;

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
    /// @param[out] x The destination UTF-8 string (<b>must have space to be able to write at least 4 characters!</b>)
    /// @param[out] n The amount of bytes written
    /// @return true if success, false if an ill-formed UTF-32 character occurred.
    bool TryUtf32CharTo8(Utf32 c, Utf8* x, u32& n);

    /// Converts and returns the first UTF-32 character in a UTF-16 string
    /// @param[in] chars The UTF-16 encoded string.
    /// @param[in] size The amount one can safely read into the input string. (use 2 if you are *sure* reading is safe)
    /// @param[out] x The output UTF-32 character.
    /// @param[out] n The amount of bytes consumed.
    /// @param bswap Whether the bytes from @p chars should be byte-swapped @a first before decoding.
    /// @return true if success, false if an ill-formed UTF-16 string occurred or couldn't read.
    bool TryUtf16CharTo32(const Utf16* chars, u32 size, Utf32& x, u32& n, bool bswap = false);

    /// Converts and writes a UTF-32 character into a UTF-16 code point
    /// @param[in] c The input UTF-32 character to be converted
    /// @param[out] x The destination of the UTF-16 string (<b>must have space to be able to write at least 2 characters!</b>)
    /// @param[out] n The amount of bytes written
    /// @return true if success, false if an ill-formed UTF-32 character occurred.
    bool TryUtf32CharTo16(Utf32 c, Utf16* x, u32& n);

    /// Converts and writes the first UTF-8 code point in a UTF-16 string
    /// @param[in]  in      The input UTF-16 encoded string.
    /// @param[in]  inSize  The amount one can safely read into the input string. (use 2 if you are *sure* reading is safe)
    /// @param[out] inNum   The amount of bytes consumed from the input.
    /// @param[out] out     The destination for the UTF-8 code point. (<b>must have space to be able to write at least 4 characters!</b>)
    /// @param[out] outNum  The amount of bytes written to the output.
    /// @return true if success, false if an ill-formed UTF-16 string occurred or couldn't read.
    bool TryUtf16CharTo8(const Utf16* in, u32 inSize, u32& inNum, Utf8* out, u32& outNum);

    /// Converts and writes the first UTF-16 code point in a UTF-8 string
    /// @param[in]  in      The input UTF-8 encoded string.
    /// @param[in]  inSize  The amount one can safely read into the input string. (use 4 if you are *sure* reading is safe)
    /// @param[out] inNum   The amount of bytes consumed from the input.
    /// @param[out] out     The destination for the UTF-16 code point. (<b>must have space to be able to write at least 2 characters!</b>)
    /// @param[out] outNum  The amount of bytes written to the output.
    /// @return true if success, false if an ill-formed UTF-8 string occurred or couldn't read.
    bool TryUtf8CharTo16(const Utf8* in, u32 inSize, u32& inNum, Utf16* out, u32& outNum);

    /// Converts and writes a Latin-1 character into a UTF-8 code point
    /// @param[in] c The input Latin-1 character to be converted
    /// @param[out] x The destination UTF-8 string (<b>must have space to be able to write at least 2 characters!</b>)
    /// @param[out] n The amount of bytes written
    void TryLatin1CharToUtf8(Latin1 c, Utf8* x, u32& n);

    /// Reads the first code point in @p From and writes it to a buffer encoded in @p To .
    /// @param[in]  in      The input string.
    /// @param[in]  inSize  The amount one can safely read into the input string. (use @p MAX_CODEPOINT_LENGTH if you are *sure* reading is safe)
    /// @param[out] inNum   The amount of bytes consumed from the input.
    /// @param[out] out     The destination for the UTF-16 code point. (<b>must have space to be able to write at least 2 characters!</b>)
    /// @param[out] outNum  The amount of bytes written to the output.
    /// @note Always outputs an entire code point; even if the conversion to and from types are equivalent.
    /// @return true if success, false if an ill-formed UTF-8 string occurred or couldn't read or write
    template <class From, class To>
    bool TryUtfConv(const From* in, u32 inSize, u32& inNum, To* out, u32& outNum) {
        static_assert(SameAs<From, Utf8> || SameAs<From, Utf16> || SameAs<From, Utf32>, "bad input character type!");
        static_assert(SameAs<To, Utf8> || SameAs<To, Utf16> || SameAs<To, Utf32>, "bad output character type!");

        if constexpr (SameAs<From, Utf8>) {
            if      constexpr (SameAs<To, Utf16>) return TryUtf8CharTo16(in, inSize, inNum, out, outNum);
            else if constexpr (SameAs<To, Utf32>) return outNum = 1, TryUtf8CharTo32(in, inSize, *out, inNum);
            else { // 8 -> 8; manual,
                const u32 n = Utf8CodeLen(*in);
                if (inSize < n) return false;
                Memory::MemCopy(out, in, n);
                inNum = outNum = n;
                return true;
            }
        } else if constexpr (SameAs<From, Utf16>) {
            if      constexpr (SameAs<To, Utf8>)  return TryUtf16CharTo8(in, inSize, inNum, out, outNum);
            else if constexpr (SameAs<To, Utf16>) return outNum = 1, TryUtf16CharTo32(in, inSize, *out, inNum);
            else { // 16 -> 16; manual,
                if (IsHighSurrogateUtf16(*in)) {
                    *out = *in; inNum = outNum = 1;
                    return true;
                } else if (inSize >= 2) {
                    out[0] = in[0]; out[1] = in[1]; inNum = outNum = 2;
                    return true;
                } else return false;
            }
        } else {
            if      constexpr (SameAs<To, Utf8>)  return TryUtf32CharTo8(*in, out, outNum);
            else if constexpr (SameAs<To, Utf16>) return TryUtf32CharTo16(*in, out, outNum);
            else { *out = *in; inNum = outNum = 1; return true; }
        }
    }

    /// Converts a UTF-32 string into a UTF-8 string.
    /// @param s32 a UTF-32 encoded string
    /// @return a re-encoded UTF-8 string
    Option<String> Utf32To8(U32Str s32);

    /// Converts a UTF-8 string into a UTF-32 string.
    /// @param s8 a UTF-8 encoded string
    /// @return a re-encoded UTF-32 string
    Option<Vec<Utf32>> Utf8To32(Str s8);

    /// Converts a UTF-16 string into a UTF-8 string.
    /// @param s16 a UTF-16 encoded string
    /// @return a re-encoded UTF-8 string
    Option<String> Utf16To8(U16Str s16);

    /// Converts a UTF-16 string into a UTF-8 string, byteswapping every 2 bytes to account for endianness.
    /// @param s16 a UTF-16 encoded string that needs to be byte-swapped.
    /// @return a re-encoded UTF-8 string
    Option<String> Utf16BswapTo8(U16Str s16);

    /// Converts a UTF-8 string into a UTF-16 string.
    /// @param s8 a UTF-8 encoded string
    /// @return a re-encoded UTF-16 string
    Option<Vec<Utf16>> Utf8To16(Str s8);

    /// Converts a UTF-16 string into a UTF-32 string.
    /// @param s16 a UTF-16 encoded string
    /// @return a re-encoded UTF-32 string
    Option<Vec<Utf32>> Utf16To32(U16Str s16);

    /// Converts a UTF-32 string into a UTF-16 string.
    /// @param s32 a UTF-32 encoded string
    /// @return a re-encoded UTF-16 string
    Vec<Utf16> Utf32To16(U32Str s32);

    String Latin1ToUtf8(Span<const Latin1> sLat);

    /// The maximum length of a codepoint in the encoding @p C.
    /// @note The max number of chars follows this table:
    /// @code
    /// | 8   | 16  | 32  |
    /// | --- | --- | --- |
    /// | 4   | 2   | 1   |
    /// @endcode
    template <class C> static constexpr u32 MAX_CODEPOINT_LENGTH =
        sizeof(C) == sizeof(Utf32) ? 1 :
        sizeof(C) == sizeof(Utf16) ? 2 :
        sizeof(C) == sizeof(Utf8)  ? 4 : -1;

    /// An iterator that goes through the @em codepoints of a string,
    /// unlike traditional string iterators which iterate through @em characters, which confuse
    /// unicode code units with actual characters.
    ///
    /// Takes in a string encoded with char type @p From, and yields out the code points in the form
    /// of characters of type @p To.
    /// If a bad character is encountered, the iterator terminates and returns an empty span for that character.
    /// Reaching the end of the string will also terminate the iterator.
    /// @tparam From The input string encoding
    /// @tparam To The output characters encoding
    template <class From, class To>
    struct CodepointIter : IIterator<const Span<const To>, CodepointIter<From, To>> {
        using Item = const Span<const To>;
        using FromStr = UtfStrFor<From>;
        friend IIterator<Item, CodepointIter>;
    private:
        FromStr text;
        To chars[MAX_CODEPOINT_LENGTH<To>] = {};
        u32 codelen = 0;
        CodepointIter(FromStr s) : text(s) { AdvanceImpl(); }
    public:
        static CodepointIter New(FromStr text) { return { text }; }
    protected:
        Item CurrentImpl() const {
            return Spans::Slice(chars, codelen);
        }

        void AdvanceImpl() {
            u32 readLen;
            if (!TryUtfConv((const From*)text.Data(), text.Length(), readLen, chars, codelen)) {
                codelen = 0;
                return;
            }
            text.Advance(readLen);
        }

        bool CanNextImpl() const {
            return text && codelen;
        }
    };

    using Utf8To32Iter = CodepointIter<Utf8, Utf32>;

    template <> struct Formatter<U16Str> {
        using FormatOptions = Empty;

        static usize FormatTo(StringWriter sw, U16Str input, const FormatOptions&);
    };
}

namespace Quasi {
    constexpr Text::U16Str operator""_str(const char16_t* data, usize length) {
        return Text::U16Str::Slice(data, length);
    }

    constexpr Text::U32Str operator""_str(const char32_t* data, usize length) {
        return Text::U32Str::Slice(data, length);
    }
}