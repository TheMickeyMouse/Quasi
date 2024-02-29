#pragma once

#include "Rect.h"
#include "Debugging.h"

namespace Maths {
#pragma region Implementation
#define ARITH(T, U) stdu::arithmetic_t<T, U>
#define ARITH_T(T, U, O) typename ARITH(T, U)::O##_t
#define ARITH_DO(U, M) ARITH(scalar, U)::M
#define COMMA ,
    
#define F_ONLY requires std::is_floating_point_v<scalar>
#define S_ONLY requires std::is_signed_v<scalar>
#define S_ONLY_U(...) std::conditional_t<std::is_signed_v<T>, __VA_ARGS__, stdu::empty>
#define B_ONLY requires std::is_same_v<scalar, uchar>
#define BOOL_ONLY requires std::is_same_v<scalar, bool>
    
#define GENERIC_T template <class T>
#define VEC2_IMPL(R, N) GENERIC_T R vec2<T>::N
#pragma region Vec2
    VEC2_IMPL(auto, len     )() const { return std::sqrt(lensq()); }
    VEC2_IMPL(T,    lensq   )() const { return x*x + y*y; }
    VEC2_IMPL(auto, dist    )(const vec2& to) const { return (*this - to).len(); }
    VEC2_IMPL(auto, distsq  )(const vec2& to) const { return (*this - to).lensq(); }
    VEC2_IMPL(bool, in_range)(const vec2& other, T d) const { return distsq(other) <= d * d; }
    
    VEC2_IMPL(T,    sum)()                  const { return x + y; }
    VEC2_IMPL(auto, dot)(const vec2& other) const { return (*this * other).sum(); }
    
    VEC2_IMPL(vec2<T>, norm)(float d) const F_ONLY { auto l = d / len(); return { x * l, y * l }; }
    
    VEC2_IMPL(vec2<T>,  lerp        )(const vec2& other, float t)     const F_ONLY { float r = 1 - t; return { x * r + t * other.x, y * r + t * other.y }; }
    VEC2_IMPL(vec2<T>&, lerp_to     )(const vec2& other, float t)           F_ONLY { *this = lerp(other, t); return *this; }
    VEC2_IMPL(vec2<T>,  towards     )(const vec2& other, float max_d) const F_ONLY { auto u = (other - *this).norm(); return *this +  u * max_d; }
    VEC2_IMPL(vec2<T>&, move_towards)(const vec2& other, float max_d)       F_ONLY { auto u = (other - *this).norm();        *this += u * max_d; return *this; }
    
    VEC2_IMPL(vec2<T>,  clamped)() const F_ONLY { return clamp(ZERO(), ONE(), *this); }
    VEC2_IMPL(vec2<T>,  max    )(const vec2& a, const vec2& b) { return a.apply([](T x, T y) { return std::max(x, y); }, b); }
    VEC2_IMPL(vec2<T>,  min    )(const vec2& a, const vec2& b) { return a.apply([](T x, T y) { return std::min(x, y); }, b); }
    VEC2_IMPL(vec2<T>,  clamp  )(const vec2& rmin, const vec2& rmax, const vec2& x) { return min(rmax, max(rmin, x)); }

    VEC2_IMPL(auto, slope)() const { return y / x; }

    VEC2_IMPL(T,       angle       )(const vec2& other) const F_ONLY { return std::acos(dot(other) / (len() * other.len())); }
    VEC2_IMPL(T,       angle_signed)(const vec2& other) const F_ONLY { return std::atan2(y * other.x - x * other.y, dot(other)); }
    VEC2_IMPL(T,       angle    )() const F_ONLY { return std::atan2f((float)y, (float)x); } /* where (1, 0) has angle 0, then going counter-clockwise */
    VEC2_IMPL(vec2<T>, polar    )() const F_ONLY { return { len(), angle() }; } /* converts (x,y) into (r,theta) */
    VEC2_IMPL(vec2<T>, cartesian)() const F_ONLY { return { x * std::cos(y), x * std::sin(y) }; } /* converts (r,theta) into (x,y) */

