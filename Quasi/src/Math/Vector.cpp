#include "Vector.h"

namespace Quasi::Math {
    template struct VectorN<2, float>;
    template struct VectorN<3, float>;
    template struct VectorN<4, float>;
    template struct VectorN<2, double>;
    template struct VectorN<3, double>;
    template struct VectorN<4, double>;
    template struct VectorN<2, int>;
    template struct VectorN<3, int>;
    template struct VectorN<4, int>;
    template struct VectorN<2, uint>;
    template struct VectorN<3, uint>;
    template struct VectorN<4, uint>;
    template struct VectorN<2, byte>;
    template struct VectorN<3, byte>;
    template struct VectorN<4, byte>;
}