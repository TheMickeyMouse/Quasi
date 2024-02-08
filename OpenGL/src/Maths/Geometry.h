#pragma once
#include "Constants.h"
#include "Vector.h"

namespace Maths::Geometry {
    // TODO: ADD MATH METHODS
    template <int N, class T>
    struct line {
        using vec_t = typename vecn<N, T>::type;
        union {
            vec_t points[2];
            struct { vec_t start, end; };
        };

        line(vec_t s, vec_t e) : start(s), end(e) {}

        vec_t as_vec() const { return end - start; }

        line  operator+ (const vec_t& off) const { return { start + off, end + off }; }
        line& operator+=(const vec_t& off) { start += off; end += off; return *this; }
        line  operator- (const vec_t& off) const { return { start - off, end - off }; }
        line& operator-=(const vec_t& off) { start -= off; end -= off; return *this; }
    };

    template <class T = float> using line2d = line<2, T>;
    template <class T = float> using line3d = line<3, T>;
    template <class T = float> using line4d = line<4, T>;
    using iline2d = line2d<int>;
    using iline3d = line3d<int>;
    using iline4d = line4d<int>;
    using uline2d = line2d<uint>;
    using uline3d = line3d<uint>;
    using uline4d = line4d<uint>;
    using fline2d = line2d<float>;
    using fline3d = line3d<float>;
    using fline4d = line4d<float>;
    using dline2d = line2d<double>;
    using dline3d = line3d<double>;
    using dline4d = line4d<double>;
    using bline2d = line2d<uchar>;
    using bline3d = line3d<uchar>;
    using bline4d = line4d<uchar>;

    // TODO: ADD MATH METHODS
    template <int N, class T>
    union triangle {
        using vec_t = typename vecn<N, T>::type;
        template <class U> using vec_of_t = typename vecn<N, U>::type;
        template <int M> using vec_of_n = typename vecn<M, T>::type;

        vec_t points[3];
        struct {
            vec_t p1, p2, p3;
        };
        struct {
            T x1, y1, x2, y2, x3, y3;
        };

        triangle(T radius) requires (N == 2 && std::is_floating_point_v<T>)
        : p1(radius, 0),
          p2(-HALF_ROOT_3 * radius,  radius * 0.5),
          p3(-HALF_ROOT_3 * radius, -radius * 0.5) {}

        triangle(vec_t a, vec_t b, vec_t c) : p1(a), p2(b), p3(c) {}

        line<N, T> line(uint i) const { return { points[i], points[(i + 1) % 3] }; }

        T signed_areaX2() const { return x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2); }
        T areaX2() const { return std::abs(signed_areaX2()); }
        T signed_area() const { return signed_areaX2() / 2; }
        T area() const { return areaX2() / 2; }

        vec3<T> barycentric(vec_t p) requires (N == 2 && std::is_floating_point_v<T>) {
            const auto [x, y] = p;
            T det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
            T lamb1 = (y2 - y3) * (x - x3) + (x3 - x2) * (y - y3) / det;
            T lamb2 = (y3 - y1) * (x - x3) + (x1 - x3) * (y - y3) / det;
            return { lamb1, lamb2, 1 - lamb1 - lamb2 };
        }

        bool contains(vec_t p) {
            const vec3<T> bary = barycentric(p);
            return 0 < bary && bary < 1;
        }

        vec_t* begin() { return points; }
        vec_t* end() { return points + 3; }
        [[nodiscard]] const vec_t* begin() const { return points; }
        [[nodiscard]] const vec_t* end() const { return points + 3; }

        auto asf() const { if constexpr (std::is_floating_point_v<T>) return *this; else return (triangle<N, float>)*this; }
        template <class U> operator triangle<N, U>() const { return { (vec_of_t<U>)p1, (vec_of_t<U>)p2, (vec_of_t<U>)p3 }; }
        template <int M>   operator triangle<M, T>() const { return { (vec_of_n<M>)p1, (vec_of_n<M>)p2, (vec_of_n<M>)p3 }; }
    };

    template <class T = float> using triangle2d = triangle<2, T>;
    template <class T = float> using triangle3d = triangle<3, T>;
    template <class T = float> using triangle4d = triangle<4, T>;
    using ftriangle2d = triangle2d<float>;
    using ftriangle3d = triangle3d<float>;
    using ftriangle4d = triangle4d<float>;
    using dtriangle2d = triangle2d<double>;
    using dtriangle3d = triangle3d<double>;
    using dtriangle4d = triangle4d<double>;
    using itriangle2d = triangle2d<int>;
    using itriangle3d = triangle3d<int>;
    using itriangle4d = triangle4d<int>;
}
