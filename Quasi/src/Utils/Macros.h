#pragma once

#define Q_INTERNAL_CAT(A, B) A##B
#define Q_CAT(A, B) Q_INTERNAL_CAT(A, B)

#define Q_UNARY(...) __VA_ARGS__
#define Q_REMOVE_SCOPE(X) Q_UNARY X
#define Q_REMOVE_FIRST(FIRST, ...) __VA_ARGS__
#define Q_GET_FIRST(FIRST, ...) FIRST

#define Q_IF_0(...)
#define Q_IF_1(...) __VA_ARGS__
#define Q_IF(CONDITION, ...) Q_CAT(Q_IF_, CONDITION)(__VA_ARGS__)
#define Q_IF_NOT(CONDITION, ...) Q_IF(Q_NOT(CONDITION), __VA_ARGS__)
#define Q_IF_ELSE_0(A, B) Q_REMOVE_SCOPE(B)
#define Q_IF_ELSE_1(A, B) Q_REMOVE_SCOPE(A)
#define Q_IF_ELSE(CONDITION, A, B) Q_CAT(Q_IF_ELSE_, CONDITION)(A, B)

#define Q_NOT_0 1
#define Q_NOT_1 0
#define Q_NOT(CONDITION) Q_CAT(Q_NOT_, CONDITION)

#define Q_IS_EMPTY(...) Q_CAT(__VA_OPT__(Q_NOT_), 1)
#define Q_HAS_ARGS(...) Q_NOT(Q_IS_EMPTY(__VA_ARGS__))
#define Q_SEQUENCE_TO_ARGS(SEQ) Q_CAT(Q_S2R_1_ SEQ, END_)
#define Q_S2R_1_(X) Q_DEFER(Q_COMMA)() X Q_S2R_2_
#define Q_S2R_2_(X) Q_DEFER(Q_COMMA)() X Q_S2R_1_
#define Q_S2R_1_END_
#define Q_S2R_2_END_

#define Q_EXPAND(...)   Q_EXPAND1_(Q_EXPAND1_(Q_EXPAND1_(Q_EXPAND1_(__VA_ARGS__))))
#define Q_EXPAND1_(...) Q_EXPAND2_(Q_EXPAND2_(Q_EXPAND2_(Q_EXPAND2_(__VA_ARGS__)))) // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define Q_EXPAND2_(...) Q_EXPAND3_(Q_EXPAND3_(Q_EXPAND3_(Q_EXPAND3_(__VA_ARGS__)))) // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define Q_EXPAND3_(...) Q_EXPAND4_(Q_EXPAND4_(Q_EXPAND4_(Q_EXPAND4_(__VA_ARGS__)))) // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define Q_EXPAND4_(...) __VA_ARGS__ // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)

#define Q_INTERNAL_TOSTR(...) #__VA_ARGS__
#define Q_TOSTR(...) Q_INTERNAL_TOSTR(__VA_ARGS__)

#define Q_DO(...) do { __VA_ARGS__ } while (false)

#define Q_COMMA() ,
#define Q_LPAREN() (
#define Q_RPAREN() )
#define Q_EMPTY()
#define Q_DEFER(id) id Q_EMPTY()

#define Q_STRLIT_LEN(STR) (sizeof(STR) - 1)
#define Q_GETTER_MUT(FN, ...) (decltype(this->FN(__VA_ARGS__)))((const std::remove_pointer_t<decltype(this)>*)this)->FN(__VA_ARGS__)

#ifdef __COUNTER__
#define Q_COUNTER() __COUNTER__
#else
#error "__COUNTER__" Macro Not Supported!
#endif

#if defined(__GNUC__) || defined(__clang__)
#define Q_FUNC_NAME() __PRETTY_FUNCTION__
#else
#ifdef _MSVC
#define Q_FUNC_NAME() __FUNCSIG__
#else
#error "Function Signature macro not supported"
#endif
#endif