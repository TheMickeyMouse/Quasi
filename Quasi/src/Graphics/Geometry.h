#pragma once
#include "TriIndices.h"
#include "Utils/Vec.h"
#include "Utils/Math/Matrix.h"
#include "Utils/Math/Vector.h"

namespace Quasi::Math {
    struct Transform3D;
}

namespace Quasi::Graphics {
    class Geometry {
    public:
        Vec<Math::fv3> vertices, normals;
        Vec<TriIndices> indices;
    private:
        Geometry(Vec<Math::fv3> v, Vec<Math::fv3> n, Vec<TriIndices> i) : vertices(std::move(v)), normals(std::move(n)), indices(std::move(i)) {}
    public:
        Geometry() = default;

        static Geometry FromVertices(Vec<Math::fv3> v, Vec<TriIndices> i, bool smooth = true);
        static Geometry FromVertAndNorm(Vec<Math::fv3> v, Vec<Math::fv3> n, Vec<TriIndices> i);

        void RecalcNormals();
        void RecalcNormalsFlat(); // this may be a little inefficient because faces need to be duplicated

        void ApplyTransform(const Math::Transform3D& model);
        void ApplyTransform(const Math::Matrix3D& model);

        void PushV(const Math::fv3& v, const Math::fv3& n);
        void PushI(TriIndices i);
        void PushVs(Span<const Math::fv3> vs, Span<const Math::fv3> ns);
        void PushIs(Span<const TriIndices> is);

        void PushTriangle(const Math::fv3& a, const Math::fv3& b, const Math::fv3& c);
        void PushPolygon(Span<const Math::fv3> vs);

        // template <FnArgs<const Vtx&> F>
        // // Mesh<FuncResult<F, const Vtx&>> GeometryConvert(F&& geometryPass) && {
        //     using R = FuncResult<F, const Vtx&>;
        //     Mesh<R> converted;
        //     converted.indices = std::move(indices);
        //     converted.modelTransform = modelTransform;
        //     converted.vertices.Reserve(vertices.Length());
        //
        //     for (const Vtx& v : vertices)
        //         converted.vertices.Push(geometryPass(v));
        //     return converted;
        // }
        //
        // MeshBatch<Vtx> NewBatch() { return { (u32)vertices.Length(), *this }; }

        // moving isnt that efficient bc vertices are easy to copy
        Geometry& Add(const Geometry& g);
        static Geometry Combine(Span<Geometry> geoms);

        void Clear();
    };
} // Quasi