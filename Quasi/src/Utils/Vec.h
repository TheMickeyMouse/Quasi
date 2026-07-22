#pragma once
#include "Func.h"
#include "Comparison.h"
#include "Option.h"
#include "Span.h"

namespace Quasi {
    namespace Vecs {
        /// @internal
        /// @brief Calculates the next capacity of a vector after a resize occurs, by default grows with powers of 2.
        inline usize GrowCap(usize cap) { return std::max<usize>(cap * 2, 2); }
        /// @brief Creates a Vec<T> from the values specified.
        template <class T> Vec<T> FromIList(IList<T> list) { return Vec<T>::FromIList(list); }
        /// @brief Creates a Vec<T> from a C-style array by copying the values.
        template <class T, usize N> Vec<T> New(const T (&arr)[N]) { return Vec<T>::New(arr); }
        /// @brief Creates a Vec<T> from a C-style array, moving the values instead of copying them.
        template <class T, usize N> Vec<T> New(T (&&arr)[N]) { return Vec<T>::New(std::move(arr)); }

        /// @brief Creates Vec<T> from a simple range
        /// @param start the number to start counting from
        /// @param end the number to stop at
        /// @return a vector of the range [@p start, @p end)
        template <Numeric N>
        Vec<N> Range(N start, N end) {
            if (end < start) return {};

            Vec<N> range = Vec<N>::WithCap((usize)(end - start));
            for (N x = 0; x < end; x += N(1)) { range.Push(x); }
            return range;
        }

        /// @brief Creates a Vec<T> from an arithmetic progression @n
        /// Examples: @code
        /// Vecs::Range(1, 5, 1);   // returns { 1, 2, 3, 4 }
        /// Vecs::Range(1, 11, 3);  // returns { 1, 4, 7, 10 }
        /// Vecs::Range(5, -2, -2); // returns { 5, 3, 1, -1 }
        /// @endcode
        /// @param start the number to start counting from
        /// @param end the number to stop at
        /// @param step the step size of the range
        /// @return an arithmetic progression { @p start, @p start + @p step, ..., @p end }
        template <Numeric N>
        Vec<N> Range(N start, N end, N step) {
            // bad range!
            if (step == 0 || (end < start) == (step < 0)) return {};

            Vec<N> range = Vec<N>::WithCap((usize)((end - start) / step));
            if (step < 0) {
                for (N x = start; x > end; x -= step) range.Push(x);
            } else {
                for (N x = start; x <= end; x += step) range.Push(x);
            }
            return range;
        }
    }

    /// @brief A data structure that stores a continuous collection of an arbitrary amount of @p T s on the heap.
    /// @tparam T the element type to store
    template <class T>
    struct Vec : IContinuous<T, Vec<T>> {
        friend ICollection<T, Vec>;
        friend IContinuous<T, Vec>;
    private:
        /// @brief the data
        T* data = nullptr;
        usize size = 0, capacity = 0;

        Vec(T* dat, usize s, usize cap) : data(dat), size(s), capacity(cap) {}
    public:
        Vec() = default;
        ~Vec() { Memory::RangeDestruct(data, size); Memory::FreeRaw(data); /* dont destruct after size, capacity is uninit'ed */ }
        Vec(Vec&& v) noexcept : data(v.data), size(v.size), capacity(v.capacity) { v.PretendClear(); }
        Vec& operator=(Vec&& v) noexcept { this->~Vec(); data = v.data; size = v.size; capacity = v.capacity; v.PretendClear(); return *this; }
    public:
        /// @brief Creates an empty Vec<T>.
        static Vec Empty() { return {}; }
        /// @brief Creates a Vec<T> from a C-style array by moving the values.
        template <usize N> static Vec New(T (&&arr)[N]) { return MoveNew(Span<T>::FromArray(arr)); }
        /// @brief Creates a Vec<T> from a C-style array by copying the values.
        template <usize N> static Vec New(const T (&arr)[N]) { return New(Span<const T>::FromArray(arr)); }
        /// @brief Creates a Vec<T> from a Span<const T> by copy the values.
        static Vec New(Span<const T> elms) {
            Vec v = WithCap(elms.Length());
            Memory::RangeConstructCopyNoOverlap(v.Data(), elms.Data(), elms.Length());
            v.size = elms.Length();
            return v;
        }
        /// @brief Creates a Vec<T> from an initializer list by copying the values.
        static Vec FromIList(IList<T> elms) { return New(Spans::FromIList(elms)); }
        /// @brief Creates a Vec<T> from a Span<T> by moving the values.
        /// @warning After calling, @p movElms is invalid.
        static Vec MoveNew(Span<T> movElms) {
            Vec v = WithCap(movElms.Length());
            Memory::RangeConstructMoveNoOverlap(v.Data(), movElms.Data(), movElms.Length());
            v.size = movElms.Length();
            return v;
        }

