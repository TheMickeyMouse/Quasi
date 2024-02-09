#pragma once
#include <concepts>

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
}