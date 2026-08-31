#pragma once
#include "Iterator.h"
#include "Range.h"
#include "Ref.h"

namespace Quasi {
#pragma region Continuous
    template <class T> struct Ref;
    template <class T> struct ArrayBox;
    template <class T, usize N> struct Array;
    struct Str;
    struct StrMut;

    /// Represents a direct pointer iterator with starting and ending bounds. Often used when iterating a contiguous memory chunk, like Vec<T> or Span<T>.
    /// A for-each loop taking in this iterator @code BufferIterator<T> { begin, end } @endcode is equivalent to the following:
    /// @code
    /// for (T* begin = ...; begin != end; ++begin) { ... }
    /// @endcode
    /// @tparam T The type to iterate through. Note that references @b do matter:
    /// - @code T = int@endcode means the iterator returns elements by value (copies included)
    /// - @code T = int&@endcode means the iterator returns elements by @b reference
    /// - @code T = int&&@endcode means the iterator returns elements by @b moving
    template <class T>
    struct BufferIterator : IIterator<T, BufferIterator<T>> {
        friend IIterator<T, BufferIterator>;
        using Item = T;
        using Ptr = RemRef<T>*;
        Ptr iter, endIter;
        BufferIterator(Ptr beg, Ptr end) : iter(beg), endIter(end) {}
    protected:
        T CurrentImpl() const {
            if constexpr (!IsRef<T>) return std::move(*iter);
            else return *iter;
        }
        void AdvanceImpl() { ++iter; }
        bool CanNextImpl() const { return iter != endIter; }
    public:
        bool operator==(const BufferIterator& it) const { return iter == it.iter; }
    };

    /// Represents a direct @b reveresed pointer iterator with starting and ending bounds. Often used when iterating a contiguous memory chunk in @b reverse, like Vec<T> or Span<T>.
    /// A for-each loop taking in this iterator @code BufferIterator<T> { begin, end } @endcode is equivalent to the following:
    /// @code
    /// for (T* begin = ...; begin != end; --begin) { ... }
    /// @endcode
    /// This is the reverse variant of @p BufferIterator.
    /// @tparam T The type to iterate through. Note that references @b do matter:
    /// - @code T = int@endcode means the iterator returns elements by value (copies included)
    /// - @code T = int&@endcode means the iterator returns elements by @b reference
    /// - @code T = int&&@endcode means the iterator returns elements by @b moving
    template <class T>
    struct RevBufferIterator : IIterator<T, RevBufferIterator<T>> {
        friend IIterator<T, RevBufferIterator>;
        using Item = T;
        using Ptr = RemRef<T>*;
        Ptr iter, endIter;
        RevBufferIterator(Ptr beg, Ptr end) : iter(beg), endIter(end) {}
    protected:
        T CurrentImpl() const {
            if constexpr (!IsRef<T>) return std::move(*iter);
            else return *iter;
        }
        void AdvanceImpl() { --iter; }
        bool CanNextImpl() const { return iter != endIter; }
    public:
        bool operator==(const RevBufferIterator& it) const { return iter == it.iter; }
        bool operator==(const BufferIterator<T>& it) const { return iter == it.iter; }
    };

    /// The base class for continuous collections, i.e. a container that stores elements in a contiguous manner,
    /// with no gaps or seams in between, analogous to a traditional C array. The collection type that implements
    /// this base class should be convertible to a @code Span<T>@endcode, the most generic continuous collection.
    ///
    /// To create and establish a continuous collection, one should write the following: @code
    /// struct MyContinuousCollection : IContinuous<MyElementType, MyContinuousCollection> { ... }
    /// @endcode
    /// and implement the following methods:
    /// @code
    /// MyElementType* DataImpl() { ... }
    /// const MyElementType* DataImpl() const { ... }
    /// usize LengthImpl() const { ... }
    /// @endcode
    /// @tparam T The element type of the collection. <b>Const-ness is significant;</b>
    /// A continuous collection holding @code const T@endcode means the underlying elements are @b immutable by default,
    /// while a continuous collection holding @p T means that the container holds ownership of the underlying elements/is allowed to @b mutate them.
    /// @tparam Super The continuous collection class with elements @p T.
    template <class T, class Super>
    struct IContinuous : ICollection<T&, Super> {
        friend ICollection<T&, Super>;
        using SpanCn = Span<const T>;
        using SpanMt = Span<T>;
    protected:
        Super& super() { return *static_cast<Super*>(this); }
        const Super& super() const { return *static_cast<const Super*>(this); }

