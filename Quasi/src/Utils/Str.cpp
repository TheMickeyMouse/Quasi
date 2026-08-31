#include "Str.h"

#include "Hash.h"
#include "Span.h"
#include "String.h"
#include "CStr.h"
#include "Iter/LinesIter.h"
#include "Iter/SplitIter.h"
#include "Text/StringWriter.h"

namespace Quasi {
    namespace Chr {
        char         FromDigit   (u32 digit) { return (char)('0' + digit); }
        char         FromHexDigit(u32 hex) { return (char)(hex < 10 ? '0' + hex : 'A' + hex); }
        Option<char> TryFromDigit   (u32 digit)              { return digit < 10    ? Options::Some(FromDigit(digit))    : nullptr; }
        Option<char> TryFromHexDigit(u32 digit)              { return digit < 16    ? Options::Some(FromHexDigit(digit)) : nullptr; }
        Option<char> TryFromDigitRadix(u32 digit, u32 radix) { return digit < radix ? Options::Some(FromHexDigit(digit)) : nullptr; }

        u32         ToDigit      (char c) { return (u32)(c - '0'); }
        u32         ToHexDigit   (char c) { return IsDigit(c) ? c - '0' : IsUpper(c) ? c - 'A' : c - 'a'; }
        Option<u32> TryToDigit   (char c) { return IsDigit(c)    ? Options::Some(ToDigit(c))    : nullptr; }
        Option<u32> TryToHexDigit(char c) { return IsHexDigit(c) ? Options::Some(ToHexDigit(c)) : nullptr; }
        Option<u32> TryToDigitRadix(char c, u32 radix) { return IsDigitRadix(c, radix) ? Options::Some(ToHexDigit(c)) : nullptr; }

