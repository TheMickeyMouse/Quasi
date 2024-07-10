#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct PlaneCreator;

    template <> struct OptionsFor<PlaneCreator> {
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct PlaneCreator : MeshConstructor<PlaneCreator> {
        PlaneCreator(Options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& out) {
            auto meshp = out.NewBatch();
            meshp.PushV(f(MData { .Position = { +1, 0, +1 } }));
            meshp.PushV(f(MData { .Position = { +1, 0, -1 } }));
            meshp.PushV(f(MData { .Position = { -1, 0, +1 } }));
            meshp.PushV(f(MData { .Position = { -1, 0, -1 } }));
            meshp.PushI(0, 2, 1);
            meshp.PushI(1, 2, 3);
        }
    };

    inline static MeshConstructor<PlaneCreator> Plane {};
} // Graphics::MeshUtils
