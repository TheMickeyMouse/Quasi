#pragma once

#include "MeshConstructor.h"
#include "Graphicals/Mesh.h"
#include "Utils/Numeric.h"

namespace Quasi::Graphics::VertexBuilder {
    struct MeshConstructData2D;
}

namespace Quasi::Graphics::MeshUtils {
    struct CircleCreator;

    template <> struct OptionsFor<CircleCreator> {
        u32 subdivisions;
        using MData = VertexBuilder::MeshConstructData2D;
    };

    struct CircleCreator : MeshConstructor<CircleCreator> {
        Options opt;

        CircleCreator(const Options& opt) : opt(opt) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            const float angle = Math::TAU / (float)opt.subdivisions;

            auto meshp = mesh.NewBatch();
            meshp.PushV(f(MData { Math::fv2::Zero() }));
            for (u32 i = 0; i < opt.subdivisions; ++i) {
                meshp.PushV(f(MData { Math::fv2::FromPolar(1.0f, Math::Radians(angle * (float)i)) }));
                meshp.PushI(0, i + 1, (i + 1) % opt.subdivisions + 1);
            }
        }
    };

    inline static MeshConstructor<CircleCreator> Circle {};
}