        /// <b>To be implemented</b>:
        /// Should return the underlying data to the elements of the collection. @code Data()[0] @endcode should be the first element.
        T* DataImpl() = delete;
        /// <b>To be implemented</b>:
        /// Should return the underlying const data to the elements of the collection. @code Data()[0] @endcode should be the first element.
        const T* DataImpl() const = delete;
        /// <b>To be implemented</b>:
        /// Should return the length of the collection. Does not need to return the whole allocated memory size;
        /// as long as @code Data()[i] @endcode is valid for every index @p i in the range @code 0 <= i < Length() @endcode.
        usize LengthImpl() const = delete;

        /// Implementation for base class contract <tt>ICollection<T&, Super></tt>
        BufferIterator<T&> IterMutImpl() { return { Data(), Data() + Length() }; }
        /// Implementation for base class contract <tt>ICollection<T&, Super></tt>
        BufferIterator<const T&> IterImpl() const { return { Data(), Data() + Length() }; }
        /// Implementation for base class contract <tt>ICollection<T&, Super></tt>
        BufferIterator<T> IntoIterImpl() { return { Data(), Data() + Length() }; }
    public:
        /// Returns the underlying data to the elements of the collection. @code Data()[0] @endcode is the first element.
        T* Data() { return super().DataImpl(); }
        /// Returns the underlying const data to the elements of the collection. @code Data()[0] @endcode is the first const element.
        const T* Data() const { return super().DataImpl(); }
        /// Returns the ending pointer after the last element to the collection. @code *(DataEnd() - 1) @endcode is the last valid element.
        T* DataEnd() { return super().DataImpl() + super().LengthImpl(); }
        /// Returns the ending const pointer after the last element to the collection. @code *(DataEnd() - 1) @endcode is the last const valid element.
        const T* DataEnd() const { return super().DataImpl() + super().LengthImpl(); }
        /// Returns the length of the collection.
        usize Length() const { return super().LengthImpl(); }

        /// Returns a reverse iterator to the elements in the collection. Yields values by mutable references.
        RevBufferIterator<T&> RevIterMut() { return { Data() + Length() - 1, Data() - 1 }; }
        /// Returns a reverse iterator to the elements by const in the collection. Yields values by const references.
        RevBufferIterator<const T&> RevIter() const { return { Data() + Length() - 1, Data() - 1 }; }
        /// Returns a reverse iterator to the elements in the collection. Yields values by value. (includes copies).
        RevBufferIterator<T> RevIntoIter() { return { Data() + Length() - 1, Data() - 1 }; }

        /// Returns the continuous collection as a @b const span.
        SpanCn AsSpan() const { return Span(super()); }
        /// Returns the continuous collection as a span.
        SpanMt AsSpan() { return Span(super()); }
        /// Returns the continuous collection as a @b mutable span.
        SpanMt AsSpanMut() requires IsMut<T> { return Span(super()); }
        /// Returns the underlying continuous bytes of the collection. Identical to @code Bytes::Slice(Data(), Length() * sizeof(T)) @endcode.
        Span<const byte> AsBytes() const;
        /// Returns the underlying continuous mutable bytes of the collection. Identical to @code BytesMut::Slice(Data(), Length() * sizeof(T)) @endcode.
        Span<byte> AsBytesMut() requires IsMut<T>;
        /// Returns the size of the collection <b>in bytes</b>. Identical to @code Length() * sizeof(T) @endcode.
        usize ByteSize() const { return Length() * sizeof(T); }
        /// If applicable (@p T is @p char or has the same size as a @p char), returns a string slice with the @b bytes of the collection.
        Str AsStr() const requires (sizeof(T) == sizeof(char));
        /// If applicable (@p T is @p char or has the same size as a @p char), returns a mutable string slice with the @b bytes of the collection.
        StrMut AsStrMut() requires (sizeof(T) == sizeof(char)) && IsMut<T>;