        /// @brief Creates a Vec<T> from an ArrayBox<T>, using its memory as the contents of the vector.
        static Vec FromBox(ArrayBox<T>& box) { return { box.Release(), box.Length(), box.Length() }; }

        /// @brief Creates a Vec<T> with pre-allocated memory, capable of storing @p cap elements but zero size.
        static Vec WithCap (usize cap)  { return { cap  ? AllocateBuffer(cap)  : nullptr, 0, cap }; }
        /// @brief Composes a Vec<T> out of its fundamental components: the data pointer, a size and capacity.
        static Vec Compose(T* data, usize size, usize cap) { return { data, size, cap }; }
        /// @brief Yields out its members variables: data, size and capacity, while clearing itself in the process.
        /// @warning the return value memory pointer is expected to be @b managed and @b freed by the user.
        [[nodiscard]] Tuple<T*, usize, usize> Decompose() { const usize prsize = size, prcap = capacity; return { Release(), prsize, prcap }; }

        /// @brief Creates a clone of itself. Useful for duplicating Vec<T>s for data processing.
        Vec Clone() const { return New(this->AsSpan()); }
    protected:
        usize LengthImpl() const { return size; }
        T* DataImpl() { return data; }
        const T* DataImpl() const { return data; }
    private:
        /// @internal
        /// @brief Sets the vector to be in the 'empty state'
        /// @warning This does @b not free the memory; it merely sets the state to @b be empty.
        /// Free the data before calling.
        void PretendClear() { data = nullptr; size = 0; capacity = 0; }
    public:
        /// @brief Clears all the elements from the vector.
        void Clear() { Memory::RangeDestruct(data, size); size = 0; }
        /// @brief Releases the underlying data held by the vector as a pointer.
        /// @warning The user is expected to @b manage and @b free the memory themselves.
        [[nodiscard]] T* Release() { T* const d = data; PretendClear(); return d; }

        /// @brief Returns the maximum amount of elements the vector can hold without reallocating more memory.
        usize Capacity()   const { return capacity; }
        /// @brief Returns whether or not the vector is empty. Equivalent to @code (vec.Length() == 0) @endcode
        bool IsEmpty()     const { return size == 0; }
        explicit operator bool() const { return !IsEmpty(); }
    private:
        /// @internal
        /// @brief Moves & grows the memory to the next chunk;
        /// Prefer this since it does calculations for the next capacity.
        void ReserveNext() { AllocToNew(Vecs::GrowCap(capacity)); }
        /// @internal
        /// @brief Moves the contents of the vector to a specified memory chunk.
        /// @param buffer the new memory buffer to move to
        void MoveBuffer(T* buffer) {
            Vec::ShiftValues(buffer, data, size);
            Memory::FreeRaw(data);
            data = buffer;
        }

        /// @internal
        /// @brief Allocates & moves the element data to a memory buffer with a specified size.
        /// @param buffSize the new memory size
        void AllocToNew(usize buffSize) { MoveBuffer(AllocateBuffer(buffSize)); capacity = buffSize; }

