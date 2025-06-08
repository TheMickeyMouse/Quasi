#pragma once

#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Stadium : IMeshBuilder2D<Stadium> {
        Math::fv2 start, end = { 0, 1 };
        float radius = 1.0f;
        u32 subdivisions = 4;
        Stadium(const Math::fv2& start = {}, const Math::fv2& end = { 0, 1 }, float r = 1.0f, u32 sub = 4)
            : start(start), end(end), radius(r), subdivisions(sub) {}

        template <FnArgs<Vertex2D> F>
        void MergeImpl(F&& f, IMeshBatch<FuncResult<F, Vertex2D>> auto&& mesh) {
            using namespace Math;

            const fv2 Y = (end - start).Norm(radius);
            const fv2 X = Y.PerpendRight();

            mesh.ReserveV((4 << subdivisions) + 2);
            mesh.ReserveI(4 << subdivisions);

            {
                mesh.PushV(f(Vertex2D { end + X }));
                mesh.PushV(f(Vertex2D { end - X }));
                mesh.PushV(f(Vertex2D { end + Y }));
                mesh.PushI(0, 2, 1);

                Rotor2D step = Rotor2D::FromComplex({ 0, 1 });
                for (u32 i = 0; i < subdivisions; ++i) {
                    const Rotor2D half = step.Halved();
                    fv2 vertex = half.Rotate(X);
                    u32 p = 0, q = (2 << i) + 1;
                    for (u32 j = 0; j < (2 << i); ++j) {
                        mesh.PushV(f(Vertex2D { end + vertex }));
                        vertex = vertex.RotateBy(step);
                        const u32 r = (q >> (u32s::CountRightZeros(q) + 1)) + 1;
                        mesh.PushI(p, q, r);
                        p = r;
                        ++q;
                    }
                    step = half;
                }
            }
            {
                auto m = mesh.NewBatch();
                m.PushV(f(Vertex2D { start - X }));
                m.PushV(f(Vertex2D { start + X }));
                m.PushV(f(Vertex2D { start - Y }));
                m.PushI(0, 2, 1);

                Rotor2D step = Rotor2D::FromComplex({ 0, 1 });
                for (u32 i = 0; i < subdivisions; ++i) {
                    const Rotor2D half = step.Halved();
                    fv2 vertex = half.Rotate(X);
                    u32 p = 0, q = (2 << i) + 1;
                    for (u32 j = 0; j < (2 << i); ++j) {
                        m.PushV(f(Vertex2D { start - vertex }));
                        vertex = vertex.RotateBy(step);
                        const u32 r = (q >> (u32s::CountRightZeros(q) + 1)) + 1;
                        m.PushI(p, q, r);
                        p = r;
                        ++q;
                    }
                    step = half;
                }
            }
            const u32 secondHalf = (2 << subdivisions) + 1;
            mesh.PushI(0, 1, secondHalf);
            mesh.PushI(0, secondHalf, secondHalf + 1);
        }
    };
}
