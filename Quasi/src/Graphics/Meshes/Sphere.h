#pragma once

#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Sphere : IMeshBuilder3D<Sphere> {
        float radius = 1.0f;
        Math::uv2 sections;
        Sphere(float r = 1, u32 lon = 32, u32 lat = 32) : radius(r), sections(lon, lat) {}

        template <FnArgs<VertexNormal3D> F>
        void MergeImpl(F&& f, MeshBatch<FuncResult<F, VertexNormal3D>>& mesh) {
            using namespace Math;
            const Rotor2D LAT_ROT = Radians(PI  / -(float)sections.y),
                          LON_ROT = Radians(TAU / (float)sections.x);

            mesh.PushV(f(VertexNormal3D { .Position = { 0, radius, 0 }, .Normal = { 0, 1, 0 } }));
            Rotor2D pitch = Rotor2D::FromComplex({ 0, 1 });
            for (u32 lat = 0; lat < sections.y; ++lat) {
                Rotor2D yaw = {};
                for (u32 lon = 0; lon < sections.x; ++lon) {
                    if (lat != 0) {
                        const fv3 pos = fv3::FromSpheric(1, yaw, pitch);
                        mesh.PushV(f(VertexNormal3D { .Position = pos * radius, .Normal = pos }));
                        yaw += LON_ROT;
                    }

                    const u32 lon_1 = (lon + 1) % sections.x,
                              off = 1 + (lat - 1) * sections.x,
                              offNext = off + sections.x;
                    if (lat == 0) {
                        mesh.PushI(0, 1 + lon, 1 + lon_1);
                    } else if (lat == sections.y - 1) {
                        mesh.PushI(off + lon_1, off + lon, 1 + sections.x * (sections.y - 1));
                    } else {
                        mesh.PushI(off + lon,   offNext + lon, off     + lon_1);
                        mesh.PushI(off + lon_1, offNext + lon, offNext + lon_1);
                    }
                }
                pitch += LAT_ROT;
            }
            mesh.PushV(f(VertexNormal3D { .Position = { 0, -radius, 0 }, .Normal = { 0, -1, 0 } }));
        }
    };
}
