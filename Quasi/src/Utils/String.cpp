#include "String.h"

#include "Text/StringWriter.h"

namespace Quasi {
    String String::WithCap(usize cap) {
        if (cap <= Small::MAXLEN) {
            return Empty();
        } else {
            return Large { AllocateString(cap), cap, 0 };
        }
    }

    String String::FromChars(Vec<char> chars) {
        if (chars.Length() <= Small::MAXLEN) {
            String string;
            Memory::MemCopyNoOverlap(string.small.data, chars.Data(), chars.Length());
            string.small.size = chars.Length();
            string.small.isLarge = false;
            return string;
        } else {
            const auto [data, size, capacity] = chars.Decompose();
            return Large { data, capacity, size };
        }
    }

    String String::FromChars(Span<const char> chars) {
        if (chars.Length() <= Small::MAXLEN) {
            String string;
            Memory::MemCopyNoOverlap(string.small.data, chars.Data(), chars.Length());
            string.small.size = chars.Length();
            string.small.isLarge = false;
            return string;
        } else {
            char* str = AllocateString(chars.Length());
            Memory::MemCopyNoOverlap(str, chars.Data(), chars.Length());
            return Large { str, chars.Length(), chars.Length() };
        }
    }

    String String::FromStr(Str str) {
        return FromChars(str.AsSpan());
    }

    String String::Compose(char* data, usize size, usize cap) {
        return FromChars(Vec<char>::Compose(data, size, cap));
    }

    String String::Clone() const {
        return FromChars(AsSpan());
    }

    String::String(const String& s) : String(s.Clone()) { }
    String& String::operator=(const String& s) {
        this->~String();
        Memory::ConstructCopyAt(this, s);
        return *this;
    }

    String::String(String&& s) noexcept {
        if (s.IsSmallString()) {
            small = Small { s.small };
        } else {
            large = Large { s.large.data, s.large.cap, s.large.size };
            s.large.data = nullptr;
        }
    }

    String& String::operator=(String&& s) noexcept {
        this->~String();
        Memory::ConstructMoveAt(this, std::move(s));
        return *this;
    }

    String::~String() {
        if (IsLargeString()) {
            Memory::FreeRaw(large.data);
            large.data = nullptr;
            large.size = 0;
        }
    }

    usize String::GrowCap(usize cap) {
        return cap * 3 / 2;
    }

    void String::OptimizeAsSmall() {
        char* string = large.data;
        const usize size = large.size;
        Memory::MemCopyNoOverlap(small.data, string, size);
        small.size = size;
        small.isLarge = false;
        Memory::FreeRaw(string);
    }

    void String::CopyToLarge(char* dest, usize size) {
        Memory::MemCopyNoOverlap(dest, small.data, small.size);
        large.data = dest;
        large.cap  = size;
        large.size = small.size;
        large.isLarge = true;
    }

    void String::MoveBuffer(char* dest, usize size) {
        Memory::MemCopyNoOverlap(dest, large.data, large.size);
        Memory::FreeRaw(large.data);
        large.data = dest;
        large.size = size;
    }

    char* String::AllocateString(usize size) {
        return (char*)Memory::AllocateRaw(size * sizeof(char));
    }

    bool String::CanFit(usize amount) const {
        return Length() + amount <= Capacity();
    }

    void String::TryGrow(usize amount) {
        if (!CanFit(amount)) Reserve(amount);
    }

    const char* String::Data()     const { return IsSmallString() ? &small.data[0] : large.data; }
    char*       String::Data()           { return IsSmallString() ? &small.data[0] : large.data; }
    usize       String::Length()   const { return IsSmallString() ?  small.size    : large.size; }
    usize       String::Capacity() const { return IsSmallString() ? Small::MAXLEN  : large.cap; }

    void String::Reserve(usize extra) {
        if (IsSmallString()) {
            if (small.size + extra > Small::MAXLEN) {
                const usize newCap = std::max<usize>(32, small.size + extra);
                return CopyToLarge(AllocateString(newCap), newCap);
            }
        } else {
            if (large.size + extra > large.cap) {
                const usize newCap = std::max<usize>(GrowCap(large.cap), large.size + extra);
                return MoveBuffer(AllocateString(newCap), newCap);
            }
        }
    }

