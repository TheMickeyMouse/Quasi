#pragma once

#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Plane : IMeshBuilder3D<Plane> {
        template <FnArgs<VertexNormal3D> F>
        void MergeImpl(F&& f, auto& mesh) {
            mesh.PushV(f(VertexNormal3D { .Position = { +1, 0, +1 }, .Normal = { 0, 1, 0 } }));
            mesh.PushV(f(VertexNormal3D { .Position = { +1, 0, -1 }, .Normal = { 0, 1, 0 } }));
            mesh.PushV(f(VertexNormal3D { .Position = { -1, 0, +1 }, .Normal = { 0, 1, 0 } }));
            mesh.PushV(f(VertexNormal3D { .Position = { -1, 0, -1 }, .Normal = { 0, 1, 0 } }));
            mesh.PushI(0, 2, 1);
            mesh.PushI(1, 2, 3);
        }
    };
} // Graphics::MeshUtils
