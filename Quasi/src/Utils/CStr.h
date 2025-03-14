#pragma once
#include "Iterator.h"
#include "Numeric.h"
#include "Tuple.h"
#include "Type.h"

namespace Quasi {
    template <class T> struct Option;
    struct Str;
    struct String;

    namespace Text {
        struct StringWriter;
    }

    struct CStr {
    private:
        const char* data = nullptr;
        usize size = 0;
        CStr(const char* data, usize size) : data(data), size(size) {}
    public:
        CStr() = default;
        CStr(Nullptr) : CStr() {}
        CStr(const char* zstr) : data(zstr) { while (data[size++]); }

        static CStr Empty() { return nullptr; }
        static Option<CStr> TrySlice(const char* data, usize size);
        static CStr SliceUnchecked(const char* data, usize size);

        static Option<CStr> TryFrom(Str s);
        static Option<CStr> TryFromWithNull(Str zs);
        static CStr FromUnchecked(Str s);

        const char* Data()    const { return data; }
        const char* DataEnd() const { return data + size; }
        usize Length()        const { return size; }

        bool  IsEmpty() const { return Length() == 0; }
        operator bool() const { return Length() != 0; }
        const char& At(usize i)             const { return Data()[i]; }
        const char& AtWrap(WrappingIndex i) const { return At(i(Length())); }

        BufferIterator<const char&> Iter() const;
        // CharsIter Chars() const;
        // Utf8CharsIter Utf8Chars() const;
        // SplitWhitespaceIter SplitWhitespace() const;
        // LinesIter Lines() const;

        Str              AsStr()           const;
        Str              AsStrWithNull()   const;
        Span<const char> AsSpan()          const;
        Span<const char> AsSpanWithNull()  const;
        Span<const byte> AsBytes()         const;
        Span<const byte> AsBytesWithNull() const;
        operator Str() const;

        const char& First() const { return At(0); }
        const char& Last()  const { return At(Length() - 1); }

        Span<const char> Subspan(usize start) const;
        Span<const char> Subspan(usize start, usize count) const;
        Str              Substr (usize start) const;
        Str              Substr (usize start, usize count) const;

        Str  First(usize num) const;
        CStr Skip(usize len)  const;
        CStr Tail()           const;
        CStr Last(usize num)  const;
        Str  Trunc(usize len) const;
        Str  Init()           const;
        Tuple<const char&, CStr>      SplitFirst()                            const;
        Tuple<Str, const char&>       SplitLast()                             const;
        Tuple<Str, CStr>              SplitAt   (usize at)                    const;
        Tuple<Str, const char&, CStr> PartitionAt(usize at)                   const;
        Tuple<Str, CStr>              SplitOn(Predicate<char> auto&& pred)    const {
            const usize i = FindIf(pred);
            return SplitAt(i == -1 ? Length() : i);
        }
        Tuple<Str, CStr>              RevSplitOn(Predicate<char> auto&& pred) const {
            const usize i = RevFindIf(pred);
            return SplitAt(i == -1 ? 0 : i + 1);
        }

        bool RefEquals     (CStr other) const;

        bool Equals          (Str other) const;
        bool EqualsIgnoreCase(Str other) const;
        bool operator==      (Str other) const;

        Comparison Cmp(Str other) const;
        Comparison CmpSized(Str other) const;
        Comparison operator<=>(Str other) const;

        String  ToString() const;

        CStr&       Advance(usize num);
        Str         TakeFirst(usize i);
        const char& TakeFirst();
    };

    CStr operator ""_cstr(const char* zstr, usize len);
}
