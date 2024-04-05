#pragma once

#include "Constants.h"

namespace Graphics::MeshUtils {
    template <stdu::fn_args<const VertexBuilder::MeshConstructData2D&> F>
    auto Circle(uint subdivisions, F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData2D {}))> {
        using Mdata = VertexBuilder::MeshConstructData2D;
        using T = decltype(f(Mdata {}));
        const float angle = Maths::TAU / (float)subdivisions;
        Mesh<T> mesh;
        mesh.GetVertices().reserve(subdivisions + 1);
        mesh.GetIndices().reserve(subdivisions);

        mesh.GetVertices().emplace_back(f(Mdata { Maths::fvec2::ZERO(), 0 }));
        for (uint i = 0; i < subdivisions; ++i) {
            mesh.GetVertices().push_back(f(Mdata { Maths::fvec2::from_polar(1.0f, angle * (float)i), i + 1 }));
            mesh.GetIndices().emplace_back(0, i + 1, (i + 1) % subdivisions + 1);
        }

        return mesh;
    }

    template <stdu::fn_args<const VertexBuilder::MeshConstructData3D&> F>
    auto CubeNormless(F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData3D {}))> {
        using Mdata = VertexBuilder::MeshConstructData3D;
        using T = decltype(f(Mdata {}));
        Mesh<T> mesh;
        mesh.GetIndices().reserve(6 * 2);
        for (Maths::Direction3D i = Maths::Direction3D::RIGHT; i < 6; ++i) {
            // some magic that calculates quad indices
            const uint ii = (uint)i, n = ii / 2, flip = (ii & 1) << n;
            const uint t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
            mesh.GetIndices().emplace_back(t1 ^ flip, t2 ^ flip, t3 ^ flip);
            mesh.GetIndices().emplace_back(t2 ^ flip, t3 ^ flip, t4 ^ flip);
        }

        mesh.GetVertices().reserve(8);
        for (Maths::Corner3D i = Maths::Corner3D::FRONT_TOP_RIGHT; i < 8; ++i) {
            mesh.GetVertices().emplace_back(f(Mdata { .Position = Maths::fvec3 { i }, .VertexIndex = (uint)i }));
        }

        return mesh;
    }

    template <stdu::fn_args<const VertexBuilder::MeshConstructData3D&> F>
    auto Cube(F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData3D {}))> {
        using Mdata = VertexBuilder::MeshConstructData3D;
        using T = decltype(f(Mdata {}));
        Mesh<T> mesh;
        auto& vert = mesh.GetVertices();
        auto& ind = mesh.GetIndices();
        vert.reserve(6 * 4);
        ind.reserve(6 * 2);
        for (Maths::Direction3D i = Maths::Direction3D::RIGHT; i < 6; ++i) {
            const uint ii = (uint)i, indoff = ii * 4;
            ind.emplace_back(indoff, indoff + 1, indoff + 2);
            ind.emplace_back(indoff + 1, indoff + 2, indoff + 3);
            const Maths::fvec3 norm { i };
            const uint n = ii / 2, flip = (ii & 1) << n;
            const uint t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t1 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 0 }));
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t2 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 1 }));
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t3 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 2 }));
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t4 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 3 }));
        }

        return mesh;
    }

    template <stdu::fn_args<const VertexBuilder::MeshConstructData3D&> F>
    auto Sphere(Maths::uvec2 sections, F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData3D {}))> {
        using Mdata = VertexBuilder::MeshConstructData3D;
        using T = decltype(f(Mdata {}));
        using namespace Maths;
        const float LATITUDE_SECT = PI / sections.y, LONGITUDE_SECT = TAU / sections.x;

        Mesh<T> mesh;
        auto& vert = mesh.GetVertices();
        auto& ind = mesh.GetIndices();
        vert.reserve(2 + (sections.y - 1) * sections.x);
        ind.reserve(2 * (sections.y - 1) * sections.x);

        vert.emplace_back(f(Mdata { .Position = { 0, 1, 0 }, .Normal = { 0, 1, 0 }, .VertexIndex = 0 }));
        uint x = 0;
        for (uint lat = 0; lat < sections.y; ++lat) {
            for (uint lon = 0; lon < sections.x; ++lon) {
                if (lat != 0) {
                    const float latAngle = HALF_PI - LATITUDE_SECT * lat,
                                lonAngle = LONGITUDE_SECT * lon;
                    const fvec3 pos = fvec3::from_spheric(1, lonAngle, latAngle);
                    vert.emplace_back(f(Mdata { .Position = pos, .Normal = pos, .VertexIndex = ++x }));
                }

                const uint lon_1 = (lon + 1) % sections.x,
                           off = 1 + (lat - 1) * sections.x,
                           offNext = off + sections.x;
                if (lat == 0) {
                    ind.emplace_back(0, 1 + lon, 1 + lon_1);
                } else if (lat == sections.y - 1) {
                    ind.emplace_back(off + lon_1, off + lon, (uint)(vert.capacity() - 1));
                } else {
                    ind.emplace_back(off + lon,   offNext + lon, off     + lon_1);
                    ind.emplace_back(off + lon_1, offNext + lon, offNext + lon_1);
                }
            }
        }
        vert.emplace_back(f(Mdata { .Position = { 0, -1, 0 }, .Normal = { 0, -1, 0 }, .VertexIndex = ++x }));
        return mesh;
    }

    template <stdu::fn_args<const VertexBuilder::MeshConstructData3D&> F>
    auto Icosphere(uint subdivisions, F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData3D {}))> {
        using Mdata = VertexBuilder::MeshConstructData3D;
        using T = decltype(f(Mdata {}));
        using namespace Maths;

        // from https://en.wikipedia.org/wiki/Regular_icosahedron#Construction
        constexpr uint CORNER_COUNT = 12, EDGE_COUNT = 30, CENTER_COUNT = 20;
        constexpr float NLEN = 1.90211'30325'90307f, // precomputed √(1 + Φ²)
                        I_1 = 1.0F / NLEN, I_PHI = PHI / NLEN;

        const std::array<fvec3, CORNER_COUNT> icoVert { {
            { 0, +I_1, +I_PHI }, { 0, -I_1, +I_PHI }, { 0, +I_1, -I_PHI }, { 0, -I_1, -I_PHI },
            { +I_1, +I_PHI, 0 }, { -I_1, +I_PHI, 0 }, { +I_1, -I_PHI, 0 }, { -I_1, -I_PHI, 0 },
            { +I_PHI, 0, +I_1 }, { +I_PHI, 0, -I_1 }, { -I_PHI, 0, +I_1 }, { -I_PHI, 0, -I_1 },
        } };

        constexpr std::array<TriIndices, CENTER_COUNT> icoInd { {
            { 0, 1, 8 },  { 0, 1, 10 }, { 2, 3, 9 },   { 2, 3, 11 },
            { 4, 8, 9 },  { 6, 8, 9 },  { 5, 10, 11 }, { 7, 10, 11 },
            { 0, 4, 8 },  { 1, 6, 8 },  { 2, 4, 9 },   { 3, 6, 9 },
            { 0, 5, 10 }, { 1, 7, 10 }, { 2, 5, 11 },  { 3, 7, 11 },
            { 0, 4, 5 },  { 2, 4, 5 },  { 1, 6, 7 },   { 3, 6, 7 },
        } };

        struct byte_pair {
            byte data;
            constexpr byte_pair(uint top, uint bottom) : data((byte)((top << 4) | bottom)) {}
            bool operator<(byte_pair other) { return data < other.data; }
        };

        constexpr std::array<byte_pair, EDGE_COUNT> edges { {
            { 0,  1 }, { 0,  4 }, { 0,  5 }, { 0,  8 },
            { 0, 10 }, { 1,  6 }, { 1,  7 }, { 1,  8 },
            { 1, 10 }, { 2,  3 }, { 2,  4 }, { 2,  5 },
            { 2,  9 }, { 2, 11 }, { 3,  6 }, { 3,  7 },
            { 3,  9 }, { 3, 11 }, { 4,  5 }, { 4,  8 },
            { 4,  9 }, { 5, 10 }, { 5, 11 }, { 6,  7 },
            { 6,  8 }, { 6,  9 }, { 7, 10 }, { 7, 11 },
            { 8,  9 }, { 10, 11 },
        } };

        const uint divisions = 1 << subdivisions;
        const uint EDGE_V_COUNT = divisions - 1,
                   CENTER_V_COUNT = (divisions - 2) * (divisions - 1) / 2,
                   CENTER_OFFSET = CORNER_COUNT + EDGE_V_COUNT * EDGE_COUNT;

        std::vector<T> vert;
        std::vector<TriIndices> inds;

        vert.resize(CORNER_COUNT + EDGE_V_COUNT * EDGE_COUNT + CENTER_V_COUNT * CENTER_COUNT);
        inds.reserve(divisions * divisions * CENTER_COUNT);
        for (uint i = 0; i < CORNER_COUNT; ++i)
            vert[i] = f(Mdata { .Position = icoVert[i], .Normal = icoVert[i], .VertexIndex = i });

        const auto findEdge = [&](byte_pair p) {
            return (uint)(std::ranges::lower_bound(edges, p.data, std::less {}, &byte_pair::data) - edges.begin());
        };

        for (uint i = 0; i < CENTER_COUNT; ++i) {
            const TriIndices& ind = icoInd[i];
            // index utilities
            const auto idxOf = [&](uint p, uint q) {
                if (p == 0 && q == 0) return ind.a;
                if (p == divisions && q == 0) return ind.b;
                if (p == divisions && q == divisions) return ind.c;
                if (q == 0 || q == p || p == divisions) { // edge
                    const bool isSide = p == divisions;
                    const uint lineStart = isSide ? ind.b : ind.a,
                               lineEnd = q ? ind.c : ind.b,
                               edgeIdx = findEdge({ lineStart, lineEnd });
                    return CORNER_COUNT + edgeIdx * EDGE_V_COUNT + (isSide ? q : p) - 1;
                }
                return CENTER_OFFSET + i * CENTER_V_COUNT + (p - 2) * (p - 1) / 2 + (q - 1);
            };
            // loops through a triangle, p = 1 to prevent the (0,0) coordinate
            for (uint p = 1; p <= divisions; ++p) {
                for (uint q = 0; q <= p; ++q) {
                    if (p == divisions && (q == 0 || q == divisions)) continue;
                    if (q == 0 || q == p || p == divisions) { // edge, interpolate between 2 points
                        const bool isSide = p == divisions;

                        const uint lineStart = isSide ? ind.b : ind.a,
                                   lineEnd = q ? ind.c : ind.b,
                                   edgeIdx = findEdge({ lineStart, lineEnd }),
                                   dist = isSide ? q : p,
                                   idx = CORNER_COUNT + edgeIdx * EDGE_V_COUNT + (dist - 1);

                        const float factor = (float)dist / (float)divisions;
                        const fvec3 pos = (icoVert[lineStart] * (1.0f - factor) + icoVert[lineEnd] * factor).norm();

                        vert[idx] = f(Mdata { .Position = pos, .Normal = pos, .VertexIndex = idx });
                        continue;
                    }
                    // center, interpolate between 3 points
                    const fvec3 pos = (icoVert[ind.a] * ((divisions - p) / (float)divisions)
                                     + icoVert[ind.b] * ((p - q)         / (float)divisions)
                                     + icoVert[ind.c] * (q               / (float)divisions)).norm();
                    const uint idx = idxOf(p, q);
                    vert[idx] = f(Mdata { .Position = pos, .Normal = pos, .VertexIndex = idx });
                }
            }
            // the indices
            const bool iSwap = (i % 4 == 1 || i % 4 == 2) ^ (8 <= i && i < 12);
            for (uint p = 0; p < divisions; ++p) {
                for (uint q = 0; q < p; ++q) {
                    const uint p1 = idxOf(p, q);
                    inds.emplace_back(p1, idxOf(p + 1, q + iSwap), idxOf(p + 1, q + !iSwap));
                    inds.emplace_back(p1, idxOf(p + !iSwap, q + 1), idxOf(p + iSwap, q + 1));
                }
                inds.emplace_back(idxOf(p, p), idxOf(p + 1, p + iSwap), idxOf(p + 1, p + !iSwap));
            }
        }

        return Mesh<T> { std::move(vert), std::move(inds) };
    }
}
