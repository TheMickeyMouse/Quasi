#pragma once

namespace stdu {
    struct empty {
        empty(auto ...) {}
    }; // used with [[no_unique_address]]

    template <class T> using is_void_t = std::is_same<T, void>;

    template <class T> constexpr bool is_void_v = std::is_same_v<T, void>;
    
    template <class T, class M> M decltype_member(M T::*) { return M {}; }

#define STDU_DECLTYPE_MEMBER(T, M) decltype(stdu::decltype_member(&T::M))

    template <class, class> struct mod_t { using type = void; };
    template <class T, std::floating_point F> requires !std::is_floating_point_v<T> struct mod_t<T, F> { using type = decltype(std::fmod((T)0, (F)1)); };
    template <std::floating_point F, class T> requires !std::is_floating_point_v<T> struct mod_t<F, T> { using type = decltype(std::fmod((F)0, (T)1)); };
    template <std::floating_point F, std::floating_point G> struct mod_t<F, G> { using type = decltype(std::fmod((G)0, (F)1)); };
    template <std::integral I, std::integral J> struct mod_t<I, J> { using type = decltype((I)0 % (J)1); };

    template <class T, class U>
    struct arithmetic_t {
        static constexpr bool value = std::is_arithmetic_v<T> && std::is_arithmetic_v<U>;
        using add_t = std::enable_if_t<value, decltype((T)0 + (U)0)>;
        using sub_t = std::enable_if_t<value, decltype((T)0 - (U)0)>;
        using mul_t = std::enable_if_t<value, decltype((T)0 * (U)0)>;
        using div_t = std::enable_if_t<value, decltype((T)0 / (U)1)>;
        using mod_t = std::enable_if_t<value, typename mod_t<T, U>::type>;
        
        static add_t add(T a, U b) {
            return (add_t)a + (add_t)b;
        }
        
        static sub_t sub(T a, U b) {
            return (sub_t)a - (sub_t)b;
        }
        
        static mul_t mul(T a, U b) {
            return (mul_t)a * (mul_t)b;
        }
        
        static div_t div(T a, U b) {
            return (div_t)a / (div_t)b;
        }
        
        static mod_t mod(T a, U b) {
            if constexpr (std::is_floating_point_v<T>) return std::fmod(a, b);
            else return (mod_t)a % (mod_t)b;
        }

        static T clamped_neg(T x) {
            if constexpr (std::is_unsigned_v<T>) return 0;
            else return -x;
        }
    };

    template <class F, class... Ts> struct or_ts {
        using type = std::conditional_t<std::is_same_v<F, void>, typename or_ts<Ts...>::type, F>;
    };
    template <class F> struct or_ts<F> { using type = F; };

    template <class F, class... Ts> using or_t = typename or_ts<F, Ts...>::type;
}