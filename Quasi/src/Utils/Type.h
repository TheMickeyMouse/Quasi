#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "Macros.h"

namespace Quasi {
    using i8   = std::int8_t;
    using i16  = std::int16_t;
    using i32  = std::int32_t;
    using i64  = std::int64_t;
    using u8   = std::uint8_t;
    using u16  = std::uint16_t;
    using u32  = std::uint32_t;
    using u64  = std::uint64_t;
    using f32  = float;
    using f64  = double;
    using f80  = long double;
    using ushort = u16;
    using ulong  = u64;
    using usize  = std::size_t;
    using isize  = std::intptr_t;

	using uchar = unsigned char;
	using byte  = unsigned char;
	using sbyte = signed char;
	using uint  = unsigned int;

	static constexpr u8    U8_MAX    = ~0;
	static constexpr u16   U16_MAX   = ~0;
	static constexpr u32   U32_MAX   = ~0;
	static constexpr u64   U64_MAX   = ~0;
	static constexpr usize USIZE_MAX = ~0;
	static constexpr i8    I8_MAX    = U8_MAX >> 1;
	static constexpr i16   I16_MAX   = U16_MAX >> 1;
	static constexpr i32   I32_MAX   = U32_MAX >> 1;
	static constexpr i64   I64_MAX   = U64_MAX >> 1;
	static constexpr isize ISIZE_MAX = USIZE_MAX >> 1;
	static constexpr i8    I8_MIN    = ~I8_MAX;
	static constexpr i16   I16_MIN   = ~I16_MAX;
	static constexpr i32   I32_MIN   = ~I32_MAX;
	static constexpr i64   I64_MIN   = ~I64_MAX;
	static constexpr isize ISIZE_MIN = ~ISIZE_MAX;
	static constexpr byte  BYTE_MAX  = U8_MAX;

    using String = std::string;
    using Str    = std::string_view;

    template <class K, class V, class Cmp = std::less<K>, class Alc = std::allocator<std::pair<const K, V>>>
    using Map = std::map<K, V, Cmp, Alc>;

    template <class T>
    using IList = std::initializer_list<T>;

    using OutStream = std::ostream;

    template <class T> using IterOf = typename T::const_iterator;
    template <class T> using IterMutOf = typename T::iterator;

    template <class... Ts> using Tuple = std::tuple<Ts...>;
	template <class T, class U> using Pair = std::pair<T, U>;

	template <class T> using NoInfer = std::type_identity_t<T>;

	struct Empty {
		Empty(auto&&...) {}
	};

	template <class T> using Nullable = T;
	template <class T> using NotNull = T;
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
	template <class R> using   ArrayElement = decltype(R {} [0]);
	template <class R> constexpr usize ArrayLength() { return sizeof(R) / sizeof(ArrayElement<R>); }

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

	template <class T> concept Integer  = std::is_integral_v<T>;
	template <class T> concept Signed   = std::is_signed_v<T>;
	template <class T> concept Unsigned = std::is_unsigned_v<T>;
	template <class T> concept Floating = std::is_floating_point_v<T>;
	template <class T> concept Numeric  = std::is_arithmetic_v<T>;

	template <class T, class U> using Common = decltype(false ? std::declval<T>() : std::declval<U>());

	template <class T> concept TrivialCopy = std::is_trivially_copyable_v<T>;
}