        /// @internal
        /// @brief Moves values to an @b uninitialized memory region, while @b destructing the original values.
        /// @param newDest the destination for the elements
        /// @param in the input element data
        /// @param num the number of elements to move
        static void ShiftValues(T* newDest, T* in, usize num) {
            for (usize i = 0; i < num; ++i) {
                Memory::ConstructAt(&newDest[i], std::move(in[i]));
                in[i].~T();
            }
        }
        /// @internal
        /// @brief Moves values to an @b uninitialized memory region in @b reverse direction, while @b destructing the original values.
        /// @param newDest the destination for the elements
        /// @param in the input element data
        /// @param num the number of elements to move
        static void ShiftValuesRev(T* newDest, T* in, usize num) {
            for (usize i = num; i --> 0; ) {
                Memory::ConstructAt(&newDest[i], std::move(in[i]));
                in[i].~T();
            }
        }
    public:
        /// @internal
        /// @brief Allocates a buffer of size @p size.
        static T* AllocateBuffer(usize size) { return Memory::AllocateArrayUninit<T>(size); }
        /// @internal
        /// @brief Checks whether a vector can hold an extra `amount` elements without needing to resize.
        bool CanFit(usize amount) const { return size + amount <= capacity; }
        /// @internal
        /// @brief Ensures the vector can now hold an extra `amount` elements. May resize if necessary.
        void TryGrow(usize amount) { if (!CanFit(amount)) Reserve(amount); }

        /// @brief Reserves memory if needed such that at least @p add
        /// more additional elements can be inserted in the given Vec<T>.
        /// It is guaranteed that @code vec.Capacity() >= vec.Length() + add @endcode after calling.
        /// @param add the amount of additional elements to reserve for
        void Reserve(usize add) {
            if (size + add <= capacity) return;
            if (size + add < Vecs::GrowCap(capacity))
                ReserveNext();
            else
                ReserveExact(add);
        }
        /// @brief Reserves memory if needed such that @b exactly @p add
        /// more additional elements can be inserted in the given Vec<T>.
        /// It is guaranteed that @code vec.Capacity() == vec.Length() + add @endcode after calling (unless capacity was greater to begin with).
        /// @param add the amount of additional elements to reserve for
        /// @warning This may cause inefficiencies with multiple @p ReserveExact(...) calls.
        /// Unless you are @b certain that number of insertions after this @b will @b not exceed @p add,
        /// prefer using @p Reserve(...) instead.
        void ReserveExact(usize add) { if (size + add > capacity) AllocToNew(size + add); }

        /// @brief Resizes the vector, shrinking/filling empty values if necessary.
        /// @param len the new size of the vector
        /// @param value the value to fill empty values with (if necessary)
        void Resize(usize len, const T& value = {}) {
            if (len <= size) Truncate(len);
            else { Reserve(len); Memory::RangeSet(&data[size], value, len - size); size = len; }
        }

        /// @brief Resizes the vector, shrinking/constructing new values with @p factory if necessary.
        /// @param len the new size of the vector
        /// @param factory the constructor/factory that will be invoked when filling new values
        void ResizeWith(usize len, Fn<T> auto&& factory) {
            if (len <= size) Truncate(len);
            else {
                Reserve(len);
                for (usize i = size; i < len; ++i) Memory::ConstructMoveAt(&data[i], factory());
                size = len;
            }
        }
        /// @brief Resizes the vector, shrinking/filling empty values with the default @p T value if necessary.
        /// @param len the new size of the vector
        void ResizeDefault(usize len) { return ResizeWith(len, Combinate::Constructor<T> {}); }
        /// @brief Resizes the vector to include @p extra more elements, filling those extra elements with @p value.
        /// @param extra the additional number of elements to add to the vector
        /// @param value the value to fill new elements with
        void ResizeExtra(usize extra, const T& value = {}) { return Resize(size + extra, value); }
        /// @brief Resizes the vector to include @p extra more elements, filling those extra elements by calling @p factory.
        /// @param extra the additional number of elements to add to the vector
        /// @param factory the constructor/factory to fill new elements with
        void ResizeExtraWith(usize extra, Fn<T> auto&& factory) { return ResizeWith(size + extra, factory); }
        /// @brief Resizes the vector to include @p extra more elements, filling those extra elements with the default @p T value.
        /// @param extra the additional number of elements to add to the vector
        void ResizeExtraDefault(usize extra) { return ResizeExtraWith(extra, Combinate::Constructor<T> {}); }