        /// Checks whether the container is @b empty.
        bool IsEmpty() const { return Length() == 0; }
        /// Checks whether the container is @b non-empty.
        explicit operator bool() const { return !IsEmpty(); }

        /// Gets the @p i th element in the collection.
        /// @warning UB if @code i >= length@endcode. For safe access, use @p TryGet .
        T& Get(usize i) { return Data()[i]; }
        /// Gets the @p i th element in the collection.
        /// @warning UB if @code i >= length@endcode. For safe access, use @p TryGet .
        const T& Get(usize i) const { return Data()[i]; }
        /// Tries to get the @p i th element in the collection. Returns null if the index is out of bounds.
        OptRef<T> TryGet(usize i) { return i < Length() ? OptRefs::SomeRef(Data()[i]) : nullptr; }
        /// Tries to get the @p i th element in the collection. Returns null if the index is out of bounds.
        OptRef<const T> TryGet(usize i) const { return i < Length() ? OptRefs::SomeRef(Data()[i]) : nullptr; }
        /// Gets the @p i th element in the collection, and wraps index that are out of bounds. Works for negative values.
        T& GetWrap(WrappingIndex i) { return Data()[i(Length())]; }
        /// Gets the @p i th element in the collection, and wraps index that are out of bounds. Works for negative values.
        const T& GetWrap(WrappingIndex i) const { return Data()[i(Length())]; }

        /// Gets the first element in the collection.
        /// @warning UB if the collection is empty. For safe access, use @p TryFirst .
        T& First() { return Data()[0]; }
        /// Gets the first element in the collection.
        /// @warning UB if the collection is empty. For safe access, use @p TryFirst .
        const T& First() const { return Data()[0]; }
        /// Gets the last element in the collection.
        /// @warning UB if the collection is empty. For safe access, use @p TryLast .
        T& Last() { return Data()[Length() - 1]; }
        /// Gets the last element in the collection.
        /// @warning UB if the collection is empty. For safe access, use @p TryLast .
        const T& Last() const { return Data()[Length() - 1]; }

        /// Tries to get the first element in the collection. Returns null if the collection is empty.
        OptRef<T> TryFirst() { return Length() ? OptRefs::SomeRef(First()) : nullptr; }
        /// Tries to get the first element in the collection. Returns null if the collection is empty.
        OptRef<const T> TryFirst() const { return Length() ? OptRefs::SomeRef(First()) : nullptr; }
        /// Tries to get the last element in the collection. Returns null if the collection is empty.
        OptRef<T> TryLast() { return Length() ? OptRefs::SomeRef(Last())  : nullptr; }
        /// Tries to get the last element in the collection. Returns null if the collection is empty.
        OptRef<const T> TryLast()  const { return Length() ? OptRefs::SomeRef(Last())  : nullptr; }

        /// Identical to @code Get(i)@endcode.
        T& operator[](usize i) { return Get(i); }
        /// Identical to @code Get(i)@endcode.
        const T& operator[](usize i) const { return Get(i); }
        /// Identical to @code GetWrap(i)@endcode.
        T& operator[](WrappingIndex i) { return GetWrap(i); }
        /// Identical to @code GetWrap(i)@endcode.
        const T& operator[](WrappingIndex i) const { return GetWrap(i); }
        /// Identical to @code Subspan(range.min, range.max - range.min)@endcode.
        SpanMt operator[](zRange range) { return Subspan(range); }
        /// Identical to @code Subspan(range.min, range.max - range.min)@endcode.
        SpanCn operator[](zRange range) const { return Subspan(range); }

