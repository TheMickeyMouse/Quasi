#pragma once

#ifndef VECTOR_IMPL
#include "Vector.h"
#endif
#include "Rect.h"

namespace Maths {
#pragma region Implementation
#define ARITH(T, U) stdu::arithmetic_t<T, U>
#define ARITH_T(T, U, O) typename ARITH(T, U)::O##_t
#define ARITH_DO(U, M) ARITH(scalar, U)::M
#define COMMA ,
    
#define F_ONLY(...) template <class S> std::enable_if_t<std::is_floating_point_v<S>, __VA_ARGS__>
#define S_ONLY(...) template <class S> std::enable_if_t<std::is_signed_v<S>, __VA_ARGS__>
#define S_ONLY_U(...) std::conditional_t<std::is_signed_v<T>, __VA_ARGS__, stdu::empty>
#define B_ONLY(...) template <class S> std::enable_if_t<std::is_same_v<S, uchar>, __VA_ARGS__>
#define BOOL_ONLY(...) template <class S> std::enable_if_t<std::is_same_v<S, bool>, __VA_ARGS__>
    
#define GENERIC_T template <class T>
#define VEC2_IMPL(R, N, FF) GENERIC_T STDU_IF_ELSE(FF, (F_ONLY(R)), (R)) vec2<T>::N
#pragma region Vec2
    VEC2_IMPL(float,    len,      0)() const { return std::sqrtf((float)lensq()); }
    VEC2_IMPL(T,        lensq,    0)() const { return x*x + y*y; }
    VEC2_IMPL(float,    dist,     0)(const vec2& to) const { return (*this - to).len(); }
    VEC2_IMPL(auto,     distsq,   0)(const vec2& to) const { return (*this - to).lensq(); }
    VEC2_IMPL(bool,     in_range, 0)(const vec2& other, T d) const { return distsq(other) <= d * d; }
    
    VEC2_IMPL(T,        sum, 0)()                  const { return x + y; }
    VEC2_IMPL(auto,     dot, 0)(const vec2& other) const { return (*this * other).sum(); }
    
    VEC2_IMPL(fvec2,    norm, 1)(float d) const { float l = d / len(); return { x * l, y * l }; }
    
    VEC2_IMPL(vec2<T>,  lerp,         1)(const vec2& other, float t)     const { float r = 1 - t; return { x * r + t * other.x, y * r + t * other.y }; }
    VEC2_IMPL(vec2<T>&, lerp_to,      1)(const vec2& other, float t)           { *this = lerp(other, t); return *this; }
    VEC2_IMPL(vec2<T>,  towards,      1)(const vec2& other, float max_d) const { auto u = (other - *this).norm(); return *this +  u * max_d; }
    VEC2_IMPL(vec2<T>&, move_towards, 1)(const vec2& other, float max_d)       { auto u = (other - *this).norm();        *this += u * max_d; return *this; }
    
    VEC2_IMPL(vec2<T>,  clamped, 1)() const { return clamp(ZERO(), ONE(), *this); }
    VEC2_IMPL(vec2<T>,  max    , 0)(const vec2& a, const vec2& b) { return a.apply([](T x, T y) { return std::max(x, y); }, b); }
    VEC2_IMPL(vec2<T>,  min    , 0)(const vec2& a, const vec2& b) { return a.apply([](T x, T y) { return std::min(x, y); }, b); }
    VEC2_IMPL(vec2<T>,  clamp  , 0)(const vec2& rmin, const vec2& rmax, const vec2& x) { return min(rmax, max(rmin, x)); }

    VEC2_IMPL(auto, slope, 0)() const { return y / x; }

    VEC2_IMPL(float, angle,        1)(const vec2& other) const { return std::acos(dot(other) / (len() * other.len())); }
    VEC2_IMPL(float, angle_signed, 1)(const vec2& other) const { return std::atan2(y * other.x - x * other.y, dot(other)); }
    VEC2_IMPL(float, angle,     0)() const { return std::atan2f((float)y, (float)x); } /* where (1, 0) has angle 0, then going counter-clockwise */
    VEC2_IMPL(fvec2, polar,     0)() const { return { len(), angle() }; } /* converts (x,y) into (r,theta) */
    VEC2_IMPL(fvec2, cartesian, 1)() const { return { x * std::cos(y), x * std::sin(y) }; } /* converts (r,theta) into (x,y) */

