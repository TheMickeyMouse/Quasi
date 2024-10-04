#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <vector>
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

    using String = std::string;
    using Str    = std::string_view;

    template <class T> using Vec = std::vector<T>;
    template <class K, class V, class Cmp = std::less<K>, class Alc = std::allocator<std::pair<const K, V>>>
    using Map = std::map<K, V, Cmp, Alc>;

    template <class T, usize N>
    using Array = std::array<T, N>;

    template <class T>
    using IList = std::initializer_list<T>;

    using OutStream = std::ostream;
	template <class T, class Alc = std::default_delete<T>> using UniqueRef = std::unique_ptr<T, Alc>;
	template <class T> UniqueRef<T> NewUnique(auto&&... args) { return std::make_unique<T>((decltype(args))args...); }

    template <class T> using IterOf = typename T::const_iterator;
    template <class T> using IterMutOf = typename T::iterator;

    template <class... Ts> using Tuple = std::tuple<Ts...>;
	template <class T, class U> using Pair = std::pair<T, U>;

	template <class T> using Implicit = std::type_identity_t<T>;

	// concepts after here

    template <class T, usize Ext = std::dynamic_extent> using Span = std::span<T, Ext>;
#pragma region Spanlike Types
    using Bytes    = Span<const byte>;
    using MutBytes = Span<byte>;

    template <class R> concept ArrayLike = std::ranges::contiguous_range<R> && std::ranges::sized_range<R>;
    template <class R> using ArrayElement = std::ranges::range_value_t<R>;
	template <class R, class T> concept ArrayOf = ArrayLike<R> && std::is_same_v<T, ArrayElement<R>>;

    template <class R> concept CollectionLike = std::ranges::range<R>;
    template <class R, class E> concept CollectionOf = CollectionLike<R> && std::is_same_v<ArrayElement<R>, E>;

    template <ArrayLike R> // from https://stackoverflow.com/questions/77097857/conversion-of-stdvector-to-stdspant
    Span<ArrayElement<R>> TakeSpan(R& r) { return Span<ArrayElement<R>>(r); }

    template <ArrayLike R>
    Span<const ArrayElement<R>> TakeSpan(const R& r) { return Span<const ArrayElement<R>>(r); }

    template <class T, class U> concept DividesSize = sizeof(U) % sizeof(T) == 0;
    template <class U, class T> requires DividesSize<T, U> || DividesSize<U, T>
    Span<U> CastSpan(Span<T> span) { return { reinterpret_cast<U*>(span.data()), span.size_bytes() / sizeof(U) }; }

	template <class T> Span<const byte> BytesOf(const T& value) { return { reinterpret_cast<const byte*>(&value), sizeof(T) }; }
	template <class T> Span<const byte> BytesOf(Span<const T> value) { return { reinterpret_cast<const byte*>(value.data()), value.size_bytes() }; }
#pragma endregion

	struct Empty {
		Empty(auto&&...) {}
	};
	template <class T> using Nullable = T;
	template <class T> using NotNull = T;
	template <class T> using Out = T;

	template <class> concept AlwaysTrue  = true;
	template <class> concept AlwaysFalse = false;

	template <class T> constexpr bool IsVoid = std::is_same_v<std::remove_cvref_t<T>, void>;

	namespace details {
		template <class T, class M> M decltype_member(M T::*) { return M {}; }
		template <class T, class M> T decltype_structure(M T::*) { return T {}; }
	}
	template <auto P> using StructureT = decltype(details::decltype_structure(P));
	template <auto P> using MemberT    = decltype(details::decltype_member(P));

    // adapted from https://stackoverflow.com/a/59522794/19968422
    namespace details {
        template <class T>
        [[nodiscard]] constexpr Str raw_typename() {
			return Q_FUNC_NAME();
        }

        constexpr usize type_junk_prefix = raw_typename<int>().find("int");
        constexpr usize type_junk_size = raw_typename<int>().size() - 3;
        // ReSharper disable once CppStaticAssertFailure
        static_assert(type_junk_prefix != Str::npos, "cannot determine type signature in this compiler");
    }

    template <typename T>
    [[nodiscard]] constexpr Str TypeName() {
        const Str fullname = details::raw_typename<T>();
        return fullname.substr(details::type_junk_prefix, fullname.size() - details::type_junk_size);
    }

    template <class T, class U>
    constexpr bool EqTyped(T&& t, U&& u) {
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>) return t == u;
        else return false;
    }

    inline struct UncheckedMarker {} Unchecked;
}
