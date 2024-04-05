// ReSharper disable CppClangTidyClangDiagnosticRedundantParens
#pragma once
#include "Mesh.h"
#include "VertexConverter.h"

namespace Graphics::MeshUtils {
#define VECTOR_FOR_2D Maths::fvec2
#define VECTOR_FOR_3D VertexNormal3D
#define VECTOR_FOR(D) STDU_CAT(VECTOR_FOR_, D)

    struct default_builder_2D {
        VECTOR_FOR_2D operator()(const VertexBuilder::MeshConstructData2D& args) const { return args.Position; }
    };

    struct default_builder_3D {
        VECTOR_FOR_3D operator()(const VertexBuilder::MeshConstructData3D& args) const { return { args.Position, args.Normal }; }
    };

#define MESH_UTIL_FN(N, D, ...) \
    template <stdu::fn_args<const VertexBuilder::MeshConstructData##D&> F> \
    auto N(__VA_ARGS__ __VA_OPT__(,) F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData##D{}))>; \
    \
    template <stdu::fn_args<const VertexBuilder::MeshConstructData##D&> F> \
    auto N(__VA_ARGS__ __VA_OPT__(,) F&& f, const Maths::mat4x4& transform) -> Mesh<decltype(f(VertexBuilder::MeshConstructData##D{}))> { \
        return N(__VA_ARGS__ __VA_OPT__(,) f).ApplyTransform(transform); \
    } \
    \
    inline Mesh<VECTOR_FOR(D)> N(__VA_ARGS__) { \
        return N(__VA_ARGS__ __VA_OPT__(,) default_builder_##D {}); \
    } \
    inline Mesh<VECTOR_FOR(D)> N(__VA_ARGS__ __VA_OPT__(,) const Maths::mat4x4& transform) { \
        return N(__VA_ARGS__ __VA_OPT__(,) default_builder_##D {}, transform); \
    } \

    MESH_UTIL_FN(Circle, 2D, uint(subdivisions));
    MESH_UTIL_FN(CubeNormless, 3D);
    MESH_UTIL_FN(Cube, 3D);
    MESH_UTIL_FN(Sphere, 3D, Maths::uvec2(sections));
    MESH_UTIL_FN(Icosphere, 3D, uint(subdivisions));

#undef MESH_UTIL_FN
#undef VECTOR_FOR
#undef VECTOR_FOR_2D
#undef VECTOR_FOR_3D
}

#include "MeshUtilsImpl.h"