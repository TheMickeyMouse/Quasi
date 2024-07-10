#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct CubeNormlessCreator;

    template <> struct OptionsFor<CubeNormlessCreator> {
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct CubeNormlessCreator : MeshConstructor<CubeNormlessCreator> {
        CubeNormlessCreator(Options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            auto meshp = mesh.NewBatch();
            for (Math::Direction3D i = Math::Direction3D::RIGHT; i < 6; ++i) {
                // some magic that calculates quad indices
                const u32 ii = (u32)i, n = ii / 2, flip = (ii & 1) << n;
                const u32 t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
                meshp.PushI(t1 ^ flip, t2 ^ flip, t3 ^ flip);
                meshp.PushI(t2 ^ flip, t3 ^ flip, t4 ^ flip);
            }

            for (Math::Corner3D i = Math::Corner3D::FRONT_TOP_RIGHT; i < 8; ++i) {
                meshp.PushV(f(MData { .Position = Math::fVector3 { i } }));
            }
        }
    };

    inline static MeshConstructor<CubeNormlessCreator> CubeNormless {};
}