        /// Returns the first @p num elements in the collection as a span. Identical to @code Subspan(0, num)@endcode.
        /// @warning UB if @p num is @b greater than the length.
        SpanMt First(usize num) { return SpanMt::Slice(Data(), num); }
        /// Returns the first @p num elements in the collection as a span. Identical to @code Subspan(0, num)@endcode.
        /// @warning UB if @p num is @b greater than the length.
        SpanCn First(usize num) const { return SpanCn::Slice(Data(), num); }
        /// Returns the elements after the first @p len in the collection as a span. Identical to @code Subspan(len)@endcode.
        /// @warning UB if @p len is @b greater than the length.
        SpanMt Skip(usize len) { return SpanMt::Slice(Data() + len, Length() - len); }
        /// Returns the elements after the first @p len in the collection as a span. Identical to @code Subspan(len)@endcode.
        /// @warning UB if @p len is @b greater than the length.
        SpanCn Skip(usize len) const { return SpanCn::Slice(Data() + len, Length() - len); }
        /// Returns the elements after the first in the collection as a span. Identical to @code Skip(1)@endcode.
        /// @warning UB if the collection is empty.
        SpanMt Tail() { return SpanMt::Slice(Data() + 1, Length() - 1); }
        /// Returns the elements after the first in the collection as a span. Identical to @code Skip(1)@endcode.
        /// @warning UB if the collection is empty.
        SpanCn Tail() const { return SpanCn::Slice(Data() + 1, Length() - 1); }
        /// Returns the last @p num elements in the collection as a span. Identical to @code Skip(Length() - num)@endcode.
        /// @warning UB if @p num is @b greater than the length.
        SpanMt Last(usize num) { return SpanMt::Slice(Data() + Length() - num, num); }
        /// Returns the last @p num elements in the collection as a span. Identical to @code Skip(Length() - num)@endcode.
        /// @warning UB if @p num is @b greater than the length.
        SpanCn Last(usize num) const { return SpanCn::Slice(Data() + Length() - num, num); }

        /// Returns the elements after and including @p start in the collection,
        /// aka those in the index range @code [start..Length()]@endcode.
        /// @warning UB if @p start is @b greater than the length.
        SpanMt Subspan(usize start) { return SpanMt::Slice(Data() + start, Length() - start); }
        /// Returns the elements after and including @p start in the collection,
        /// aka those in the index range @code [start..Length()]@endcode.
        /// @warning UB if @p start is @b greater than the length.
        SpanCn Subspan(usize start) const { return SpanCn::Slice(Data() + start, Length() - start); }
        /// Returns the @p count elements starting at @p start in the collection,
        /// aka those in the index range @code [start..start+count]@endcode.
        /// @warning UB if @p start + @p count is @b greater than the length.
        SpanMt Subspan(usize start, usize count) { return SpanMt::Slice(Data() + start, count); }
        /// Returns the @p count elements starting at @p start in the collection,
        /// aka those in the index range @code [start..start+count]@endcode.
        /// @warning UB if @p start + @p count is @b greater than the length.
        SpanCn Subspan(usize start, usize count) const { return SpanCn::Slice(Data() + start, count); }
        /// Returns the elements in the index range @p range in the collection.
        /// @warning UB if @p range.max is @b greater than the length.
        SpanMt Subspan(zRange range) { return SpanMt::Slice(Data() + range.min, range.max - range.min); }
        /// Returns the elements in the index range @p range in the collection.
        /// @warning UB if @p range.max is @b greater than the length.
        SpanCn Subspan(zRange range) const { return SpanCn::Slice(Data() + range.min, range.max - range.min); }

        /// Swaps the @p i th and @p j th element in the collection. (requires the collection to be mutable).
        /// @warning UB if either @p i or @p j are out of bounds.
        void Swap(usize i, usize j) requires IsMut<T> { std::swap(Get(i), Get(j)); }
        /// Reverses the elements in the collection in place by swapping them. (requires the collection to be mutable).
        void Reverse() requires IsMut<T> {
            for (usize i = 0, j = Length() - 1; i < j; ++i, --j) Swap(i, j);
        }

