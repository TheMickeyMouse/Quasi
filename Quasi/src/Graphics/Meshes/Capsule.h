#pragma once

#include "MeshBuilder.h"
#include "Mesh.h"
#include "Utils/Math/Vector.h"

namespace Quasi::Graphics::Meshes {
    struct Capsule : IMeshBuilder3D<Capsule> {
        Math::fv3 start = 0, end = { 0, 1, 0 };
        float radius = 1;
        Math::uv2 sections = { 32, 16 };
        Capsule(const Math::fv3& start = 0, const Math::fv3& end = { 0, 1, 0 }, float r = 1, u32 lon = 32, u32 lat = 16)
            : start(start), end(end), radius(r), sections(lon, lat) {}

        template <FnArgs<VertexNormal3D> F>
        void MergeImpl(F&& f, IMeshBatch<FuncResult<F, VertexNormal3D>> auto&& mesh) {
            using namespace Math;
            const fv3 forward = (end - start).Norm();
            const Rotor3D orientUp = Rotor3D::OrientY(forward);
            const Matrix3x3 LON_ROT = Rotor3D::RotateAxis(forward, Radians(TAU / -(float)sections.x)).AsMatrixLinear(),
                            LAT_ROT = Rotor3D::RotateAxis(orientUp.KHat(), Radians(HALF_PI / -(float)sections.y)).AsMatrixLinear();

            mesh.PushV(f(VertexNormal3D { .Position = end + forward * radius, .Normal = forward }));
            fv3 pitchBase = forward;
            for (int lat = -(int)sections.y; lat < (int)sections.y; ++lat) {
                fv3 pos = pitchBase;
                for (u32 lon = 0; lon < sections.x; ++lon) {
                    if (lat != -(int)sections.y) {
                        pos = LON_ROT * pos;
                        mesh.PushV(f(VertexNormal3D { .Position = (lat < 0 ? end : start) + radius * pos, .Normal = pos }));
                    }

                    const u32 lon_1 = (lon + 1) % sections.x,
                              off = 1 + (lat + sections.y - 1) * sections.x,
                              offNext = off + sections.x;
                    if (lat == -(int)sections.y) {
                        mesh.PushI(0, 1 + lon, 1 + lon_1);
                    } else if (lat == sections.y - 1) {
                        mesh.PushI(off + lon_1, off + lon, 1 + sections.x * (2 * sections.y - 1));
                    } else {
                        mesh.PushI(off + lon,   offNext + lon, off     + lon_1);
                        mesh.PushI(off + lon_1, offNext + lon, offNext + lon_1);
                    }
                }
                pitchBase = LAT_ROT * pitchBase;
            }
            mesh.PushV(f(VertexNormal3D { .Position = start - forward * radius, .Normal = -forward }));
        }
    };
}
