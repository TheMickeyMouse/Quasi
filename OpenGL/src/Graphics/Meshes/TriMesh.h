#pragma once
#include "Mesh.h"

namespace Graphics
{
    template <typename TVertex = Maths::Vector3>
    class TriMesh final : public Mesh<TVertex>
    {
    private:
        TVertex points[3];
    public:
        TriMesh();
        TriMesh(TVertex _points[3]);
        TriMesh(const TVertex& point1, const TVertex& point2, const TVertex& point3);
        ~TriMesh() override;

        TVertex GetVertices();
        void GetData(unsigned int* vertexSize, TVertex** vertices, unsigned int* indexSize, unsigned int** indices) override;
        // friend void AddTo(DynamicVertexBuffer<TVertex>& vb, DynamicIndexBuffer& ib);
        void AddTo(DynamicVertexBuffer<TVertex>& vb, DynamicIndexBuffer& ib) override;
        
        void Clone(Mesh<TVertex>*& other) const override;
        
        void Transform(const Maths::Matrix3D& transform) override;

        template<typename T = TVertex>
        friend TriMesh<T> operator*(const Maths::Matrix3D& transform, const TriMesh<T>& mesh);
    };

    template <typename TVertex>
    TriMesh<TVertex>::TriMesh() {}

    template <typename TVertex>
    TriMesh<TVertex>::TriMesh(TVertex _points[3])
    {
        memcpy(points, _points, sizeof(TVertex) * 3);
    }

    template <typename TVertex>
    TriMesh<TVertex>::TriMesh(const TVertex& point1, const TVertex& point2, const TVertex& point3)
    {
        points[0] = point1;
        points[1] = point2;
        points[2] = point3;
    }

    template <typename TVertex>
    TriMesh<TVertex>::~TriMesh()
    {
    }

    template <typename TVertex>
    TVertex TriMesh<TVertex>::GetVertices() {
        return points;
    }

    template <typename TVertex>
    void TriMesh<TVertex>::GetData(unsigned* vertexSize, TVertex** vertices, unsigned* indexSize, unsigned** indices)
    {
        if (vertexSize) *vertexSize = 3;
        if (vertices) memcpy(*vertices, points, sizeof(TVertex) * 3);
        
        if (indexSize) *indexSize = 3;
        if (indices) { unsigned int i[] = { 0, 1, 2 }; memcpy(*indices, i, 3); }
    }

    template<typename TVertex>
    void TriMesh<TVertex>::AddTo(DynamicVertexBuffer<TVertex>& vb, DynamicIndexBuffer& ib)
    {
        vb.AddData(points, 3);
        unsigned i[3] = { 0, 1, 2 };
        ib.AddData(i, 3, 3);
    }

    template <typename TVertex>
    void TriMesh<TVertex>::Clone(Mesh<TVertex>*& other) const {
        other = new TriMesh<TVertex>(*this);
    }

    template <typename TVertex>
    void TriMesh<TVertex>::Transform(const Maths::Matrix3D& transform)
    {
        *this = transform * *this;
    }

    template <typename TVertex>
    TriMesh<TVertex> operator*(const Maths::Matrix3D& transform, const TriMesh<TVertex>& mesh)
    {
        TVertex v[3] = { transform * mesh.points[0], transform * mesh.points[1], transform * mesh.points[2] };
        return TriMesh<TVertex>( v );
    }
}