        /// Checks whether the elements in the collection are equal to those in the other span.
        /// Returns @p false if the collection and the span have different length.
        ///
        /// Identical to @code eq(a[0], b[0]) && eq(a[1], b[1]) && ... && eq(a[len-1], b[len-1]) @endcode.
        /// @param other The elements to check equality against.
        /// @param eq The equality operator for determining equality. Uses @code operator==@endcode by default.
        template <EqualPred<T> Eq = Cmp::Equals<void>>
        bool Equals(SpanCn other, Eq&& eq = Eq {}) const {
            if (Length() != other.Length()) return false;
            for (usize i = 0; i < Length(); ++i) if (!eq(Get(i), other[i])) return false;
            return true;
        }
        /// Identical to @code Equals(other)@endcode.
        bool operator==(SpanCn other) const { return Equals(other); }

        /// Checks whether all of the elements in the collection follow a predicate.
        /// Identical to
        /// @code pred(self[0]) && pred(self[1]) && ... && pred(self[len-1]) @endcode
        /// @param pred The predicate to check against each element for.
        /// Uses the boolean conversion operator by default.
        template <Predicate<T> P = Identity>
        bool All(P&& pred = P {}) const {
            for (const T& x : *this) { if (!pred(x)) return false; }
            return true;
        }

        /// Checks whether any of the elements in the collection follow a predicate.
        /// Identical to
        /// @code pred(self[0]) || pred(self[1]) || ... || pred(self[len-1]) @endcode
        /// @param pred The predicate to check against each element for.
        /// Uses the boolean conversion operator by default.
        template <Predicate<T> P = Identity>
        bool Any(P&& pred = P {}) const {
            for (const T& x : *this) { if (pred(x)) return true; }
            return false;
        }

        /// Compares the collection to a span with the specified comparsion function.
        /// Uses lexicographic comparsion on the elements to provide a total ordering.
        /// @param other The span to compare against.
        /// @param cmp The comparison function used for comparing elements.
        /// Uses @code operator<=>@endcode by default.
        template <Comparator<T> Cmpr = Cmp::Compare<void>>
        Comparison Cmp(SpanCn other, Cmpr&& cmp = Cmpr {}) const {
            for (usize i = 0; i < std::min(Length(), other.Length()); ++i) {
                const Comparison c = cmp(Get(i), other[i]);
                if (c != Cmp::EQUAL) return c;
            }
            return Cmp::Between(Length(), other.Length());
        }

        /// Compares the collection to a span with the specified comparsion function.
        /// Uses lexicographic comparsion on the elements to provide a total ordering,
        /// but returns early if the sizes are different.
        /// @code a > b @endcode will be true if @p a is longer than @p b .
        /// @param other The span to compare against.
        /// @param cmp The comparison function used for comparing elements.
        /// Uses the @code operator<=>@endcode by default.
        template <Comparator<T> Cmpr = Cmp::Compare<void>>
        Comparison CmpSized(Span<const T> other, Cmpr&& cmp = Cmpr {}) const { return CmpSizedBy(other, cmp); }
        /// Identical to @code Cmp(other)@endcode.
        Comparison operator<=>(Span<const T> other) const { return Cmp(other); }

        /// Tries to find the first instance of @p target
        /// within the collection and returns the index. Searches forwards.
        /// @param target The target to search for.
        /// @return The index of the target if found, null otherwise.
        OptionUsize FindIndex(const T& target) const { return FindIndexIf(Cmp::Equals { target }); }
        /// Tries to find the last instance of @p target
        /// within the collection and returns the index. Searches backwards.
        /// @param target The target to search for.
        /// @return The index of the target if found, null otherwise.
        OptionUsize RevFindIndex(const T& target) const { return RevFindIndexIf(Cmp::Equals { target }); }
        /// Tries to find the first instance in which @p pred returns true
        /// within the collection and returns the index. Searches forwards.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return The index of the target if found, null otherwise.
        OptionUsize FindIndexIf(Predicate<T> auto&& pred) const {
            for (usize i = 0; i < Length(); ++i) if (pred(Get(i))) return i;
            return nullptr;
        }
        /// Tries to find the last instance in which @p pred returns true
        /// within the collection and returns the index. Searches backwards.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return The index of the target if found, null otherwise.
        OptionUsize RevFindIndexIf(Predicate<T> auto&& pred) const {
            for (usize i = Length(); i --> 0; )  if (pred(Get(i))) return i; return nullptr;
        }