    void String::ReserveExact(usize extra) {
        if (!small.isLarge && small.size + extra > Small::MAXLEN) {
            const usize newCap = small.size + extra;
            return CopyToLarge(AllocateString(newCap), newCap);
        } else if (large.isLarge && large.size + extra > large.cap) {
            const usize newCap = large.size + extra;
            return MoveBuffer(AllocateString(newCap), newCap);
        }
    }

    void String::SetEmptyUnsafe() {
        small = {};
    }

    Nullable<char*> String::ReleaseLargeString() {
        if (IsLargeString()) {
            char* string = large.data;
            large.data = nullptr;
            return string;
        }
        return nullptr;
    }

    Option<Vec<char>> String::IntoLargeString() {
        if (IsLargeString()) {
            char* string = large.data;
            const usize size = large.size, cap = large.cap;
            SetEmptyUnsafe();
            return Vec<char>::Compose(string, size, cap);
        }
        else return nullptr;
    }

    Option<StrMut> String::TryAsSmallString() {
        if (IsSmallString()) {
            return StrMut::Slice(small.data, small.size);
        }
        return Option<StrMut>::None();
    }

    void String::TryOptimize() {
        if (IsLargeString() && large.size <= Small::MAXLEN) {
            OptimizeAsSmall();
        }
    }

    Vec<char> String::IntoChars() {
        if (IsLargeString()) {
            char* string = large.data;
            const usize size = large.size, cap = large.cap;
            SetEmptyUnsafe();
            return Vec<char>::Compose(string, size, cap);
        } else {
            Vec chars = Vec<char>::New(small.data);
            SetEmptyUnsafe();
            return chars;
        }
    }

    Vec<byte> String::IntoBytes() {
        const auto [data, size, capacity] = IntoChars().Decompose();
        return Vec<byte>::Compose((byte*)data, size, capacity);
    }

    ArrayBox<char> String::IntoBox() {
        if (IsLargeString()) {
            char* string = large.data;
            const usize size = large.size;
            SetEmptyUnsafe();
            return ArrayBox<char>::Own(string, size);
        } else {
            const usize size = small.size;
            char* memory = AllocateString(size);
            SetEmptyUnsafe();
            return ArrayBox<char>::Own(memory, size);
        }
    }

    void String::Append(char c) {
        TryGrow(1);
        if (IsSmallString()) {
            small.data[small.size++] = c;
        } else {
            large.data[large.size++] = c;
        }
    }

    void String::AppendStr(Str str) {
        TryGrow(str.Length());
        if (IsSmallString()) {
            Memory::MemCopyNoOverlap(&small.data[small.size], str.Data(), str.Length());
            small.size += str.Length();
        } else {
            Memory::MemCopyNoOverlap(&large.data[large.size], str.Data(), str.Length());
            large.size += str.Length();
        }
    }

    void String::Insert(char c, usize i) {
        TryGrow(1);
        if (IsSmallString()) {
            Memory::MemCopyRev(&small.data[i + 1], &small.data[i], small.size - i);
            small.data[i] = c;
            ++small.size;
        } else {
            Memory::MemCopyRev(&large.data[i + 1], &large.data[i], large.size - i);
            large.data[i] = c;
            ++large.size;
        }
    }

    void String::InsertStr(Str str, usize i) {
        TryGrow(str.Length());
        if (IsSmallString()) {
            Memory::MemCopyRev(&small.data[i + str.Length()], &small.data[i], small.size - i);
            Memory::MemCopyNoOverlap(&small.data[i], str.Data(), str.Length());
            small.size += str.Length();
        } else {
            Memory::MemCopyRev(&large.data[i + str.Length()], &large.data[i], large.size - i);
            Memory::MemCopyNoOverlap(&large.data[i], str.Data(), str.Length());
            large.size += str.Length();
        }
    }

