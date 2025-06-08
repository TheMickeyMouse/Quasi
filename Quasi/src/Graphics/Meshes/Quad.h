#pragma once

#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Quad : IMeshBuilder2D<Quad> {
        template <FnArgs<Vertex2D> F>
        void MergeImpl(F&& f, IMeshBatch<FuncResult<F, Vertex2D>> auto&& mesh) {
            mesh.PushV(f(Vertex2D { .Position = { +1, +1 } }));
            mesh.PushV(f(Vertex2D { .Position = { +1, -1 } }));
            mesh.PushV(f(Vertex2D { .Position = { -1, +1 } }));
            mesh.PushV(f(Vertex2D { .Position = { -1, -1 } }));
            mesh.PushI(0, 2, 1);
            mesh.PushI(1, 2, 3);
        }
    };
} // Graphics::MeshUtils