    VEC2_IMPL(S_ONLY(vec2<T>), perpend, 0)() const { return { y, -x }; } /* perpendicular vector (rotated 90 deg), (0, 1) -> (1, 0) */
    VEC2_IMPL(vec2<T>,  rotated, 1)(float angle) const {
        float sin = std::sin(angle), cos = std::cos(angle);
        return { x * cos - y * sin, x * sin + y * cos };
    } /* clockwise */
    VEC2_IMPL(vec2<T>&, rotate,  1)(float angle) { *this = rotated(angle); } /* clockwise */
    VEC2_IMPL(vec2<T>,  rotated, 1)(float angle, const vec2& origin) const { return (*this - origin).rotated(angle) + origin; } /* clockwise */
    VEC2_IMPL(vec2<T>&, rotate,  1)(float angle, const vec2& origin)       { *this = rotated(angle, origin); return *this; } /* clockwise */
    VEC2_IMPL(vec2<T>,  reflected,    1)(const vec2& normal) const { return reflected_uc(normal.norm()); }
    VEC2_IMPL(vec2<T>&, reflect,      1)(const vec2& normal)       { reflect(normal.norm()); return *this; }
    VEC2_IMPL(vec2<T>,  reflected_uc, 1)(const vec2& normal) const { return *this - 2 * dot(normal) * normal; } /* this assumes normal is normalized */
    VEC2_IMPL(vec2<T>&, reflect_uc,   1)(const vec2& normal)       { *this = reflected_uc(normal); return *this; } /* this assumes normal is normalized */

    VEC2_IMPL(vec3<T>, with_z, 0)(T z) const { return { x, y, z }; }
    
    VEC2_IMPL(_rect_origin_inbetween_<vec2<T>>, as_origin, 0)() const { return { *this }; }
    VEC2_IMPL(_rect_size_inbetween_<vec2<T>>,   as_size,   0)() const { return { *this }; }
    VEC2_IMPL(auto, to, 0)(const vec2& other) const { return rect2<T> { *this, other }.corrected(); }
    VEC2_IMPL(auto, to, 0)(const _rect_size_inbetween_<vec2>& other) const { return (rect2<T> { *this, other }).corrected(); }
    VEC2_IMPL(bool, is_in, 0)(const rect<dimension, T>& region) const { return region.contains(*this); }

    VEC2_IMPL(BOOL_ONLY(bool), all, 0)() const { return x && y; }
    VEC2_IMPL(BOOL_ONLY(bool), any, 0)() const { return x || y; }
    
    template <class T> template <class F, class... Ts>
    auto vec2<T>::apply(F func, const vec2<Ts>&... args) const -> vec2<decltype(func((scalar)0, ((Ts)0)...))> {
        return vec2<decltype(func((T)0, ((Ts)0)...))> { func(x, args.x...), func(y, args.y...) };
    }

    VEC2_IMPL(, vec2, 0)(Direction2D dir, T scale) : x(0), y(0) {
        using enum Direction2D;
        if (dir == UNIT) {
            x = y = scale; return;
        }
        if (dir < 0 || dir > DOWN) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }

