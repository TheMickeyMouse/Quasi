#pragma once
#include <algorithm>

#include "RenderObject.h"
#include "TriIndices.h"

namespace Quasi::Graphics {
    template <IVertex Vtx> struct MeshBatch;

    template <IVertex Vtx>
    class Mesh {
        template <class T2D, class T3D>
        using VersionOf = IfElse<Vtx::DIMENSION == 2, T2D,
                          IfElse<Vtx::DIMENSION == 3, T3D, void>>;
        using Transform = VersionOf<Math::Transform2D, Math::Transform3D>;
    public:
        Vec<Vtx> vertices;
        Vec<TriIndices> indices;
        Transform modelTransform;
    public:
        Mesh() = default;
        Mesh(IList<Vtx> v, IList<TriIndices> i) : vertices(Vecs::FromIList(v)), indices(Vecs::FromIList(i)) {}
        Mesh(Vec<Vtx> v, Vec<TriIndices> i)
            : vertices(std::move(v)), indices(std::move(i)) {}

        void SetTransform(const Transform& model) { modelTransform = model; }
        void ApplyTransform(const Transform& model) { modelTransform.Apply(model); }
        Mesh& EmbedTransform();

        void AddTo(RenderData& rd) const;

        void PushVertex(const Vtx& v) { vertices.Push(v); }
        void PushIndex(TriIndices i) { indices.Push(i); }
        void PushVertices(const Collection<Vtx> auto& vs) { for (const Vtx& v : vs) PushVertex(v); }
        void PushVertices(IList<Vtx> vs) { for (const Vtx& v : vs) PushVertex(v); }
        void PushIndices(const Collection<TriIndices> auto& is) { for (const TriIndices& i : is) PushIndex(i); }
        void PushIndices(IList<TriIndices> is) { for (const TriIndices& i : is) PushIndex(i); }
        void PushTriangle(const Vtx& a, const Vtx& b, const Vtx& c) {
            PushIndex({ (u32)vertices.Length(), (u32)vertices.Length() + 1, (u32)vertices.Length() + 2 });
            PushVertex(a); PushVertex(b); PushVertex(c);
        }
        void PushITri(const Vtx& a, const Vtx& b, const Vtx& c, const TriIndices& i) {
            PushVertex(a); PushVertex(b); PushVertex(c);
            PushIndex(i);
        }
        void PushPolygon(const Collection<Vtx> auto& vs) {
            const u32 begin = vertices.Length();
            PushVertices(vs);
            for (u32 i = begin + 1; i < vertices.Length() - 1; ++i) PushIndex({ begin, i, i + 1 });
        }
        void PushPolygon(IList<Vtx> vs) { PushPolygon(Span { vs }); }

        template <FnArgs<const Vtx&> F>
        Mesh<FuncResult<F, const Vtx&>> GeometryConvert(F&& geometryPass) && {
            using R = FuncResult<F, const Vtx&>;
            Mesh<R> converted;
            converted.indices = std::move(indices);
            converted.modelTransform = modelTransform;
            converted.vertices.Reserve(vertices.Length());

            for (const Vtx& v : vertices)
                converted.vertices.Push(geometryPass(v));
            return converted;
        }

        MeshBatch<Vtx> NewBatch() { return { (u32)vertices.Length(), *this }; }

        // moving isnt that efficient bc vertices are easy to copy
        Mesh& Add(const Mesh& m);
        static Mesh Combine(Span<Mesh> meshes);

        void Clear();

        friend class GraphicsDevice;
        friend class RenderData;
    };

    template <class T, class V>
    concept IMeshBatch = requires(T mesh, const V& vtx) {
        { mesh.PushV(vtx) }                    -> SameAs<void>;
        { mesh.PushI(u32 {}, u32 {}, u32 {}) } -> SameAs<void>;
        { mesh.ReserveV(u32 {}) }              -> SameAs<void>;
        { mesh.VertAt(u32 {}) }                -> SameAs<V&>;
    };

    template <IVertex Vtx>
    struct MeshBatch {
        u32 iOffset;
        Ref<Mesh<Vtx>> mesh;

        void PushV(const Vtx& v) { mesh->PushVertex(v); }
        void ResizeV(u32 amount) { mesh->vertices.ResizeExtra(amount); }
        void ReserveV(u32 amount) { mesh->vertices.Reserve(amount); }
        Vtx& VertAt(u32 i) { return mesh->vertices[iOffset + i]; }
        const Vtx& VertAt(u32 i) const { return mesh->vertices[iOffset + i]; }
        void PushVs(const Collection<Vtx> auto& vs) { mesh->PushVertices(vs); }
        void PushVs(IList<Vtx> vs) { mesh->PushVertices(vs); }
        u32 VertCount() const { return mesh.vertices.Length() - iOffset; }

        TriIndices* IndexData() { return mesh->indices.Data() + iOffset; }
        void PushI(TriIndices i) { mesh->PushIndex(i + iOffset); }
        void PushI(u32 i, u32 j, u32 k) { mesh->PushIndex({ i + iOffset, j + iOffset, k + iOffset }); }
        void ResizeI(u32 amount) { mesh->indices.ResizeExtra(amount); }
        void ReserveI(u32 amount) { mesh->indices.Reserve(amount); }
        void PushIs(const Collection<TriIndices> auto& is) { mesh->PushIndices(is); }
        void PushIs(IList<TriIndices> is) { mesh->PushIndices(is); }

        void PushTri(const Vtx& a, const Vtx& b, const Vtx& c) {
            PushV(a); PushV(b); PushV(c);
            const u32 i = (u32)mesh->vertices.Length();
            mesh->PushIndex({ i - 3, i - 2, i - 1 });
        }
        void PushITri(const Vtx& a, const Vtx& b, const Vtx& c, const TriIndices& i) {
            PushV(a); PushV(b); PushV(c);
            PushI(i);
        }

        void PushSpan(const Collection<Vtx> auto& vs, const Collection<TriIndices> auto& is) {
            for (const auto& v : vs) PushV(v);
            for (const auto& i : is) PushI(i);
        }

        void PushPolygon(const Collection<Vtx> auto& vs) { mesh->PushPolygon(vs); }
        void PushPolygon(IList<Vtx> vs) { PushPolygon(Spans::FromIList(vs)); }

        MeshBatch NewBatch() { return { (u32)mesh->vertices.Length(), mesh }; }
        void Reload() { iOffset = mesh.vertices.Length(); }
    };
}

#include "Mesh.tpp"
