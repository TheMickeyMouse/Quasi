#include "UTF.h"
#include "Utils/Memory.h"

namespace Quasi::Text {
    u32 Utf8CodeLen(Utf8 firstByte) {
        // UTF8:
        // 0yyyzzzz                            (1) -> 0x00000 ~ 0x00007F
        // 110xxxyy 10yyzzzz                   (2) -> 0x00080 ~ 0x0007FF
        // 1110wwww 10xxxxyy 10yyzzzz          (3) -> 0x00800 ~ 0x00FFFF
        // 11110uvv 10vvwwww 10xxxxyy 10yyzzzz (4) -> 0x10000 ~ 0x10FFFF
        // error: 0
        if ((firstByte & 0x80) == 0)    return 1;
        if ((firstByte & 0xE0) == 0xC0) return 2;
        if ((firstByte & 0xF0) == 0xE0) return 3;
        if ((firstByte & 0xF8) == 0xF0) return 4;
        return 0;
    }

    u32 Utf32CodeLen(Utf32 x) {
        if (x < 0x80) return 1;
        if (x < 0x800) return 2;
        if (x < 0x10000) return 3;
        if (x < 0x11000) return 4;
        return 0;
    }

    bool IsHighSurrogateUtf16(Utf16 x) {
        return (x & 0xFC'00) == 0xD8'00;
    }

    bool TryUtf8CharTo32(const Utf8* chars, u32 size, Utf32& x, u32& n) {
        if (size < 1) return false;
        n = Utf8CodeLen(*chars);
        if (size < n) return false;
        switch (n) {
            default: return false;
            case 1: x = chars[0]; return true;
            case 2: {
                // b2 = 10yyzzzz 110xxxyy
                const u32 b2 = Memory::ReadU16(chars);
                // want: xxx'yyyy'zzzz
                x = ((b2 & 0x1F) << 6) | ((b2 & 0x3F'00) >> 8);
                return (b2 & 0xC0'00) == 0x80'00;
            }
            case 3: {
                // b3 = 00000000 10yyzzzz 10xxxxyy 1110wwww
                const u32 b3 = Memory::ReadU16(chars) | ((u32)chars[2] << 16);
                // x  = 00000000 wwwwxxxx yy'yyzzzz
                x = ((b3 & 0x3F'00'00) >> 16) | ((b3 & 0x3F'00) >> 2) | ((b3 & 0x0F) << 12);
                return (b3 & 0xC0'C0'00) == 0x80'80'00;
            }
            case 4: {
                // b4 = 10yyzzzz 10xxxxyy 10vvwwww 11110uvv
                const u32 b4 = Memory::ReadU32(chars);
                // x  = 00000000 000uvvvv wwwwxxxx yy'yyzzzz
                x = ((b4 & 0x3F'00'00'00) >> 24) | ((b4 & 0x3F'00'00) >> 10)
                  | ((b4 & 0x3F'00)       << 4)  | ((b4 & 0x07)       << 18);
                return (b4 & 0xC0'C0'C0'00) == 0x80'80'80'00;
            }
        }
    }

    bool TryUtf32CharTo8(Utf32 c, Utf8* x, u32 size, u32& n) {
        n = Utf32CodeLen(c);
        if (size < n) return false;
        switch (n) {
            default: return false;
            case 1: x[0] = c; return true;
            case 2: {
                // xxxyy;yyzzzz -> 10yyzzzz 110xxxyy
                u32 b2 = (c >> 6) | (c << 8);
                b2 &= 0x3F'1F;
                b2 |= 0x80'C0;
                Memory::WriteU16((u16)b2, x);
                return true;
            }
            case 3: {
                // wwww;xxxxyy;yyzzzz -> 10yyzzzz 10xxxxyy 1110wwww
                u32 b3 = (c >> 12) | ((c & 0x0F'C0) << 2) | (c << 16);
                b3 &= 0x3F'3F'0F;
                b3 |= 0x80'80'E0;
                x[0] = b3 & 0xFF;
                x[1] = (b3 >> 8) & 0xFF;
                x[2] = (b3 >> 16) & 0xFF;
                return true;
            }
            case 4: {
                // uvv;vvwwww;xxxxyy;yyzzzz -> 10yyzzzz 10xxxxyy 10vvwwww 11110uvv
                u32 b4 = (c >> 18) | ((c & 0x3F000) >> 4) | ((c & 0xFC0) << 10) | (c << 24);
                b4 &= 0x3F'3F'3F'07;
                b4 |= 0x80'80'80'F0;
                Memory::WriteU32(b4, x);
                return true;
            }
        }
    }

    bool TryUtf16CharTo32(const Utf16* chars, u32 size, Utf32& x, u32& n) {
        if (size < 1) return false;
        if (IsHighSurrogateUtf16(*chars)) {
            n = 2;
            if (size < 2) return false;
            // b4 = xxxxxxxx 110111xx yyyyyyyy 110110yy (if system is BE)
            // b4 = 110111xx xxxxxxxx 110110yy yyyyyyyy (if system is LE)
            u32 b4 = Memory::ReadU32(chars);
            if constexpr (Memory::IsBigEndian()) {
                b4 = ((b4 >> 8) & 0x00FF00FF) | ((b4 << 8) & 0xFF00FF00);
            }
            x = ((b4 & 0x3FF) << 10 | (b4 & 0x3FF'0000) >> 16) + 0x10000;
            return (b4 & 0xFC00'0000) == 0xDC00'0000;
        } else {
            n = 1;
            x = chars[0];
            return true;
        }
    }

    bool TryUtf32CharTo16(Utf32 c, Utf16* x, u32 size, u32& n) {
        if (c >= 0x10000) {
            c -= 0x10000;
            n = 2;
            if (size < 2) return false;
            x[0] = 0xD800 | (c >> 10);
            x[1] = 0xDC00 | (c & 0x3FF);
        } else {
            n = 1;
            if (size < 1) return false;
            x[0] = c;
        }
        return true;
    }

    bool TryUtf16CharTo8(const Utf16* in, u32 inSize, u32& inNum, Utf8* out, u32 outSize, u32& outNum) {
        Utf32 c32;
        if (!TryUtf16CharTo32(in, inSize, c32, inNum)) return false;
        if (!TryUtf32CharTo8 (c32, out, outSize, outNum)) return false;
        return true;
    }

    bool TryUtf8CharTo16(const Utf8* in, u32 inSize, u32& inNum, Utf16* out, u32 outSize, u32& outNum) {
        Utf32 c32;
        if (!TryUtf8CharTo32 (in, inSize, c32, inNum)) return false;
        if (!TryUtf32CharTo16(c32, out, outSize, outNum)) return false;
        return true;
    }

    bool TryLatin1CharToUtf8(Latin1 c, Utf8* x, u32 size, u32& n) {
        if (c > 0x7F) { // 8-bit is always set
            n = 2;
            if (size < 2) return false;
            x[0] = 0xC0 | (c >> 6);
            x[1] = c & 0xBF;
        } else {
            n = 1;
            if (size < 1) return false;
            x[0] = c;
        }
        return true;
    }

    Option<String> Utf32To8(Span<const Utf32> s32) {
        String s8;
        for (usize i = 0; i < s32.Length(); i++) {
            u32 n;
            char codepoint[4];
            if (!TryUtf32CharTo8(s32[i], (Utf8*)codepoint, 4, n))
                return nullptr;
            s8.AppendStr(Str::Slice(codepoint, n));
        }
        return s8;
    }

    Option<Vec<Utf32>> Utf8To32(Str s8) {
        Vec<Utf32> s32;
        for (usize i = 0; i < s8.Length();) {
            u32 n;
            Utf32 char32;
            if (TryUtf8CharTo32((Utf8*)&s8[i], s8.Length() - i, char32, n))
                return nullptr;
            s32.Push(char32);
            i += n;
        }
        return s32;
    }

    Option<String> Utf16To8(Span<const Utf16> s16) {
        String s8;
        for (usize i = 0; i < s16.Length();) {
            u32 n, m;
            char codepoint[4];
            if (!TryUtf16CharTo8(&s16[i], s16.Length() - i, m, (Utf8*)codepoint, 4, n))
                return nullptr;
            s8.AppendStr(Str::Slice(codepoint, n));
            i += m;
        }
        return s8;
    }

    Option<Vec<Utf16>> Utf8To16(Str s8) {
        Vec<Utf16> s16;
        for (usize i = 0; i < s8.Length();) {
            u32 n, m;
            Utf16 codepoint[2];
            if (TryUtf8CharTo16((Utf8*)&s8[i], s8.Length() - i, m, codepoint, 2, n))
                return nullptr;
            s16.Extend(Spans::Slice(codepoint, n));
            i += m;
        }
        return s16;
    }

    Option<Vec<Utf32>> Utf16To32(Span<const Utf16> s16) {
        Vec<Utf32> s32;
        for (usize i = 0; i < s16.Length();) {
            u32 n;
            Utf32 char32;
            if (TryUtf16CharTo32(&s16[i], s16.Length() - i, char32, n))
                return nullptr;
            s32.Push(char32);
            i += n;
        }
        return s32;
    }

    Vec<Utf16> Utf32To16(Span<const Utf32> s32) {
        Vec<Utf16> s16;
        for (usize i = 0; i < s32.Length(); i++) {
            u32 n;
            Utf16 codepoint[2];
            TryUtf32CharTo16(s32[i], codepoint, 2, n);
            s16.Extend(Spans::Slice(codepoint, n));
        }
        return s16;
    }

    String Latin1ToUtf8(Span<const Latin1> sLat) {
        String s8;
        for (usize i = 0; i < sLat.Length(); i++) {
            u32 n;
            char codepoint[2];
            TryLatin1CharToUtf8(sLat[i], (Utf8*)codepoint, 2, n);
            s8.AppendStr(Str::Slice(codepoint, n));
        }
        return s8;
    }
}
