#pragma once
#include "Type.h"

namespace Quasi::Memory {
    template <class T> constexpr usize AlignOf() { return alignof(T); }
    template <class T> constexpr usize SizeOf() { return sizeof(T); }

    template <class T, class U> U& Transmute(T& t)      { return *reinterpret_cast<U*>(&t); }
    template <class T, class U> U* TransmutePtr(T* t)   { return reinterpret_cast<U*>(t); }
    template <class T, class U> U& TransmuteRef(T& t) requires (sizeof(T) == sizeof(U)) { return __builtin_bit_cast(U, t); }
    template <class T>       void* DowncastPtr(T* t)    { return (void*)t; }
    template <class T>          T* UpcastPtr(void* t)   { return (T*)t; }

    template <class T> const T&& AsConst   (T&& val)       { return const_cast<const T&&>(val); }
    template <class T> const T*  AsConstPtr(T* ptr)        { return const_cast<const T*> (ptr); }
    template <class T> T&&       AsMut     (const T&& val) { return const_cast<T&&>(val); }
    template <class T> T*        AsMutPtr  (const T* ptr)  { return const_cast<T*> (ptr); }


    inline void* AllocateRaw(usize size) { return malloc(size); }
    template <class T> T* Allocate(auto&&... args) { return new T { args... }; }
    template <class T> T* AllocateArray(usize size, auto&&... args) { return new T[size] { args... }; }
    template <class T> T* AllocateUninit() { return ::operator new (sizeof(T)); }
    template <class T> T* AllocateArrayUninit(usize size) { return ::operator new (size * sizeof(T)); }
    inline void FreeRaw(void* mem) { return free(mem); }
    template <class T> void Free(T* mem) { delete mem; }
    template <class T> void FreeArray(T* mem) { delete[] mem; }
    template <class T> void FreeNoDestruct(T* mem) { ::operator delete(mem); }
    template <class T> void FreeArrayNoDestruct(T* mem) { ::operator delete[](mem); }

    template <class T> void ConstructAt(T* dest, auto&&... args) {
        new (dest) T(std::forward<decltype(args)>(args)...);
    }
    template <class T> void DestructAt(T* dest) { dest->~T(); }

    // effectivly disables the destructor on value
    template <class T> void ForgetValue(T* value) {
        // move the data to "raw" storage, to prevent deallocation
        alignas(T) char temp[sizeof(T)];
        new (temp) T(std::move(*value));
    }

    template <class T>
    Span<T> ReleaseData(Vec<T>&& vector) {
        Span<T> data = vector;
        ForgetValue(&vector);
        return data;
    }

#define Q_GETTER_MUT(FN, ...) (decltype(this->FN(__VA_ARGS__)))(Memory::AsConstPtr(this))->FN(__VA_ARGS__)
#define QGetterMut$ Q_GETTER_MUT
}
