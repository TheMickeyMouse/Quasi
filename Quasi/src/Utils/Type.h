#pragma once
#include <initializer_list>
#include "Numeric.h"

namespace Quasi {
	template <class T> struct Ref;
	template <class T> struct OptRef;
	template <class T, usize N> struct Array;
	template <class T> struct Option;

	/// Alias for @p std::initializer_list.
    template <class T>
    using IList = std::initializer_list<T>;

	/// Disables type inference for this input parameter.
	/// Internally, this has no affect on the type and is aliased to @p std::type_identity_t.
	template <class T> using NoInfer = std::type_identity_t<T>;

	/// An empty type that has no size and can be constructed with any arguments. Similar to @p std::monostate .
	/// @note When the constructor is called, the arguments are ignored.
	/// @p operator== always returns @p true .
	struct Empty {
		Empty(auto&&...) {}
		bool operator==(const Empty&) const { return true; }
	};

	/// Annotates that the value may be nullable.
	/// <em>This is a non-enforcing type decoration and has no affect.</em>
	template <class T> using Nullable = T;
	/// Annotates that the value is not null.
	/// <em>This is a non-enforcing type decoration and has no affect.</em>
	template <class T> using NotNull = T;
	/// Annotates that the input parameter is in/out.
	/// <em>This is a non-enforcing type decoration and has no affect.</em>
	template <class T> using InOut = T;
	/// Annotates that the parameter is an out parameter.
	/// <em>This is a non-enforcing type decoration and has no affect.</em>
	template <class T> using Out = T;

	/// Type alias for @p std::nullptr_t . Identical to @p decltype(nullptr) .
	using Nullptr = std::nullptr_t;

	/// Indicates that the function should be @b unchecked and may cause UB for bad inputs.
	/// <em>This is a function hint and is non-enforcing.</em>
	enum UncheckedMarker { Unchecked };
	/// Indicates that the function should be @b checked and <b>shall not</b> be UB for bad inputs.
	/// <em>This is a function hint and is non-enforcing.</em>
	enum CheckedMarker { Checked };
	/// Indicates that a value should be uninitialized.
	/// <em>This is a constructor hint and is non-enforcing.</em>
	enum UninitMarker { Uninit };

	/// A marker for @p T. Does not actually store @p T, only indicates that @p T was used.
	/// Useful for function parameter type inference without actually needing to take in a @p T .
	template <class T> struct TypeMarker {};

	/// The universally-true concept.
	template <class> concept AlwaysTrue  = true;
	/// The universally-false concept.
	template <class> concept AlwaysFalse = false;

	/// Checks whether a type is @p const . Returns @p false for const references.
	template <class T> concept IsConst = std::is_const_v<T>;
	/// Checks whether a type is not @p const . Returns @p false for mutable references.
	template <class T> concept IsMut = !IsConst<T>;
	/// Removes the const-qualifier of the type @p T . Does not remove const-ness on references.
	template <class T> using RemConst = std::remove_const_t<T>;

	/// Checks if @p T is a reference type.
	template <class T> concept IsRef = std::is_reference_v<T>;
	/// Checks if @p T is a pointer type.
	template <class T> concept IsPtr  = std::is_pointer_v<T>;
	/// Removes the reference-qualifer on @p T .
	template <class T> using RemRef = std::remove_reference_t<T>;
	/// Removes the pointer on @p T, i.e. dereferences the type @p T .
	template <class T> using RemPtr = std::remove_pointer_t<T>;

	/// Checks if @p T is a const-reference type.
	template <class T> concept IsConstRef = IsRef<T> && IsConst<RemRef<T>>;
	/// Checks if @p T is a const-pointer type.
	template <class T> concept IsConstPtr = IsPtr<T> && IsConst<RemPtr<T>>;
	/// Checks if @p T is a mutable-reference type.
	template <class T> concept IsMutRef = IsRef<T> && IsMut<RemRef<T>>;
	/// Checks if @p T is a mutable-pointer type.
	template <class T> concept IsMutPtr = IsPtr<T> && IsMut<RemPtr<T>>;
	/// Removes the const-reference qualifer on @p T .
	template <class T> using RemCRef = RemConst<RemRef<T>>;
	/// Removes the const-pointer qualifer on @p T .
	template <class T> using RemCPtr = RemConst<RemPtr<T>>;

	/// Converts the reference type @p T into its const-reference version.
	template <class T> using ToConstRef = const RemRef<T>&;
	/// Converts the const-reference type @p T into its mutable-reference version.
	template <class T> using ToMutRef = RemCRef<T>&;

	/// Marks the value as constant. Identical to @p std::as_const .
	template <class T> const T& AsConst(T&& x) { return x; }

	/// Removes all the qualifiers (const, reference and volatile qualifiers) on the type @p T .
	template <class T> using RemQual = std::remove_cvref_t<T>;

	template <class R> concept IsRawArray   = std::is_array_v<R>;
	template <class R> using   ArrayElement = RemRef<decltype(R {} [0])>;
	template <class R> constexpr usize ArrayLength = sizeof(R) / sizeof(ArrayElement<R>);

