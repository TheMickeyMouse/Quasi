#pragma once
#include "Type.h"

namespace Quasi::Memory {
    /// Transmutes a @a reference to @p T by reinterpreting the memory into a @a reference to @p U .
    template <class U, class T> U& TransmuteRef(T& t) { return *reinterpret_cast<U*>(&t); }
    /// Transmutes a @a pointer to @p T by reinterpreting the memory into a @a pointer to @p U .
    template <class U, class T> U* TransmutePtr(T* t) { return reinterpret_cast<U*>(t); }
    /// Transmutes the value stored in @p T into a value of @p U . Equivalent to @code (U)TransmuteRef<U>(t) @endcode
    /// @note Can be run at compile time, uses @p __builtin_bitcast under the hood.
    template <class U, class T> constexpr U Transmute(T t) requires (sizeof(T) == sizeof(U)) { return __builtin_bit_cast(U, t); }

    /// Returns the const-reference for @ val . Useful for when the const member function needs to be called on a regular reference.
    template <class T> const T& AsConst(T& val) { return const_cast<const T&>(val); }
    /// Returns the const-reference for @ val . Useful for when the const member function needs to be called on a regular reference.
    template <class T> const T& AsConst(T&& val) { return const_cast<const T&>(val); }
    /// Returns the const-pointer for @ val . Useful for when the const member function needs to be called on a regular pointer.
    template <class T> const T* AsConstPtr(T* ptr) { return const_cast<const T*>(ptr); }
    /// Casts away the const-reference for @ val .
    /// @note Breaks const-correctness. Using this is likely a sign of bad code.
    template <class T> T& AsMut(const T& val) { return const_cast<T&>(val); }
    /// Casts away the const-pointer for @ val .
    /// @note Breaks const-correctness. Using this is likely a sign of bad code.
    template <class T> T* AsMutPtr(const T* ptr) { return const_cast<T*>(ptr); }

    /// Allocates a memory chunk on the heap with @p size bytes.
    /// @note Uses @p malloc under the hood.
    void* AllocateRaw(usize size);
    /// Allocates a @p T on the heap with the constructor arguments specified in @p args...
    /// @note Uses @p new under the hood.
    template <class T> T* Allocate(auto&&... args) { return new T { std::forward<decltype(args)>(args)... }; }
    /// Allocates a @p T on the heap @b without calling the constructor, leaving the memory uninitialized.
    /// @note Uses @code ::operator new@endcode under the hood.
    /// @warning The constructor @b must be called by the user themselves,
    /// otherwise reading from the memory chunk is undefined behavior.
    template <class T> T* AllocateUninit() { return (T*) ::operator new (sizeof(T)); }

    /// Frees the memory chunk allocated by @p AllocateRaw . After calling, the memory chunk is invalidated.
    /// @note Uses @p free under the hood.
    void FreeRaw(void* mem);
    /// Frees a @p T on the heap while also calling the destructor.
    /// @note Uses @p delete under the hood.
    template <class T> void Free(T* mem) { delete mem; }
    /// Frees a @p T on the heap @b without calling the destructor.
    /// @note Uses @code ::operator delete@endcode under the hood.
    /// @warning The destructor @b must be called by the user themselves,
    /// otherwise some resources may not be cleaned up.
    template <class T> void FreeNoDestruct(T* mem) { ::operator delete(mem); }

    /// Constructs a @p T on the memory address @p dest by calling the in-place constructor with arguments @p args...
    /// @note Uses @code new (dest) T(...)@endcode under the hood.
    template <class T> void ConstructAt(T* dest, auto&&... args) { new (dest) T(std::forward<decltype(args)>(args)...); }
    /// Copy-constructs a @p T on the memory address @p dest with @p obj .
    /// @note Uses @code new (dest) T(...)@endcode under the hood.
    template <class T> void ConstructCopyAt(T* dest, const T& obj) { new (dest) T(obj); }
    /// Move-constructs a @p T on the memory address @p dest by moving @p obj.
    /// @note Uses @code new (dest) T(...)@endcode under the hood.
    template <class T> void ConstructMoveAt(T* dest, T&& args) { new (dest) T(std::move(args)); }

