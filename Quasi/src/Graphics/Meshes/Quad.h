#pragma once

#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Quad : IMeshBuilder2D<Quad> {
        template <FnArgs<Vertex2D> F>
        void MergeImpl(F&& f, auto& out) {
            out.PushV(f(Vertex2D { .Position = { +1, +1 } }));
            out.PushV(f(Vertex2D { .Position = { +1, -1 } }));
            out.PushV(f(Vertex2D { .Position = { -1, +1 } }));
            out.PushV(f(Vertex2D { .Position = { -1, -1 } }));
            out.PushI(0, 2, 1);
            out.PushI(1, 2, 3);
        }
    };
} // Graphics::MeshUtils
