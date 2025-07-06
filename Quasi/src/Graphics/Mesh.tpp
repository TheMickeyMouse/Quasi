#pragma once
#include <ranges>

namespace Quasi::Graphics {
    template <IVertex Vtx> Mesh<Vtx>& Mesh<Vtx>::EmbedTransform() {
        const auto transform = modelTransform.TransformMatrix().AsTransform();
        for (auto& v : vertices) {
            v = v.Mul(transform);
        }
        modelTransform = {};
        return *this;
    }


    template <IVertex Vtx>
    void Mesh<Vtx>::AddTo(RenderData& rd) const {
        rd.PushIndicesOffseted(indices, sizeof(Vtx));

        const auto transform = modelTransform.TransformMatrix().AsTransform();
        for (const Vtx& v : vertices) {
            rd.PushVertex(v.Mul(transform));
        }
    }

    template <IVertex Vtx>
    Mesh<Vtx>& Mesh<Vtx>::Add(const Mesh& m) {
        auto batch = NewBatch();
        const auto mLocal = m.modelTransform.TransformMatrix(),
                   modelInverse = modelTransform.TransformMatrix().InvTRS();
        const auto composition = (modelInverse * mLocal).AsTransform();
        batch.PushSpan(m.vertices.Iter().Map([&] (const Vtx& v) {
            return v.Mul(composition);
        }), m.indices);
        return *this;
    }

    template <IVertex Vtx>
    Mesh<Vtx> Mesh<Vtx>::Combine(Span<Mesh> meshes) {
        Mesh combined = std::move(meshes.FirstMut());
        for (Mesh& m : meshes.SkipMut(1))
            combined.Add(std::move(m));
        return combined;
    }

    template <IVertex Vtx>
    void Mesh<Vtx>::Clear() {
        vertices.Clear();
        indices.Clear();
    }
}