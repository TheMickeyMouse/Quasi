#pragma once
#include "Func.h"
#include "Hash.h"

namespace Quasi {
    template <class T> struct OptRef;
    template <class T> struct Option;

    template <class T> struct Span;
    struct Str;

    namespace Options {
        /// Returns an optional that holds some value @p val .
        template <class T> Option<RemQual<T>> Some(T&& val);
    }

    /// The base class that encapsulates nullability or emptiness for a type.
    /// Unless @p T is a reference type, the nullable type should be convertible
    /// to an @code Option<T>@endcode, the most generic nullable type. Otherwise,
    /// should be convertible to an @code OptRef<T>@endcode,
    /// the most generic nullable @em reference type.
    ///
    /// To create and establish a nullable type, one should write the following: @code
    /// struct MyNullable : INullable<MyUnderlying, MyNullable> { ... }
    /// @endcode
    /// and implement the following methods:
    /// @code
    /// bool HasValue() const;
    /// const T& Unwrap() const;
    /// T& Unwrap();
    /// void PretendHasValue();
    /// void SetNull();
    /// static Super None();
    /// static Super Some(T&&);
    /// @endcode
    /// @tparam T The underlying type of the nullable type. <b>Const-ness and references are significant;</b>
    /// If @p T is a reference, then @p INullable contains a non-owning reference, otherwise it will own and manage
    /// the underlying type.
    /// @tparam Super The nullable class that holds an underlying @p T.
    template <class T, class Super>
    struct INullable {
        using SomeType = T;
        using V = RemQual<T>;
        static constexpr bool USE_RREF = DifferentTo<const T&, T&&>;
    protected:
        Super& super() { return *static_cast<Super*>(this); }
        const Super& super() const { return *static_cast<const Super*>(this); }

        T& UnwrapImpl() = delete;
        const T& UnwrapImpl() const = delete;
        bool HasValueImpl() const = delete;
        void SetNullImpl() = delete;
        void SetImpl(const T& value) = delete;
        void SetImpl(T&& value) requires USE_RREF = delete;
        static Super NoneImpl() = delete;
        static Super SomeImpl(const T&) = delete;
        static Super SomeImpl(T&&) requires USE_RREF = delete;
    public:
        /// Checks if the type holds a value, or is actually null. Returns @p true if it does.
        bool HasValue() const { return super().HasValueImpl(); }
        /// Checks if the type is null, or actually holds a value. Returns @p false if null.
        bool IsNull() const { return !HasValue(); }
        /// Identical to @p HasValue() .
        explicit operator bool() const { return HasValue(); }

        /// Sets the value to null.
        void SetNull() { super().SetNull(); }
        /// Sets the underlying value to @p value, clearing null or any existing value in the process.
        void Set(const T& value) { super().SetImpl(value); }
        /// Sets the underlying value to @p value by moving, clearing null or any existing value in the process.
        void Set(T&& value) requires USE_RREF { super().SetImpl(std::move(value)); }

        /// Checks for the value being non-null @em and
        /// returns whether the underlying value satisfies a predicate @p pred .
        /// Both tests have to pass for it to return @p true .
        ///
        /// @b Example: @code
        /// const auto isEven = [] (int x) { return x % 2 == 0; };
        /// Option<int>::None() .HasValueAnd(isEven); // false, null
        /// Option<int>::Some(3).HasValueAnd(isEven); // false, has value but doesn't pass
        /// Option<int>::Some(4).HasValueAnd(isEven); // true,  has value *and* passes predicate
        /// @endcode
        bool HasValueAnd(Fn<bool, const T&> auto&& pred) const { return HasValue() && pred(Unwrap()); }
        /// Checks if the value is null @em or
        /// whether the underlying value satisfies a predicate @p pred .
        /// Either test can pass for it to return @p true .
        ///
        /// @b Example: @code
        /// const auto isEven = [] (int x) { return x % 2 == 0; };
        /// Option<int>::None() .IsNullOr(isEven); // true,  null
        /// Option<int>::Some(4).IsNullOr(isEven); // true,  passes predicate
        /// Option<int>::Some(3).IsNullOr(isEven); // false, has value *and* doesn't pass
        /// @endcode
        bool IsNullOr(Fn<bool, const T&> auto&& pred) const { return IsNull() || pred(Unwrap()); }

