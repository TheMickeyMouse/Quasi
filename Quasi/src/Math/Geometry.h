#pragma once
#include <algorithm>

#include "Constants.h"
#include "Vector.h"

namespace Quasi::Math {
    // TODO: ADD MATH METHODS
    template <u32 N, class T>
    struct Line {
        using vec_t = VectorN<N, T>;
        vec_t start, end;

        Line(vec_t s, vec_t e) : start(s), end(e) {}

        vec_t as_vec() const { return end - start; }

        Line  operator+ (const vec_t& off) const { return { start + off, end + off }; }
        Line& operator+=(const vec_t& off) { start += off; end += off; return *this; }
        Line  operator- (const vec_t& off) const { return { start - off, end - off }; }
        Line& operator-=(const vec_t& off) { start -= off; end -= off; return *this; }

        vec_t NearestTo(const vec_t& p) const {
            const float t = (p - start).dot(end - start) / (end - start).lensq();
            return start.lerp(end, std::clamp(t, 0.0f, 1.0f));
        }
    };

    template <class T = float> using Line2D = Line<2, T>;
    template <class T = float> using Line3D = Line<3, T>;
    template <class T = float> using Line4D = Line<4, T>;
    using iLine2D = Line2D<int>;
    using iLine3D = Line3D<int>;
    using iLine4D = Line4D<int>;
    using uLine2D = Line2D<uint>;
    using uLine3D = Line3D<uint>;
    using uLine4D = Line4D<uint>;
    using fLine2D = Line2D<float>;
    using fLine3D = Line3D<float>;
    using fLine4D = Line4D<float>;
    using dLine2D = Line2D<double>;
    using dLine3D = Line3D<double>;
    using dLine4D = Line4D<double>;
    using bLine2D = Line2D<byte>;
    using bLine3D = Line3D<byte>;
    using bLine4D = Line4D<byte>;

    // TODO: ADD MATH METHODS
    template <u32 N, class T>
    union Triangle {
        using vec_t = VectorN<N, T>;
        template <class U> using vec_of_t = VectorN<N, U>;
        template <int M> using vec_of_n = VectorN<M, T>;

        vec_t points[3];
        struct {
            vec_t p1, p2, p3;
        };
        struct {
            T x1, y1, x2, y2, x3, y3;
        };

        Triangle(T radius) requires (N == 2 && std::is_floating_point_v<T>)
        : p1(radius, 0),
          p2(-HALF_ROOT_3 * radius,  radius * 0.5),
          p3(-HALF_ROOT_3 * radius, -radius * 0.5) {}

        Triangle(vec_t a, vec_t b, vec_t c) : p1(a), p2(b), p3(c) {}

        Line<N, T> line(u32 i) const { return { points[i], points[(i + 1) % 3] }; }

        T signed_areaX2() const { return x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2); }
        T areaX2() const { return std::abs(signed_areaX2()); }
        T signed_area() const { return signed_areaX2() / 2; }
        T area() const { return areaX2() / 2; }

        Vector3<T> barycentric(vec_t p) const requires (N == 2 && std::is_floating_point_v<T>) {
            const auto [x, y] = p;
            T det   = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
            T lamb1 = (y2 - y3) * (x  - x3) + (x3 - x2) * (y  - y3) / det;
            T lamb2 = (y3 - y1) * (x  - x3) + (x1 - x3) * (y  - y3) / det;
            return { lamb1, lamb2, 1 - lamb1 - lamb2 };
        }

        bool contains(vec_t p) const {
            const Vector3<T> bary = barycentric(p);
            return 0.0f < bary && bary < 1.0f;
        }

        vec_t* begin() { return points; }
        vec_t* end() { return points + 3; }
        [[nodiscard]] const vec_t* begin() const { return points; }
        [[nodiscard]] const vec_t* end() const { return points + 3; }

        vec_t& point(u32 i) { return points[i]; }
        [[nodiscard]] const vec_t& point(u32 i) const { return points[i]; }

        auto asf() const { if constexpr (std::is_floating_point_v<T>) return *this; else return (Triangle<N, float>)*this; }
        template <class U> operator Triangle<N, U>() const { return { (vec_of_t<U>)p1, (vec_of_t<U>)p2, (vec_of_t<U>)p3 }; }
        template <uint M>  operator Triangle<M, T>() const { return { (vec_of_n<M>)p1, (vec_of_n<M>)p2, (vec_of_n<M>)p3 }; }
    };

    template <class T = float> using Triangle2D = Triangle<2, T>;
    template <class T = float> using Triangle3D = Triangle<3, T>;
    template <class T = float> using Triangle4D = Triangle<4, T>;
    using fTriangle2D = Triangle2D<float>;
    using fTriangle3D = Triangle3D<float>;
    using fTriangle4D = Triangle4D<float>;
    using dTriangle2D = Triangle2D<double>;
    using dTriangle3D = Triangle3D<double>;
    using dTriangle4D = Triangle4D<double>;
    using iTriangle2D = Triangle2D<int>;
    using iTriangle3D = Triangle3D<int>;
    using iTriangle4D = Triangle4D<int>;
}