    char String::Pop() {
        if (IsSmallString())
            return small.data[small.size--];
        else {
            const char last = large.data[large.size--];
            if (large.size <= Small::MAXLEN)
                OptimizeAsSmall();
            return last;
        }
    }
    char String::Pop(usize index) {
        if (IsSmallString()) {
            const char out = small.data[index];
            Memory::MemCopy(&small.data[index], &small.data[index + 1], small.size - index);
            --small.size;
            return out;
        } else {
            const char out = large.data[index];
            if (large.size - 1 <= Small::MAXLEN) {
                char* str = large.data;
                const usize size = large.size;
                SetEmptyUnsafe();
                Memory::MemCopyNoOverlap( small.data,         str,            index);
                Memory::MemCopyNoOverlap(&small.data[index], &str[index + 1], size - index - 1);
                Memory::FreeRaw(str);
                small.size = size;
            } else {
                Memory::MemCopy(&large.data[index], &large.data[index + 1], large.size - index - 1);
                --large.size;
            }
            return out;
        }
    }

    void String::RemoveAllOf(char c) {
        return RemoveIf([=] (Str s) { return s[0] == c ? 1 : 0; });
    }

    void String::RemoveAllOf(Str str) {
        if (str.IsEmpty()) return;
        return RemoveIf([&] (Str s) { return s.StartsWith(str) ? str.Length() : 0; });
    }

    void String::RemoveAllOfEach(Span<const char> anyc) {
        if (anyc.IsEmpty()) return;
        const Chr::Set set = { anyc };
        return RemoveIf([&] (Str s) { return set(s[0]) ? 1 : 0; });
    }

    void String::RemoveAllOfEach(Span<const Str> anystr) {
        if (anystr.IsEmpty()) return;
        return RemoveIf([&] (Str str) {
           const usize i = str.StartsWithOneOf(anystr);
           return i == -1 ? 0 : anystr[i].Length();
        });
    }

    void String::Clear() {
        if (IsLargeString())
            large.size = 0;
        else
            small.size = 0;
    }

    String String::Split(usize index) {
        String right = FromStr(Substr(index));
        if (IsSmallString()) {
            small.size = index;
        } else if (IsLargeString()) {
            large.size = index;
            if (index <= Small::MAXLEN) {
                OptimizeAsSmall();
            }
        }
        return right;
    }

    void String::ShrinkToFit() {
        if (IsSmallString()) return;
        MoveBuffer(AllocateString(large.size), large.size);
    }

    void String::ShrinkTo(usize mincap) {
        if (IsSmallString()) return;
        const usize size = std::max(large.size, mincap);
        MoveBuffer(AllocateString(size), size);
    }

    void String::Truncate(usize len) {
        if (IsSmallString()) {
            small.size = len;
        } else {
            large.size = len;
        }
    }

    String String::Escape() const {
        return AsStr().Escape();
    }

    usize String::WriteEscape(Text::StringWriter output) const {
        return AsStr().WriteEscape(output);
    }

    Option<String> String::Unescape() const {
        return AsStr().Unescape();
    }

    String String::operator+(char rhs) const {
        return operator+(Str::Slice(&rhs, 1));
    }

    String& String::operator+=(char rhs) {
        Append(rhs);
        return *this;
    }

    String String::operator+(Str rhs) const {
        String sum = WithCap(Length() + rhs.Length());
        Memory::MemCopyNoOverlap(sum.Data(),            Data(),     Length());
        Memory::MemCopyNoOverlap(sum.Data() + Length(), rhs.Data(), rhs.Length());
        if (sum.IsSmallString())
            sum.small.size = Length() + rhs.Length();
        else
            sum.large.size = sum.large.cap;
        return sum;
    }

    String& String::operator+=(Str rhs) {
        AppendStr(rhs);
        return *this;
    }

    String  String::operator+ (const String& rhs) const { return operator+ (rhs.AsStr()); }
    String& String::operator+=(const String& rhs)       { return operator+=(rhs.AsStr()); }
    String  String::operator+ (const char* rhs)   const { return operator+ (Str { rhs }); }
    String& String::operator+=(const char* rhs)         { return operator+=(Str { rhs }); }
}