    /// Destorys the @p T referenced in @p dest by calling the pointer's destructor.
    /// @warning After calling, the value referenced by @p dest is undefined.
    template <class T> void DestructAt(T* dest) { dest->~T(); }

    /// Prevents calling the destructor on the value pointed to by @p ptr .
    /// @warning After calling, the value referenced by @p dest is undefined.
    /// By calling this, you @b ensure that any resources not cleaned up normally
    /// by the destructor for @p T will be managed by your code.
    template <class T> void ForgetValue(T* ptr) {
        // move the data to "raw" storage, to prevent deallocation
        alignas(T) char temp[sizeof(T)];
        new (temp) T(std::move(*ptr));
    }
    /// Checks whether the system endianness is <b>little endian</b>. Can be computed in compile time.
    constexpr bool IsLittleEndian() { return std::endian::native == std::endian::little; }
    /// Checks whether the system endianness is <b>big endian</b>. Can be computed in compile time.
    constexpr bool IsBigEndian() { return std::endian::native == std::endian::big; }

    /// Reads 2 bytes from @p bytes and interprets it as a @p u16 in @b big endian.
    /// That is, bytes @code 12 34@endcode will get read as the number @p 0x1234 .
    /// @warning Undefined behavior if @p bytes+2 lies outside the valid memory region,
    /// even if the result is masked out. For the safer version, see @p ReadZeroExtU16Big .
    u16 ReadU16Big(const void* bytes);
    /// Reads 4 bytes from @p bytes and interprets it as a @p u32 in @b big endian.
    /// That is, bytes @code 12 34 56 78@endcode will get read as the number @p 0x12345678 .
    /// @warning Undefined behavior if @p bytes+4 lies outside the valid memory region,
    /// even if the result is masked out. For the safer version, see @p ReadZeroExtU32Big .
    u32 ReadU32Big(const void* bytes);
    /// Reads 8 bytes from @p bytes and interprets it as a @p u64 in @b big endian.
    /// That is, bytes @code 01 23 45 67 89 AB CD EF@endcode will get read as the number @p 0x01234567'89ABCDEF .
    /// @warning Undefined behavior if @p bytes+8 lies outside the valid memory region,
    /// even if the result is masked out. For the safer version, see @p ReadZeroExtU64Big .
    u64 ReadU64Big(const void* bytes);

    /// Reads 2 bytes from @p bytes and interprets it as a @p u16 in @b little endian.
    /// That is, bytes @code 12 34@endcode will get read as the number @p 0x3412 .
    /// @warning Undefined behavior if @p bytes+2 lies outside the valid memory region,
    /// even if the result is masked out. For the safer version, see @p ReadZeroExtU16 .
    u16 ReadU16(const void* bytes);
    /// Reads 4 bytes from @p bytes and interprets it as a @p u32 in @b little endian.
    /// That is, bytes @code 12 34 56 78@endcode will get read as the number @p 0x78563412 .
    /// @warning Undefined behavior if @p bytes+4 lies outside the valid memory region,
    /// even if the result is masked out. For the safer version, see @p ReadZeroExtU32 .
    u32 ReadU32(const void* bytes);
    /// Reads 8 bytes from @p bytes and interprets it as a @p u64 in @b little endian.
    /// That is, bytes @code 01 23 45 67 89 AB CD EF@endcode will get read as the number @p 0xEFCDAB89'67452310 .
    /// @warning Undefined behavior if @p bytes+8 lies outside the valid memory region,
    /// even if the result is masked out. For the safer version, see @p ReadZeroExtU64 .
    u64 ReadU64(const void* bytes);