        /// Checks whether @p target is contained in the collection.
        /// Identical to @code FindIndex(target).HasValue()@endcode.
        /// @param target The target to search for.
        /// @return @p true if found, @p false otherwise.
        bool Contains(const T& target) const { return FindIndex(target).HasValue(); }
        /// Checks whether an element for which @p pred returns true is contained in the collection.
        /// Identical to @code FindIndexIf(pred).HasValue()@endcode and @code Any(pred)@endcode.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return @p true if found, @p false otherwise.
        bool ContainsIf(Predicate<T> auto&& pred) const { return FindIndexIf(pred).HasValue(); }

        /// Finds the first instance in which @p pred returns true within the collection
        /// and returns the reference. Searches forwards.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return The reference to the target if found, null otherwise.
        OptRef<const T> FindIf(Predicate<T> auto&& pred) const {
            for (usize i = 0; i < Length(); ++i) if (pred(Get(i))) return Get(i); return nullptr;
        }
        /// Finds the first instance in which @p pred returns true within the collection
        /// and returns the reference. Searches forwards.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return The reference to the target if found, null otherwise.
        OptRef<T> FindIf(Predicate<T> auto&& pred) {
            for (usize i = 0; i < Length(); ++i) if (pred(Get(i))) return Get(i); return nullptr;
        }
        /// Finds the last instance in which @p pred returns true within the collection
        /// and returns the reference. Searches backwards.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return The reference to the target if found, null otherwise.
        OptRef<const T> RevFindIf(Predicate<T> auto&& pred) const {
            for (usize i = Length(); i --> 0; ) if (pred(Get(i))) return Get(i); return nullptr;
        }
        /// Finds the last instance in which @p pred returns true within the collection
        /// and returns the reference. Searches backwards.
        /// @param pred The predicate which returns @p true for the target element.
        /// @return The reference to the target if found, null otherwise.
        OptRef<T> RevFindIf(Predicate<T> auto&& pred) {
            for (usize i = Length(); i --> 0; ) if (pred(Get(i))) return Get(i); return nullptr;
        }

        /// Searches for the first instance of the target subspan @p target in the collection.
        /// @param target The target subspan to search for.
        /// @return The index to the beginning of the first occurence,
        /// aka the first index @p i which satisfies @code self.Subspan(i, target.Length()) == target@endcode.
        OptionUsize Search(Span<const T> target) const {
            for (usize i = 0; i <= Length() - target.Length(); ++i)
                if (Subspan(i, target.Length()) == target) return i;
            return nullptr;
        }
        /// Searches for the last instance of the target subspan @p target in the collection.
        /// @param target The target subspan to search for.
        /// @return The index to the beginning of the first occurence,
        /// aka the last index @p i which satisfies @code self.Subspan(i, target.Length()) == target@endcode.
        OptionUsize RevSerach(Span<const T> target) const {
            for (usize i = Length() - target.Length(); i --> 0; )
                if (Subspan(i, target.Length()) == target) return i;
            return nullptr;
        }
        /// Checks whether the collection contains the continuous subspan @p target.
        /// @param target The target subspan to search for.
        /// @return @p true if @p target occurs in the collection, @p false otherwise.
        bool ContainsSpan(Span<const T> target) const { return Search(target).HasValue(); }

