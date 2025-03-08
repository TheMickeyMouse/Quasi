#pragma once
#include "Type.h"

namespace Quasi::Memory {
    template <class T> constexpr usize AlignOf() { return alignof(T); }
    template <class T> constexpr usize SizeOf() { return sizeof(T); }

    template <class U, class T> U& TransmuteRef(T& t)       { return *reinterpret_cast<U*>(&t); }
    template <class U, class T> U* TransmutePtr(T* t)       { return reinterpret_cast<U*>(t); }
    template <class U, class T> U  Transmute(T t) requires (sizeof(T) == sizeof(U)) { return __builtin_bit_cast(U, t); }
    template <class T> const byte* AsBytePtr(const T* t)    { return (const byte*)t; }
    template <IsMut T>       byte* AsBytePtr(T* t)          { return (byte*)t; }
    template <class T>       void* DowncastPtr(T* t)        { return (void*)t; }
    template <class T>          T* UpcastPtr(void* t)       { return (T*)t; }

    template <class T> const T&  AsConst   (T& val)       { return const_cast<const T&>(val); }
    template <class T> const T*  AsConstPtr(T* ptr)       { return const_cast<const T*>(ptr); }
    template <class T> T&        AsMut     (const T& val) { return const_cast<T&>(val); }
    template <class T> T*        AsMutPtr  (const T* ptr) { return const_cast<T*>(ptr); }

    template <class Der, BaseOf<Der> Base> AddConstIf<Der, Base>* DynCastPtr(Base* base) {
        return dynamic_cast<AddConstIf<Der, Base>*>(base);
    }

    void* AllocateRaw(usize size);
    template <class T> T* Allocate(auto&&... args) { return new T { args... }; }
    template <class T> T* AllocateArray(usize size, auto&&... args) { return new T[size] { args... }; }
    template <class T> T* AllocateUninit() { return (T*) ::operator new (sizeof(T)); }
    template <class T> T* AllocateArrayUninit(usize size) { return (T*) ::operator new (size * sizeof(T)); }
    void FreeRaw(void* mem);
    template <class T> void Free(T* mem) { delete mem; }
    template <class T> void FreeArray(T* mem) { delete[] mem; }
    template <class T> void FreeNoDestruct(T* mem) { ::operator delete(mem); }
    template <class T> void FreeArrayNoDestruct(T* mem) { ::operator delete[](mem); }

    template <class T> void ConstructAt(T* dest, auto&&... args) {
        new (dest) T(std::forward<decltype(args)>(args)...);
    }
    template <class T> void ConstructCopyAt(T* dest, const T& obj) {
        new (dest) T(obj);
    }
    template <class T> void ConstructMoveAt(T* dest, T&& args) {
        new (dest) T(std::move(args));
    }
    template <class T> void DestructAt(T* dest) { dest->~T(); }

    // effectivly disables the destructor on value
    template <class T> void ForgetValue(T* value) {
        // move the data to "raw" storage, to prevent deallocation
        alignas(T) char temp[sizeof(T)];
        new (temp) T(std::move(*value));
    }

    u16 ReadU16(const void* bytes);
    i16 ReadI16(const void* bytes);
    u32 ReadU32(const void* bytes);
    i32 ReadI32(const void* bytes);
    u64 ReadU64(const void* bytes);
    i64 ReadI64(const void* bytes);
    u16 ReadU16Little(const void* bytes);
    i16 ReadI16Little(const void* bytes);
    u32 ReadU32Little(const void* bytes);
    i32 ReadI32Little(const void* bytes);
    u64 ReadU64Little(const void* bytes);
    i64 ReadI64Little(const void* bytes);

    void WriteU16(u16 x, void* out);
    void WriteI16(i16 x, void* out);
    void WriteU32(u32 x, void* out);
    void WriteI32(i32 x, void* out);
    void WriteU64(u64 x, void* out);
    void WriteI64(i64 x, void* out);
    void WriteU16Little(u16 x, void* out);
    void WriteI16Little(i16 x, void* out);
    void WriteU32Little(u32 x, void* out);
    void WriteI32Little(i32 x, void* out);
    void WriteU64Little(u64 x, void* out);
    void WriteI64Little(i64 x, void* out);

    u16 ByteSwap16(u16 x);
    u32 ByteSwap32(u32 x);
    u64 ByteSwap64(u64 x);

    void MemCopy(void* out, const void* in, usize bytes);
    // WARNING: undefined behavior on overlapping pointer ranges
    void MemCopyNoOverlap(void* __restrict__ out, const void* __restrict__ in, usize bytes);
    void MemCopyRev(void* out, const void* in, usize bytes);

    template <class T> constexpr void RangeSet(T* out, const T& fill, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = fill;
    }

    template <class T> constexpr void RangeCopy(T* out, const T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = in[i];
    }

    // WARNING: undefined behavior on overlapping pointer ranges
    template <class T> constexpr void RangeCopyNoOverlap(T* __restrict__ out, const T* __restrict__ in, usize count) {
        RangeCopy(out, in, count);
    }

    template <class T> constexpr void RangeCopyRev(T* out, const T* in, usize count) {
        for (usize i = count; i --> 0; )
            out[i] = in[i];
    }

    template <class T> constexpr void RangeMove(T* out, T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = std::move(in[i]);
    }

    // WARNING: undefined behavior on overlapping pointer ranges
    template <class T> constexpr void RangeMoveNoOverlap(T* __restrict__ out, T* __restrict__ in, usize count) {
        RangeMove(out, in, count);
    }

    template <class T> constexpr void RangeMoveRev(T* out, T* in, usize count) {
        for (usize i = count; i --> 0;)
            out[i] = std::move(in[i]);
    }

    template <class T> constexpr void RangeSwap(T* out, T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            std::swap(out[i], in[i]);
    }

    template <class T> constexpr void RangeSwapNoOverlap(T* __restrict__ out, T* __restrict__ in, usize count) {
        RangeSwap(out, in, count);
    }

    template <class T> constexpr void RangeDestruct(T* data, usize count) {
        for (usize i = 0; i < count; ++i) data[i].~T();
    }

#define Q_GETTER_MUT(FN, ...) (decltype(this->FN(__VA_ARGS__)))(Memory::AsConstPtr(this))->FN(__VA_ARGS__)
#define QGetterMut$ Q_GETTER_MUT

    // usage: just do Memory::QAlloca$(T, 32) or something like that
#define Q_ALLOCA(T, SIZE) UpcastPtr<T>(_alloca(sizeof(T) * (SIZE)))
#define QAlloca$(...) Q_ALLOCA(__VA_ARGS__)
}

namespace Quasi {
    u16 operator ""_u16(const char* s, usize n);
    u32 operator ""_u32(const char* s, usize n);
    u64 operator ""_u64(const char* s, usize n);
}