        /// Unwraps the nullable type and directly returns the underlying value.
        /// @warning UB if null.
        T& Unwrap() { return super().UnwrapImpl(); }
        /// Identical to @p Unwrap() .
        /// @warning UB if null.
        T& operator*() { return Unwrap(); }
        /// Arrow operator for nullable types.
        /// Allows for accesing members/calling methods directly from the container.
        /// @warning UB if null.
        V* operator->() { return &Unwrap(); }
        /// Unwraps the nullable type and directly returns the underlying value.
        /// @warning UB if null.
        const T& Unwrap() const { return super().UnwrapImpl(); }
        /// Identical to @p Unwrap() .
        /// @warning UB if null.
        const T& operator*() const { return Unwrap(); }
        /// Arrow operator for nullable types.
        /// Allows for accesing members/calling methods directly from the container.
        /// @warning UB if null.
        const V* operator->() const { return &Unwrap(); }

        /// Returns an optional reference @p OptRef of the underlying type, and null if it doesn't exist.
        OptRef<T> AsRef() { return HasValue() ? Unwrap() : OptRef<T>::None(); }
        /// Returns a span of one value containing the underlying type if it exists, and the empty span if it doesn't.
        Span<T> AsSpan() { return HasValue() ? Span<T>::Only(Unwrap()) : Span<T>::Empty(); }
        /// Returns the object as the corresponding @p Option object.
        Option<T> AsOption() const { return HasValue() ? Option<T>::Some(Unwrap()) : nullptr; }
        /// Returns an optional reference @p OptRef of the underlying type, and null if it doesn't exist.
        OptRef<const T> AsRef() const { return HasValue() ? SomeRef(Unwrap()) : nullptr; }
        /// Returns a span of one value containing the underlying type if it exists, and the empty span if it doesn't.
        Span<const T> AsSpan() const { return HasValue() ? Span<const T>::Only(Unwrap()) : Span<const T>::Empty(); }

        /// Returns the underlying value if it exists, otherwise returns the value @p otherwise .
        T UnwrapOr(const T& otherwise) const { return HasValue() ? Unwrap() : otherwise; }
        /// Returns the underlying value if it exists, otherwise returns the value @p otherwise .
        T UnwrapOr(T&& otherwise) requires USE_RREF { return HasValue() ? Unwrap() : std::move(otherwise); }
        /// Returns the underlying value if it exists, otherwise returns the value produced by @p otherwise .
        T UnwrapOrElse(Fn<T> auto&& otherwise) { return HasValue() ? Unwrap() : otherwise(); }

        /// Returns self if a value exists, otherwise returns
        /// the value @p otherwise, which may be null (behaves like pythonic OR).
        Super Or(const Super& otherwise) const { return HasValue() ? *this : otherwise; }
        /// Returns self if a value exists, otherwise returns
        /// the value @p otherwise, which may be null (behaves like pythonic OR).
        Super Or(Super&& otherwise) const { return HasValue() ? *this : std::move(otherwise); }
        /// Returns self if a value exists, otherwise returns
        /// the value produced by @p otherwise, which may be null (behaves like pythonic OR).
        Super OrElse(Fn<Super> auto&& otherwise) const { return HasValue() ? *this : otherwise(); }

        /// Returns @p onlyif if the value exists, otherwise null if it doesn't. (behaves like pythonic AND)
        Super And(const Super& onlyif) const { return HasValue() ? onlyif : None(); }
        /// Returns @p onlyif if the value exists, otherwise null if it doesn't. (behaves like pythonic AND)
        Super And(Super&& onlyif) const { return HasValue() ? std::move(onlyif) : None(); }

        /// Returns the result of @p onlyif called on the contained value if it exists, which may be null,
        /// or an empty optional otherwise. Used to chain fallible function calls with optionals.
        ///
        /// @b Example: @code
        /// const auto f = [] (int x) -> Option<int> { return x ? 10 / x : nullptr; }
        /// Option<int>::None() .AndThen(f); // None,    no value to begin with
        /// Option<int>::Some(2).AndThen(f); // Some(5), 10 / 2 = 5
        /// Option<int>::Some(0).AndThen(f); // None,    returned from function
        /// @endcode
        template <FnArgs<T> F> requires Implements<FnResult<F, T>, INullable, typename FnResult<F, T>::SomeType>
        FnResult<F, T> AndThen(F&& onlyif) const { return HasValue() ? onlyif(Unwrap()) : nullptr; }