    /// Reads 2 bytes from @p bytes and interprets it as a @p u16 in the system's @b native endian.
    /// @warning Undefined behavior if @p bytes+2 lies outside the valid memory region.
    u16 ReadU16Native(const void* bytes);
    /// Reads 4 bytes from @p bytes and interprets it as a @p u32 in the system's @b native endian.
    /// @warning Undefined behavior if @p bytes+4 lies outside the valid memory region.
    u32 ReadU32Native(const void* bytes);
    /// Reads 8 bytes from @p bytes and interprets it as a @p u64 in the system's @b native endian.
    /// @warning Undefined behavior if @p bytes+8 lies outside the valid memory region.
    u64 ReadU64Native(const void* bytes);

    /// Tries and reads 2 bytes from @p bytes and interprets it as a @p u16 in @b little endian,
    /// while filling the rest of the integer in zeros if @p len is not enough to cover the whole 2 bytes.
    u16 ReadZeroExtU16(const void* bytes, usize len);
    /// Tries and reads 4 bytes from @p bytes and interprets it as a @p u32 in @b little endian,
    /// while filling the rest of the integer in zeros if @p len is not enough to cover the whole 4 bytes.
    u32 ReadZeroExtU32(const void* bytes, usize len);
    /// Tries and reads 8 bytes from @p bytes and interprets it as a @p u64 in @b little endian,
    /// while filling the rest of the integer in zeros if @p len is not enough to cover the whole 8 bytes.
    u64 ReadZeroExtU64(const void* bytes, usize len);

    /// Tries and reads 2 bytes from @p bytes and interprets it as a @p u16 in @b big endian,
    /// while filling the rest of the integer in zeros if @p len is not enough to cover the whole 2 bytes.
    u16 ReadZeroExtU16Big(const void* bytes, usize len);
    /// Tries and reads 4 bytes from @p bytes and interprets it as a @p u32 in @b big endian,
    /// while filling the rest of the integer in zeros if @p len is not enough to cover the whole 4 bytes.
    u32 ReadZeroExtU32Big(const void* bytes, usize len);
    /// Tries and reads 8 bytes from @p bytes and interprets it as a @p u64 in @b big endian,
    /// while filling the rest of the integer in zeros if @p len is not enough to cover the whole 8 bytes.
    u64 ReadZeroExtU64Big(const void* bytes, usize len);

    /// Writes 2 bytes from the @p u16 value @p x @p out in @b big endian.
    /// That is, the number @p 0x1234 will be written as the bytes @code 12 34@endcode.
    /// @warning Undefined behavior if @p out+2 lies outside the valid memory region.
    void WriteU16Big(u16 x, void* out);
    /// Writes 4 bytes from the @p u32 value @p x @p out in @b big endian.
    /// That is, the number @p 0x12345678 will be written as the bytes @code 12 34 56 78@endcode.
    /// @warning Undefined behavior if @p out+4 lies outside the valid memory region.
    void WriteU32Big(u32 x, void* out);
    /// Writes 8 bytes from the @p u64 value @p x @p out in @b big endian.
    /// That is, the number @p 0x01234567'89ABCDEF will be written as the bytes @code 01 23 45 67 89 AB CD EF@endcode.
    /// @warning Undefined behavior if @p out+8 lies outside the valid memory region.
    void WriteU64Big(u64 x, void* out);

    /// Writes 2 bytes from the @p u16 value @p x @p out in @b little endian.
    /// That is, the number @p 0x1234 will be written as the bytes @code 34 12@endcode.
    /// @warning Undefined behavior if @p out+2 lies outside the valid memory region.
    void WriteU16(u16 x, void* out);
    /// Writes 4 bytes from the @p u32 value @p x @p out in @b little endian.
    /// That is, the number @p 0x12345678 will be written as the bytes @code 78 56 34 12@endcode.
    /// @warning Undefined behavior if @p out+4 lies outside the valid memory region.
    void WriteU32(u32 x, void* out);
    /// Writes 8 bytes from the @p u64 value @p x @p out in @b little endian.
    /// That is, the number @p 0x01234567'89ABCDEF will be written as the bytes @code EF CD AB 89 67 45 23 10@endcode.
    /// @warning Undefined behavior if @p out+8 lies outside the valid memory region.
    void WriteU64(u64 x, void* out);