    VEC2_IMPL(vec2<T>, perpend)() const S_ONLY { return { y, -x }; } /* perpendicular vector (rotated 90 deg), (0, 1) -> (1, 0) */
    VEC2_IMPL(vec2<T>,  rotated)(float angle) const F_ONLY {
        float sin = std::sin(angle), cos = std::cos(angle);
        return { x * cos - y * sin, x * sin + y * cos };
    } /* clockwise */
    VEC2_IMPL(vec2<T>&, rotate )(float angle) F_ONLY { return *this = rotated(angle); } /* clockwise */
    VEC2_IMPL(vec2<T>,  rotated)(float angle, const vec2& origin) const F_ONLY { return (*this - origin).rotated(angle) + origin; } /* clockwise */
    VEC2_IMPL(vec2<T>&, rotate )(float angle, const vec2& origin)       F_ONLY { return *this = rotated(angle, origin); } /* clockwise */
    VEC2_IMPL(vec2<T>,  reflected   )(const vec2& normal) const F_ONLY { return reflected_uc(normal.norm()); }
    VEC2_IMPL(vec2<T>&, reflect     )(const vec2& normal)       F_ONLY { reflect_uc(normal.norm()); return *this; }
    VEC2_IMPL(vec2<T>,  reflected_uc)(const vec2& normal) const F_ONLY { return *this - 2 * dot(normal) * normal; } /* this assumes normal is normalized */
    VEC2_IMPL(vec2<T>&, reflect_uc  )(const vec2& normal)       F_ONLY { return *this = reflected_uc(normal); } /* this assumes normal is normalized */

    VEC2_IMPL(vec2<T>,  project)(const vec2& axis) const F_ONLY { return axis * (dot(axis) / axis.lensq()); }

    VEC2_IMPL(vec3<T>, with_z)(T z) const { return { x, y, z }; }
    
    VEC2_IMPL(_rect_origin_inbetween_<vec2<T>>, as_origin)() const { return { *this }; }
    VEC2_IMPL(_rect_size_inbetween_<vec2<T>>,   as_size  )() const { return { *this }; }
    VEC2_IMPL(auto, to)(const vec2& other) const { return rect2<T> { *this, other }.corrected(); }
    VEC2_IMPL(auto, to)(const _rect_size_inbetween_<vec2>& other) const { return (rect2<T> { *this, other }).corrected(); }
    VEC2_IMPL(bool, is_in)(const rect<dimension, T>& region) const { return region.contains(*this); }

    VEC2_IMPL(bool, all)() const BOOL_ONLY { return x && y; }
    VEC2_IMPL(bool, any)() const BOOL_ONLY { return x || y; }
    
    template <class T> template <class F, class... Ts>
    auto vec2<T>::apply(F func, const vec2<Ts>&... args) const -> vec2<decltype(func((scalar)0, ((Ts)0)...))> {
        return vec2<decltype(func((T)0, ((Ts)0)...))> { func(x, args.x...), func(y, args.y...) };
    }

    VEC2_IMPL(, vec2)(Direction2D dir, T scale) : x(0), y(0) {
        using enum Direction2D;
        if (dir == UNIT) {
            x = y = scale; return;
        }
        if (dir < 0 || dir > DOWN) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }

