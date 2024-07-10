#pragma once
#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"

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
            meshp.PushV(f(MData { Math::fVector2::ZERO() }));
            for (uint i = 0; i < opt.subdivisions; ++i) {
                meshp.PushV(f(MData { Math::fVector2::from_polar(1.0f, angle * (float)i) }));
                meshp.PushI(0, i + 1, (i + 1) % opt.subdivisions + 1);
            }
        }
    };

    inline static MeshConstructor<CircleCreator> Circle {};
}
