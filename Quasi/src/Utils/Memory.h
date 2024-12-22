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

    inline void* AllocateRaw(usize size) { return ::operator new (size); }
    template <class T> T* Allocate(auto&&... args) { return new T { args... }; }
    template <class T> T* AllocateArray(usize size, auto&&... args) { return new T[size] { args... }; }
    template <class T> T* AllocateUninit() { return ::operator new (sizeof(T)); }
    template <class T> T* AllocateArrayUninit(usize size) { return ::operator new (size * sizeof(T)); }
    inline void FreeRaw(void* mem) { return ::operator delete(mem); }
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

    template <class T> constexpr void MemSet(T* out, const T& fill, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = fill;
    }

    constexpr void MemCopy(byte* out, const byte* in, usize bytes) {
        for (usize i = 0; i < bytes; ++i)
            out[i] = in[i];
    }

    // WARNING: undefined behavior on overlapping pointer ranges
    constexpr void MemCopyNoOverlap(byte* __restrict__ out, const byte* __restrict__ in, usize bytes) {
        MemCopy(out, in, bytes);
    }

    constexpr void MemCopyRev(byte* out, const byte* in, usize bytes) {
        for (usize i = bytes; i --> 0; ) // no overflow :)
            out[i] = in[i];
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
}
