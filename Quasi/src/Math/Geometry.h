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

        Line(const vec_t& s, const vec_t& e) : start(s), end(e) {}

        vec_t forward() const { return end - start; }

        Line  operator+ (const vec_t& off) const { return { start + off, end + off }; }
        Line& operator+=(const vec_t& off) { start += off; end += off; return *this; }
        Line  operator- (const vec_t& off) const { return { start - off, end - off }; }
        Line& operator-=(const vec_t& off) { start -= off; end -= off; return *this; }

        [[nodiscard]] float len() const { return start.dist(end); }
        [[nodiscard]] float lensq() const { return start.distsq(end); }

        [[nodiscard]] vec_t lerp(float t) const { return start + (end - start) * t; }
        [[nodiscard]] Tuple<float, float> intersect_solutions(const Line& other) const {
            const float det = (start.x - end.x) * (other.start.y - other.end.y) -
                              (start.y - end.y) * (other.start.x - other.end.x);
            const float t   = (start.x - other.start.x) * (other.start.y - other.end.y) -
                              (start.y - other.start.y) * (other.start.x - other.end.x);
            const float u   = (start.x - end.x) * (start.y - other.start.y) -
                              (start.y - end.y) * (start.x - other.start.x);
            if (std::abs(det) < EPSILON)
                return { -1, -1 };
            return { t / det, -u / det };
        }

        [[nodiscard]] bool intersects(const Line& other) const {
            const auto [t, u] = intersect_solutions(other);
            return 0 <= u && u <= 1 && 0 <= t && t <= 1;
        }

        [[nodiscard]] Option<vec_t> intersection(const Line& other) const {
            const auto [t, u] = intersect_solutions(other);
            return 0 <= u && u <= 1 && 0 <= t && t <= 1 ? Some(lerp(t)) : nullptr;
        }

        [[nodiscard]] float nearest_to_solution(const vec_t& p) const {
            return (p - start).dot(end - start) / (end - start).lensq();
        }

        [[nodiscard]] vec_t nearest_to(const vec_t& p) const {
            return lerp(std::clamp(nearest_to_solution(p), 0.0f, 1.0f));
        }

        [[nodiscard]] float dist_to_signed(const vec_t& p) const {
            return (p - start).dot((end - start).norm().perpend());
        }

        [[nodiscard]] float dist_to(const vec_t& p) const { return std::abs(dist_to_signed(p)); }

        [[nodiscard]] Tuple<float, float> nearest_between_solutions(const Line& other) const {
            const auto [t, u] = intersect_solutions(other);
            const bool tOutBounds = t < 0 || t > 1, uOutBounds = u < 0 || u > 1;
            switch (tOutBounds * 2 + uOutBounds) {
                case 0b00: return { t, u };
                case 0b01:
                    return { nearest_to_solution(u < 0 ? other.start : other.end), u < 0 ? 0.0f : 1.0f };
                case 0b10:
                    return { t < 0 ? 0.0f : 1.0f, other.nearest_to_solution(t < 0 ? start : end) };
                case 0b11: {
                    const float uPrime = other.nearest_to_solution(t < 0 ? start : end);
                    if (0 < uPrime && uPrime < 1)
                        return { t < 0 ? 0.0f : 1.0f, uPrime };
                    const float tPrime = nearest_to_solution(u < 0 ? other.start : other.end);
                    return { std::clamp(tPrime, 0.0f, 1.0f), u < 0.0f ? 0.0f : 1.0f };
                }
                default: return {};
            }
        }

        [[nodiscard]] Tuple<vec_t, vec_t> nearest_between(const Line& other) const {
            const auto [t, u] = nearest_between_solutions(other);
            return { lerp(t), other.lerp(u) };
        }

        [[nodiscard]] bool leftside(const vec_t& p) const {
            return (end.x - start.x) * (p.y - start.y) - (end.y - start.y) * (p.x - start.x) > 0;
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

        Triangle(const vec_t& a, const vec_t& b, const vec_t& c) : p1(a), p2(b), p3(c) {}

        [[nodiscard]] Line<N, T> line(u32 i) const { return { points[i], points[(i + 1) % 3] }; }

        [[nodiscard]] T signed_areaX2() const { return x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2); }
        [[nodiscard]] T areaX2() const { return std::abs(signed_areaX2()); }
        [[nodiscard]] T signed_area() const { return signed_areaX2() / 2; }
        [[nodiscard]] T area() const { return areaX2() / 2; }

        [[nodiscard]] vec_t center() const { return (p1 + p2 + p3) / (T)3; }
        [[nodiscard]] bool is_clockwise() const { return signed_areaX2() > 0; }

        [[nodiscard]] Vector3<T> barycentric(const vec_t& p) const requires (N == 2 && std::is_floating_point_v<T>) {
            const auto [x, y] = p;
            T det   =  (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
            T lamb1 = ((y2 - y3) * (x  - x3) + (x3 - x2) * (y  - y3)) / det;
            T lamb2 = ((y3 - y1) * (x  - x3) + (x1 - x3) * (y  - y3)) / det;
            return { lamb1, lamb2, 1 - lamb1 - lamb2 };
        }

        static bool is_valid_bary(const Vector3<T>& uvw) {
            return 0 <= uvw && uvw <= 1;
        }

        [[nodiscard]] vec_t from_barycentric(float u, float v) const requires (N == 2 && std::is_floating_point_v<T>) {
            return p1 * u + p2 * v + p3 * (1 - u - v);
        }

        [[nodiscard]] u32 categorize(const vec_t& p) const { return categorize_bary(barycentric(p)); }

        static u32 categorize_bary(const Vector3<T>& bc) {
            const auto [u, v, w] = bc;
            if (u < 1.0f / 3.0f)
                return 1 + (v < u) - (w < u);
            return v < w ? 2 : 0;
        }

        [[nodiscard]] vec_t project(const vec_t& p) const { return project_bary(barycentric(p)); }

        [[nodiscard]] vec_t project_bary(const Vector3<T>& bc) const {
            const auto [u, v, w] = bc;
            const u32 cat = categorize_bary(bc);
            switch (cat) { // see: https://www.desmos.com/calculator/gr29tnw1qx
                case 0:  return p1 + (p2 - p1) * ((v - w) / (1 - 3 * w));
                case 1:  return p2 + (p3 - p2) * ((u - v) / (1 - 3 * u));
                case 2:  return p3 + (p1 - p3) * ((w - u) / (1 - 3 * v));
                default: return 0;
            }
        }

        [[nodiscard]] bool contains(vec_t p) const {
            const Vector3<T> bary = barycentric(p);
            return is_valid_bary(bary);
        }

        vec_t* begin() { return points; }
        vec_t* end() { return points + 3; }
        [[nodiscard]] const vec_t* begin() const { return points; }
        [[nodiscard]] const vec_t* end() const { return points + 3; }

        vec_t& point(u32 i) { return points[i]; }
        [[nodiscard]] const vec_t& point(u32 i) const { return points[i]; }
        vec_t& cyclic_point(u32 i) { return points[i % 3]; }
        [[nodiscard]] const vec_t& cyclic_point(u32 i) const { return points[i % 3]; }

        [[nodiscard]] auto asf() const { if constexpr (std::is_floating_point_v<T>) return *this; else return (Triangle<N, float>)*this; }
        template <class U> [[nodiscard]] operator Triangle<N, U>() const { return { (vec_of_t<U>)p1, (vec_of_t<U>)p2, (vec_of_t<U>)p3 }; }
        template <uint M>  [[nodiscard]] operator Triangle<M, T>() const { return { (vec_of_n<M>)p1, (vec_of_n<M>)p2, (vec_of_n<M>)p3 }; }
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