        /// @brief Shrinks the vector's capacity as much as possible by allocating to a new memory chunk of size @p this->Length().
        void ShrinkToFit() { AllocToNew(size); }
        /// @brief Tries to shrink the vector's capacity to @p minimum while preserving all elements by allocating to a new memory chunk.
        /// @note This is equivalent to @code Truncate(std::max(size, minimum)) @endcode.
        void ShrinkTo(usize minimum) { AllocToNew(std::max(size, minimum)); }

        /// @brief Creates an @p ArrayBox<T> that owns this vector's original memory and with size @p this->Length().
        [[nodiscard]] ArrayBox<T> IntoBox()      { return ArrayBox<T>::Own(Release(), size); }
        /// @brief Creates an @p ArrayBox<T> that owns this vector's original memory and with size @p this->Capacity().
        /// @warning Does not waste any memory but <b>may contain uninitialized values</b>.
        [[nodiscard]] ArrayBox<T> IntoBoxWhole() { return ArrayBox<T>::Own(Release(), capacity); } // releases

        /// @brief Returns the memory chunk past the length that is not yet used by the vector in the form of a @p Span<T>.
        /// @warning This may hold <b>uninitialized memory</b>.
        Span<T> SpareCapacity() { return { data + size, capacity - size }; }

        /// @brief Returns the whole memory chunk allocated by the vector.
        /// @warning May contain <b>uninitialized memory</b>.
        Span<T> EntireAsSpan() { return { data, capacity }; }
        /// @brief Returns the whole memory chunk allocated by the vector.
        /// @warning May contain <b>uninitialized memory</b>.
        Span<const T> EntireAsSpan() const { return { data, capacity }; }

        /// @brief Truncates the vector to length @p len, destroying elements in the process.
        /// @param len  the new length of the vector
        /// @note if @code len >= size @endcode, this has no affect.
        void Truncate(usize len) { if (len < size) { Memory::RangeDestruct(&data[len], size - len); size = len; } }
        /// @brief Shortens the vector by @p amount, destroying elements in the process.
        /// This is equivalent to @code Truncate(Length() - amount) @endcode
        /// @param amount the amount to shorten the vector by
        /// @note if @code amount >= len @endcode, the vector is effectively cleared.
        void Shorten(usize amount) { amount = std::min(amount, size); Memory::RangeDestruct(data, size - amount); size -= amount; }

        /// @brief Pops the last element of a vector.
        /// @warning If the vector is empty, this is UB.
        /// Use @p TryPop() instead unless you are @b certain that the vector is @b not empty
        void Pop() { --size; data[size].~T(); }
        /// @brief Pops the last element of a vector and returns it.
        /// @returns the last element of the vector that was just removed
        /// @warning If the vector is empty, this is UB.
        /// Use @p TryTake() instead unless you are @b certain that the vector is @b not empty
        T Take() { T last = std::move(data[--size]); data[size].~T(); return last; }
        /// @brief Pops the element at @p index of a vector.
        /// @param index the index of the element that should be popped
        /// @warning If @p index >= @p Length(), this is UB.
        /// Use @p TryPop(...) instead unless you are @b certain that the vector is @b not empty
        void Pop(usize index) {
            data[index].~T();
            Vec::ShiftValues(&data[index], &data[index + 1], size - index - 1);
        }
        /// @brief Pops the element at @p index of a vector and returns it.
        /// @param index the index of the element that should be popped
        /// @warning If @p index >= @p Length(), this is UB.
        /// Use @p TryTake(...) instead unless you are @b certain that the vector is @b not empty
        T Take(usize index) {
            T out = std::move(data[index]);
            data.~T();
            Pop(index);
            return out;
        }

