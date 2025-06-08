#pragma once
#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Arrow2D : IMeshBuilder2D<Arrow2D> {
        Math::fv2 start, end;
        float scale = 1.0f;
        Arrow2D(const Math::fv2& start, const Math::fv2& end, float scale = 1.0f)
            : start(start), end(end), scale(scale) {}

        static constexpr float DEFAULT_TIP_SIZE = 0.35f, MAX_TIP_SIZE = 0.25f, THICKNESS_FACTOR = 0.1f;

        template <FnArgs<Vertex2D> F>
        void MergeImpl(F&& f, IMeshBatch<FuncResult<F, Vertex2D>> auto&& mesh) {
            const auto [fwd, len] = (end - start).NormAndLen();
            const float tipSize   = std::min(len * MAX_TIP_SIZE, scale * DEFAULT_TIP_SIZE);
            /*   ^
             *  / \
             * /   \
             *  | |
             *  | |
             */
            // tip bit
            const Math::fv2 tipF = tipSize * fwd;
            mesh.PushV(f(Vertex2D { end }));
            mesh.PushV(f(Vertex2D { end + tipF.ComplexMul({ -1, +0.5f }) }));
            mesh.PushV(f(Vertex2D { end + tipF.ComplexMul({ -1, -0.5f }) }));
            mesh.PushI(0, 1, 2);

            const Math::fv2 thick = tipF.PerpendLeft() * THICKNESS_FACTOR, beforeTip = end - tipF;
            // length
            mesh.PushV(f(Vertex2D { start + thick }));
            mesh.PushV(f(Vertex2D { start - thick }));
            mesh.PushV(f(Vertex2D { beforeTip + thick }));
            mesh.PushV(f(Vertex2D { beforeTip - thick }));
            mesh.PushI(3, 4, 5);
            mesh.PushI(4, 6, 5);
        }
    };
}