	/// Checks if the input type is @b identical to @p U .
	template <class T, class U> concept SameAs = std::is_same_v<T, U>;
	/// Checks if the input type is @b not @p U .
	template <class T, class U> concept DifferentTo = !std::is_same_v<T, U>;
	/// Checks if the input type is 'similar' to @p U, i.e. identical if ignoring qualifiers.
	template <class T, class U> concept SimilarTo = std::is_same_v<RemQual<T>, RemQual<U>>;
	/// Checks if the input type is 'distant' to @p U, i.e. distinct even when ignoring qualifiers.
	template <class T, class U> concept DistantTo = !std::is_same_v<RemQual<T>, RemQual<U>>;

	/// Checks if the input type derives from/is a subclass of/extends the base class @p Base .
	template <class Der, class Base> concept Extends = std::is_base_of_v<Base, Der>;
	/// Checks if the input type is a base class of the derived class @p Der .
	template <class Base, class Der> concept BaseOf = std::is_base_of_v<Base, Der>;
	/// Checks if the input type is convertible to the type @p To .
	template <class From, class To> concept ConvTo = std::is_convertible_v<From, To>;
	/// Checks if the input type is convertible from the type @p From.
	template <class To, class From> concept ConvFrom = std::is_convertible_v<From, To>;

	/// Checks if the input type 'implements' the CRTP interface @code Interface<Args...>@endcode.
	template <class Super, template <class...> class Interface, class... Args>
	concept Implements = Extends<RemQual<Super>, Interface<Args..., RemQual<Super>>>;

	namespace details {
		template <bool>
		struct IfElseBranch        { template <class WhenTrue, class WhenFalse> using Result = WhenTrue; };
		template <>
		struct IfElseBranch<false> { template <class WhenTrue, class WhenFalse> using Result = WhenFalse; };
	}

	/// Yields the type @p WhenTrue when the condition is @p true, and yields @p WhenFalse otherwise.
	/// Analogous to the ternary operator but for types.
	/// @warning The input types are not lazily evaluated. If either type contains a compiler error,
	/// <em>even if the type is not chosen</em>, the net expression will still contain compiler errors.
	template <bool Cond, class WhenTrue, class WhenFalse>
	using IfElse = typename details::IfElseBranch<Cond>::template Result<WhenTrue, WhenFalse>;

	/// Adds the const qualifier to @p T <em>if and only if</em> the type @p ConstIf is const.
	template <class T, class ConstIf> using AddConstIf = IfElse<IsConst<ConstIf>, const T, T>;
	/// Removes the const qualifier from @p T <em>if and only if</em> the type @p NoConstIf is @b not const.
	template <class T, class NoConstIf> using RemConstIf = IfElse<IsConst<NoConstIf>, T, RemConst<T>>;

	/// Yields the common convertible type between types @p T and @p U .
	template <class T, class U> using Common = RemQual<decltype(false ? std::declval<const T&>() : std::declval<const U&>())>;

	/// Checks if the input type can be trivially copied.
	template <class T> concept TrivialCopy = std::is_trivially_copyable_v<T>;
	/// Checks if the input type can be trivially destroyed.
	template <class T> concept TrivialDestruct = std::is_trivially_destructible_v<T>;

	namespace details {
		template <class T> struct StrongArray { using Result = T; };
		template <class T, usize N> struct StrongArray<T[N]> { using Result = Array<T, N>; };
	}

	/// Converts types into their 'strong' version, i.e. a more managable form.
	/// @b Example: @code
	/// Strong<int>  // -> still int
	/// Strong<T&>   // -> Ref<T>
	/// Strong<T[N]> // -> Array<T, N>
	/// @endcode
	template <class T>
	using Strong = IfElse<IsRef<T>,	Ref<T>, typename details::StrongArray<T>::Result>;

	/// Converts types into their 'optional strong' version.
	/// @b Example: @code
	/// OptStrong<int>  // -> Option<int>
	/// OptStrong<T&>   // -> OptRef<T>
	/// OptStrong<T[N]> // -> Option<Array<T, N>>
	/// @endcode
	template <class T>
	using OptStrong = IfElse<IsRef<T>, OptRef<T>, T>;

	// constexpr tools:

	/// A type that contains an integer sequence in its template input parameters.
	template <usize...> struct IntSeq {};

	template <usize N, usize... Is> struct MakeIntRange {
		using Result = typename MakeIntRange<N - 1, N - 1, Is...>::Result; };
	template <usize... Is> struct MakeIntRange<0, Is...> { using Result = IntSeq<Is...>; };

	/// Yields the integer sequence type that has the integer range @code [0..N]@endcode.
	template <usize N> using IntRangeSeq = typename MakeIntRange<N>::Result;

	/// Finds and returns the index of the type within the list.
	/// @b Example: @code
	/// IndexOfType<void,  int, char, float, void, double>() // 3
	/// IndexOfType<int,   int, char, float, void, double>() // 0
	/// IndexOfType<void*, int, char, float, void, double>() // error!
	/// @endcode
	/// @warning Will produce an error if @p T is not in the type list.
	template <class T, class First, class... Ts>
	static constexpr usize IndexOfType() {
		if constexpr (sizeof...(Ts) == 0 || SameAs<T, First>)
			return 0;
		else return 1 + IndexOfType<T, Ts...>();
	}
}
