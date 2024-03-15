// ReSharper disable CppClangTidyClangDiagnosticRedundantParens
#pragma once
#include "Mesh.h"
#include "Vector.h"

namespace Graphics::MeshUtils {
    template <class T>
    struct identity_vec {
        T operator()(const T& t, uint) { return t; }
    };

#define MESH_UTIL_FN(N, V, ...) \
    template <stdu::fn_args<V, uint> F> \
    auto N(__VA_ARGS__ __VA_OPT__(,) F f) -> Mesh<decltype(f(V{}, 0))>; \
    \
    template <stdu::fn_args<V, uint> F> \
    auto N(__VA_ARGS__ __VA_OPT__(,) F f, const Maths::mat4x4& transform) -> Mesh<decltype(f(V{}, 0))> { \
        return N(__VA_ARGS__ __VA_OPT__(,) f).ApplyTransform(transform); \
    } \
    \
    inline Mesh<V> N(__VA_ARGS__) { \
        return N(__VA_ARGS__ __VA_OPT__(,) identity_vec<V> {}); \
    } \
    inline Mesh<V> N(__VA_ARGS__ __VA_OPT__(,) const Maths::mat4x4& transform) { \
        return N(__VA_ARGS__ __VA_OPT__(,) identity_vec<V> {}, transform); \
    } \

    MESH_UTIL_FN(SimpleCircleMesh, Maths::fvec2, int(subdivisions));
    MESH_UTIL_FN(SimpleCubeMesh, Maths::fvec3);
    template<stdu::fn_args<VertexNormal3D, uint> F>
    auto CubeMeshNorm(F f) -> Mesh<decltype(f(VertexNormal3D {}, 0))>;

    template<stdu::fn_args<VertexNormal3D, uint> F>
    auto CubeMeshNorm(F f, const Maths::mat4x4& transform) -> Mesh<decltype(f(VertexNormal3D {}, 0))> {
        return CubeMeshNorm(f).ApplyTransform(transform);
    }

    inline Mesh<VertexNormal3D> CubeMeshNorm() { return CubeMeshNorm(identity_vec<VertexNormal3D> {}); }

    inline Mesh<VertexNormal3D> CubeMeshNorm(const Maths::mat4x4& transform) {
        return CubeMeshNorm(identity_vec<VertexNormal3D> {}, transform);
    };

#undef MESH_UTIL_FN
}

#include "MeshUtilsImpl.h"