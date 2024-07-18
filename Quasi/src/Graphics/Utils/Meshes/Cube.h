#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct CubeCreator;

    template <> struct OptionsFor<CubeCreator> {
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct CubeCreator : MeshConstructor<CubeCreator> {
        CubeCreator(Options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            for (u32 i = 0; i < 6; ++i) {
                auto meshp = mesh.NewBatch();

                meshp.PushI(0, 1, 2);
                meshp.PushI(1, 2, 3);

                const Math::fVector3 norm = Math::fVector3::from_direction(i, 1);
                const u32 n = i / 2, flip = (i & 1) << n;
                const u32 t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
                meshp.PushV(f(MData { .Position = Math::fVector3::from_corner(t1 ^ flip, 1), .Normal = norm }));
                meshp.PushV(f(MData { .Position = Math::fVector3::from_corner(t2 ^ flip, 1), .Normal = norm }));
                meshp.PushV(f(MData { .Position = Math::fVector3::from_corner(t3 ^ flip, 1), .Normal = norm }));
                meshp.PushV(f(MData { .Position = Math::fVector3::from_corner(t4 ^ flip, 1), .Normal = norm }));
            }
        }
    };

    inline static MeshConstructor<CubeCreator> Cube {};
}
