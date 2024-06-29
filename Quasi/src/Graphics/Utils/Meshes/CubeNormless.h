#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct CubeNormlessCreator;

    template <> struct OptionsFor<CubeNormlessCreator> {};

    struct CubeNormlessCreator : MeshConstructor<CubeNormlessCreator> {
        using MData = VertexBuilder::MeshConstructData3D;

        CubeNormlessCreator(Options) {}

        template <FnArgs<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
            Mesh<T> mesh;
            mesh.indices.reserve(6 * 2);

            for (Math::Direction3D i = Math::Direction3D::RIGHT; i < 6; ++i) {
                // some magic that calculates quad indices
                const u32 ii = (u32)i, n = ii / 2, flip = (ii & 1) << n;
                const u32 t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
                mesh.indices.emplace_back(t1 ^ flip, t2 ^ flip, t3 ^ flip);
                mesh.indices.emplace_back(t2 ^ flip, t3 ^ flip, t4 ^ flip);
            }

            mesh.vertices.reserve(8);
            for (Math::Corner3D i = Math::Corner3D::FRONT_TOP_RIGHT; i < 8; ++i) {
                mesh.vertices.emplace_back(f(MData { .Position = Math::fVector3 { i }, .VertexIndex = (u32)i }));
            }

            return mesh;
        }
    };

    inline static MeshConstructor<CubeNormlessCreator> CubeNormless {};
}
