#pragma once

#include "Mesh.h"
#include "Math/Transform2D.h"

namespace Quasi::Graphics::Meshes {
    template <class F>
    void PushTri(auto& out, F& f, const Vertex2D (&tri)[3], const TriIndices& i = { 0, 1, 2 }) {
        out.PushITri(f(tri[0]), f(tri[1]), f(tri[2]), i);
    }

    template <class Super>
    struct IMeshBuilder2D {
        const Super& super() const { return static_cast<const Super&>(*this); }
        Super& super() { return static_cast<Super&>(*this); }

        template <FnArgs<Vertex2D> F>
        void Merge(F&& f, MeshBatch<FuncResult<F, Vertex2D>> out) {
            return super().MergeImpl(f, out);
        }

        template <FnArgs<Vertex2D> F>
        void Merge(F&& f, const Math::ITransformation2D auto& transform, MeshBatch<FuncResult<F, Vertex2D>> out) {
            return super().MergeImpl([&] (const Vertex2D& data) { return f(data).Mul(transform); }, out);
        }

        template <FnArgs<Vertex2D> F>
        Mesh<FuncResult<F, Vertex2D>> Create(F&& f) {
            Mesh<FuncResult<F, Vertex2D>> out;
            this->Merge(f, out.NewBatch());
            return out;
        }

        template <FnArgs<Vertex2D> F>
        Mesh<FuncResult<F, Vertex2D>> Create(F&& f, const Math::ITransformation2D auto& transform) {
            Mesh<FuncResult<F, Vertex2D>> out;
            this->Merge(f, transform, out.NewBatch());
            return out;
        }
    };

    template <class Super>
    struct IMeshBuilder3D {
        const Super& super() const { return static_cast<const Super&>(*this); }
        Super& super() { return static_cast<Super&>(*this); }

        template <FnArgs<VertexNormal3D> F>
        void Merge(F&& f, MeshBatch<FuncResult<F, VertexNormal3D>> out) {
            return super().MergeImpl(f, out);
        }

        template <FnArgs<VertexNormal3D> F>
        void Merge(F&& f, const Math::ITransformation3D auto& transform, MeshBatch<FuncResult<F, VertexNormal3D>> out) {
            return super().MergeImpl([&] (const VertexNormal3D& data) { return f(data).Mul(transform); }, out);
        }

        template <FnArgs<VertexNormal3D> F>
        Mesh<FuncResult<F, VertexNormal3D>> Create(F&& f) {
            Mesh<FuncResult<F, VertexNormal3D>> out;
            this->Merge(f, out.NewBatch());
            return out;
        }

        template <FnArgs<VertexNormal3D> F>
        Mesh<FuncResult<F, VertexNormal3D>> Create(F&& f, const Math::ITransformation3D auto& transform) {
            Mesh<FuncResult<F, VertexNormal3D>> out;
            this->Merge(f, transform, out.NewBatch());
            return out;
        }
    };
}
