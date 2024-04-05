#pragma once
#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"
#include "NumTypes.h"

namespace Graphics::MeshUtils {
    struct CircleCreator;

    template <> struct OptionsFor<CircleCreator> {
        uint subdivisions;
    };

    struct CircleCreator : MeshConstructor<CircleCreator> {
        Options opt;
        using MData = VertexBuilder::MeshConstructData2D;

        CircleCreator(const Options& opt) : opt(opt) {}

        template <stdu::fn_args<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
            const float angle = Maths::TAU / (float)opt.subdivisions;
            Mesh<T> mesh;
            mesh.GetVertices().reserve(opt.subdivisions + 1);
            mesh.GetIndices().reserve(opt.subdivisions);

            mesh.GetVertices().emplace_back(f(MData { Maths::fvec2::ZERO(), 0 }));
            for (uint i = 0; i < opt.subdivisions; ++i) {
                mesh.GetVertices().push_back(f(MData { Maths::fvec2::from_polar(1.0f, angle * (float)i), i + 1 }));
                mesh.GetIndices().emplace_back(0, i + 1, (i + 1) % opt.subdivisions + 1);
            }

            return mesh;
        }
    };

    inline static MeshConstructor<CircleCreator> Circle {};
}