        /// @brief Tries to pop the last element in the vector.
        /// @return @p true if the pop was successful (vector wasn't empty), @p false otherwise
        bool      TryPop() { if (IsEmpty()) return false; Pop(); return true; }
        /// @brief Tries to pop the last element in the vector and return it.
        /// @return @p Some(...) if the pop was successful (vector wasn't empty), @p None otherwise
        Option<T> TryTake() { if (IsEmpty()) return nullptr; return Take(); }
        /// @brief Tries to pop the element at @p index in the vector.
        /// @param index the index of the element that should be popped
        /// @return @p true if the pop was successful (@p index is valid), @p false otherwise
        bool      TryPop(usize index) { if (index >= size) return false; Pop(index); return true; }
        /// @brief Tries to pop the element at @p index in the vector and return it.
        /// @param index the index of the element that should be popped
        /// @return @p Some(...) if the pop was successful (@p index is valid), @p None otherwise
        Option<T> TryTake(usize index) { if (index >= size) return nullptr; return Take(index); }

        /// @brief Pushes an object at the back of the vector.
        /// @param obj the object to be appended at the end
        /// @return a reference the object that was just appended
        T& Push(T obj) { TryGrow(1); Memory::ConstructMoveAt(&data[size], std::move(obj)); return data[size++]; }

        /// @brief Inserts an object in the middle of a vector.
        /// After insertion, @p obj will be at index @p idx.
        /// @param obj the object to be inserted
        /// @param idx the index where the object will be inserted
        /// @note if @p idx >= @p Length(), this has the same effect as @code Push(obj) @endcode
        void Insert(T obj, usize idx) {
            if (idx >= size) {
                Push(std::move(obj));
            } else {
                TryGrow(1);
                Vec::ShiftValuesRev(&data[idx + 1], &data[idx], size - idx);
                Memory::ConstructMoveAt(&data[idx], std::move(obj));
                ++size;
            }
        }
        /// @brief Inserts a span of objects by @b copying them to the middle of a vector.
        /// After insertion, @p obj[i] will be at index @p idx+i
        /// @param vals the objects to be copied & inserted into the vector
        /// @param idx the index where the object will be inserted
        /// @note if @p idx >= @p Length(), this has the same effect as @code Extend(vals) @endcode
        void InsertSpan(Span<const T> vals, usize idx) {
            if (vals.IsEmpty()) return;
            if (idx >= size) {
                Extend(vals);
            } else {
                TryGrow(vals.Length());
                Vec::ShiftValuesRev(&data[idx + vals.Length()], &data[idx], size - idx);
                Memory::RangeConstructCopy(&data[idx], vals.Data(), vals.Length());
                size += vals.Length();
            }
        }
        /// @brief Inserts a span of objects by @b moving them to the middle of a vector.
        /// After insertion, @p obj[i] will be at index @p idx+i
        /// @param vals the objects to be moved & inserted into the vector
        /// @param idx the index where the object will be inserted
        /// @note if @p idx >= @p Length(), this has the same effect as @code ExtendMove(vals) @endcode
        /// @warning After calling, @p vals is now invalid.
        void MoveInsertSpan(Span<T> vals, usize idx) {
            if (vals.IsEmpty()) return;
            if (idx >= size) {
                ExtendMove(vals);
            } else {
                TryGrow(vals.Length());
                Vec::ShiftValuesRev(&data[idx + vals.Length()], &data[idx], size - idx);
                Memory::RangeConstructMove(&data[idx], vals.Data(), vals.Length());
                size += vals.Length();
            }
        }

        /// @brief Replaces a range of values [@p start, @p start + @p num) within a Vec<T> with @p values by copying. @n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ 3, 6, 9, 12, 0 });
        /// numbers.Replace(1, 2, Spans::Vals({ -1, -2, -3 }));
        /// // numbers == { 3, -1, -2, -3, 12, 0 }
        /// @endcode
        /// @param start the beginning index to replace
        /// @param num the number of elements to replace
        /// @param values the values to replace the range with
        void Replace(usize start, usize num, Span<const T> values) {
            Memory::RangeDestruct(&data[start], num);
            if (values.Length() > num) { // have to insert
                const usize numToInsert = values.Length() - num;
                TryGrow(numToInsert);
                Vec::ShiftValuesRev(&data[start + values.Length()], &data[start + num], size - start - num);
                size += numToInsert;
            } else if (values.Length() < num) { // have to erase
                Vec::ShiftValues(&data[start + values.Length()], &data[start + num], size - start - num);
                size -= num - values.Length();
            }
            Memory::RangeConstructCopy(&data[start], values.Data(), values.Length());
        }