        /// Out of this and the object @p extra, returns the one that has a value <em>if and only if</em>
        /// exactly one of them has a value, otherwise returns null.
        ///
        /// @b Example: @code
        /// const Option<int> none = nullptr, x = 1, y = 2;
        /// none.Xor(none); // None
        /// x.Xor(none);    // Some(1)
        /// none.Xor(y);    // Some(2)
        /// x.Xor(y);       // None
        /// @endcode
        Super Xor(const Super& extra) const { return HasValue() ^ extra.HasValue() ? (HasValue() ? *this : extra) : None(); }
        /// Out of this and the object @p extra, returns the one that has a value <em>if and only if</em>
        /// exactly one of them has a value, otherwise returns null.
        ///
        /// @b Example: @code
        /// const Option<int> none = nullptr, x = 1, y = 2;
        /// none.Xor(none); // None
        /// x.Xor(none);    // Some(1)
        /// none.Xor(y);    // Some(2)
        /// x.Xor(y);       // None
        /// @endcode
        Super Xor(Super&& extra) const { return HasValue() ^ extra.HasValue() ? (HasValue() ? *this : std::move(extra)) : None(); }

        /// Returns the result of @p map called on the contained value if it exists,
        /// or an empty optional otherwise. Used to chain function calls with optionals.
        ///
        /// @b Example: @code
        /// const auto f = [] (int x) -> int { return x * x; }
        /// Option<int>::None() .Map(f); // None,    no value to begin with
        /// Option<int>::Some(2).Map(f); // Some(5), 2 * 2 = 4
        /// @endcode
        template <FnArgs<T> F>
        Option<FnResult<F, T>> Map(F&& map) const { return HasValue() ? Options::Some(map(Unwrap())) : nullptr; }

        /// Returns the result of @p map called on the contained value if it exists,
        /// or returns the value @p otherwise.
        ///
        /// @b Example: @code
        /// const auto f = [] (int x) -> int { return x * x; }
        /// Option<int>::None() .MapOr(f, -1); // -1, no value to begin with
        /// Option<int>::Some(2).MapOr(f, -1); //  4, 2 * 2 = 4
        /// @endcode
        template <class U, Fn<U, T> F>
        U MapOr(F&& map, U&& otherwise) const { return HasValue() ? map(Unwrap()) : (decltype(otherwise))otherwise; }

        /// Returns the result of @p map called on the contained value if it exists,
        /// or returns the value produced by @p otherwise.
        ///
        /// @b Example: @code
        /// const auto f = [] (int x) -> int { return x * x; }
        /// const auto g = [] { return -1; }
        /// Option<int>::None() .MapOr(f, g); // -1, result of g
        /// Option<int>::Some(2).MapOr(f, g); //  4, 2 * 2 = 4
        /// @endcode
        template <FnArgs<T> F, FnArgs<> G>
        FnResult<F, T> MapOrElse(F&& map, G&& otherwise) const { return HasValue() ? map(Unwrap()) : otherwise(); }

        /// Calls @p inspect on the underlying value if it exists, otherwise does nothing.
        /// Returns self for chaining convenience.
        Super& Inspect(Fn<void, T&> auto&& inspect) { if (HasValue()) inspect(Unwrap()); return *this; }
        /// Calls @p inspect on the underlying value if it exists, otherwise does nothing.
        /// Returns self for chaining convenience.
        const Super& Inspect(Fn<void, const T&> auto&& inspect) const { if (HasValue()) inspect(Unwrap()); return *this; }

        /// Returns self only if the underlying value exists @em and it passes the predicate @p pred, otherwise returns null.
        Super Filter(Fn<bool, T> auto&& pred) const { return this->HasValue() && pred(this->Unwrap()) ? *this : super().NoneImpl(); }

        /// Sets the underlying value to @p value, and then returns a reference to it.
        T& Insert(const T& value) { Set(value); return Unwrap(); }
        /// Sets the underlying value to @p value, and then returns a reference to it.
        T& Insert(T&& value) requires USE_RREF { Set(std::move(value)); return Unwrap(); }

        /// Tries to get the underlying value, while inserting @p value if it doesn't exist,
        /// and then returns a reference to it.
        T& GetOrInsert(const T& value) { if (IsNull()) Insert(value); return Unwrap(); }
        /// Tries to get the underlying value, while inserting @p value if it doesn't exist,
        /// and then returns a reference to it.
        T& GetOrInsert(T&& value) requires USE_RREF { if (IsNull()) Insert(std::move(value)); return Unwrap(); }
        /// Tries to get the underlying value, while inserting the value produced by @p gen if it doesn't exist,
        /// and then returns a reference to it.
        T& GetOrInsertFrom(Fn<T> auto&& gen) { if (IsNull()) Insert(gen()); return Unwrap(); }

        // TODO: T::View AsView() requires Viewable<T>;
        // TODO: OptionIter AsIter();
        // TODO: Option<Tuple<T, U>> Zip<U>(Option<U> extra) const;
        // TODO: Option<R> ZipWith<U, F, R>(Option<U> extra, F&& zipper) const;
        // TODO: Tuple<Option<A>, Option<B>> Unzip() const requires T is Tuple<A, B>;
        // TODO: Option<I> Flatten() const requires T is Option<I>;

