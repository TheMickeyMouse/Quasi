#pragma once

#define STDU_ENUM_BINARY_OP(E, op) static constexpr E operator op (int a, E b) { return (E)(a op (int)b); } \
                                   static constexpr E operator op (E a, int b) { return (E)((int)a op b); } /*NOLINT(bugprone-macro-parentheses)*/ \
                                   static constexpr E operator op (E a, E b) { return (E)((int)a op (int)b); } /*NOLINT(bugprone-macro-parentheses)*/

#define STDU_ENUM_CMP(E, op) static constexpr bool operator op (int a, E b) { return a op (int)b; } \
                             static constexpr bool operator op (E a, int b) { return (int)a op b; } \
                             static constexpr bool operator op (E a, E b) { return (int)a op (int)b; } \

#define STDU_ENUM_UNARY_OP(E, op) static constexpr E operator op (E x) { return (E)(op (int)x); }

#define STDU_ENUM_SELF_BINARY_OP(E, op) static constexpr E operator op##=  (E& a, int b) { a = (E)((int)a op b); return a; } /*NOLINT(bugprone-macro-parentheses)*/
#define STDU_ENUM_SELF_UNARY_OP(E, op)  static constexpr E operator op##op (E& a) { a = (E)((int)a op 1); return a; }               /*NOLINT(bugprone-macro-parentheses)*/ \
                                        static constexpr E operator op##op (E& a, int) { E t = a; a = (E)((int)a op 1); return t; } /*NOLINT(bugprone-macro-parentheses)*/

#define STDU_IMPL_ENUM_OPERATORS(E) \
    STDU_ENUM_BINARY_OP(E, +) \
    STDU_ENUM_BINARY_OP(E, -) \
    STDU_ENUM_BINARY_OP(E, *) \
    STDU_ENUM_BINARY_OP(E, /) \
    STDU_ENUM_BINARY_OP(E, %) \
    STDU_ENUM_BINARY_OP(E, &) \
    STDU_ENUM_BINARY_OP(E, |) \
    STDU_ENUM_BINARY_OP(E, ^) \
    STDU_ENUM_BINARY_OP(E, <<) \
    STDU_ENUM_BINARY_OP(E, >>) \
\
    STDU_ENUM_CMP(E, <) \
    STDU_ENUM_CMP(E, <=) \
    STDU_ENUM_CMP(E, >) \
    STDU_ENUM_CMP(E, >=) \
    STDU_ENUM_CMP(E, ==) \
    STDU_ENUM_CMP(E, !=) \
\
    STDU_ENUM_UNARY_OP(E, +) \
    STDU_ENUM_UNARY_OP(E, -) \
    STDU_ENUM_UNARY_OP(E, ~) \
\
    STDU_ENUM_SELF_BINARY_OP(E, +) \
    STDU_ENUM_SELF_BINARY_OP(E, -) \
    STDU_ENUM_SELF_BINARY_OP(E, *) \
    STDU_ENUM_SELF_BINARY_OP(E, /) \
    STDU_ENUM_SELF_BINARY_OP(E, %) \
\
    STDU_ENUM_SELF_BINARY_OP(E, &) \
    STDU_ENUM_SELF_BINARY_OP(E, |) \
    STDU_ENUM_SELF_BINARY_OP(E, ^) \
    STDU_ENUM_SELF_BINARY_OP(E, <<) \
    STDU_ENUM_SELF_BINARY_OP(E, >>) \
\
    STDU_ENUM_SELF_UNARY_OP(E, +) \
    STDU_ENUM_SELF_UNARY_OP(E, -) \

#define STDU_MATCH_ENUM(E, S) case _Enum_t::E: return S;
#define STDU_MATCH_SEQ(SEQ) STDU_CAT(__STDU_EM_1__ SEQ, END__)
#define __STDU_EM_1__(E, S) STDU_MATCH_ENUM(E, S) __STDU_EM_2__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __STDU_EM_2__(E, S) STDU_MATCH_ENUM(E, S) __STDU_EM_1__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __STDU_EM_1__END__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __STDU_EM_2__END__  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define STDU_ENUM_TOSTR(TYPE, NAME, SEQ, DEFAULT) const char* NAME(TYPE e) { \
    using _Enum_t = TYPE; switch (e) { STDU_MATCH_SEQ(SEQ) default: return DEFAULT; } }
