#include "Tri.h"

namespace Quasi::Graphics::Primitives {
    Tri Tri::FromGeometry(Math::fTriangle3D geom) {
        return { geom.p1, geom.p2, geom.p3 };
    }

    void Tri::Transform(const Math::Matrix3D& transform) {
        *this = transform * *this;
    }

    Tri operator*(const Math::Matrix3D& transform, const Tri& mesh) {
        return Tri { transform * mesh.a, transform * mesh.b, transform * mesh.c };
    }
}