        /// Unwraps the value, asserting if no value exists. A safer version of @p Unwrap .
        T& Assert();
        /// Unwraps the value, asserting with message @p msg if no value exists. A safer version of @p Unwrap .
        T& Assert(Str msg);
        /// Unwraps the value, asserting if no value exists. A safer version of @p Unwrap .
        const T& Assert() const;
        /// Unwraps the value, asserting with message @p msg if no value exists. A safer version of @p Unwrap .
        const T& Assert(Str msg) const;

        /// Returns the null value.
        static Super None() { return Super::NoneImpl(); }
        /// Returns the object containing some value @p t .
        static Super Some(const T& t) { return Super::SomeImpl(t); }
        /// Returns the object containing some value @p t , moving it into the object.
        static Super Some(T&& t) requires USE_RREF { return Super::SomeImpl(std::move(t)); }

        bool operator==(const INullable&) const = default;
    };

    /// Represents a value that may or may not be present. Manages and owns an optional contained value.
    /// Any instance of @p Option always either contains a value or does not contain a value (null).
    /// Commonly used as the return type of a function that may fail.
    ///
    /// Under the hood, is stored as a tuple of the value itself and a boolean flag which tells the
    /// object whether the underlying value exists or not.
    template <class T>
    struct Option : INullable<T, Option<T>> {
        friend INullable<T, Option>;
    private:
        [[no_unique_address]] T value;
        bool isSome = true;
    public:
        /// Creates the null value.
        Option() : Option(nullptr) {}
        /// Creates the null value.
        Option(Nullptr) : isSome(false) {}
        /// Creates the object holding @p value .
        Option(const T& value) : value(value) {}
        /// Creates the object holding @p value by moving it.
        Option(T&& value) : value(std::move(value)) {}
    protected:
        static Option SomeImpl(const T& value) { return { value }; }
        static Option SomeImpl(T&& value) { return { std::move(value) }; }
        static Option NoneImpl() { return { nullptr }; }

        bool HasValueImpl() const { return isSome; }
        T& UnwrapImpl() { return value; }
        const T& UnwrapImpl() const { return value; }

        void SetNullImpl() { isSome = false; }
        void SetImpl(const T& v) { isSome = true; value = v; }
        void SetImpl(T&& v)      { isSome = true; value = std::move(v); }
    public:
        bool operator==(const Option&) const = default;
        bool operator==(const T& other) const { return isSome && value == other; }

        /// Hashes the index.
        /// Guarenteed to be equivalent to the hash of the equivalent integer,
        /// and if the value is empty, the hash is defined to be the empty hash (0).
        Hashing::Hash GetHashCode() const {
            return isSome ? Hashing::HashObject(value) : Hashing::EmptyHash();
        }
    };

    /// Represents an optional/nullable index of type @p usize, and may hold an integer or null.
    /// Under the hood, this takes up as much space as a regular @p usize, as the value
    /// @p usizes::MAX is used as the indicator for null.
    struct OptionUsize : INullable<usize, OptionUsize> {
        friend INullable;
    private:
        usize value = -1;
    public:
        /// Creates the null value.
        OptionUsize() = default;
        /// Creates the null value.
        OptionUsize(Nullptr) : OptionUsize() {}
        /// Creates an index of @p value .
        OptionUsize(usize value) : value(value) {}
        /// Creates an index of @p value .
        OptionUsize(ConvTo<usize> auto value) : value(value) {}
        /// Creates an index of @p optval from the regular @code Option<usize>@endcode.
        OptionUsize(Option<usize> optval) : value(optval.UnwrapOr(-1)) {}
    protected:
        static OptionUsize SomeImpl(usize value) { return { value }; }
        static OptionUsize NoneImpl() { return { nullptr }; }

        bool HasValueImpl() const { return value != -1; }
        usize& UnwrapImpl() { return value; }
        const usize& UnwrapImpl() const { return value; }

        void SetNullImpl()    { value = -1; }
        void SetImpl(usize v) { value = v; }
    public:
        bool operator==(const OptionUsize&) const = default;
        bool operator==(usize v) const { return value == v; }

        /// Hashes the index. Guarenteed to be equivalent to the hash of the equivalent integer.
        Hashing::Hash GetHashCode() const { return Hashing::HashInt(value); }
    };

    namespace Options {
        template <class T>
        Option<RemQual<T>> Some(T&& val) {
            return Option<RemQual<T>> { std::forward<T>(val) };
        }
    }
}