    VEC2_IMPL(, vec2)(Corner2D cor, T scale) : x(0), y(0) {
        using enum Corner2D;
        if (cor < 0 || cor > BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }
    
#undef VEC2_IMPL
#pragma endregion // vec2
#pragma region Vec3
#define VEC3_IMPL(R, N) GENERIC_T R vec3<T>::N
    VEC3_IMPL(auto, len     )() const { return std::sqrt(lensq()); }
    VEC3_IMPL(T,    lensq   )() const { return x*x + y*y + z*z; }
    VEC3_IMPL(auto, dist    )(const vec3& to) const { return (*this - to).len(); }
    VEC3_IMPL(auto, distsq  )(const vec3& to) const { return (*this - to).lensq(); }
    VEC3_IMPL(bool, in_range)(const vec3& other, T d) const { return distsq(other) <= d * d; }
    
    VEC3_IMPL(T,    sum)()                  const { return x + y + z; }
    VEC3_IMPL(auto, dot)(const vec3& other) const { return (*this * other).sum(); }
    
    VEC3_IMPL(vec3<T>, norm)(float d) const F_ONLY { auto l = d / len(); return { x * l, y * l, z * l }; }
    
    VEC3_IMPL(vec3<T>,  lerp        )(const vec3& other, float t)     const F_ONLY { float r = 1 - t; return { x * r + t * other.x, y * r + t * other.y, z * r + t * other.z }; }
    VEC3_IMPL(vec3<T>&, lerp_to     )(const vec3& other, float t)           F_ONLY { *this = lerp(other, t); return *this; }
    VEC3_IMPL(vec3<T>,  towards     )(const vec3& other, float max_d) const F_ONLY { auto u = (other - *this).norm(max_d); return *this + u; }
    VEC3_IMPL(vec3<T>&, move_towards)(const vec3& other, float max_d)       F_ONLY { auto u = (other - *this).norm(max_d); *this += u; return *this; }
    
    VEC3_IMPL(vec3<T>,  clamped)() const F_ONLY { return clamp(ZERO(), ONE(), *this); }
    VEC3_IMPL(vec3<T>,  max    )(const vec3& a, const vec3& b) { return a.apply([](T x, T y) { return std::max(x, y); }, b); }
    VEC3_IMPL(vec3<T>,  min    )(const vec3& a, const vec3& b) { return a.apply([](T x, T y) { return std::min(x, y); }, b); }
    VEC3_IMPL(vec3<T>,  clamp  )(const vec3& rmin, const vec3& rmax, const vec3& x) { return min(rmax, max(rmin, x)); }
    
    VEC3_IMPL(vec3<T>,  cross)(const vec3& other) const { return { (T)(y * other.z - z * other.y), (T)(z * other.x - x * other.z), (T)(x * other.y - y * other.x) }; }

    VEC3_IMPL(T,       yaw  )() const F_ONLY { return std::atan2(x, z); }
    VEC3_IMPL(T,       pitch)() const F_ONLY { return std::asin(y / len()); }
    VEC3_IMPL(vec3<T>, spheric  )() const F_ONLY { return { len(), yaw(), pitch() }; }
    VEC3_IMPL(vec3<T>, cartesian)() const F_ONLY { return x * vec3 { std::sin(y) * std::cos(z), std::sin(z), std::cos(y) * std::cos(z) }; }

    VEC3_IMPL(T, angle       )(const vec3& other) const F_ONLY { return std::acos(dot(other) / (len() * other.len())); }
    VEC3_IMPL(T, angle_signed)(const vec3& other, const vec3& normal) const F_ONLY { return std::atan2(cross(other).dot(normal), dot(other)); }
    
    VEC3_IMPL(vec2<T>,  projected   )() const F_ONLY { return xy() / z; }
    VEC3_IMPL(vec3<T>,  reflected   )(const vec3& normal) const F_ONLY { return reflected_uc(normal.norm()); }
    VEC3_IMPL(vec3<T>&, reflect     )(const vec3& normal)       F_ONLY { reflect_uc(normal.norm()); return *this; }
    VEC3_IMPL(vec3<T>,  reflected_uc)(const vec3& normal) const F_ONLY { return *this - 2 * dot(normal) * normal; } /* this assumes normal is normalized */
    VEC3_IMPL(vec3<T>&, reflect_uc  )(const vec3& normal)       F_ONLY { *this = reflected_uc(normal); return *this; } /* this assumes normal is normalized */

    VEC3_IMPL(color,   color)(uchar a) const B_ONLY { return { x, y, z, a }; }
    VEC3_IMPL(colorf,  color)(float a) const F_ONLY { return { (float)x, (float)y, (float)z, a }; }
    VEC3_IMPL(color3,  color)() const B_ONLY { return { x, y, z }; }
    VEC3_IMPL(color3f, color)() const F_ONLY { return { (float)x, (float)y, (float)z }; }
    
    VEC3_IMPL(vec2<T>, xy    )()    const { return { x, y }; }
    VEC3_IMPL(vec4<T>, with_w)(T w) const { return { x, y, z, w }; }

    VEC3_IMPL(_rect_origin_inbetween_<vec3<T>>, as_origin)() const { return { *this }; }
    VEC3_IMPL(_rect_size_inbetween_<vec3<T>>,   as_size  )() const { return { *this }; }
    VEC3_IMPL(auto, to)(const vec3& other) const { return rect3<T> { *this, other }.corrected(); }
    VEC3_IMPL(auto, to)(const _rect_size_inbetween_<vec3>& other) const { return (rect3<T> { *this, other }).corrected(); }
    VEC3_IMPL(bool, is_in)(const rect<dimension, T>& region) const { return region.contains(*this); }

    VEC3_IMPL(bool, all)() const BOOL_ONLY { return x && y && z; }
    VEC3_IMPL(bool, any)() const BOOL_ONLY { return x || y || z; }
    
    template <class T> template <class F, class... Ts>
    auto vec3<T>::apply(F func, const vec3<Ts>&... args) const -> vec3<decltype(func((scalar)0, ((Ts)0)...))> {
        return vec3<decltype(func((T)0, ((Ts)0)...))> { func(x, args.x...), func(y, args.y...), func(z, args.z...) };
    }

    VEC3_IMPL(, vec3)(Direction3D dir, T scale) : x(0), y(0), z(0) {
        using enum Direction3D;
        if (dir == UNIT) {
            x = y = z = scale; return;
        }
        if (dir < 0 || dir > BACK) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }

    VEC3_IMPL(, vec3)(Corner3D cor, T scale) : x(0), y(0), z(0) {
        using enum Corner3D;
        if (cor < 0 || cor > BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? ARITH_DO(int, clamped_neg)(scale) : scale;
        z = (int)(cor & SIDE_BACK  ) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }
    
#undef VEC3_IMPL
#pragma endregion // vec3
#pragma region Vec4
#define VEC4_IMPL(R, N) GENERIC_T R vec4<T>::N
    VEC4_IMPL(auto, len     )() const { return std::sqrt(lensq()); }
    VEC4_IMPL(T,    lensq   )() const { return x*x + y*y + z*z + w*w; }
    VEC4_IMPL(auto, dist    )(const vec4& to) const { return (*this - to).len(); }
    VEC4_IMPL(auto, distsq  )(const vec4& to) const { return (*this - to).lensq(); }
    VEC4_IMPL(bool, in_range)(const vec4& other, T d) const { return distsq(other) <= d * d; }
    
    VEC4_IMPL(T,    sum)()                  const { return x + y + z + w; }
    VEC4_IMPL(auto, dot)(const vec4& other) const { return (*this * other).sum(); }
    
    VEC4_IMPL(vec4<T>, norm)(float d) const F_ONLY { auto l = d / len(); return { x * l, y * l, z * l, w * l }; }
    
    VEC4_IMPL(vec4<T>,  lerp        )(const vec4& other, float t)     const F_ONLY { float r = 1 - t; return { x*r + t*other.x, y*r + t*other.y, z*r + t*other.z, w*r + t*other.w }; }
    VEC4_IMPL(vec4<T>&, lerp_to     )(const vec4& other, float t)           F_ONLY { *this = lerp(other, t); return *this; }
    VEC4_IMPL(vec4<T>,  towards     )(const vec4& other, float max_d) const F_ONLY { auto u = (other - *this).norm(max_d); return *this + u; }
    VEC4_IMPL(vec4<T>&, move_towards)(const vec4& other, float max_d)       F_ONLY { auto u = (other - *this).norm(max_d); *this += u; return *this; }

    VEC4_IMPL(vec3<T>,  projected)() const F_ONLY { return xyz() / w; }
    
    VEC4_IMPL(T, angle)(const vec4& other) const F_ONLY { return std::acos(dot(other) / (len() * other.len())); }
    
    VEC4_IMPL(vec4<T>,  clamped)() const F_ONLY { return clamp(ZERO(), ONE(), *this); }
    VEC4_IMPL(vec4<T>,  max    )(const vec4& a, const vec4& b) { return a.apply([](T x, T y) { return std::max(x, y); }, b); }
    VEC4_IMPL(vec4<T>,  min    )(const vec4& a, const vec4& b) { return a.apply([](T x, T y) { return std::min(x, y); }, b); }
    VEC4_IMPL(vec4<T>,  clamp  )(const vec4& rmin, const vec4& rmax, const vec4& x) { return min(rmax, max(rmin, x)); }
    
    VEC4_IMPL(colorf, color)() const F_ONLY { return { (float)x, (float)y, (float)z }; }
    VEC4_IMPL(color,  color)() const B_ONLY { return { x, y, z }; }
    
    VEC4_IMPL(vec2<T>, xy )() const { return { x, y }; }
    VEC4_IMPL(vec3<T>, xyz)() const { return { x, y, z }; }

    VEC4_IMPL(_rect_origin_inbetween_<vec4<T>>, as_origin)() const { return { *this }; }
    VEC4_IMPL(_rect_size_inbetween_<vec4<T>>,   as_size  )() const { return { *this }; }
    VEC4_IMPL(auto, to)(const vec4& other) const { return rect4<T> { *this, other }.corrected(); }
    VEC4_IMPL(auto, to)(const _rect_size_inbetween_<vec4>& other) const { return (rect4<T> { *this, other }).corrected(); }
    VEC4_IMPL(bool, is_in)(const rect<dimension, T>& region) const { return region.contains(*this); }

    VEC4_IMPL(bool, all)() const BOOL_ONLY { return x && y && z && w; }
    VEC4_IMPL(bool, any)() const BOOL_ONLY { return x || y || z || w; }

    template <class T> template <class F, class... Ts>
    auto vec4<T>::apply(F func, const vec4<Ts>&... args) const -> vec4<decltype(func((scalar)0, ((Ts)0)...))> {
        return vec4<decltype(func((T)0, ((Ts)0)...))> { func(x, args.x...), func(y, args.y...), func(z, args.z...), func(w, args.w...) };
    }

    VEC4_IMPL(, vec4)(Direction4D dir, T scale) : x(0), y(0), z(0), w(0) {
        using enum Direction4D;
        if (dir == UNIT) {
            x = y = z = w = scale; return;
        }
        if (dir < 0 || dir > OUT) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? ARITH_DO(int, clamped_neg)(scale) : scale;
    }

    VEC4_IMPL(, vec4)(Corner4D cor, T scale) : x(0), y(0), z(0), w(0) {
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
    namespace Color {
        static constexpr uchar from_hexcode(char D0, char D1) {
            return (uchar)((D0 - (D0 < ':' ? '0' : 'W')) << 4 | (D1 - (D1 < ':' ? '0' : 'W')));
        }
    }

    constexpr color::color(const char (&hex)[10])
        : r(Color::from_hexcode(hex[1], hex[2])), g(Color::from_hexcode(hex[3], hex[4])),
          b(Color::from_hexcode(hex[5], hex[6])), a(Color::from_hexcode(hex[7], hex[8])) {
        ASSERT(hex[0] == '#');
    }

    constexpr color::color(const char (&hex)[8])
        : r(Color::from_hexcode(hex[1], hex[2])), g(Color::from_hexcode(hex[3], hex[4])),
          b(Color::from_hexcode(hex[5], hex[6])), a(255) {
        ASSERT(hex[0] == '#');
    }

    constexpr colorf::colorf(const char (&hex)[10])
        : r((float)Color::from_hexcode(hex[1], hex[2]) / 255.0f), g((float)Color::from_hexcode(hex[3], hex[4]) / 255.0f),
          b((float)Color::from_hexcode(hex[5], hex[6]) / 255.0f), a((float)Color::from_hexcode(hex[7], hex[8]) / 255.0f) {
        ASSERT(hex[0] == '#');
    }

    constexpr colorf::colorf(const char (&hex)[8])
        : r((float)Color::from_hexcode(hex[1], hex[2]) / 255.0f), g((float)Color::from_hexcode(hex[3], hex[4]) / 255.0f),
          b((float)Color::from_hexcode(hex[5], hex[6]) / 255.0f), a(1) {
        ASSERT(hex[0] == '#');
    }

    constexpr color3::color3(const char (&hex)[8])
        : r(Color::from_hexcode(hex[1], hex[2])), g(Color::from_hexcode(hex[3], hex[4])),
          b(Color::from_hexcode(hex[5], hex[6])) {
        ASSERT(hex[0] == '#');
    }

    constexpr color3f::color3f(const char (&hex)[8])
        : r((float)Color::from_hexcode(hex[1], hex[2]) / 255.0f), g((float)Color::from_hexcode(hex[3], hex[4]) / 255.0f),
          b((float)Color::from_hexcode(hex[5], hex[6]) / 255.0f) {
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

#pragma region Structured Bindings
#define IMPL_SB_VEC_T(V) \
    template <class T> struct tuple_size<Maths::V<T>> : integral_constant<size_t, Maths::V<T>::dimension> {}; \
    template <size_t I, class T> struct tuple_element<I, Maths::V<T>> { \
        using type = T; \
        static_assert(I < Maths::V<T>::dimension, "structured bindings out of bounds for " #V); }; \
    template <size_t I, class T> T   get(const Maths::V<T>& v) { return Maths::vec_get_i<I, Maths::V<T>>(v); } \
    template <size_t I, class T> T&  get(Maths::V<T>& v)  { return Maths::vec_get_i<I, Maths::V<T>>(v); } \
    template <size_t I, class T> T&& get(Maths::V<T>&& v) { return Maths::vec_get_i<I, Maths::V<T>>(std::move(v)); } \

#define IMPL_SB_VEC(V) \
    template <> struct tuple_size<Maths::V> : integral_constant<size_t, Maths::V::dimension> {}; \
    template <size_t I> struct tuple_element<I, Maths::V> { \
        using type = typename Maths::V::scalar; \
        static_assert(I < Maths::V::dimension, "structured bindings out of bounds for " #V); }; \
    template <size_t I> typename Maths::V::scalar   get(const Maths::V& v) { return Maths::vec_get_i<I, Maths::V>(v); } \
    template <size_t I> typename Maths::V::scalar&  get(Maths::V& v)  { return Maths::vec_get_i<I, Maths::V>(v); } \
    template <size_t I> typename Maths::V::scalar&& get(Maths::V&& v) { return Maths::vec_get_i<I, Maths::V>(std::move(v)); } \

namespace Maths {
    template<std::size_t I, class T>
    typename T::scalar vec_get_i(const T& v) {
        static_assert(I < T::dimension, "structured bindings out of bounds");
        return v.scalars[I];
    }

    template<std::size_t I, class T>
    typename T::scalar& vec_get_i(T& v) {
        static_assert(I < T::dimension, "structured bindings out of bounds");
        return v.scalars[I];
    }

    template<std::size_t I, class T>
    typename T::scalar&& vec_get_i(T&& v) {
        static_assert(I < T::dimension, "structured bindings out of bounds");
        return std::move(v.scalars[I]);
    }
}

namespace std {
    IMPL_SB_VEC_T(vec2)
    IMPL_SB_VEC_T(vec3)
    IMPL_SB_VEC_T(vec4)
    IMPL_SB_VEC(color3)
    IMPL_SB_VEC(color3f)
    IMPL_SB_VEC(color)
    IMPL_SB_VEC(colorf)
}

#undef IMPL_SB_VEC_T
#undef IMPL_SB_VEC
#pragma endregion