        bool IsDigit   (char c) { return '0' <= c && c <= '9'; }
        bool IsHexDigit(char c) { return IsDigit(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f') ; }
        bool IsDigitRadix(char c, u32 radix) {
            if (radix <= 10) return '0' <= c && c < '0' + radix;
            return IsDigit(c) || ('A' <= c && c < 'A' + radix - 10) || ('a' <= c && c < 'a' + radix - 10);
        }

        Option<char> EscapeRepr(char c) {
            switch (c) {
                case '\'': return '\'';
                case '\"': return '"';
                case '\\': return '\\';
                case '\a': return 'a';
                case '\b': return 'b';
                case '\f': return 'f';
                case '\n': return 'n';
                case '\r': return 'r';
                case '\t': return 't';
                case '\v': return 'v';
                default:   return nullptr;
            }
        }

        usize WriteEscape(char c, char* out) {
            if (const auto escVer = EscapeRepr(c)) {
                Memory::WriteU16(*escVer << 8 | '\\', out);
                return 2;
            } else if (!IsPrintable(c)) {
                static constexpr char hexdig[17] = "0123456789ABCDEF";
                Memory::WriteU32(hexdig[c & 0xF] << 24 | hexdig[c >> 4] << 16 | "x\\"_u32 , out);
                return 4;
            } else {
                out[0] = c;
                return 1;
            }
        }

        Option<char> UnescapeRepr(char c) {
            switch (c) {
                case '\'': return '\'';
                case '"':  return '\"';
                case '?':  return '\?';
                case '\\': return '\\';
                case 'a':  return '\a';
                case 'b':  return '\b';
                case 'f':  return '\f';
                case 'n':  return '\n';
                case 'r':  return '\r';
                case 't':  return '\t';
                case 'v':  return '\v';
                default:   return nullptr;
            }
        }

        char ToLower(char c) { return IsUpper(c) ? (char)(c |  32) : c; }
        char ToUpper(char c) { return IsLower(c) ? (char)(c & ~32) : c; }
        bool IsLower(char c) { return 'a' <= c && c <= 'z'; }
        bool IsUpper(char c) { return 'A' <= c && c <= 'Z'; }

        bool IsBlank     (char c) { return c == ' ' || c == '\t'; }
        bool IsWhitespace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
        bool IsAlpha     (char c) { return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'); }
        bool IsNumeric   (char c) { return IsDigit(c); }
        bool IsAlphaNum  (char c) { return IsAlpha(c) || IsNumeric(c); }
        bool IsPrintable (char c) { return 0x1f < c && c < 0x7f; }
        bool IsControl   (char c) { return (0x00 <= c && c <= 0x1f) || c == 0x7f; }
        bool IsGraphic   (char c) { return IsPrintable(c) && c != ' '; }
        bool IsPunct     (char c) { return IsGraphic(c) && !IsAlphaNum(c); }


        Set::Set(Span<const char> chars) {
            for (const char c : chars) {
                bitmask[(uchar)c / 64] |= 1 << ((uchar)c & 63);
            }
        }

        bool Set::operator()(char c) const {
            return bitmask[(uchar)c / 64] & 1 << ((uchar)c & 63);
        }
        bool Set::operator()(Str str) const { return operator()(str[0]); }
    }

#define strdef template <class Char, class Super>
#define strcls StringHolder<Char, Super>

    strdef Hashing::Hash strcls::GetHashCode() const {
        return Hashing::HashBytes(this->AsBytes());
    }

    strdef Iter::SplitIter<Str> strcls::Split(Str sep) const { return Iter::SplitIter<Str>::New(AsStr(), sep); }
    strdef Iter::LinesIter strcls::Lines() const { return Iter::LinesIter::New(AsStr()); }
    strdef usize strcls::CountLines() const { return CountChars('\n') + 1; }
    strdef usize strcls::CountChars(Char c) const {
        usize count = 0;
        for (usize i = 0; i < this->Length(); ++i) {
            count += this->Data()[i] == c;
        }
        return count;
    }

    strdef usize strcls::Utf8Length() const {
        // from https://stackoverflow.com/questions/4063146/getting-the-actual-length-of-a-utf-8-encoded-stdstring
        usize utf8len = 0;
        const char* s = this->Data();
        for (usize i = 0; i < this->Length(); ++i) utf8len += (s[i] & 0xC0) != 0x80;
        return utf8len;
    }

    strdef Str strcls::AsStr() const { return Str::Slice(this->Data(), this->Length() * sizeof(Char)); }
    strdef StrMut strcls::AsStrMut() const requires IsMut<Char> { return StrMut::Slice((char*)this->Data(), this->Length() * sizeof(Char)); }
    strdef strcls::operator Str() const { return AsStr(); }
    strdef strcls::operator StrMut() const requires IsMut<Char> { return AsStrMut(); }

    strdef auto strcls::Substr(usize start) const              -> StrChar { return Substr(start, this->Length() - start); }
    strdef auto strcls::Substr(usize start, usize count) const -> StrChar { return StrChar::Slice((Char*)this->Data() + start, count * sizeof(Char)); }

    strdef Char& strcls::First() const { return (Char&)this->Get(0); }
    strdef Char& strcls::Last()  const { return (Char&)this->Get(this->Length() - 1); }

    strdef auto strcls::First(usize num) const -> StrChar { return Substr(0, num); }
    strdef auto strcls::Skip(usize len)  const -> StrChar { return Substr(len); }
    strdef auto strcls::Tail()           const -> StrChar { return Substr(1); }
    strdef auto strcls::Last(usize num)  const -> StrChar { return Substr(this->Length() - num, num); }
    strdef auto strcls::Trunc(usize len) const -> StrChar { return Substr(0, this->Length() - len); }
    strdef auto strcls::Init()           const -> StrChar { return Substr(this->Length() - 1); }
    strdef auto strcls::SplitFirst()         const -> Tuple<Char&, StrChar>          { return { First(),  Tail() }; }
    strdef auto strcls::SplitLast()          const -> Tuple<StrChar, Char&>          { return { Init(),   Last() }; }
    strdef auto strcls::CutAt(usize i)       const -> Tuple<StrChar, StrChar>        { return { First(i), Skip(i) }; }
    strdef auto strcls::SplitAt(usize i)     const -> Tuple<StrChar, StrChar>        { return { First(i), Skip(i + 1) }; }
    strdef auto strcls::PartitionAt(usize i) const -> Tuple<StrChar, Char&, StrChar> { return { First(i), (Char&)this->Get(i), Skip(i + 1) }; }

    strdef auto strcls::SplitOnce(Char c) const -> Tuple<StrChar, StrChar> {
        const OptionUsize i = this->FindIndex(c);
        return i ? SplitAt(*i) : Tuple { (StrChar)super(), StrChar {} };
    }
    strdef auto strcls::SplitOnce(Str sep) const -> Tuple<StrChar, StrChar> {
        const OptionUsize i = Search(sep);
        if (i)
            return { First(*i), Skip(*i + sep.Length()) };
        else return { (StrChar)super(), {} };
    }

    strdef bool strcls::RefEquals     (Str other) const { return this->Data() == other.Data() && this->Length() == other.Length(); }
    strdef bool strcls::ContainsBuffer(Str buf)   const { return buf.Data() >= this->Data() && this->DataEnd() >= buf.DataEnd(); }
    strdef bool strcls::OverlapsBuffer(Str buf)   const {
        const char* end = this->DataEnd(), *bufEnd = buf.DataEnd();
        return end >= buf.Data() && bufEnd >= this->Data();
    }

    strdef bool strcls::Equals(Str other) const {
        if (this->Length() != other.Length()) return false;
        usize i = 0;
        for (; i < (this->Length() & -8); i += 8)
            if (Memory::ReadU64Native(this->Data() + i) != Memory::ReadU64Native(other.Data() + i)) return false;

        for (; i < this->Length(); ++i)
            if (this->Get(i) != other.Get(i)) return false;
        return true;
    }

    strdef bool strcls::EqualsIgnoreCase(Str other) const {
        if (this->Length() != other.Length()) return false;
        for (usize i = 0; i < this->Length(); ++i)
            if (Chr::ToUpper(this->Get(i)) != Chr::ToUpper(other.Get(i))) return false;
        return true;
    }

    strdef bool strcls::operator==(Str other) const { return Equals(other); }
    strdef bool strcls::operator==(const String& other) const { return Equals(other); }
    strdef bool strcls::operator==(const Char* other) const { return Equals(other); }

    strdef Comparison strcls::Cmp(Str other) const {
        if (this->Length() != other.Length()) return Cmp::Between(this->Length(), other.Length());
        usize i = 0;
        for (; i < this->Length(); i += 8) {
            const auto cmp = Cmp::Between(Memory::ReadU64(this->Data() + i), Memory::ReadU64(other.Data() + i));
            if (cmp != Cmp::EQUAL) return cmp;
        }
        for (usize j = 0; j < (this->Length() & 7); ++j) {
            const auto cmp = Cmp::Between(this->Get(i - 8 + j), other.Get(i - 8 + j));
            if (cmp != Cmp::EQUAL) return cmp;
        }
        return Cmp::EQUAL;
    }
    strdef Comparison strcls::operator<=>(Str other) const { return Cmp(other); }

    strdef void  strcls::Reverse() requires IsMut<Char> { this->AsSpanMut().Reverse(); }

    strdef OptionUsize strcls::Search   (Str str) const {
        for (usize i = 0; i + str.Length() < this->Length(); ++i)
            if (Substr(i, str.Length()) == str) return i;
        return nullptr;
    }
    strdef OptionUsize strcls::RevSearch(Str str) const {
        for (usize i = this->Length() + 1; --i >= str.Length(); )
            if (Substr(i - str.Length(), str.Length()) == str) return i;
        return nullptr;
    }
    strdef bool strcls::Contains(Str str) const { return (bool)Search(str); }
    strdef bool strcls::Contains(char c) const { return (bool)this->FindIndex(c); }

    strdef Tuple<OptionUsize, OptRef<const Char>> strcls::FindOneOf(Str anyc) const {
        for (usize i = 0; i < this->Length(); ++i) {
            for (const char& c : anyc) {
                if (this->Get(i) == c) return { i, c };
            }
        }
        return { nullptr, nullptr };
    }
    strdef Tuple<OptionUsize, OptRef<const Char>> strcls::RevFindOneOf(Str anyc) const {
        for (usize i = this->Length(); i --> 0; ) {
            for (const char& c : anyc) {
                if (this->Get(i) == c) return { i, c };
            }
        }
        return { nullptr, nullptr };
    }
    strdef Tuple<OptionUsize, OptRef<const Str>> strcls::FindOneOf(Span<const Str> anystr) const {
        for (usize i = 0; i < this->Length(); ++i) {
            for (const Str& s : anystr)
                if (Skip(i).StartsWith(s)) return { i, s };
        }
        return { nullptr, nullptr };
    }
    strdef Tuple<OptionUsize, OptRef<const Str>> strcls::RevFindOneOf(Span<const Str> anystr) const {
        for (usize i = 0; i < this->Length(); ++i) {
            for (const Str& s : anystr)
                if (Skip(i).StartsWith(s)) return { i, s };
        }
        return { nullptr, nullptr };
    }

    strdef bool strcls::StartsWith(Char prefix) const { return this->Length() >= 1 && this->Get(0) == prefix; }
    strdef bool strcls::EndsWith  (Char suffix) const { return this->Length() >= 1 && this->Get(this->Length() - 1) == suffix; }
    strdef bool strcls::StartsWith(Str prefix)  const { return this->Length() >= prefix.Length() && First(prefix.Length()) == prefix; }
    strdef bool strcls::EndsWith  (Str suffix)  const { return this->Length() >= suffix.Length() && Last (suffix.Length()) == suffix; }

    strdef auto strcls::Trim     (Char c) const -> StrChar { return TrimIf     (Cmp::Equals { c }); }
    strdef auto strcls::TrimStart(Char c) const -> StrChar { return TrimStartIf(Cmp::Equals { c }); }
    strdef auto strcls::TrimEnd  (Char c) const -> StrChar { return TrimEndIf  (Cmp::Equals { c }); }

    strdef auto strcls::RemovePrefix(Char prefix) const -> StrChar { return Skip (StartsWith(prefix)); }
    strdef auto strcls::RemoveSuffix(Char suffix) const -> StrChar { return Trunc(EndsWith(suffix));   }
    strdef auto strcls::RemovePrefix(Str prefix)  const -> StrChar { return Skip (StartsWith(prefix) ? prefix.Length() : 0); }
    strdef auto strcls::RemoveSuffix(Str suffix)  const -> StrChar { return Trunc(EndsWith(suffix)   ? suffix.Length() : 0); }
    strdef auto strcls::RemovePrefixOneOf(Str prefix) const -> Tuple<StrChar, OptRef<const Char>> {
        for (const char& c : prefix) {
            if (StartsWith(c)) return { RemovePrefix(c), c };
        }
        return { (StrChar)super(), nullptr };
    }
    strdef auto strcls::RemoveSuffixOneOf(Str suffix) const -> Tuple<StrChar, OptRef<const Char>> {
        for (const char& c : suffix) {
            if (EndsWith(c)) return { RemoveSuffix(c), c };
        }
        return { (StrChar)super(), nullptr };
    }
    strdef auto strcls::RemovePrefixOneOf(Span<const Str> prefix) const -> Tuple<StrChar, OptRef<const Str>> {
        for (const Str& s : prefix) {
            if (StartsWith(s)) return { RemovePrefix(s), s };
        }
        return { (StrChar)super(), nullptr };
    }
    strdef auto strcls::RemoveSuffixOneOf(Span<const Str> suffix) const -> Tuple<StrChar, OptRef<const Str>> {
        for (const Str& s : suffix) {
            if (EndsWith(s)) return { RemoveSuffix(s), s };
        }
        return { (StrChar)super(), nullptr };
    }

    strdef String strcls::ToString() const {
        return String::FromStr(*this);
    }

    strdef String strcls::Reversed() const {
        String rev = String::WithCap(this->Length());
        for (usize i = this->Length(); i --> 0; )
            rev.Append(this->Get(i));
        return rev;
    }

    strdef String strcls::ToUpper() const {
        String upper = String::WithCap(this->Length());
        for (const char c : super()) upper.Append(Chr::ToUpper(c));
        return upper;
    }
    strdef String strcls::ToLower() const {
        String lower = String::WithCap(this->Length());
        for (const char c : super()) lower.Append(Chr::ToLower(c));
        return lower;
    }

    strdef void strcls::MakeUpper() requires IsMut<Char> {
        for (char& c : super()) c = Chr::ToUpper(c);
    }

    strdef void strcls::MakeLower() requires IsMut<Char> {
        for (char& c : super()) c = Chr::ToLower(c);
    }

    strdef String strcls::Repeat(usize n) const {
        String rep = String::WithCap(this->Length() * n);
        for (usize i = 0; i < n; ++i)
            for (const char c : super())
                rep.Append(c);
        return rep;
    }

    strdef String strcls::Replace(Char from, Str to) const {
        return ReplaceIf([=] (Str str, OptionUsize& len) { if (str[0] == from) len = 1; return to; });
    }

    strdef String strcls::Replace(Str from, Str to) const {
        if (from.IsEmpty()) return ToString();
        return ReplaceIf([&] (Str str, OptionUsize& len) {
            if (str.StartsWith(from)) len = from.Length();
            return to;
        });
    }

    Str& Str::Advance(usize num) {
        data += num;
        size -= num;
        return *this;
    }

    Str& Str::Shorten(usize amount) {
        size -= amount;
        return *this;
    }

    Str Str::TakeFirst(usize num) {
        const Str first = First(num);
        Advance(num);
        return first;
    }

    Str Str::TakeLast(usize num) {
        const Str last = Last(num);
        Shorten(num);
        return last;
    }

    const char& Str::TakeFirst() {
        const char& first = First();
        Advance(1);
        return first;
    }

    const char& Str::TakeLast() {
        const char& last = Last();
        Shorten(1);
        return last;
    }

    Str Str::TakeAfter(usize i) {
        const Str after = Skip(i);
        size = i;
        return after;
    }

    String Str::Escape() const {
        String ss = String::WithCap(this->Length() + 2);
        ss += '"';
        for (const char c : *this) {
            char buf[4];
            ss += Str::Slice(buf, Chr::WriteEscape(c, buf));
        }
        ss += '"';
        return ss;
    }

    usize Str::WriteEscape(Text::StringWriter output) const {
        usize len = 2;
        output.Write('"');
        for (const char c : *this) {
            char buf[4];
            len += output.Write(Str::Slice(buf, Chr::WriteEscape(c, buf)));
        }
        output.Write('"');
        return len;
    }

    Option<String> Str::Unescape() const {
        // early check, so that we never have to check again
        if (Last() == '\\') return nullptr;

        String ss = String::WithCap(this->Length());
        for (usize i = 0; i < this->Length(); ++i) {
            if (Get(i) == '\\') {
                ss += Chr::UnescapeRepr(Get(i + 1)).UnwrapOr(Get(i + 1));
                ++i;
            } else {
                ss += Get(i);
            }
        }
        return ss;
    }

    StrMut Str::AsMut() { return StrMut::Slice(Memory::AsMutPtr(data), size); }

    StrMut& StrMut::Advance(usize num) {
        data += num;
        size -= num;
        return *this;
    }

    StrMut& StrMut::Shorten(usize amount) {
        size -= amount;
        return *this;
    }

    StrMut StrMut::TakeFirst(usize num) {
        const StrMut first = First(num);
        Advance(num);
        return first;
    }

    StrMut StrMut::TakeLast(usize num) {
        const StrMut last = Last(num);
        Shorten(num);
        return last;
    }

    char& StrMut::TakeFirst() {
        char& first = First();
        Advance(1);
        return first;
    }

    char& StrMut::TakeLast() {
        char& last = Last();
        Shorten(1);
        return last;
    }

    StrMut StrMut::TakeAfter(usize i) {
        const StrMut after = Skip(i);
        size = i;
        return after;
    }

    template struct StringHolder<const char, Str>;
    template struct StringHolder<char, StrMut>;
    template struct StringHolder<char, String>;
    template struct StringHolder<const char, CStr>;
} // Quasi