    VEC2_IMPL(, vec2, 0)(Corner2D cor, T scale) : x(0), y(0) {
        using enum Corner2D;
        if (cor < 0 || cor > BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }
    
#undef VEC2_IMPL
#pragma endregion // vec2
#pragma region Vec3
#define VEC3_IMPL(R, N, FF) GENERIC_T STDU_IF_ELSE(FF, (F_ONLY(R)), (R)) vec3<T>::N
    VEC3_IMPL(float,    len,      0)() const { return std::sqrtf((float)lensq()); }
    VEC3_IMPL(T,        lensq,    0)() const { return x*x + y*y + z*z; }
    VEC3_IMPL(float,    dist,     0)(const vec3& to) const { return (*this - to).len(); }
    VEC3_IMPL(auto,     distsq,   0)(const vec3& to) const { return (*this - to).lensq(); }
    VEC3_IMPL(bool,     in_range, 0)(const vec3& other, T d) const { return distsq(other) <= d * d; }
    
    VEC3_IMPL(T,        sum, 0)()                  const { return x + y + z; }
    VEC3_IMPL(auto,     dot, 0)(const vec3& other) const { return (*this * other).sum(); }
    
    VEC3_IMPL(fvec3,    norm, 1)(float d) const { float l = d / len(); return { x * l, y * l, z * l }; }
    
    VEC3_IMPL(vec3<T>,  lerp,         1)(const vec3& other, float t)     const { float r = 1 - t; return { x * r + t * other.x, y * r + t * other.y, z * r + t * other.z }; }
    VEC3_IMPL(vec3<T>&, lerp_to,      1)(const vec3& other, float t)           { *this = lerp(other, t); return *this; }
    VEC3_IMPL(vec3<T>,  towards,      1)(const vec3& other, float max_d) const { auto u = (other - *this).norm(max_d); return *this + u; }
    VEC3_IMPL(vec3<T>&, move_towards, 1)(const vec3& other, float max_d)       { auto u = (other - *this).norm(max_d); *this += u; return *this; }
    
    VEC3_IMPL(vec3<T>,  clamped, 1)() const { return clamp(ZERO(), ONE(), *this); }
    VEC3_IMPL(vec3<T>,  max,     0)(const vec3& a, const vec3& b) { return a.apply([](T x, T y) { return std::max(x, y); }, b); }
    VEC3_IMPL(vec3<T>,  min,     0)(const vec3& a, const vec3& b) { return a.apply([](T x, T y) { return std::min(x, y); }, b); }
    VEC3_IMPL(vec3<T>,  clamp,   0)(const vec3& rmin, const vec3& rmax, const vec3& x) { return min(rmax, max(rmin, x)); }
    
    VEC3_IMPL(vec3<T>,  cross, 0)(const vec3& other) const { return { (T)(y * other.z - z * other.y), (T)(z * other.x - x * other.z), (T)(x * other.y - y * other.x) }; }

    VEC3_IMPL(float,    altitude,  1)() const { return std::acos(z / len()); }
    VEC3_IMPL(float,    azimuth,   1)() const { return xy().angle(); }
    VEC3_IMPL(fvec3,    spheric,   1)() const { return { len(), altitude(), azimuth() }; }
    VEC3_IMPL(fvec3,    cartesian, 1)() const { float sin = std::sin(y); return x * fvec3 { sin * std::cos(z), sin * std::sin(z), std::cos(y) }; }

    VEC3_IMPL(float,    angle,        1)(const vec3& other) const { return std::acos(dot(other) / (len() * other.len())); }
    VEC3_IMPL(float,    angle_signed, 1)(const vec3& other, const vec3& normal) const { return std::atan2(cross(other).dot(normal), dot(other)); }
    
    VEC3_IMPL(vec2<T>,  projected,    1)() const { return xy() / z; }
    VEC3_IMPL(vec3<T>,  reflected,    1)(const vec3& normal) const { return reflected_uc(normal.norm()); }
    VEC3_IMPL(vec3<T>&, reflect,      1)(const vec3& normal)       { reflect(normal.norm()); return *this; }
    VEC3_IMPL(vec3<T>,  reflected_uc, 1)(const vec3& normal) const { return *this - 2 * dot(normal) * normal; } /* this assumes normal is normalized */
    VEC3_IMPL(vec3<T>&, reflect_uc,   1)(const vec3& normal)       { *this = reflected_uc(normal); return *this; } /* this assumes normal is normalized */

    VEC3_IMPL(B_ONLY(color),   color, 0)(uchar a) const { return { x, y, z, a }; }
    VEC3_IMPL(F_ONLY(colorf),  color, 0)(float a) const { return { x, y, z, a }; }
    VEC3_IMPL(B_ONLY(color3),  color, 0)() const { return { x, y, z }; }
    VEC3_IMPL(F_ONLY(color3f), color, 0)() const { return { x, y, z }; }
    
    VEC3_IMPL(vec2<T>, xy,     0)()    const { return { x, y }; }
    VEC3_IMPL(vec4<T>, with_w, 0)(T w) const { return { x, y, z, w }; }

    VEC3_IMPL(_rect_origin_inbetween_<vec3<T>>, as_origin, 0)() const { return { *this }; }
    VEC3_IMPL(_rect_size_inbetween_<vec3<T>>,   as_size,   0)() const { return { *this }; }
    VEC3_IMPL(auto, to, 0)(const vec3& other) const { return rect3<T> { *this, other }.corrected(); }
    VEC3_IMPL(auto, to, 0)(const _rect_size_inbetween_<vec3>& other) const { return (rect3<T> { *this, other }).corrected(); }
    VEC3_IMPL(bool, is_in, 0)(const rect<dimension, T>& region) const { return region.contains(*this); }

    VEC3_IMPL(BOOL_ONLY(bool), all, 0)() const { return x && y && z; }
    VEC3_IMPL(BOOL_ONLY(bool), any, 0)() const { return x || y || z; }
    
    template <class T> template <class F, class... Ts>
    auto vec3<T>::apply(F func, const vec3<Ts>&... args) const -> vec3<decltype(func((scalar)0, ((Ts)0)...))> {
        return vec3<decltype(func((T)0, ((Ts)0)...))> { func(x, args.x...), func(y, args.y...), func(z, args.z...) };
    }

    VEC3_IMPL(, vec3, 0)(Direction3D dir, T scale) : x(0), y(0), z(0) {
        using enum Direction3D;
        if (dir == UNIT) {
            x = y = z = scale; return;
        }
        if (dir < 0 || dir > BACK) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }

    VEC3_IMPL(, vec3, 0)(Corner3D cor, T scale) : x(0), y(0), z(0) {
        using enum Corner3D;
        if (cor < 0 || cor > BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        y = (int)(cor & SIDE_BACK  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }
    
#undef VEC3_IMPL
#pragma endregion // vec3
#pragma region Vec4
#define VEC4_IMPL(R, N, FF) GENERIC_T STDU_IF_ELSE(FF, (F_ONLY(R)), (R)) vec4<T>::N
    VEC4_IMPL(float,    len,      0)() const { return std::sqrtf((float)lensq()); }
    VEC4_IMPL(T,        lensq,    0)() const { return x*x + y*y + z*z + w*w; }
    VEC4_IMPL(float,    dist,     0)(const vec4& to) const { return (*this - to).len(); }
    VEC4_IMPL(auto,     distsq,   0)(const vec4& to) const { return (*this - to).lensq(); }
    VEC4_IMPL(bool,     in_range, 0)(const vec4& other, T d) const { return distsq(other) <= d * d; }
    
    VEC4_IMPL(T,        sum, 0)()                  const { return x + y + z + w; }
    VEC4_IMPL(auto,     dot, 0)(const vec4& other) const { return (*this * other).sum(); }
    
    VEC4_IMPL(fvec4,    norm, 1)(float d) const { float l = d / len(); return { x * l, y * l, z * l, w * l }; }
    
    VEC4_IMPL(vec4<T>,  lerp,         1)(const vec4& other, float t)     const { float r = 1 - t; return { x*r + t*other.x, y*r + t*other.y, z*r + t*other.z, w*r + t*other.w }; }
    VEC4_IMPL(vec4<T>&, lerp_to,      1)(const vec4& other, float t)           { *this = lerp(other, t); return *this; }
    VEC4_IMPL(vec4<T>,  towards,      1)(const vec4& other, float max_d) const { auto u = (other - *this).norm(max_d); return *this + u; }
    VEC4_IMPL(vec4<T>&, move_towards, 1)(const vec4& other, float max_d)       { auto u = (other - *this).norm(max_d); *this += u; return *this; }

    VEC4_IMPL(vec3<T>,  projected, 1)() const { return xyz() / w; }
    
    VEC4_IMPL(float,    angle, 1)(const vec4& other) const { return std::acos(dot(other) / (len() * other.len())); }
    
    VEC4_IMPL(vec4<T>,  clamped, 1)() const { return clamp(ZERO(), ONE(), *this); }
    VEC4_IMPL(vec4<T>,  max,     0)(const vec4& a, const vec4& b) { return a.apply([](T x, T y) { return std::max(x, y); }, b); }
    VEC4_IMPL(vec4<T>,  min,     0)(const vec4& a, const vec4& b) { return a.apply([](T x, T y) { return std::min(x, y); }, b); }
    VEC4_IMPL(vec4<T>,  clamp,   0)(const vec4& rmin, const vec4& rmax, const vec4& x) { return min(rmax, max(rmin, x)); }
    
    VEC4_IMPL(F_ONLY(colorf), color,   0)() const { return { x, y, z }; }
    VEC4_IMPL(B_ONLY(color),  color,   0)() const { return { x, y, z }; }
    
    VEC4_IMPL(vec2<T>, xy,  0)() const { return { x, y }; }
    VEC4_IMPL(vec3<T>, xyz, 0)() const { return { x, y, z }; }

    VEC4_IMPL(_rect_origin_inbetween_<vec4<T>>, as_origin, 0)() const { return { *this }; }
    VEC4_IMPL(_rect_size_inbetween_<vec4<T>>,   as_size,   0)() const { return { *this }; }
    VEC4_IMPL(auto, to, 0)(const vec4& other) const { return rect4<T> { *this, other }.corrected(); }
    VEC4_IMPL(auto, to, 0)(const _rect_size_inbetween_<vec4>& other) const { return (rect4<T> { *this, other }).corrected(); }
    VEC4_IMPL(bool, is_in, 0)(const rect<dimension, T>& region) const { return region.contains(*this); }

    VEC4_IMPL(BOOL_ONLY(bool), all, 0)() const { return x && y && z && w; }
    VEC4_IMPL(BOOL_ONLY(bool), any, 0)() const { return x || y || z || w; }

    template <class T> template <class F, class... Ts>
    auto vec4<T>::apply(F func, const vec4<Ts>&... args) const -> vec4<decltype(func((scalar)0, ((Ts)0)...))> {
        return vec4<decltype(func((T)0, ((Ts)0)...))> { func(x, args.x...), func(y, args.y...), func(z, args.z...), func(w, args.w...) };
    }

    VEC4_IMPL(, vec4, 0)(Direction4D dir, T scale) : x(0), y(0), z(0), w(0) {
        using enum Direction4D;
        if (dir == UNIT) {
            x = y = z = w = scale; return;
        }
        if (dir < 0 || dir > OUT) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }

    VEC4_IMPL(, vec4, 0)(Corner4D cor, T scale) : x(0), y(0), z(0), w(0) {
        using enum Corner4D;
        if (cor < 0 || cor > OUTER_BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        z = (int)(cor & SIDE_BACK  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        w = (int)(cor & SIDE_OUTER ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }
        
#undef VEC4_IMPL
#pragma endregion // vec4
#pragma region Color Constructors
    // 'a'-10 = W, '9'+1 = ':'
    static constexpr uchar hexcode(char D0, char D1) {
        return (uchar)((D0 - (D0 < ':' ? '0' : 'W')) << 4 | (D1 - (D1 < ':' ? '0' : 'W')));
    }

    constexpr color::color(const char (&hex)[10])
        : r(hexcode(hex[1], hex[2])), g(hexcode(hex[3], hex[4])),
          b(hexcode(hex[5], hex[6])), a(hexcode(hex[7], hex[8])) {
        ASSERT(hex[0] == '#');
    }

    constexpr color::color(const char (&hex)[8])
        : r(hexcode(hex[1], hex[2])), g(hexcode(hex[3], hex[4])),
          b(hexcode(hex[5], hex[6])), a(255) {
        ASSERT(hex[0] == '#');
    }

    constexpr colorf::colorf(const char (&hex)[10])
        : r((float)hexcode(hex[1], hex[2]) / 255.0f), g((float)hexcode(hex[3], hex[4]) / 255.0f),
          b((float)hexcode(hex[5], hex[6]) / 255.0f), a((float)hexcode(hex[7], hex[8]) / 255.0f) {
        ASSERT(hex[0] == '#');
    }

    constexpr colorf::colorf(const char (&hex)[8])
        : r((float)hexcode(hex[1], hex[2]) / 255.0f), g((float)hexcode(hex[3], hex[4]) / 255.0f),
          b((float)hexcode(hex[5], hex[6]) / 255.0f), a(1) {
        ASSERT(hex[0] == '#');
    }

    constexpr color3::color3(const char (&hex)[8])
        : r(hexcode(hex[1], hex[2])), g(hexcode(hex[3], hex[4])),
          b(hexcode(hex[5], hex[6])) {
        ASSERT(hex[0] == '#');
    }

    constexpr color3f::color3f(const char (&hex)[8])
        : r((float)hexcode(hex[1], hex[2]) / 255.0f), g((float)hexcode(hex[3], hex[4]) / 255.0f),
          b((float)hexcode(hex[5], hex[6]) / 255.0f) {
        ASSERT(hex[0] == '#');
    }
#pragma endregion // color constructors
#pragma endregion // implementation

#undef F_ONLY
#undef S_ONLY
#undef S_ONLY_T
#undef B_ONLY
#undef BOOL_ONLY
#undef GENERIC_T

#undef COMMA
    
#undef ARITH_DO
#undef ARITH
#undef ARITH_T
}