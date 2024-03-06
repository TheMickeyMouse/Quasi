#pragma once

#define STDU_INTERNAL_CAT(A, B) A##B
#define STDU_CAT(A, B) STDU_INTERNAL_CAT(A, B)

#define STDU_UNARY(...) __VA_ARGS__
#define STDU_REMOVE_SCOPE(X) STDU_UNARY X

#define STDU_IF_0(...)
#define STDU_IF_1(...) __VA_ARGS__
#define STDU_IF(CONDITION, ...) STDU_CAT(STDU_IF_, CONDITION)(__VA_ARGS__)
#define STDU_IF_NOT(CONDITION, ...) STDU_IF(STDU_NOT(CONDITION), __VA_ARGS__)
#define STDU_IF_ELSE_0(A, B) STDU_REMOVE_SCOPE(B)
#define STDU_IF_ELSE_1(A, B) STDU_REMOVE_SCOPE(A)
#define STDU_IF_ELSE(CONDITION, A, B) STDU_CAT(STDU_IF_ELSE_, CONDITION)(A, B)

#define STDU_NOT_0 1
#define STDU_NOT_1 0
#define STDU_NOT(CONDITION) STDU_CAT(STDU_NOT_, CONDITION)

#define STDU_IS_EMPTY(...) STDU_CAT(__VA_OPT__(STDU_NOT_), 1)
#define STDU_HAS_ARGS(...) STDU_NOT(STDU_IS_EMPTY(__VA_ARGS__))

#define STDU_EXPAND(...) __EXPAND1__(__EXPAND1__(__EXPAND1__(__EXPAND1__(__VA_ARGS__))))
#define __EXPAND1__(...) __EXPAND2__(__EXPAND2__(__EXPAND2__(__EXPAND2__(__VA_ARGS__)))) // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __EXPAND2__(...) __EXPAND3__(__EXPAND3__(__EXPAND3__(__EXPAND3__(__VA_ARGS__)))) // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __EXPAND3__(...) __EXPAND4__(__EXPAND4__(__EXPAND4__(__EXPAND4__(__VA_ARGS__)))) // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __EXPAND4__(...) __VA_ARGS__ // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)

#define STDU_INTERNAL_TOSTR(...) #__VA_ARGS__
#define STDU_TOSTR(...) STDU_INTERNAL_TOSTR(__VA_ARGS__)

#define STDU_DO(...) do { __VA_ARGS__ } while (false)