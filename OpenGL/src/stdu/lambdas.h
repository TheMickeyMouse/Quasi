#pragma once
#include "macros.h"

#define LAMB(...) STDU_LAMBDA(__VA_ARGS__)
#define LAMB_L(...) STDU_LAMBDA_L(__VA_ARGS__)

#define STDU_LAMBDA(...) STDU_CAT(STDU_LAMBDA_, __STDU_GET_LAMBDA_ARGS_NUM__(__VA_ARGS__))(__VA_ARGS__)
#define STDU_LAMBDA_L(...) STDU_CAT(STDU_LAMBDA_L, __STDU_GET_LAMBDA_ARGS_NUM__(__VA_ARGS__))(__VA_ARGS__)

#define __STDU_GET_LAMBDA_ARGS__(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, ...) A11
#define __STDU_GET_LAMBDA_ARGS_NUM__(...) __STDU_GET_LAMBDA_ARGS__(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define STDU_LAMBDA_0(R) [&] { return R; }
#define STDU_LAMBDA_1(A1, R) [&](A1) { return R; }
#define STDU_LAMBDA_2(A1, A2, R) [&](A1, A2) { return R; }
#define STDU_LAMBDA_3(A1, A2, A3, R) [&](A1, A2, A3) { return R; }
#define STDU_LAMBDA_4(A1, A2, A3, A4, R) [&](A1, A2, A3, A4) { return R; }
#define STDU_LAMBDA_5(A1, A2, A3, A4, A5, R) [&](A1, A2, A3, A4, A5) { return R; }
#define STDU_LAMBDA_6(A1, A2, A3, A4, A5, A6, R) [&](A1, A2, A3, A4, A5, A6) { return R; }
#define STDU_LAMBDA_7(A1, A2, A3, A4, A5, A6, A7, R) [&](A1, A2, A3, A4, A5, A6, A7) { return R; }
#define STDU_LAMBDA_8(A1, A2, A3, A4, A5, A6, A7, A8, R) [&](A1, A2, A3, A4, A5, A6, A7, A8) { return R; }
#define STDU_LAMBDA_9(A1, A2, A3, A4, A5, A6, A7, A8, A9, R) [&](A1, A2, A3, A4, A5, A6, A7, A8, A9) { return R; }

#define STDU_LAMBDA_L0(A1, R) [&] { R; }
#define STDU_LAMBDA_L1(A1, R) [&](A1) { R; }
#define STDU_LAMBDA_L2(A1, A2, R) [&](A1, A2) { R; }
#define STDU_LAMBDA_L3(A1, A2, A3, R) [&](A1, A2, A3) { R; }
#define STDU_LAMBDA_L4(A1, A2, A3, A4, R) [&](A1, A2, A3, A4) { R; }
#define STDU_LAMBDA_L5(A1, A2, A3, A4, A5, R) [&](A1, A2, A3, A4, A5) { R; }
#define STDU_LAMBDA_L6(A1, A2, A3, A4, A5, A6, R) [&](A1, A2, A3, A4, A5, A6) { R; }
#define STDU_LAMBDA_L7(A1, A2, A3, A4, A5, A6, A7, R) [&](A1, A2, A3, A4, A5, A6, A7) { R; }
#define STDU_LAMBDA_L8(A1, A2, A3, A4, A5, A6, A7, A8, R) [&](A1, A2, A3, A4, A5, A6, A7, A8) { R; }
#define STDU_LAMBDA_L9(A1, A2, A3, A4, A5, A6, A7, A8, A9, R) [&](A1, A2, A3, A4, A5, A6, A7, A8, A9) { R; }

