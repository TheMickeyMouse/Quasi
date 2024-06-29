#pragma once
#include "Macros.h"

#define LAMB(...) Q_LAMBDA(__VA_ARGS__)
#define LAMB_L(...) Q_LAMBDA_L(__VA_ARGS__)

#define Q_LAMBDA(...) Q_CAT(Q_LAMBDA_, __Q_GET_LAMBDA_ARGS_NUM__(__VA_ARGS__))(__VA_ARGS__)
#define Q_LAMBDA_L(...) Q_CAT(Q_LAMBDA_L, __Q_GET_LAMBDA_ARGS_NUM__(__VA_ARGS__))(__VA_ARGS__)

#define __Q_GET_LAMBDA_ARGS__(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, ...) A11
#define __Q_GET_LAMBDA_ARGS_NUM__(...) __Q_GET_LAMBDA_ARGS__(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define Q_LAMBDA_0(R) [&] { return R; }
#define Q_LAMBDA_1(A1, R) [&](A1) { return R; }
#define Q_LAMBDA_2(A1, A2, R) [&](A1, A2) { return R; }
#define Q_LAMBDA_3(A1, A2, A3, R) [&](A1, A2, A3) { return R; }
#define Q_LAMBDA_4(A1, A2, A3, A4, R) [&](A1, A2, A3, A4) { return R; }
#define Q_LAMBDA_5(A1, A2, A3, A4, A5, R) [&](A1, A2, A3, A4, A5) { return R; }
#define Q_LAMBDA_6(A1, A2, A3, A4, A5, A6, R) [&](A1, A2, A3, A4, A5, A6) { return R; }
#define Q_LAMBDA_7(A1, A2, A3, A4, A5, A6, A7, R) [&](A1, A2, A3, A4, A5, A6, A7) { return R; }
#define Q_LAMBDA_8(A1, A2, A3, A4, A5, A6, A7, A8, R) [&](A1, A2, A3, A4, A5, A6, A7, A8) { return R; }
#define Q_LAMBDA_9(A1, A2, A3, A4, A5, A6, A7, A8, A9, R) [&](A1, A2, A3, A4, A5, A6, A7, A8, A9) { return R; }

#define Q_LAMBDA_L0(A1, R) [&] { R; }
#define Q_LAMBDA_L1(A1, R) [&](A1) { R; }
#define Q_LAMBDA_L2(A1, A2, R) [&](A1, A2) { R; }
#define Q_LAMBDA_L3(A1, A2, A3, R) [&](A1, A2, A3) { R; }
#define Q_LAMBDA_L4(A1, A2, A3, A4, R) [&](A1, A2, A3, A4) { R; }
#define Q_LAMBDA_L5(A1, A2, A3, A4, A5, R) [&](A1, A2, A3, A4, A5) { R; }
#define Q_LAMBDA_L6(A1, A2, A3, A4, A5, A6, R) [&](A1, A2, A3, A4, A5, A6) { R; }
#define Q_LAMBDA_L7(A1, A2, A3, A4, A5, A6, A7, R) [&](A1, A2, A3, A4, A5, A6, A7) { R; }
#define Q_LAMBDA_L8(A1, A2, A3, A4, A5, A6, A7, A8, R) [&](A1, A2, A3, A4, A5, A6, A7, A8) { R; }
#define Q_LAMBDA_L9(A1, A2, A3, A4, A5, A6, A7, A8, A9, R) [&](A1, A2, A3, A4, A5, A6, A7, A8, A9) { R; }