        /// @brief Replaces a range of values [@p start, @p start + @p num) within a Vec<T> with @p values by moving.
        /// See @p Replace(...) for more info.
        /// @param start the beginning index to replace
        /// @param num the number of elements to replace
        /// @param values the values to replace the range with
        /// @warning After calling, @p values is invalid.
        void ReplaceMove(usize start, usize num, Span<T> values) {
            Memory::RangeDestruct(&data[start], num);
            if (values.Length() > num) { // have to insert
                const usize numToInsert = values.Length() - num;
                TryGrow(numToInsert);
                Vec::ShiftValuesRev(&data[start + values.Length()], &data[start + num], size - start - num);
                size += numToInsert;
            } else if (values.Length() < num) { // have to erase
                Vec::ShiftValues(&data[start + values.Length()], &data[start + num], size - start - num);
                size -= num - values.Length();
            }
            Memory::RangeConstructMove(&data[start], values.Data(), values.Length());
        }

        /// @brief Replaces a range of values [@p start, @p start + @p num) within a Vec<T> with @p values by copying,
        /// and returns the values that were replaced. @n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ 3, 6, 9, 12, 0 });
        /// Vec<int> replaced = numbers.ReplaceAndTake(1, 2, Spans::Vals({ -1, -2, -3 }));
        /// // numbers == { 3, -1, -2, -3, 12, 0 } && replaced == { 6, 9 }
        /// @endcode
        /// @param start the beginning index to replace
        /// @param num the number of elements to replace
        /// @param values the values to replace the range with
        /// @returns the original values that were replaced
        Vec ReplaceAndTake(usize start, usize num, Span<const T> values) {
            Vec original = MoveNew(this->Subspan(start, num));
            Memory::RangeDestruct(&data[start], num);
            if (values.Length() > num) { // have to insert
                const usize numToInsert = values.Length() - num;
                TryGrow(numToInsert);
                Vec::ShiftValuesRev(&data[start + values.Length()], &data[start + num], size - start - num);
                size += numToInsert;
            } else if (values.Length() < num) { // have to erase
                Vec::ShiftValues(&data[start + values.Length()], &data[start + num], size - start - num);
                size -= num;
            }
            Memory::RangeConstructCopy(&data[start], values.Data(), values.Length());
            return original;
        }

        /// @brief Replaces a range of values [@p start, @p start + @p num) within a Vec<T> with @p values by moving.
        /// See @p ReplaceAndTake(...) for more info.
        /// @param start the beginning index to replace
        /// @param num the number of elements to replace
        /// @param values the values to replace the range with
        /// @returns the original values that were replaced
        /// @warning After calling, @p values is invalid.
        Vec ReplaceMoveAndTake(usize start, usize num, Span<T> values) {
            Vec original = MoveNew(this->Subspan(start, num));
            Memory::RangeDestruct(&data[start], num);
            if (values.Length() == num) {
            } else if (values.Length() > num) { // have to insert
                const usize numToInsert = values.Length() - num;
                TryGrow(numToInsert);
                Vec::ShiftValuesRev(&data[start + values.Length()], &data[start + num], size - start - num);
                size += numToInsert;
            } else { // have to erase
                Vec::ShiftValues(&data[start + values.Length()], &data[start + num], size - start - num);
                size -= num;
            }
            Memory::RangeConstructMove(&data[start], values.Data(), values.Length());
            return original;
        }

        /// @brief Appends a list of elements to the end of a vector.
        /// @param items a collection of items to put in the vector
        void Extend(Collection<T> auto&& items) {
            if constexpr (ContinuousAny<decltype(items)>) {
                TryGrow(items.Length());
            }
            for (auto&& i : items) {
                this->Push((decltype(i))i);
            }
        }

