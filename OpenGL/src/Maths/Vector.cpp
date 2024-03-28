#include "Vector.h"
#include "Color.h"

namespace Maths {
    template struct vecn<2, float>;
    template struct vecn<3, float>;
    template struct vecn<4, float>;
    template struct vecn<2, double>;
    template struct vecn<3, double>;
    template struct vecn<4, double>;
    template struct vecn<2, int>;
    template struct vecn<3, int>;
    template struct vecn<4, int>;
    template struct vecn<2, uint>;
    template struct vecn<3, uint>;
    template struct vecn<4, uint>;
    template struct vecn<2, uchar>;
    template struct vecn<3, uchar>;
    template struct vecn<4, uchar>;
}