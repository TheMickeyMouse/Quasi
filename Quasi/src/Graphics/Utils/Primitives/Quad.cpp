#include "Quad.h"

namespace Quasi::Graphics::Primitives {
    Quad Quad::FromCornerToCorners(const fVector3& corner, const fVector3& toC1, const fVector3& toC2) {
        fVector3 h1 = toC1 / 2.0f;
        fVector3 h2 = toC2 / 2.0f;
        return { corner + h1 + h2, h1, h2 };
    }

    Quad Quad::FromCenterToCorners(const fVector3& center, const fVector3& toC1, const fVector3& toC2) {
        return { center, (toC1 + toC2) / 2.0f, (toC2 - toC1) / 2.0f };
    }

    void Quad::Transform(const Math::Matrix3D& transform) {
        *this = transform * *this;
    }

    Quad operator*(const Math::Matrix3D& transform, const Quad& mesh) {
        return Quad { transform * mesh.center, transform * mesh.x, transform * mesh.y };
    }
}