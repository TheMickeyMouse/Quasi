#pragma once
#include <concepts>
#include <span>

#include "Debugging.h"

namespace stdu {
    struct empty {
        empty(auto ...) {}
    }; // used with [[no_unique_address]]

    template <class T> using is_void_t = std::is_same<T, void>;

    template <class T> constexpr bool is_void_v = std::is_same_v<T, void>;
    
    template <class T, class M> M decltype_member(M T::*) { return M {}; }

#define STDU_DECLTYPE_MEMBER(T, M) decltype(stdu::decltype_member(&T::M))

    template <class T, class U>
    struct arithmetic_t {
        static constexpr bool value = std::is_arithmetic_v<T> && std::is_arithmetic_v<U>;

        static T clamped_neg(T x) {
            if constexpr (std::is_unsigned_v<T>) return 0;
            else return -x;
        }

        static bool lt(T a, U b) { return a  < b; }
        static bool le(T a, U b) { return a <= b; }
        static bool eq(T a, U b) { return a == b; }
        static bool ge(T a, U b) { return a >= b; }
        static bool gt(T a, U b) { return a  > b; }
        static bool ne(T a, U b) { return a != b; }
    };

    template <class T, class U>
    concept convertible_to = std::is_convertible_v<T, U>;

    template <class R>
    concept array_like = std::ranges::contiguous_range<R> && std::ranges::sized_range<R>;

    template <class R>
    using element_t = std::ranges::range_value_t<R>;

    template <array_like R> // from https://stackoverflow.com/questions/77097857/conversion-of-stdvector-to-stdspant
    std::span<element_t<R>> to_span(R& r) { return std::span<std::ranges::range_value_t<R>>(r); }

    template <array_like R>
    std::span<const element_t<R>> to_cspan(const R& r) { return std::span<const std::ranges::range_value_t<R>>(r); }

    template <class T, class U>
    concept divides = sizeof(U) % sizeof(T) == 0;

    template <class U, class T> requires divides<T, U> || divides<U, T>
    std::span<U> span_cast(std::span<T> span) { return { (U*)span.data(), span.size_bytes() / sizeof(U) }; }

    using byte_span = std::span<uchar>;
    using cbyte_span = std::span<const uchar>;
}