        /// Sorts the elements in the collection in order of smallest to largest.
        /// @note <b>Does not sort the collection stabily.</b>
        /// @b Complexity: Given N as the size of the collection:
        /// - O(N log N) comparisons/calls to @p cmp
        /// - O(N log N) element swaps/copies/moves
        /// @param cmp The comparison function to use when ordering elements.
        /// Uses @code operator<=>@endcode by default
        template <Comparator<T> Cmpr = Cmp::Compare<void>>
        void Sort(Cmpr&& cmp = Cmpr {}) requires IsMut<T>;
        /// Sorts the elements in the collection in order of smallest to largest using the key function @p keyf.
        /// @note <b>Does not sort the collection stabily.</b>
        /// @b Complexity: Given N as the size of the collection:
        /// - O(N log N) comparisons/calls to @p cmp
        /// - O(N log N) element swaps/copies/moves
        /// @param keyf The key extraction function to use when ordering elements.
        void SortByKey(FnArgs<const T&> auto&& keyf) requires IsMut<T>;

        // void SortStable() TODO i cant be bothered to do this
        // void SortStableBy(Fn<bool, const T&, const T&> auto&& cmp)
        // void SortStableByKey(auto&& keyf)

        /// Checks whether the elements of the collection are ordered from smallest to largest.
        /// Identical to @code self[0] < self[1] && self[1] < self[2] && ... && self[len-2] < self[len-1] @endcode.
        /// @param cmp The comparison function to use to test for ordering.
        /// Uses @code operator<=>@endcode by default.
        template <Comparator<T> Cmpr = Cmp::Compare<void>>
        bool IsSorted(Cmpr&& cmp = Cmpr {}) const;

        /// Rotates the elements left by @p num.
        /// The elements in the index range @code [num..len]@endcode gets shifted to @code [0..len-num]@endcode
        /// and the range @code [0..num]@endcode gets wrapped back to the end (@code [len-num..len]@endcode)
        ///
        /// @b Example: @code
        /// Vec<int> numbers = Vecs::New({ 1, 2, 3, 4, 5, 6 });
        /// numbers.RotateLeft(2);
        /// // numbers == { 3, 4, 5, 6, 1, 2 }
        /// @endcode
        /// @param num The amount to shift left by.
        /// @warning UB if @p num is greater than the length of the collection.
        void RotateLeft(usize num) requires IsMut<T>;
        /// Rotates the elements right by @p num.
        /// The elements in the index range @code [0..len-num]@endcode gets shifted to @code [num..len]@endcode
        /// and the range @code [len-num..len]@endcode gets wrapped back to the end (@code [0..num]@endcode)
        ///
        /// @b Example: @code
        /// Vec<int> numbers = Vecs::New({ 1, 2, 3, 4, 5, 6 });
        /// numbers.RotateRight(2);
        /// // numbers == { 5, 6, 1, 2, 3, 4 }
        /// @endcode
        /// @param num The amount to shift right by.
        /// @warning UB if @p num is greater than the length of the collection.
        void RotateRight(usize num) requires IsMut<T>;

        /// Rotates the elements by the signed amount @p off,
        /// where a negative offset indicates a rotation towards the @b beginning of the collection and
        /// positive offset indicates a rotation towards the @b end of the collection.
        /// @note Performs @code RotateLeft(-off)@endcode if @p off is negative,
        /// @code RotateRight(off)@endcode if @p off is positive, and nothing if @p off is zero.
        /// @param off The index offset to rotate elements by.
        /// @warning UB if @p off is outside the valid range @code -len <= off <= len@endcode
        void RotateSigned(isize off) requires IsMut<T> { return off == 0 ? void() : off < 0 ? RotateLeft(-off) : RotateRight(off); }
        /// Rotates the elements by the signed amount @p off.
        /// Same as @p RotateSigned(...), but @b without the limitation of having to be
        /// within the range of .@code -len <= off <= len@endcode
        /// @param off The index offset to rotate elements by.
        void RotateSigned(WrappingIndex off) requires IsMut<T> { return RotateRight(off(Length())); }
        /// Rotates the elements by the signed amount @p off.
        /// Same as @p RotateSigned(...), but @b without the limitation of having to be
        /// within the range of .@code -len <= off <= len@endcode
        /// @param off The index offset to rotate elements by.
        void RotateSignedWrapped(isize off) requires IsMut<T> { return RotateSigned(off % Wrap); }

