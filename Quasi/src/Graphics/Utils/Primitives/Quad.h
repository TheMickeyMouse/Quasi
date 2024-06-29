#pragma once

#include <array>

#include "Mesh.h"
#include "Vector.h"

namespace Quasi::Graphics::Primitives {
    class Quad {
    private:
        using fVector3 = Math::fVector3;
        fVector3 center, x, y;
    public:
        Quad() = default;
        Quad(const fVector3& center, const fVector3& x, const fVector3& y) : center(center), x(x), y(y) {}

        static Quad FromCornerToCorners(const fVector3& corner, const fVector3& toC1, const fVector3& toC2);
        static Quad FromCenterToCorners(const fVector3& center, const fVector3& toC1, const fVector3& toC2);

        void Transform(const Math::Matrix3D& transform);
        friend Quad operator*(const Math::Matrix3D& transform, const Quad& mesh);

        template <class T> Mesh<T> IntoMesh(fVector3 T::* prop = &T::Position);
        template <class T, class F>
        Mesh<T> IntoMesh(F f, std::remove_reference_t<decltype(f(fVector3 {}))> T::* prop = &T::Position);

        [[nodiscard]] fVector3 Origin() const { return center; }
        [[nodiscard]] fVector3 Normal() const { return x.cross(y).norm(); }
        Quad& Center() { center = 0; return *this; }

        template <class T> friend class Mesh;
    };

    Quad operator*(const Math::Matrix3D& transform, const Quad& mesh);

    template <class T>
    Mesh<T> Quad::IntoMesh(fVector3 T::* prop) {
        return IntoMesh(std::identity {}, prop);
    }

    template <class T, class F>
    Mesh<T> Quad::IntoMesh(F f, std::remove_reference_t<decltype(f(fVector3 {}))> T::* prop) {
        Vec<T> vert;
        vert.resize(4);
        vert[0].*prop = f(center + x + y);
        vert[1].*prop = f(center + x - y);
        vert[2].*prop = f(center - x + y);
        vert[3].*prop = f(center - x - y);

        return Mesh<T> { std::move(vert), { { 0, 1, 2 }, { 1, 2, 3 } } };
    }
}

namespace Quasi::Graphics {
    template <class T>
    template <Fn<T, Math::fVector3> F>
    void Mesh<T>::AddQuad(const Primitives::Quad& quad, F&& f) {
        const u32 i = (u32)vertices.size();
        vertices.emplace_back(f(quad.center + quad.x + quad.y));
        vertices.emplace_back(f(quad.center + quad.x - quad.y));
        vertices.emplace_back(f(quad.center - quad.x + quad.y));
        vertices.emplace_back(f(quad.center - quad.x - quad.y));
        indices.emplace_back(i, i + 1, i + 2);
        indices.emplace_back(i + 1, i + 2, i + 3);
    }
}