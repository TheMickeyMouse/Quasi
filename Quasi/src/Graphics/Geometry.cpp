#include "Geometry.h"

namespace Quasi::Graphics {
    Geometry Geometry::FromVertices(Vec<Math::fv3> v, Vec<TriIndices> i, bool smooth) {
        Geometry g { std::move(v), {}, std::move(i) };
        if (smooth) g.RecalcNormals(); else g.RecalcNormalsFlat();
        return g;
    }

    Geometry Geometry::FromVertAndNorm(Vec<Math::fv3> v, Vec<Math::fv3> n, Vec<TriIndices> i) {
        return { std::move(v), std::move(n), std::move(i) };
    }

    void Geometry::RecalcNormals() {
        normals.Resize(vertices.Length());
        for (const auto [i, j, k] : indices) {
            const Math::fv3 n = (vertices[j] - vertices[i]).Cross(vertices[k] - vertices[i]);
            normals[i] += n;
            normals[j] += n;
            normals[k] += n;
        }
        // auto averages normals with larger faces being weighted more
        for (auto& n : normals) {
            n = n.Norm();
        }
    }

    void Geometry::RecalcNormalsFlat() {
        Vec<Math::fv3> points;
        normals.Clear();
        for (const auto [i, j, k] : indices) {
            const Math::fv3& A = vertices[i], &B = vertices[j], &C = vertices[k], n = (B - A).Cross(C - A);
            points.Push(A);
            points.Push(B);
            points.Push(C);
            normals.Push(n); normals.Push(n); normals.Push(n); // dup 3 for each point
        }
        vertices = std::move(points);
    }

    void Geometry::ApplyTransform(const Math::Transform3D& model) {

    }
} // Quasi