    /// Writes 2 bytes from the @p u16 value @p x @p out in the system's @b native endian.
    /// @warning Undefined behavior if @p out+2 lies outside the valid memory region.
    void WriteU16Native(u16 x, void* out);
    /// Writes 4 bytes from the @p u32 value @p x @p out in the system's @b native endian.
    /// @warning Undefined behavior if @p out+4 lies outside the valid memory region.
    void WriteU32Native(u32 x, void* out);
    /// Writes 8 bytes from the @p u64 value @p x @p out in the system's @b native endian.
    /// @warning Undefined behavior if @p out+8 lies outside the valid memory region.
    void WriteU64Native(u64 x, void* out);

    /// Swaps between endiannes for the @p u16 value @p x,
    /// that is, @p 0x1234 gets swapped to @p 0x3412 .
    u16 ByteSwap16(u16 x);
    /// Swaps between endiannes for the @p u32 value @p x,
    /// that is, @p 0x12345678 gets swapped to @p 0x78563412 .
    u32 ByteSwap32(u32 x);
    /// Swaps between endiannes for the @p u64 value @p x,
    /// that is, @p 0x01234567'89ABCDEF gets swapped to @p 0xEFCDAB89'67452310 .
    u64 ByteSwap64(u64 x);

    /// Copies @p bytes number of bytes from @p in and writes them to @p out .
    /// @note If one is @b certain that the input and output memory <b>do not</b> overlap,
    /// one may use @p MemCopyNoOverlap instead.
    /// @warning Will yield undesired results if @p out is in the range @code [in, in+bytes]@endcode;
    /// what you want is @b reverse version, @p MemCopyRev, instead.
    void MemCopy(void* out, const void* in, usize bytes);

    /// Copies @p bytes number of bytes from @p in and writes them to @p out;
    /// may be faster than @p MemCopy since the function @b assumes that the input and output pointers @b don't overlap.
    /// @warning Undefined behavior on overlapping pointer ranges.
    void MemCopyNoOverlap(void* __restrict__ out, const void* __restrict__ in, usize bytes);

    /// Copies @p bytes number of bytes from @p in and writes them to @p out <b>in reverse</b>.
    /// @warning Will yield undesired results if @p in is in the range @code [out, out+bytes]@endcode;
    /// what you want is the @b forward version, @p MemCopy, instead.
    void MemCopyRev(void* out, const void* in, usize bytes);

    /// Sets all @p bytes number of bytes in @p dest to the constant @p value .
    void MemSet(void* dest, byte value, usize bytes);

    /// Swaps @p bytes number of bytes in memory buffer @p a and @p b .
    /// @warning Undefined behavior if the memory regions @p a and @p b overlap.
    void MemSwap(void* __restrict__ a, void* __restrict__ b, usize bytes);

    /// Reverses the bytes in the memory buffer @p arr with length @p bytes .
    void MemReverse(void* arr, usize bytes);

    /// Fills a range of @p T given by the memory buffer @p out and size @p count by copying the value @p fill .
    template <class T> constexpr void RangeSet(T* out, const T& fill, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = fill;
    }

    /// Copies the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// @note If one is @b certain that the input and output memory <b>do not</b> overlap,
    /// one may use @p RangeCopyNoOverlap instead.
    /// @warning Will yield undesired results if @p out is in the range @code [in, in+bytes]@endcode;
    /// what you want is @b reverse version, @p RangeCopyRev, instead.
    template <class T> constexpr void RangeCopy(T* out, const T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = in[i];
    }

    /// Copies the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// may be faster than @p RangeCopy since the function @b assumes that the input and output pointers @b don't overlap.
    /// @warning Undefined behavior on overlapping pointer ranges.
    template <class T> constexpr void RangeCopyNoOverlap(T* __restrict__ out, const T* __restrict__ in, usize count) {
        RangeCopy(out, in, count);
    }