        /// Sets all the values inside the collection to @p value by copy-assigning it to each element.
        /// @param value The value to fill the collection with.
        void Fill(const T& value) requires IsMut<T> { for (T& t : *this) t = value; }
        /// Replaces and fills all the values inside the collection with the result of @p factory .
        /// @param factory The factory function that returns new values to fill in.
        /// @note If the index is required when calling the function, one may use @p Combinate::Counting .
        template <Fn<T> F = Fns::Constructor<T>>
        void FillWith(F&& factory = F {}) requires IsMut<T> { for (T& t : *this) t = factory(); }
        /// Copies elements from a span into the collection.
        /// @param span The span of elements to copy from.
        /// @warning If @p span is longer than the collection, this is UB.
        /// Only copies @p span.Length() elements if @p span is shorter than the collection.
        void CloneFrom(Span<const T> span) requires IsMut<T> { Memory::RangeCopy(Data(), span.Data(), span.Length()); }
        /// Moves the elements from a span into the collection.
        /// @param span The span of elements to move from.
        /// @warning After calling, the elements of @p span are moved out.
        /// If @p span is longer than the collection, this is UB.
        /// Only moves @p span.Length() elements if @p span is shorter than the collection.
        void MoveFrom(SpanMt span) requires IsMut<T> { Memory::RangeMove(Data(), span.Data(), span.Length()); }
        /// Swaps the elements from a span with the ones in the collection.
        /// @param span The span of elements to move from.
        /// @warning If @p span is longer than the collection, this is UB.
        /// Only swaps @p span.Length() elements if @p span is shorter than the collection.
        void SwapWith(SpanMt span) requires IsMut<T> { Memory::RangeSwap(Data(), span.Data(), span.Length()); }
        // void CopyFromSelf(IntegerRange, usize dest)

        // Tuple<Span, Span<AddConstIf<SimdT, T>>, Span> AsSimd() const;

        /* Span<AddConstIf<ArrayElement<T>, T>> */ auto AsFlattened() const requires IsRawArray<T>;
        // {
        //     return Spans::Slice((AddConstIf<ArrayElement<T>, T>*)Data(), Length() * ArrayLength<T>());
        // }

        /// Copies all the elements in the collection into a newly allocated @code Vec<T>@endcode.
        Vec<RemConst<T>> CollectToVec() const;
        /// Moves all the elements in the collection into a newly allocated @code Vec<T>@endcode.
        /// @warning The elements inside the collection are moved out.
        Vec<RemConst<T>> MoveToVec() requires IsMut<T>;
        /// Repeats the contents of the collection @p num times,
        /// and collects those into a newly allocated @code Vec<T>@endcode.
        ///
        /// @b Example:
        /// @code
        /// Vec<int> repeated = Spans::Vals({ 1, 2, 3 }).Repeat(3);
        /// // repeated == { 1, 2, 3, 1, 2, 3, 1, 2, 3 }
        /// @endcode
        Vec<RemConst<T>> Repeat(usize num) const;

        // Vec<ConcatResult<T>> Concat() const requires CanConcat<T>;
        // Vec<ConcatResult<T>> Join(const auto& sep) const requires CanConcat<T> && CanConcat<T, decltype(sep)>;

        /// Hashes the collection using @b all the elements inside it.
        /// Used for when the key of a HashMap is a continuous itself.
        /// @note Guarentees the hash code of the empty collection is @p 0 .
        Hashing::Hash GetHashCode() const {
            usize seed = Length();
            for (const T& value : *this) {
                seed ^= HashObject(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return Hashing::AsHash(seed);
        }
    };

    /// Checks whether the input type is a @b collection of @p Item.
    template <class C, class Item> concept Continuous = Implements<C, IContinuous, Item>;
    /// Checks whether the input type is a @b collection of any kind.
    template <class C> concept ContinuousAny = Implements<C, IContinuous, RemRef<CollectionItem<C>>>;
#pragma endregion
}