        /// @brief Extends the vector with some elements by appending it to the back by moving them.
        /// @param items the items to move into the end of the vector
        void ExtendMove(Span<T> items) {
            TryGrow(items.Length());
            Memory::RangeConstructMove(&data[size], items.Data(), items.Length());
            size += items.Length();
        }
        /// @brief Extends the vector with some elements by appending it to the back by copying them.
        /// @param items the items to copy into the end of the vector
        void Extend(Span<const T> items) {
            TryGrow(items.Length());
            Memory::RangeConstructCopy(&data[size], items.Data(), items.Length());
            size += items.Length();
        }

        /// @brief Keeps elements in a vecotr if and only if the pass a predicate. @n
        /// Example:
        /// @code
        /// Vec<int> numbers = Vecs::New({ 2, 3, 5, 6, 7, 9 });
        /// numbers.Keep([] (int x) { return x % 2 == 1; });
        /// // numbers == { 3, 5, 7, 9 }
        /// @endcode
        /// @param pred the predicate to check the elements with.
        void Keep(Predicate<T> auto&& pred) {
            usize slow = -1; // this will wrap to 0 when added
            for (usize i = 0; i < size; ++i) {
                if (pred(data[i])) {
                    if (++slow == i) continue;
                    Memory::ConstructMoveAt(&data[slow], std::move(data[i]));
                }
                data[i].~T();
            }
            size = slow + 1;
        }

        /// @brief Removes the first instance of a particular item in the vector.
        /// @param item the item to remove
        void Remove(const T& item) { OptionUsize i = this->Find(item); if (i) Pop(*i); }
        /// @brief Removes all <b>contiguously duplicate</b> elements in the vector. @n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ 0, 1, 1, 2, 3, 3, 3, 1 });
        /// numbers.RemoveDups();
        /// // numbers == { 0, 1, 2, 3, 1 }
        /// @endcode
        /// @note If you want to fully remove duplicates, you can first @p Sort() the vector
        /// so that duplicates are lined up together, and then call @p RemoveDups().
        void RemoveDups() { return RemoveDupIf(Cmp::Equals {}); }
        /// @brief Removes all <b>contiguously duplicate</b> elements in the vector,
        /// where duplicate elements are deteremined by whether their keys match.
        /// @param keyf the key function that extracts key values to be compared with
        /// @note See @p RemoveDups()
        void RemoveDupKeys(FnArgs<const T&> auto&& keyf);
        /// @brief Removes all <b>contiguously duplicate</b> elements in the vector,
        /// where duplicate elements are deteremined by whether they pass the equality test given by @p eq.
        /// @param eq the predicate to test whether two elements are equal
        /// @note See @p RemoveDups()
        void RemoveDupIf(EqualPred<T> auto&& eq) {
            if (size <= 1) return;
            usize slow = 0;
            for (usize i = 1; i < size; ++i) {
                if (!eq(data[slow], data[i])) {
                    if (++slow == i) continue;
                    Memory::ConstructMoveAt(&data[slow], std::move(data[i]));
                }
                data[i].~T();
            }
            size = slow + 1;
        }

        /// @brief Erases all elements after (including) the index @p begin.
        /// @param begin the starting index to remove
        /// @note This has the same effect as @p Truncate(...). Has no effect if @p begin is out of bounds
        void Erase(usize begin) {
            Truncate(begin);
        }
        /// @brief Erases elements inside the vector, shrinking it down in the process. @n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ 0, 1, 2, 3, 4 });
        /// numbers.Erase(2, 3);
        /// // numbers == { 0, 1, 4 }
        /// @endcode
        /// @param begin the starting index to remove
        /// @param num the number of elements to remove
        /// @note If either @p begin is out of bounds or @p == 0, this has no effect.
        void Erase(usize begin, usize num) {
            if (begin >= size || num == 0) return;
            Memory::RangeDestruct(&data[begin], num);
            Vec::ShiftValues(&data[begin], &data[begin + num], size - begin - num);
            size -= num;
        }

