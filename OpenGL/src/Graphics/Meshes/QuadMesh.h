#pragma once
#include "Mesh.h"
#include "Vector.h"

namespace Graphics
{
    template <typename TVertex = Maths::Vector3>
    class QuadMesh final : public Mesh<TVertex>
    {
    private:
        TVertex points[4];
    public:
        QuadMesh();
        QuadMesh(TVertex _points[4]);
        QuadMesh(const TVertex& point1, const TVertex& point2, const TVertex& point3, const TVertex& point4);
        ~QuadMesh() override = default;

        TVertex* GetVertices();
        void GetData(unsigned int* vertexSize, TVertex** vertices, unsigned int* indexSize, unsigned int** indices) override;
        // friend void AddTo(DynamicVertexBuffer<TVertex>& vb, DynamicIndexBuffer& ib);
        void AddTo(DynamicVertexBuffer<TVertex>& vb, DynamicIndexBuffer& ib) override;

        void Clone(Mesh<TVertex>*& other) const override;

        void Transform(const Maths::Matrix3D& transform) override;

        template<typename T = TVertex>
        friend QuadMesh<T> operator*(const Maths::Matrix3D& transform, const QuadMesh<T>& mesh);
    };

    template <typename TVertex>
    QuadMesh<TVertex>::QuadMesh() {}

    template <typename TVertex>
    QuadMesh<TVertex>::QuadMesh(TVertex _points[4])
    {
        memcpy(points, _points, sizeof(TVertex) * 4);
    }

    template <typename TVertex>
    QuadMesh<TVertex>::QuadMesh(const TVertex& point1, const TVertex& point2, const TVertex& point3,
        const TVertex& point4)
    {
        points[0] = point1;
        points[1] = point2;
        points[2] = point3;
        points[3] = point4;
    }

    template <typename TVertex>
    TVertex* QuadMesh<TVertex>::GetVertices() {
        return points;
    }

    template <typename TVertex>
    void QuadMesh<TVertex>::GetData(unsigned* vertexSize, TVertex** vertices, unsigned* indexSize, unsigned** indices)
    {
        if (vertexSize) *vertexSize = 4;
        if (vertices) memcpy(*vertices, points, sizeof(TVertex) * 4);
        
        if (indexSize) *indexSize = 6;
        if (indices) { unsigned int i[] = { 0, 1, 2, 1, 2, 3 }; memcpy(*indices, i, 6); }
    }

    template<typename TVertex>
    void QuadMesh<TVertex>::AddTo(DynamicVertexBuffer<TVertex>& vb, DynamicIndexBuffer& ib)
    {
        vb.AddData(points, 4);
        unsigned i[6] = { 0, 1, 2, 1, 2, 3 };
        ib.AddData(i, 6, 4);
    }

    template <typename TVertex>
    void QuadMesh<TVertex>::Clone(Mesh<TVertex>*& other) const {
        other = new QuadMesh<TVertex>(*this);
    }

    template <typename TVertex>
    void QuadMesh<TVertex>::Transform(const Maths::Matrix3D& transform)
    {
        *this = transform * *this;
    }

    template <typename TVertex>
    QuadMesh<TVertex> operator*(const Maths::Matrix3D& transform, const QuadMesh<TVertex>& mesh)
    {
        TVertex v[4] = { transform * mesh.points[0], transform * mesh.points[1], transform * mesh.points[2], transform * mesh.points[3] };
        return QuadMesh<TVertex>( v );
    }
}
