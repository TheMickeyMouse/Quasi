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
        void MergeImpl(F&& f, MeshBatch<FuncResult<F, Vertex2D>> mesh) {
            using namespace Math;

            const fv2 X = (end - start).Norm(radius);
            const fv2 Y = X.PerpendLeft();

            const u32 halfWay = 2 << subdivisions, total = halfWay * 2 + 1, quarter = halfWay / 2;
            mesh.ReserveV(total + 1);

            mesh.VertAt(0)               = f(Vertex2D { start + Y });
            mesh.VertAt(quarter)         = f(Vertex2D { start - X });
            mesh.VertAt(halfWay)         = f(Vertex2D { start - Y });
            mesh.VertAt(halfWay + 1)     = f(Vertex2D { end   - Y });
            mesh.VertAt(total - quarter) = f(Vertex2D { end   + X });
            mesh.VertAt(total)           = f(Vertex2D { end   + Y });
            mesh.PushI(0, quarter, halfWay);
            mesh.PushI(halfWay + 1, total - quarter, total);
            mesh.PushI(0, halfWay, halfWay + 1);
            mesh.PushI(0, halfWay + 1, total);

            Rotor2D step = Rotor2D::FromComplex({ 0, 1 });
            for (u32 k = 1; k <= subdivisions; ++k) {
                const u32 n = 1 << (subdivisions - k), skip = n << 1;
                const Rotor2D half = step.Halved();
                Rotor2D r = half;
                for (usize j = n; j < halfWay; j += skip) {
                    const u32 jp = j + halfWay + 1;
                    mesh.VertAt(j)  = f(Vertex2D { r.Rotate(Y) + start });
                    mesh.VertAt(jp) = f(Vertex2D { r.InvRotate(Y) + end });
                    mesh.PushI(j  - n, j,  (j  + n) == halfWay ? 0 : (j  + n));
                    mesh.PushI(jp - n, jp, (jp + n) == total   ? 0 : (jp + n));
                    r += step;
                }
                step = half;
            }
        }
    };
}
