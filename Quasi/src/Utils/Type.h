#pragma once

#include <map>

#include "Numeric.h"

namespace Quasi {
    template <class T>
    using IList = std::initializer_list<T>;

    template <class T> using IterOf = typename T::const_iterator;
    template <class T> using IterMutOf = typename T::iterator;

	template <class T> using NoInfer = std::type_identity_t<T>;

	struct Empty {
		Empty(auto&&...) {}
	};

	template <class T> using Nullable = T;
	template <class T> using NotNull = T;
	template <class T> using InOut = T;
	template <class T> using Out = T;
	using Nullptr = std::nullptr_t;
    inline struct UncheckedMarker {} Unchecked;

	template <class> concept AlwaysTrue  = true;
	template <class> concept AlwaysFalse = false;

	template <class T> concept IsConst  = std::is_const_v<T>;
	template <class T> concept IsMut    = !IsConst<T>;
	template <class T> using   RemConst = std::remove_const_t<T>;

	template <class T> concept IsRef  = std::is_reference_v<T>;
	template <class T> concept IsPtr  = std::is_pointer_v<T>;
	template <class T> using   RemRef = std::remove_reference_t<T>;
	template <class T> using   RemPtr = std::remove_pointer_t<T>;

	template <class T> concept IsConstRef  = IsRef<T> && IsConst<RemRef<T>>;
	template <class T> concept IsConstPtr  = IsPtr<T> && IsConst<RemPtr<T>>;
	template <class T> concept IsMutRef    = IsRef<T> && IsMut<RemRef<T>>;
	template <class T> concept IsMutPtr    = IsPtr<T> && IsMut<RemPtr<T>>;
	template <class T> using   RemConstRef = RemRef<RemConst<T>>;
	template <class T> using   RemConstPtr = RemPtr<RemConst<T>>;
	template <class T> using   RemMutRef   = RemRef<T>;
	template <class T> using   RemMutPtr   = RemPtr<T>;

	template <class T> using   ToConstRef  = const RemRef<T>&;
	template <class T> using   ToMutRef    = RemConstRef<T>&;

	template <class T> using   RemQual = std::remove_cvref_t<T>;

	template <class R> concept IsRawArray   = std::is_array_v<R>;
	template <class R> using   ArrayElement = RemRef<decltype(R {} [0])>;
	template <class R> constexpr usize ArrayLength = sizeof(R) / sizeof(ArrayElement<R>);

	template <class T, class U>      concept SameAs      =  std::is_same_v<T, U>;
	template <class T, class U>      concept DifferentTo = !std::is_same_v<T, U>;
	template <class T, class U>      concept SimilarTo   =  std::is_same_v<RemQual<T>, RemQual<U>>;
	template <class T, class U>      concept DistantTo   = !std::is_same_v<RemQual<T>, RemQual<U>>;
	template <class Der, class Base> concept Extends     =  std::is_base_of_v<Base, Der>;
	template <class Base, class Der> concept BaseOf      =  std::is_base_of_v<Base, Der>;
	template <class From, class To>  concept ConvTo      =  std::is_convertible_v<From, To>;
	template <class To, class From>  concept ConvFrom    =  std::is_convertible_v<From, To>;

	template <class Super, template <class...> class Interface, class... Args>
	concept Implements = Extends<RemQual<Super>, Interface<Args..., RemQual<Super>>>;

	namespace details {
		template <bool>
		struct IfElseBranch        { template <class WhenTrue, class WhenFalse> using Result = WhenTrue; };
		template <>
		struct IfElseBranch<false> { template <class WhenTrue, class WhenFalse> using Result = WhenFalse; };
	}

	template <bool Cond, class WhenTrue, class WhenFalse>
	using IfElse = typename details::IfElseBranch<Cond>::template Result<WhenTrue, WhenFalse>;

	template <class T, class ConstIf>   using AddConstIf = IfElse<IsConst<ConstIf>,   const T, T>;
	template <class T, class NoConstIf> using RemConstIf = IfElse<IsConst<NoConstIf>, T, RemConst<T>>;

	template <class T, class U> using Common = decltype(false ? std::declval<T>() : std::declval<U>());

	template <bool Packed> struct MakeCommonResult {
		template <class T, class... Ts>
		using Result = decltype(false ?
			std::declval<T>() :
			std::declval<typename MakeCommonResult<(sizeof...(Ts) > 1)>::template Result<Ts...>>());
	};
	template <> struct MakeCommonResult<false> { template <class T> using Result = T; };

	template <class F, class... Ts> using CommonResult =
		typename MakeCommonResult<(sizeof...(Ts) > 1)>::template Result<
			decltype(std::declval<F>()(std::declval<const Ts&>()))...
		>;

	template <class T> concept TrivialCopy     = std::is_trivially_copyable_v<T>;
	template <class T> concept TrivialDestruct = std::is_trivially_destructible_v<T>;

	template <usize...> struct IntSeq {};

	template <usize N, usize... Is> struct MakeIntRange {
		using Result = typename MakeIntRange<N - 1, N - 1, Is...>::Result; };

	template <usize... Is> struct MakeIntRange<0, Is...> { using Result = IntSeq<Is...>; };

	template <usize N> using IntRangeSeq = typename MakeIntRange<N>::Result;

	template <class T, class First, class... Ts>
	static constexpr usize IndexOfType() {
		if constexpr (sizeof...(Ts) == 0 || SameAs<T, First>)
			return 0;
		else return 1 + IndexOfType<T, Ts...>();
	}
}
