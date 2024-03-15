#include "Quad.h"

namespace Graphics::Primitives {
    Quad Quad::FromCornerToCorners(const fvec3& corner, const fvec3& toC1, const fvec3& toC2) {
        fvec3 h1 = toC1 / 2.0f;
        fvec3 h2 = toC2 / 2.0f;
        return { corner + h1 + h2, h1, h2 };
    }

    Quad Quad::FromCenterToCorners(const fvec3& center, const fvec3& toC1, const fvec3& toC2) {
        return { center, (toC1 + toC2) / 2.0f, (toC2 - toC1) / 2.0f };
    }

    std::array<Maths::fvec3, 4> Quad::GetVertices() const {
        return std::array {
            center + x + y,
            center + x - y,
            center - x + y,
            center - x - y
        };
    }

    void Quad::Transform(const Maths::mat3D& transform) {
        *this = transform * *this;
    }

    Quad operator*(const Maths::mat3D& transform, const Quad& mesh) {
        return Quad { transform * mesh.center, transform * mesh.x, transform * mesh.y };
    }
}