    /// Copies the range of values with length @p count from the buffer @p in to the buffer @p out <b>in reverse</b>.
    /// @warning Will yield undesired results if @p in is in the range @code [out, out+bytes]@endcode;
    /// what you want is the @b forward version, @p RangeCopy, instead.
    template <class T> constexpr void RangeCopyRev(T* out, const T* in, usize count) {
        for (usize i = count; i --> 0; )
            out[i] = in[i];
    }

    /// Moves the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// @note If one is @b certain that the input and output memory <b>do not</b> overlap,
    /// one may use @p RangeMoveNoOverlap instead.
    /// @warning Will yield undesired results if @p out is in the range @code [in, in+bytes]@endcode;
    /// what you want is @b reverse version, @p RangeMoveRev, instead.
    template <class T> constexpr void RangeMove(T* out, T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            out[i] = std::move(in[i]);
    }

    /// Moves the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// may be faster than @p RangeMove since the function @b assumes that the input and output pointers @b don't overlap.
    /// @warning Undefined behavior on overlapping pointer ranges.
    template <class T> constexpr void RangeMoveNoOverlap(T* __restrict__ out, T* __restrict__ in, usize count) {
        RangeMove(out, in, count);
    }

    /// Moves the range of values with length @p count from the buffer @p in to the buffer @p out <b>in reverse</b>.
    /// @warning Will yield undesired results if @p in is in the range @code [out, out+bytes]@endcode;
    /// what you want is the @b forward version, @p RangeMove, instead.
    template <class T> constexpr void RangeMoveRev(T* out, T* in, usize count) {
        for (usize i = count; i --> 0;)
            out[i] = std::move(in[i]);
    }

    /// Copy-Constructs a range of @p T given by the memory buffer @p out and size @p count by copying the value @p fill .
    template <class T> constexpr void RangeConstruct(T* out, const T& fill, usize count) {
        for (usize i = 0; i < count; ++i)
            Memory::ConstructCopyAt(&out[i], fill);
    }

    /// Copy-Constructs the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// @note If one is @b certain that the input and output memory <b>do not</b> overlap,
    /// one may use @p RangeConstructCopyNoOverlap instead.
    /// @warning Will yield undesired results if @p out is in the range @code [in, in+bytes]@endcode;
    /// what you want is @b reverse version, @p RangeConstructCopyRev, instead.
    template <class T> constexpr void RangeConstructCopy(T* out, const T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            Memory::ConstructCopyAt(&out[i], in[i]);
    }

    /// Copy-Constructs the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// may be faster than @p RangeCopy since the function @b assumes that the input and output pointers @b don't overlap.
    /// @warning Undefined behavior on overlapping pointer ranges.
    template <class T> constexpr void RangeConstructCopyNoOverlap(T* __restrict__ out, const T* __restrict__ in, usize count) {
        RangeConstructCopy(out, in, count);
    }

    /// Copy-Constructs the range of values with length @p count from the buffer @p in to the buffer @p out <b>in reverse</b>.
    /// @warning Will yield undesired results if @p in is in the range @code [out, out+bytes]@endcode;
    /// what you want is the @b forward version, @p RangeConstructCopy, instead.
    template <class T> constexpr void RangeConstructCopyRev(T* out, const T* in, usize count) {
        for (usize i = count; i --> 0; )
            Memory::ConstructCopyAt(&out[i], in[i]);
    }

    /// Move-Constructs the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// @note If one is @b certain that the input and output memory <b>do not</b> overlap,
    /// one may use @p RangeConstructMoveNoOverlap instead.
    /// @warning Will yield undesired results if @p out is in the range @code [in, in+bytes]@endcode;
    /// what you want is @b reverse version, @p RangeConstructMoveRev, instead.
    template <class T> constexpr void RangeConstructMove(T* out, T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            Memory::ConstructMoveAt(&out[i], std::move(in[i]));
    }

