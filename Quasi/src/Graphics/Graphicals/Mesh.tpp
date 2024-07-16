#pragma once
#include <ranges>

namespace Quasi::Graphics {
    template <IVertex Vtx> Mesh<Vtx>& Mesh<Vtx>::EmbedTransform() {
        for (auto& v : vertices) {
            v = VertexMul(v, modelTransform);
        }
        modelTransform = {};
        return *this;
    }


    template <IVertex Vtx>
    void Mesh<Vtx>::AddTo(RenderData& rd) const {
        rd.PushIndicesOffseted(indices, sizeof(Vtx));

        for (const Vtx& v : vertices) {
            rd.PushVertex(VertexMul(v, modelTransform));
        }
    }

    template <IVertex Vtx>
    Mesh<Vtx>& Mesh<Vtx>::Add(const Mesh& m) {
        auto batch = NewBatch();
        batch.PushSpan(m.vertices | std::views::transform([&] (const Vtx& v) {
            return VertexMul(VertexMul(v, m.modelTransform), modelTransform.Inverse());
        }), m.indices);
        return *this;
    }

    template <IVertex Vtx>
    Mesh<Vtx> Mesh<Vtx>::Combine(IList<Mesh> meshes) {
        Mesh combined = std::move(meshes.begin()[0]);
        for (usize i = 0; i < meshes.size(); ++i)
            combined.Add(meshes.begin()[i]);
        return combined;
    }

    template <IVertex Vtx> template <Fn<void, Ref<Vtx>[]> G>
    void Mesh<Vtx>::GeometryPassTris(G&& g) {
        Ref<Vtx> localVtx[3];
        for (const auto [i, j, k] : indices) {
            localVtx[0] = vertices[i];
            localVtx[1] = vertices[j];
            localVtx[2] = vertices[k];
            g(localVtx);
        }
    }

    template <IVertex Vtx> template <Fn<void, Ref<Vtx>> G>
    void Mesh<Vtx>::GeometryPass(G&& g) {
        for (auto& v : vertices) {
            g(v);
        }
    }

    template <IVertex Vtx>
    template <IVertex U, Fn<void, typename Mesh<U>::BatchProxy, Ref<const Vtx>[]> G>
    Mesh<U> Mesh<Vtx>::GeometryMapTris(G&& g) const {
        Mesh<U> newGeometry;
        Ref<const Vtx> localVtx[3];
        for (const auto [i, j, k] : indices) {
            auto batch = newGeometry.NewBatch();
            localVtx[0] = vertices[i];
            localVtx[1] = vertices[j];
            localVtx[2] = vertices[k];
            g(batch, localVtx);
        }
        return newGeometry;
    }

    template <IVertex Vtx>
    template <IVertex U, Fn<U, Ref<const Vtx>> G>
    Mesh<U> Mesh<Vtx>::GeometryMap(G&& g) const {
        Mesh<U> newGeometry;
        newGeometry.vertices.reserve(vertices.size());
        newGeometry.indices = indices;
        for (const auto& v : vertices) {
            newGeometry.PushVertex(g(v));
        }
        return newGeometry;
    }

    template <IVertex Vtx>
    void Mesh<Vtx>::Clear() {
        vertices.clear();
        indices.clear();
    }
}