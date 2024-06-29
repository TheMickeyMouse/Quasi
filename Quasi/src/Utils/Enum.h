#pragma once
#include "Text.h"

namespace Quasi::Text {
    template <class E> concept EnumType = std::is_enum_v<E>;

    template <EnumType E>
    constexpr E EnumRangeMin() {
        if constexpr (requires { E::MIN; })
            return E::MIN;
        else return E { 0 };
    }

    template <EnumType E>
    constexpr E EnumRangeMax() {
        if constexpr (requires { E::NUM; })
            return E::NUM;
        else if constexpr (requires { E::MAX; })
            return E::MAX;
        else return nullptr;
    }

    namespace details {
        template <class E, E S1, E S2>
        Str enum_name_rec(E target) {
            if constexpr (S1 == S2) return ValueName<S2>();
            else {
                if (S1 == target) {
                    return ValueName<S1>();
                }
                return enum_name_rec<E, (E)((int)S1 + 1), S2>(target);
            }
        }
    }

    template <EnumType E>
    Str EnumName(E e) {
        constexpr E begin = EnumRangeMin<E>(), end = EnumRangeMax<E>();
        return details::enum_name_rec<E, begin, end>(e);
    }
}


namespace Quasi {
    template <Text::EnumType E, class T>
    struct EnumMap {
        Array<T, Text::EnumRangeMax<E>()> arr;

        constexpr EnumMap() = default;
        constexpr EnumMap(IList<std::pair<E, T>> pairs) {
            for (const auto& [e, t] : pairs) arr[(usize)e] = t;
        }

        [[nodiscard]] const T& operator[](E e) const { return arr[(usize)e]; }
        T& operator[](E e) { return arr[(usize)e]; }
    };
}


#define Q_ENUM_BINARY_OP(E, op) static constexpr E operator op (int a, E b) { return (E)(a op (int)b); } \
                                   static constexpr E operator op (E a, int b) { return (E)((int)a op b); } /*NOLINT(bugprone-macro-parentheses)*/ \
                                   static constexpr E operator op (E a, E b) { return (E)((int)a op (int)b); } /*NOLINT(bugprone-macro-parentheses)*/

#define Q_ENUM_CMP(E, op) static constexpr bool operator op (int a, E b) { return a op (int)b; } \
                             static constexpr bool operator op (E a, int b) { return (int)a op b; } \
                             static constexpr bool operator op (E a, E b) { return (int)a op (int)b; } \

#define Q_ENUM_UNARY_OP(E, op) static constexpr E operator op (E x) { return (E)(op (int)x); }

#define Q_ENUM_SELF_BINARY_OP(E, op) static constexpr E operator op##=  (E& a, int b) { a = (E)((int)a op b); return a; } /*NOLINT(bugprone-macro-parentheses)*/
#define Q_ENUM_SELF_UNARY_OP(E, op)  static constexpr E operator op##op (E& a) { a = (E)((int)a op 1); return a; }               /*NOLINT(bugprone-macro-parentheses)*/ \
                                        static constexpr E operator op##op (E& a, int) { E t = a; a = (E)((int)a op 1); return t; } /*NOLINT(bugprone-macro-parentheses)*/

#define Q_IMPL_ENUM_OPERATORS(E) \
    Q_ENUM_BINARY_OP(E, +) \
    Q_ENUM_BINARY_OP(E, -) \
    Q_ENUM_BINARY_OP(E, *) \
    Q_ENUM_BINARY_OP(E, /) \
    Q_ENUM_BINARY_OP(E, %) \
    Q_ENUM_BINARY_OP(E, &) \
    Q_ENUM_BINARY_OP(E, |) \
    Q_ENUM_BINARY_OP(E, ^) \
    Q_ENUM_BINARY_OP(E, <<) \
    Q_ENUM_BINARY_OP(E, >>) \
\
    Q_ENUM_CMP(E, <) \
    Q_ENUM_CMP(E, <=) \
    Q_ENUM_CMP(E, >) \
    Q_ENUM_CMP(E, >=) \
    Q_ENUM_CMP(E, ==) \
    Q_ENUM_CMP(E, !=) \
\
    Q_ENUM_UNARY_OP(E, +) \
    Q_ENUM_UNARY_OP(E, -) \
    Q_ENUM_UNARY_OP(E, ~) \
\
    Q_ENUM_SELF_BINARY_OP(E, +) \
    Q_ENUM_SELF_BINARY_OP(E, -) \
    Q_ENUM_SELF_BINARY_OP(E, *) \
    Q_ENUM_SELF_BINARY_OP(E, /) \
    Q_ENUM_SELF_BINARY_OP(E, %) \
\
    Q_ENUM_SELF_BINARY_OP(E, &) \
    Q_ENUM_SELF_BINARY_OP(E, |) \
    Q_ENUM_SELF_BINARY_OP(E, ^) \
    Q_ENUM_SELF_BINARY_OP(E, <<) \
    Q_ENUM_SELF_BINARY_OP(E, >>) \
\
    Q_ENUM_SELF_UNARY_OP(E, +) \
    Q_ENUM_SELF_UNARY_OP(E, -) \

#define Q_MATCH_ENUM(E, S) case _Enum_t::E: return S;
#define Q_MATCH_SEQ(SEQ) Q_CAT(__Q_EM_1__ SEQ, END__)
#define __Q_EM_1__(E, S) Q_MATCH_ENUM(E, S) __Q_EM_2__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __Q_EM_2__(E, S) Q_MATCH_ENUM(E, S) __Q_EM_1__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __Q_EM_1__END__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __Q_EM_2__END__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define Q_ENUM_TOSTR(TYPE, NAME, SEQ, DEFAULT) Quasi::Str NAME(TYPE e) { \
    using _Enum_t = TYPE; switch (e) { Q_MATCH_SEQ(SEQ) default: return DEFAULT; } }