    /// Move-Constructs the range of values with length @p count from the buffer @p in to the buffer @p out .
    /// may be faster than @p RangeMove since the function @b assumes that the input and output pointers @b don't overlap.
    /// @warning Undefined behavior on overlapping pointer ranges.
    // WARNING: undefined behavior on overlapping pointer ranges
    template <class T> constexpr void RangeConstructMoveNoOverlap(T* __restrict__ out, T* __restrict__ in, usize count) {
        RangeConstructMove(out, in, count);
    }

    /// Move-Constructs the range of values with length @p count from the buffer @p in to the buffer @p out <b>in reverse</b>.
    /// @warning Will yield undesired results if @p in is in the range @code [out, out+bytes]@endcode;
    /// what you want is the @b forward version, @p RangeConstructMove, instead.
    template <class T> constexpr void RangeConstructMoveRev(T* out, T* in, usize count) {
        for (usize i = count; i --> 0;)
            Memory::ConstructMoveAt(&out[i], std::move(in[i]));
    }

    /// Swaps the @p T elements in the memory buffers @p a and @p b with size @p count .
    /// @note If one is @b certain that the input and output memory <b>do not</b> overlap,
    /// one may use @p RangeSwapNoOverlap instead.
    template <class T> constexpr void RangeSwap(T* out, T* in, usize count) {
        for (usize i = 0; i < count; ++i)
            std::swap(out[i], in[i]);
    }

    /// Swaps the @p T elements in the memory buffers @p a and @p b with size @p count .
    /// @warning Undefined behavior if the memory regions @p a and @p b overlap.
    template <class T> constexpr void RangeSwapNoOverlap(T* __restrict__ out, T* __restrict__ in, usize count) {
        RangeSwap(out, in, count);
    }

    /// Destroys the range of values .
    template <class T> constexpr void RangeDestruct(T* data, usize count) {
        T* dataEnd = data + count;
        for (; data < dataEnd; ++data) data->~T();
    }

#define Q_GETTER_MUT(FN, ...) (decltype(this->FN(__VA_ARGS__)))(Memory::AsConstPtr(this))->FN(__VA_ARGS__)
    /// Defines the mutable version of a getter function on a class to reduce boilerplate.
    /// Does not write the definition for you, only the implementation.
    ///
    /// @b Example: @code
    /// const T& GetNthElement(int i) const { ... }
    /// T& GetNthElement(int i) { return QGetterMut$(GetNthElement, i); }
    /// @endcode
#define QGetterMut$ Q_GETTER_MUT

    // internal use; only used for casting
    template <class T> T* _Alloca(void* ptr) { return (T*)ptr; }

#define Q_ALLOCA(T, SIZE) _Alloca<T>(_alloca(sizeof(T) * (SIZE)))

    /// Allocates an array of type @p T with size @p SIZE on the stack,
    /// in which the length can be specified in runtime. Uses @p _alloca under the hood.
    ///
    /// The memory will be automatically popped off the stack,
    /// so there's no need to free the memory allocated using this function.
    ///
    /// @b Example:
    /// @code
    /// int size = SomeFunctionDoneInRuntime();
    /// int* array = Memory::QAlloca$(int, size);
    /// @endcode
    /// @note The returned memory is <b>uninitialized and not destroyed</b> when out of scope by default.
    /// Callers must construct and destroy the values themselves.
    /// @warning May result in UB if too much memory is allocated onto the stack, causing a stack overflow.
#define QAlloca$(T, SIZE) Q_ALLOCA(T, SIZE)

#define Q_WITH(EXPR, VNAME, ...) if (auto VNAME = EXPR; true)