        /// @brief Erases elements inside the vector, shrinking it down in the process,
        /// while returning the erased elements as a new vector. 8@n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ 0, 1, 2, 3, 4 });
        /// Vec<int> erased = numbers.EraseAndTake(2, 3);
        /// // numbers == { 0, 1, 4 } && erased == { 2, 3 }
        /// @endcode
        /// @param begin the starting index to remove
        /// @param num the number of elements to remove
        Vec EraseAndTake(usize begin, usize num) {
            if (begin >= size || num == 0) return Empty();
            Vec erased = Vec::MoveNew(this->Subspan(begin, num));
            Memory::RangeDestruct(&data[begin], num);
            Vec::ShiftValues(&data[begin], &data[begin + num], size - begin - num);
            size -= num;
            return erased;
        }

        /// @brief Splits a Vec<T> into 2 parts, keeping the front and returning the back in the form of a new vector. @n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ 2, 9, 4, -3, 1 });
        /// Vec<int> trailing = numbers.SplitOff(2);
        /// // numbers == { 2, 9 } && trailing == { 4, -3, 1 }
        /// @endcode
        /// [0, @p index) is preserved, [@p index, Length()) is returned.
        /// @param index the index to split the vector at
        /// @return the tail of the original vector as a new one
        Vec SplitOff(usize index) {
            Vec tail = Vec::MoveNew(this->SubspanMut(index));
            Memory::RangeDestruct(&data[index], size - index);
            size = index;
            return tail;
        }

        /// @brief Flattens the vector by taking every subelement <em>within</em> each element,
        /// and compiling them into a new vector. @n
        /// Example: @code
        /// const auto vec = Vec<Array<int, 3>>::New({ { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { 10, 11, 12 } });
        /// Vec<int> flatVec = vec.Flattened();
        /// // flatVec == { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
        /// @endcode
        /// @returns[Vec<CollectionItem<T>>] a flattened version of the vector
        /* Vec<CollectionItem<T>> */ auto Flattened() const requires CollectionAny<T> {
            Vec<CollectionItem<T>> flattened;
            for (const T& item : *this) {
                for (const auto& sub : item) flattened.Push(sub);
            }
            return flattened;
        }

        /// @brief Flattens the vector by moving every subelement <em>within</em> each element,
        /// and compiling them into a new vector. See @ref Flattened() for more info.
        /// @returns[Vec<CollectionItem<T>>] a flattened version of the vector
        /* Vec<CollectionItem<T>> */ auto IntoFlattened() requires CollectionAny<T> {
            Vec<CollectionItem<T>> flattened;
            for (T& item : *this) {
                for (auto& sub : item.IntoIter()) flattened.Push(std::move(sub));
            }
            return flattened;
        }

        /// @brief Maps the vector into a new vector with the function @p mapper on every element. @n
        /// Example: @code
        /// Vec<int> numbers = Vecs::New({ -2, 1, 0, 4 });
        /// Vec<float> mapped = numbers.MapEach([] (int x) { return std::pow(2.0f, x); });
        /// // mapped == { 0.25f, 2.0f, 1.0f, 16.0f }
        /// @endcode
        /// @tparam F the function map type of @p mapper
        /// @param mapper the mapping function
        /// @return a new vector with each element getting applied by @p mapper
        template <FnArgs<const T&> F>
        Vec<FuncResult<F, const T&>> MapEach(const F& mapper) const {
            using R = FuncResult<F, const T&>;
            Vec<R> result = Vec<R>::WithCap(size);
            for (const T* beg = data; beg != data + size; ++beg) result.Push(mapper(*beg));
            return result;
        }
    };

    template <class T, class Super> Vec<RemConst<T>> IContinuous<T, Super>::CollectToVec() const { return Vec<RemConst<T>>::New(*this); }
    template <class T, class Super> Vec<RemConst<T>> IContinuous<T, Super>::MoveToVec() requires IsMut<T> { return Vec<RemConst<T>>::MoveNew(*this); }
    template <class T, class Super> Vec<RemConst<T>> IContinuous<T, Super>::Repeat(usize num) const {
        Vec rep = Vec<RemConst<T>>::WithCap(Length() * num);
        for (usize i = 0; i < num; ++i) rep.Extend(*this);
        return rep;
    }
}