    /// Starts a scope which controls the lifetime of some expression.
    /// Commonly used with RAII objects for automatically opening/closing certain scopes.
    ///
    /// Has two main uses: @p QWith$(expr) creates an unnamed value that will live in the scope,
    /// while @code QWith$(name, expr)@endcode creates a named value that can be accessed later.
    ///
    /// @b Example:
    /// @code
    /// QWith$(Lock(x)) {
    ///     AcquireResource(x);
    /// } // lock is cleaned up after
    /// @endcode
    /// @code
    /// QWith$(f, OpenFile("example.txt")) {
    ///     f.Read();
    /// } // f is cleaned up after this
    /// @endcode
#define QWith$(V, ...) Q_UNARY(Q_WITH Q_EMPTY() (__VA_ARGS__ __VA_OPT__(,) V, Q_CAT(_, Q_COUNTER())))
}

namespace Quasi {
    /// The string literal operator which takes in an ASCII string @p s
    /// and returns its bytes as an integer (BE), that is, @p "Hi"_u16 returns @p 0x4869 .
    /// Can be computed at compile time.
    /// @note Ignores any characters after the first two and fills in zeros if the string isn't long enough.
    constexpr u16 operator ""_u16(const char* s, usize n) {
        return n == 0 ? 0 :
               n == 1 ? (u8)s[0] : (u32)(u8)s[0] << 8  | (u32)(u8)s[1];
    }

    /// The string literal operator which takes in an ASCII string @p s
    /// and returns its bytes as an integer (BE), that is, @p "blue"_u32 returns @p 0x626C7565 .
    /// Can be computed at compile time.
    /// @note Ignores any characters after the first four and fills in zeros if the string isn't long enough.
    constexpr u32 operator ""_u32(const char* s, usize n) {
        switch (n) {
            case 0:  return 0;
            case 1:  return (u32)(u8)s[0];
            case 2:  return (u32)(u8)s[0] << 8  | (u32)(u8)s[1];
            case 3:  return (u32)(u8)s[0] << 16 | (u32)(u8)s[1] << 8 | (u32)(u8)s[2];
            default: return (u32)(u8)s[0] << 24 | (u32)(u8)s[1] << 16 | (u32)(u8)s[2] << 8  | (u32)(u8)s[3];
        }
    }

    /// The string literal operator which takes in an ASCII string @p s
    /// and returns its bytes as an integer (BE), that is, @p "Shift"_u64 returns @p 0x5368696674 .
    /// Can be computed at compile time.
    /// @note Ignores any characters after the first eight and fills in zeros if the string isn't long enough.
    constexpr u64 operator ""_u64(const char* s, usize n) {
        switch (n) {
            case 0:  return 0;
            case 1:  return (u64)(u8)s[0];
            case 2:  return (u64)(u8)s[0] << 8  | (u64)(u8)s[1];
            case 3:  return (u64)(u8)s[0] << 16 | (u64)(u8)s[1] << 8  | (u64)(u8)s[2];
            case 4:  return (u64)(u8)s[0] << 24 | (u64)(u8)s[1] << 16 | (u64)(u8)s[2] << 8  | (u64)(u8)s[3];
            case 5:  return (u64)(u8)s[0] << 32 | (u64)(u8)s[1] << 24 | (u64)(u8)s[2] << 16 | (u64)(u8)s[3] << 8  | (u64)(u8)s[4];
            case 6:  return (u64)(u8)s[0] << 40 | (u64)(u8)s[1] << 32 | (u64)(u8)s[2] << 24 | (u64)(u8)s[3] << 16 | (u64)(u8)s[4] << 8  | (u64)(u8)s[5];
            case 7:  return (u64)(u8)s[0] << 48 | (u64)(u8)s[1] << 40 | (u64)(u8)s[2] << 32 | (u64)(u8)s[3] << 24 | (u64)(u8)s[4] << 16 | (u64)(u8)s[5] << 8 | (u64)(u8)s[6];
            default: return (u64)(u8)s[0] << 56 | (u64)(u8)s[1] << 48 | (u64)(u8)s[2] << 40 | (u64)(u8)s[3] << 32 | (u64)(u8)s[4] << 24 | (u64)(u8)s[5] << 16 | (u64)(u8)s[6] <<  8 | (u64)(u8)s[7];
        }
    }
}