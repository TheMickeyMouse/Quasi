#pragma once
#include <type_traits>
#include <cmath>
#include <string>
#include <format>

#include "Corner.h"
#include "Direction.h"
#include "NumTypes.h"
#include "opengl.h"
#include "Debugging.h"
#include <stdu/macros.h>
#include <stdu/types.h>

namespace Maths {
#pragma region Declaration
#pragma region Concepts and Decls
    template <class T> T neg_t(T x) { return -x; }
    template <std::unsigned_integral S> S neg_t(S) { return 0; }
    
    template <class T> T mod_t(T a, T b) { return a % b; }
    template <std::floating_point F> F mod_t(F a, F b) { return std::fmod(a, b); }
    template <class T> T mul_t(T a, T b) { return (T)(a * b); }
    
    template <class T> struct vec2;
    template <class T> struct vec3;
    template <class T> struct vec4;

    template <int, class> struct vecn { using type = void; };
    template <class T> struct vecn<2, T> { using type = vec2<T>; };
    template <class T> struct vecn<3, T> { using type = vec3<T>; };
    template <class T> struct vecn<4, T> { using type = vec4<T>; };
    
    template <class> struct is_vec_t : std::false_type {};
    template <class T> struct is_vec_t<vec2<T>> : std::true_type {};
    template <class T> struct is_vec_t<vec3<T>> : std::true_type {};
    template <class T> struct is_vec_t<vec4<T>> : std::true_type {};

    template <class V>
    constexpr bool is_vec_v = is_vec_t<V>::value;
    
    template <class V>
    concept vec_t = is_vec_v<V>;

    using fvec2 = vec2<float>;
    using fvec3 = vec3<float>;
    using fvec4 = vec4<float>;
    using dvec2 = vec2<double>;
    using dvec3 = vec3<double>;
    using dvec4 = vec4<double>;
    using ivec2 = vec2<int>;
    using ivec3 = vec3<int>;
    using ivec4 = vec4<int>;
    using uvec2 = vec2<uint>;
    using uvec3 = vec3<uint>;
    using uvec4 = vec4<uint>;
    using bvec2 = vec2<uchar>;
    using bvec3 = vec3<uchar>;
    using bvec4 = vec4<uchar>;

    struct color3;
    struct color3f;
    struct colorf;
    struct color;

    template <class> struct is_color_t : std::false_type {};
    template <> struct is_color_t<color3>  : std::true_type {};
    template <> struct is_color_t<color3f> : std::true_type {};
    template <> struct is_color_t<color>   : std::true_type {};
    template <> struct is_color_t<colorf>  : std::true_type {};

    template <class T>
    constexpr bool is_color_v = is_color_t<T>::value;
    
    template <class T>
    concept color_t = is_color_v<T>;
    
#pragma endregion // concepts and decls
#pragma region Vector Swizzle Impl
#define VECTOR_SWIZZLING 1

#if VECTOR_SWIZZLING == 1
    // a bit of template magic going off over here
    template <uchar N> using vector_swizzle_data = const char[N+1];
    template <uchar D, vector_swizzle_data<D> T>
    struct swizzle { // N is the dimension of conversion, D is the dimension of the vector itself, and T is the member mapping
        uchar packed = 0;
        uchar N = 0;

        static constexpr uchar into_id(char expr) {
            for (uchar i = 0; i < D; ++i) if (expr == T[i]) return i;
            return -1;
        }

        template <uchar n>
        constexpr swizzle(const char (&args)[n]) {
            packed = into_id(args[0]);
            N = n - 1;
            for (uchar i = 1; i < N; ++i) packed |= into_id(args[i]) << (2 * i);
        }

        constexpr uchar operator[](uchar i) const {
            return 3 & (packed >> (i * 2));
        }
    };

    // swizzles a vecD<T> into a vec{S.N}<T> with reflection data V and of swizzle S 
    template <class T, uchar D, vector_swizzle_data<D> V, swizzle<D, V> S>
    // ReSharper disable once CppNotAllPathsReturnValue
    typename vecn<S.N, T>::type swizzle_impl(T* v) {
        if constexpr (S.N == 2) return { v[S[0]], v[S[1]] };
        if constexpr (S.N == 3) return { v[S[0]], v[S[1]], v[S[2]] };
        if constexpr (S.N == 4) return { v[S[0]], v[S[1]], v[S[2]], v[S[3]] };
        if constexpr (S.N < 2 || S.N > 4) return {};
    }
#endif
#pragma endregion // vector swizzle impl

#pragma region Templated Vectors
#define R(T) const T&
#define SCALAR_T(V) typename V::scalar
    
#define S_ONLY(T) template <class S = scalar> std::enable_if_t<std::is_signed_v<S>, T>
    
#define VEC2DEF(NAME, SCALAR, _X, _Y, DEF_OP, DEF_EQ, CEXPR, ...) \
    struct NAME { \
        using scalar = SCALAR; \
        static constexpr int dimension = 2; \
        \
        SCALAR _X, _Y; \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR s = 0) : _X(s), _Y(s) {} \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR _X, SCALAR _Y) : _X(_X), _Y(_Y) {} \
        \
        STDU_IF(DEF_OP, \
        STDU_IF(CEXPR, constexpr) SCALAR operator[] (uint i) const { return ((const SCALAR*)this)[i]; } /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator+(R(NAME) v) const; NAME operator+(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator+=(R(NAME) v); NAME& operator+=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator-(R(NAME) v) const; NAME operator-(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator-=(R(NAME) v); NAME& operator-=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator*(R(NAME) v) const; NAME operator*(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator*=(R(NAME) v); NAME& operator*=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator/(R(NAME) v) const; NAME operator/(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator/=(R(NAME) v); NAME& operator/=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator%(R(NAME) v) const; NAME operator%(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator%=(R(NAME) v); NAME& operator%=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        STDU_IF(DEF_EQ, \
        NAME operator+() const { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
        S_ONLY(NAME) operator-() const { return { -_X, -_Y }; } /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        \
        STDU_IF(CEXPR, constexpr) bool operator==(const NAME& other) const { return _X == other._X && _Y == other._Y; } /* NOLINT(bugprone-macro-parentheses) */ \
        STDU_IF(CEXPR, constexpr) bool operator!=(const NAME& other) const { return !(*this == other); } \
        \
        STDU_IF(VECTOR_SWIZZLING, \
        static constexpr vector_swizzle_data<2> params = #_X#_Y; \
        template <swizzle<2, params> S> \
        typename vecn<S.N, SCALAR>::type get() { return swizzle_impl<SCALAR, 2, params, S>((SCALAR*)this); } \
        ) \
        \
        SCALAR* begin() { return &_X; } \
        const SCALAR* begin() const { return &_X; } \
        SCALAR* end() { return begin() + 2; } \
        const SCALAR* end() const { return begin() + 2; } \
        \
        std::string str() const { return std::format("("#_X": {}, "#_Y": {})", _X, _Y); } \
        \
        __VA_ARGS__ /* extra declarations */ \
    }

#define VEC3DEF(NAME, SCALAR, _X, _Y, _Z, DEF_OP, DEF_EQ, CEXPR, ...) \
    struct NAME { \
        using scalar = SCALAR; \
        static constexpr int dimension = 3; \
        \
        SCALAR _X, _Y, _Z; \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR s = 0) : _X(s), _Y(s), _Z(s) {} \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR _X, SCALAR _Y, SCALAR _Z) : _X(_X), _Y(_Y), _Z(_Z) {} \
        \
        STDU_IF(CEXPR, constexpr) SCALAR operator[] (uint i) const { return ((const SCALAR*)this)[i]; } /* NOLINT(bugprone-macro-parentheses) */ \
        \
        STDU_IF(DEF_OP, \
        NAME operator+(R(NAME) v) const; NAME operator+(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator+=(R(NAME) v); NAME& operator+=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator-(R(NAME) v) const; NAME operator-(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator-=(R(NAME) v); NAME& operator-=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator*(R(NAME) v) const; NAME operator*(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator*=(R(NAME) v); NAME& operator*=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator/(R(NAME) v) const; NAME operator/(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator/=(R(NAME) v); NAME& operator/=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator%(R(NAME) v) const; NAME operator%(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator%=(R(NAME) v); NAME& operator%=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator+() const { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
        S_ONLY(NAME) operator-() const { return { -_X, -_Y, -_Z }; } /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        \
        STDU_IF(DEF_EQ, \
        STDU_IF(CEXPR, constexpr) bool operator==(const NAME& other) const { return _X == other._X && _Y == other._Y && _Z == other._Z; } /* NOLINT(bugprone-macro-parentheses) */ \
        STDU_IF(CEXPR, constexpr) bool operator!=(const NAME& other) const { return !(*this == other); } \
        ) \
        STDU_IF(VECTOR_SWIZZLING, \
        static constexpr vector_swizzle_data<3> params = #_X#_Y#_Z; \
        template <swizzle<3, params> S> \
        STDU_IF(CEXPR, constexpr) typename vecn<S.N, SCALAR>::type get() { return swizzle_impl<SCALAR, 3, params, S>((SCALAR*)this); } \
        ) \
        \
        SCALAR* begin() { return &_X; } \
        const SCALAR* begin() const { return &_X; } \
        SCALAR* end() { return begin() + 3; } \
        const SCALAR* end() const { return begin() + 3; } \
        \
        std::string str() const { return std::format("("#_X": {}, "#_Y": {}, "#_Z": {})", _X, _Y, _Z); } \
        \
        __VA_ARGS__ /* extra declarations */ \
    }

#define VEC4DEF(NAME, SCALAR, _X, _Y, _Z, _W, DEF_OP, DEF_EQ, DEF_W_VAL, CEXPR, ...) \
    struct NAME { \
        using scalar = SCALAR; \
        static constexpr int dimension = 4; \
        \
        SCALAR _X, _Y, _Z, _W; \
        STDU_IF(CEXPR, constexpr) NAME () : _X(0), _Y(0), _Z(0), _W(0) {} \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR s, SCALAR _W = DEF_W_VAL) : _X(s), _Y(s), _Z(s), _W(_W) {} \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR _X, SCALAR _Y, SCALAR _Z, SCALAR _W = DEF_W_VAL) : _X(_X), _Y(_Y), _Z(_Z), _W(_W) {} \
        \
        STDU_IF(CEXPR, constexpr) SCALAR operator[] (uint i) const { return ((const SCALAR*)this)[i]; } /* NOLINT(bugprone-macro-parentheses) */ \
        \
        STDU_IF(DEF_OP, \
        NAME operator+(R(NAME) v) const; NAME operator+(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator+=(R(NAME) v); NAME& operator+=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator-(R(NAME) v) const; NAME operator-(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator-=(R(NAME) v); NAME& operator-=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator*(R(NAME) v) const; NAME operator*(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator*=(R(NAME) v); NAME& operator*=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator/(R(NAME) v) const; NAME operator/(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */ \
        NAME& operator/=(R(NAME) v); NAME& operator/=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */ \
        \
        NAME operator%(R(NAME) v) const; NAME operator%(SCALAR v) const; /* NOLINT(bugprone-macro-parentheses) */\
        NAME& operator%=(R(NAME) v); NAME& operator%=(SCALAR v); /* NOLINT(bugprone-macro-parentheses) */\
        \
        NAME operator+() const { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
        S_ONLY(NAME) operator-() const { return { -_X, -_Y, -_Z, -_W }; } /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        \
        STDU_IF(DEF_EQ, \
        STDU_IF(CEXPR, constexpr) bool operator==(const NAME& other) const { return _X == other._X && _Y == other._Y && _Z == other._Z && _W == other._W; } /* NOLINT(bugprone-macro-parentheses) */ \
        STDU_IF(CEXPR, constexpr) bool operator!=(const NAME& other) const { return !(*this == other); } \
        ) \
        STDU_IF(VECTOR_SWIZZLING, \
        static constexpr vector_swizzle_data<4> params = #_X#_Y#_Z#_W; \
        template <swizzle<4, params> S> \
        STDU_IF(CEXPR, constexpr) typename vecn<S.N, SCALAR>::type get() { return swizzle_impl<SCALAR, 4, params, S>((SCALAR*)this); } \
        ) \
        \
        SCALAR* begin() { return &_X; } \
        const SCALAR* begin() const { return &_X; } \
        SCALAR* end() { return begin() + 4; } \
        const SCALAR* end() const { return begin() + 4; } \
        \
        std::string str() const { return std::format("("#_X": {}, "#_Y": {}, "#_Z": {}, "#_W": {})", _X, _Y, _Z, _W); } \
        \
        __VA_ARGS__ /* extra declarations */ \
    }

#define B_ONLY(T) template <class S = scalar> std::enable_if_t<std::is_same_v<S, uchar>, T>
#define S_ONLY_U(U) std::conditional_t<std::is_signed_v<scalar>, U, stdu::empty>
#define F_ONLY(T) template <class S = scalar> std::enable_if_t<std::is_floating_point_v<S>, T>
#define COMMON_VEC_MATH(T) \
    float len() const; \
    scalar lensq() const; \
    float dist(R(T) to) const; \
    scalar distsq(R(T) to) const; \
    bool in_range(R(T) other, scalar d) const; \
    \
    scalar sum() const; \
    \
    scalar dot(R(T) other) const; \
    \
    F_ONLY(T<float>) norm(float d = 1) const; /* NOLINT(bugprone-macro-parentheses) */ \
    \
    F_ONLY(T) lerp(R(T) other, float t) const; \
    F_ONLY(T&) lerp_to(R(T) other, float t); /* NOLINT(bugprone-macro-parentheses) */ \
    F_ONLY(T) towards(R(T) other, float max_d) const; \
    F_ONLY(T&) move_towards(R(T) other, float max_d); /* NOLINT(bugprone-macro-parentheses) */ \
    \
    F_ONLY(float) angle(R(T) other) const; \
    \
    F_ONLY(T) clamped() const; \
    static T max(R(T) a, R(T) b); \
    static T min(R(T) a, R(T) b); \
    static T clamp(R(T) rmin, R(T) rmax, R(T) x);
#pragma endregion // templated vectors
    
#pragma region Vec2
    template <class T>
    VEC2DEF(vec2, T, x, y, 1, 1, 1, \
        vec2(Direction2D dir, T scale = 1); \
        vec2(Corner2D dir,    T scale = 1); \
        \
        template <vec_t V> requires (V::dimension == dimension) \
        operator V() const { return { (SCALAR_T(V))x, (SCALAR_T(V))y }; } \
        template <class U> operator vec3<U>() const { return { (U)x, (U)y, 0 }; } \
        template <class U> operator vec4<U>() const { return { (U)x, (U)y, 0, 0 }; } \
        template <class U> vec2<U> as() const { return vec2<U>(*this); } \
        \
        static constexpr vec2           RIGHT() { return {  1,  0 }; } /* constexpr vars dont work with templates */ \
        static constexpr S_ONLY_U(vec2) LEFT()  { return { -1,  0 }; } \
        static constexpr vec2           UP()    { return {  0,  1 }; } \
        static constexpr S_ONLY_U(vec2) DOWN()  { return {  0, -1 }; } \
        static constexpr vec2           ZERO()  { return {  0,  0 }; } \
        static constexpr vec2           ONE()   { return {  1,  1 }; } \
        \
        COMMON_VEC_MATH(vec2) \
        \
        auto slope() const; \
        \
        F_ONLY(float) angle_signed(const vec2& other) const; \
        float angle() const; \
        fvec2 polar() const; \
        F_ONLY(fvec2) cartesian() const; \
        \
        S_ONLY(vec2 ) perpend() const; \
        \
        F_ONLY(vec2 ) rotated(float angle) const; \
        F_ONLY(vec2&) rotate(float angle); \
        F_ONLY(vec2 ) rotated(float angle, const vec2& origin) const; \
        F_ONLY(vec2&) rotate(float angle, const vec2& origin); \
        \
        F_ONLY(vec2 ) reflected(const vec2& normal) const; \
        F_ONLY(vec2&) reflect(const vec2& normal); \
        F_ONLY(vec2 ) reflected_uc(const vec2& normal) const; \
        F_ONLY(vec2&) reflect_uc(const vec2& normal); \
        \
        vec3<T> with_z(T z) const; \
    );
#pragma endregion // vec2
#pragma region Vec3
    template <class T>
    VEC3DEF(vec3, T, x, y, z, 1, 1, 1,
        vec3(Direction3D dir, T scale = 1); \
        vec3(Corner3D dir,    T scale = 1); \
        \
        template <vec_t V> requires (V::dimension == dimension) \
        operator V() const { return { (SCALAR_T(V))x, (SCALAR_T(V))y, (SCALAR_T(V))z }; } \
        template <class U> operator vec2<U>() const { return { (U)x, (U)y }; } \
        template <class U> operator vec4<U>() const { return { (U)x, (U)y, (U)z, 0 }; } \
        template <class U> vec3<U> as() const { return vec3<U>(*this); } \
        \
        static constexpr vec3           RIGHT() { return {  1,  0,  0 }; }
        static constexpr S_ONLY_U(vec3) LEFT()  { return { -1,  0,  0 }; }
        static constexpr vec3           UP()    { return {  0,  1,  0 }; }
        static constexpr S_ONLY_U(vec3) DOWN()  { return {  0, -1,  0 }; }
        static constexpr vec3           FRONT() { return {  0,  0,  1 }; }
        static constexpr S_ONLY_U(vec3) BACK()  { return {  0,  0, -1 }; }
        static constexpr vec3           ZERO()  { return {  0,  0,  0 }; }
        static constexpr vec3           ONE()   { return {  1,  1,  1 }; }
        \
        COMMON_VEC_MATH(vec3) \
        vec2<T> xy() const; \
        \
        vec3 cross(const vec3& other) const; \
        \
        F_ONLY(float) angle_signed(const vec3& other, const vec3& normal) const; /* normal should be normalized */ \
        \
        F_ONLY(float) altitude() const; \
        F_ONLY(float) azimuth() const; \
        F_ONLY(fvec3) spheric() const; /* 3d polar coordinates, (x,y,z) into (r,theta,phi) */ \
        F_ONLY(fvec3) cartesian() const; /* converts (r,theta,phi) into (x,y,z) */ \
        \
        F_ONLY(vec2<T>) projected() const;
        F_ONLY(vec3 ) reflected(const vec3& normal) const; \
        F_ONLY(vec3&) reflect(const vec3& normal); \
        F_ONLY(vec3 ) reflected_uc(const vec3& normal) const; /* this assumes normal is normalized */ \
        F_ONLY(vec3&) reflect_uc(const vec3& normal); /* this assumes normal is normalized */ \
        \
        B_ONLY(color) color(uchar a) const; \
        F_ONLY(colorf) color(float a) const; \
        B_ONLY(color3) color() const; \
        F_ONLY(color3f) color() const; \
        \
        vec4<T> with_w(T w = 1) const; \
    );
#pragma endregion // vec3
#pragma region Vec4
    template <class T>
    VEC4DEF(vec4, T, x, y, z, w, 1, 1, 1, 1,
        vec4(Direction4D dir, T scale = 1); \
        vec4(Corner4D    cor, T scale = 1); \
        \
        template <vec_t V> requires (V::dimension == dimension) \
        operator V() const { return { (SCALAR_T(V))x, (SCALAR_T(V))y, (SCALAR_T(V))z, (SCALAR_T(V))w }; } \
        template <class U> operator vec2<U>() const { return { (U)x, (U)y }; } \
        template <class U> operator vec3<U>() const { return { (U)x, (U)y, (U)z }; } \
        template <class U> vec4<U> as() const { return vec4<U>(*this); } \
        \
        static constexpr vec4           RIGHT() { return {  1,  0,  0,  0 }; }
        static constexpr S_ONLY_U(vec4) LEFT()  { return { -1,  0,  0,  0 }; }
        static constexpr vec4           UP()    { return {  0,  1,  0,  0 }; }
        static constexpr S_ONLY_U(vec4) DOWN()  { return {  0, -1,  0,  0 }; }
        static constexpr vec4           FRONT() { return {  0,  0,  1,  0 }; }
        static constexpr S_ONLY_U(vec4) BACK()  { return {  0,  0, -1,  0 }; }
        static constexpr vec4           IN()    { return {  0,  0,  0,  1 }; }
        static constexpr S_ONLY_U(vec4) OUT()   { return {  0,  0,  0, -1 }; }
        static constexpr vec4           ZERO()  { return {  0,  0,  0,  0 }; }
        static constexpr vec4           ONE()   { return {  1,  1,  1,  1 }; }
        \
        COMMON_VEC_MATH(vec4) \
        vec3<T> xyz() const; \
        vec2<T> xy() const; \
        \
        F_ONLY(vec3<T>) projected() const; \
        \
        B_ONLY(color) color() const; \
        F_ONLY(colorf) color() const; \
    );
#pragma endregion // vec4

#pragma region Color
#define COLOR_COMMON(T, HAS_A, C) \
    constexpr T(const char (&hex)[1+6+1]); \
    STDU_IF(HAS_A, constexpr T(const char (&hex)[1+8+1]);)\
    OPENGL_API float luminance() const; \
    \
    OPENGL_API bvec3 as_rgb()   const; \
    OPENGL_API bvec4 as_rgba()  const; \
    OPENGL_API fvec3 as_rgbf()  const; \
    OPENGL_API fvec4 as_rgbaf() const; \
    OPENGL_API fvec3 as_hsl()   const; \
    OPENGL_API fvec4 as_hsla()  const; \
    OPENGL_API static T from_hsl(float hue, float saturation = 1, float lightness = 1 STDU_IF(HAS_A, , float alpha = 1)); /* hue: [0-360], sat: [0-1], L: [0-1] */ \
    OPENGL_API static T from_hsl(STDU_IF_ELSE(HAS_A, (fvec4 hsla), (fvec3 hsl))); /* hue: [0-360], sat: [0-1], L: [0-1] */ \
    OPENGL_API fvec3 as_hsv()  const; \
    OPENGL_API fvec4 as_hsva() const; \
    OPENGL_API static T from_hsv(float hue, float saturation = 1, float value = 1 STDU_IF(HAS_A, , float alpha = 1)); /* hue: [0-360], sat: [0-1], val: [0-1] */ \
    OPENGL_API static T from_hsv(STDU_IF_ELSE(HAS_A, (fvec4 hsva), (fvec3 hsv))); /* hue: [0-360], sat: [0-1], val: [0-1] */ \
    \
    OPENGL_API \
    STDU_IF_ELSE(HAS_A, (without_alpha_t rgb() const;), (with_alpha_t with_alpha(scalar alpha = 1) const;)) \
    \
    OPENGL_API \
    STDU_IF_ELSE(HAS_A, (operator without_alpha_t() const;), (operator with_alpha_t()    const;)) \
    \
    static constexpr T BLACK()      { return "#000000"; } /* solid black: rgb(000, 000, 000) or #000000 */ \
    static constexpr T DARK_GRAY()  { return "#404040"; } /*   25% white: rgb(064, 064, 064) or #404040 */ \
    static constexpr T GRAY()       { return "#808080"; } /*   50% white: rgb(128, 128, 128) or #808080 */ \
    static constexpr T LIGHT_GRAY() { return "#c0c0c0"; } /*   75% white: rgb(192, 192, 192) or #c0c0c0 */ \
    static constexpr T WHITE()      { return "#ffffff"; } /* solid white: rgb(255, 255, 255) or #ffffff */ \
    static constexpr T SILVER() { return LIGHT_GRAY(); }  /* same as light_gray, rgb(192, 192, 192) or #c0c0c0 */ \
    \
    static constexpr T RED()     { return "#ff0000"; } /* red    (100%   red):              rgb(255, 000, 000) or #ff0000 */ \
    static constexpr T ORANGE()  { return "#ff8000"; } /* orange (100%   red +  50% green): rgb(255, 128, 000) or #ff8000 */ \
    static constexpr T YELLOW()  { return "#ffff00"; } /* yellow (100%   red + 100% green): rgb(255, 255, 000) or #ffff00 */ \
    static constexpr T LIME()    { return "#80ff00"; } /* lime   (100% green +  50%   red): rgb(128, 255, 000) or #80ff00 */ \
    static constexpr T GREEN()   { return "#00ff00"; } /* green  (100% green):              rgb(000, 255, 000) or #00ff00 */ \
    static constexpr T SEAFOAM() { return "#00ff80"; } /* seafoam(100% green +  50%  blue): rgb(000, 255, 128) or #00ff80 */ \
    static constexpr T CYAN()    { return "#00ffff"; } /* cyan   (100% green + 100%  blue): rgb(000, 255, 255) or #00ffff */ \
    static constexpr T AZURE()   { return "#0080ff"; } /* azure  (100%  blue +  50% green): rgb(000, 128, 255) or #0080ff */ \
    static constexpr T BLUE()    { return "#0000ff"; } /* blue   (100%  blue):              rgb(000, 000, 255) or #0000ff */ \
    static constexpr T PURPLE()  { return "#8000ff"; } /* purple (100%  blue +  50%   red): rgb(128, 000, 255) or #8000ff */ \
    static constexpr T MAGENTA() { return "#ff00ff"; } /* magenta(100%  blue + 100%   red): rgb(255, 000, 255) or #ff00ff */ \
    static constexpr T ROSE()    { return "#ff0080"; } /* rose   (100%   red +  50%  blue): rgb(255, 000, 128) or #ff0080 */ \
    \
    static constexpr T LIGHT_RED()     { return "#ff8080"; } /* light_red    (100%   red +  50% green +  50%  blue): rgb(255, 128, 128) or #ff8080 */ \
    static constexpr T LIGHT_YELLOW()  { return "#ffff80"; } /* light_yellow (100%   red + 100% green +  50%  blue): rgb(255, 255, 128) or #ffff80 */ \
    static constexpr T LIGHT_GREEN()   { return "#80ff80"; } /* light_green  ( 50%   red + 100% green +  50%  blue): rgb(128, 255, 128) or #80ff80 */ \
    static constexpr T LIGHT_CYAN()    { return "#80ffff"; } /* light_cyan   ( 50%   red + 100% green + 100%  blue): rgb(128, 255, 255) or #80ffff */ \
    static constexpr T LIGHT_BLUE()    { return "#8080ff"; } /* light_blue   ( 50%   red +  50% green + 100%  blue): rgb(128, 128, 255) or #8080ff */ \
    static constexpr T LIGHT_MAGENTA() { return "#ff80ff"; } /* light_magenta(100%   red +  50% green + 100%  blue): rgb(255, 128, 255) or #ff80ff */ \
    static constexpr T SALMON()     { return LIGHT_RED();     } /* same as light_red,     rgb(255, 128, 128) or #ff8080 */ \
    static constexpr T LEMON()      { return LIGHT_YELLOW();  } /* same as light_yellow,  rgb(255, 255, 128) or #ffff80 */ \
    static constexpr T MINT()       { return LIGHT_GREEN();   } /* same as light_green,   rgb(128, 255, 128) or #80ff80 */ \
    static constexpr T SKY()        { return LIGHT_CYAN();    } /* same as light_cyan,    rgb(128, 255, 255) or #80ffff */ \
    static constexpr T CORNFLOWER() { return LIGHT_BLUE();    } /* same as light_blue,    rgb(128, 128, 255) or #8080ff */ \
    static constexpr T PINK()       { return LIGHT_MAGENTA(); } /* same as light_magenta, rgb(255, 128, 255) or #ff80ff */ \
    \
    static constexpr T DARK_RED()     { return "#800000"; } /* dark_red   ( 50%   red):              rgb(128, 000, 000) or #800000 */ \
    static constexpr T DARK_YELLOW()  { return "#808000"; } /* dark_yellow( 50%   red +  50% green): rgb(128, 128, 000) or #808000 */ \
    static constexpr T DARK_GREEN()   { return "#008000"; } /* dark_green ( 50% green):              rgb(000, 128, 000) or #008000 */ \
    static constexpr T DARK_CYAN()    { return "#008080"; } /* dark_cyan  ( 50% green +  50%  blue): rgb(000, 128, 128) or #008080 */ \
    static constexpr T DARK_BLUE()    { return "#000080"; } /* dark_blue  ( 50%  blue):              rgb(000, 000, 128) or #000080 */ \
    static constexpr T DARK_MAGENTA() { return "#800080"; } /* dark_purple( 50%  blue +  50%   red): rgb(128, 000, 128) or #800080 */ \
    static constexpr T MAROON()  { return DARK_RED();     } /* same as dark_red,    rgb(128, 000, 000) or #800000 */ \
    static constexpr T OLIVE()   { return DARK_YELLOW();  } /* same as dark_yellow, rgb(128, 128, 000) or #808000 */ \
    static constexpr T AVOCADO() { return DARK_GREEN();   } /* same as dark_green,  rgb(000, 128, 000) or #008000 */ \
    static constexpr T TEAL()    { return DARK_CYAN();    } /* same as dark_cyan,   rgb(000, 128, 128) or #008080 */ \
    static constexpr T NAVY()    { return DARK_BLUE();    } /* same as dark_blue,   rgb(000, 000, 128) or #000080 */ \
    static constexpr T VIOLET()  { return DARK_MAGENTA(); } /* same as dark_purple, rgb(128, 000, 128) or #800080 */ \
    \
    static constexpr T BETTER_RED()        { return "#ff3030"; } /* more appealing red:     rgb(255, 048, 048) or #ff3030 */ \
    static constexpr T BETTER_ORANGE()     { return "#ff802b"; } /* more appealing orange:  rgb(255, 128, 043) or #ff802b */ \
    static constexpr T BETTER_YELLOW()     { return "#ffc014"; } /* more appealing yellow:  rgb(255, 192, 020) or #ffc014 */ \
    static constexpr T BETTER_LIME()       { return "#87f725"; } /* more appealing lime:    rgb(135, 247, 037) or #87f725 */ \
    static constexpr T BETTER_GREEN()      { return "#0c8501"; } /* more appealing green:   rgb(012, 133, 001) or #0c8501 */ \
    static constexpr T BETTER_AQUA()       { return "#28cef7"; } /* more appealing aqua:    rgb(040, 206, 247) or #28cef7 */ \
    static constexpr T BETTER_CYAN()       { return "#17a0b3"; } /* more appealing cyan:    rgb(023, 160, 179) or #17a0b3 */ \
    static constexpr T BETTER_BLUE()       { return "#1041e3"; } /* more appealing blue:    rgb(016, 065, 227) or #1041e3 */ \
    static constexpr T BETTER_PURPLE()     { return "#7818ed"; } /* more appealing purple:  rgb(120, 024, 237) or #7818ed */ \
    static constexpr T BETTER_MAGENTA()    { return "#cb2aeb"; } /* more appealing magenta: rgb(203, 042, 235) or #cb2aeb */ \
    static constexpr T BETTER_PINK()       { return "#fa57c6"; } /* more appealing pink:    rgb(250, 087, 198) or #fa57c6 */ \
    static constexpr T BETTER_BROWN()      { return "#45200d"; } /* more appealing brown:   rgb(069, 032, 013) or #45200d */ \
    static constexpr T BETTER_BLACK()      { return "#16191d"; } /* more appealing black:   rgb(022, 025, 029) or #16191d */ \
    static constexpr T BETTER_DARK_GRAY()  { return "#4c4b5d"; } /* more appealing gray:    rgb(076, 075, 093) or #4c4b5d */ \
    static constexpr T BETTER_GRAY()       { return "#747e86"; } /* more appealing gray:    rgb(116, 126, 134) or #747e86 */ \
    static constexpr T BETTER_LIGHT_GRAY() { return "#afbac1"; } /* more appealing gray:    rgb(175, 186, 193) or #afbac1 */ \
    static constexpr T BETTER_WHITE()      { return "#e8f7f9"; } /* more appealing white:   rgb(232, 247, 249) or #e8f7f9 */ \
    \
    STDU_IF(HAS_A, static constexpr T CLEAR() { return "#00000000"; } /* complete transparency. RGBA is (0, 0, 0, 0). */) \
    
    VEC3DEF(color3f, float, r, g, b,    0, 0,      1, OPENGL_API operator color3 () const; using with_alpha_t    = colorf;  COLOR_COMMON(color3f, 0, 255.0f));
    VEC3DEF(color3,  uchar, r, g, b,    0, 1,      1, OPENGL_API operator color3f() const; using with_alpha_t    = color;   COLOR_COMMON(color3,  0, 1));
    VEC4DEF(colorf,  float, r, g, b, a, 0, 0, 1,   1, OPENGL_API operator color  () const; using without_alpha_t = color3f; COLOR_COMMON(colorf,  1, 255.0f));
    VEC4DEF(color,   uchar, r, g, b, a, 0, 1, 255, 1, OPENGL_API operator colorf () const; using without_alpha_t = color3;  COLOR_COMMON(color,   1, 1));
#pragma endregion // color
    
#undef COLOR_COMMON
#undef VEC2DEF
#undef VEC3DEF
#undef VEC4DEF
#undef SCALAR_T
#undef F_ONLY
#undef B_ONLY
#undef S_ONLY
#undef S_ONLY_U
#undef COMMON_VEC_MATH
#undef R
#pragma endregion // declaration

#pragma region Implementation
#define F_ONLY(...) template <class S> std::enable_if_t<std::is_floating_point_v<S>, __VA_ARGS__>
#define S_ONLY(...) template <class S> std::enable_if_t<std::is_signed_v<S>, __VA_ARGS__>
#define S_ONLY_U(...) std::conditional_t<std::is_signed_v<T>, __VA_ARGS__, stdu::empty>
#define B_ONLY(...) template <class S> std::enable_if_t<std::is_same_v<S, uchar>, __VA_ARGS__>
#define GENERIC_T template <class T>
#define VEC2_IMPL(R, N, FF) GENERIC_T STDU_IF_ELSE(FF, (F_ONLY(R)), (R)) vec2<T>::N
#pragma region Vec2
    VEC2_IMPL(float,    len,      0)() const { return std::sqrtf((float)lensq()); }
    VEC2_IMPL(T,        lensq,    0)() const { return x*x + y*y; }
    VEC2_IMPL(float,    dist,     0)(const vec2& to) const { return (*this - to).len(); }
    VEC2_IMPL(T,        distsq,   0)(const vec2& to) const { return (*this - to).lensq(); }
    VEC2_IMPL(bool,     in_range, 0)(const vec2& other, T d) const { return distsq(other) <= d * d; }
    
    VEC2_IMPL(T,        sum, 0)()                  const { return x + y; }
    VEC2_IMPL(T,        dot, 0)(const vec2& other) const { return (*this * other).sum(); }
    
    VEC2_IMPL(fvec2,    norm, 1)(float d) const { float l = d / len(); return { x * l, y * l }; }
    
    VEC2_IMPL(vec2<T>,  lerp,         1)(const vec2& other, float t)     const { float r = 1 - t; return { x * r + t * other.x, y * r + t * other.y }; }
    VEC2_IMPL(vec2<T>&, lerp_to,      1)(const vec2& other, float t)           { *this = lerp(other, t); return *this; }
    VEC2_IMPL(vec2<T>,  towards,      1)(const vec2& other, float max_d) const { auto u = (other - *this).norm(); return *this +  u * max_d; }
    VEC2_IMPL(vec2<T>&, move_towards, 1)(const vec2& other, float max_d)       { auto u = (other - *this).norm();        *this += u * max_d; return *this; }
    
    VEC2_IMPL(vec2<T>,  clamped, 1)() const { return clamp(ZERO(), ONE(), *this); }
    VEC2_IMPL(vec2<T>,  max    , 0)(const vec2& a, const vec2& b) { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }
    VEC2_IMPL(vec2<T>,  min    , 0)(const vec2& a, const vec2& b) { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }
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

#define IMPL_VEC2_OP(OP) VEC2_IMPL(vec2<T>,  operator OP,    0)(const vec2& other) const { return { (T)(x OP other.x), (T)(y OP other.y) }; } /* NOLINT(bugprone-macro-parentheses) */ \
                         VEC2_IMPL(vec2<T>,  operator OP,    0)(T other)           const { return { (T)(x OP other),   (T)(y OP other)   }; } /* NOLINT(bugprone-macro-parentheses) */ \
                         VEC2_IMPL(vec2<T>&, operator OP##=, 0)(const vec2& other) { (T)(x OP##= other.x); (T)(y OP##= other.y); return *this; } \
                         VEC2_IMPL(vec2<T>&, operator OP##=, 0)(T other)           { (T)(x OP##= other);   (T)(y OP##= other);   return *this; }

    IMPL_VEC2_OP(+)
    IMPL_VEC2_OP(-)
    IMPL_VEC2_OP(*)
    IMPL_VEC2_OP(/)
    
    VEC2_IMPL(vec2<T>,  operator%,  0)(const vec2& v) const { return { mod_t(x, v.x), mod_t(y, v.y) }; }
    VEC2_IMPL(vec2<T>,  operator%,  0)(T v)           const { return { mod_t(x, v),   mod_t(y, v)   }; }
    VEC2_IMPL(vec2<T>&, operator%=, 0)(const vec2& v) { x = mod_t(x, v.x); y = mod_t(y, v.y); return *this; }
    VEC2_IMPL(vec2<T>&, operator%=, 0)(T v)           { x = mod_t(x, v);   y = mod_t(y, v);   return *this; }

#undef IMPL_VEC2_OP

    VEC2_IMPL(, vec2, 0)(Direction2D dir, T scale) : x(0), y(0) {
        using enum Direction2D;
        if (dir == UNIT) {
            x = y = scale; return;
        }
        if (dir < 0 || dir > DOWN) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? neg_t(scale) : scale;
    }

    VEC2_IMPL(, vec2, 0)(Corner2D cor, T scale) : x(0), y(0) {
        using enum Corner2D;
        if (cor < 0 || cor > BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? neg_t(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? neg_t(scale) : scale;
    }
    
#undef VEC2_IMPL
#pragma endregion // vec2
#pragma region Vec3
#define VEC3_IMPL(R, N, FF) GENERIC_T STDU_IF_ELSE(FF, (F_ONLY(R)), (R)) vec3<T>::N
    VEC3_IMPL(float,    len,      0)() const { return std::sqrtf((float)lensq()); }
    VEC3_IMPL(T,        lensq,    0)() const { return x*x + y*y + z*z; }
    VEC3_IMPL(float,    dist,     0)(const vec3& to) const { return (*this - to).len(); }
    VEC3_IMPL(T,        distsq,   0)(const vec3& to) const { return (*this - to).lensq(); }
    VEC3_IMPL(bool,     in_range, 0)(const vec3& other, T d) const { return distsq(other) <= d * d; }
    
    VEC3_IMPL(T,        sum, 0)()                  const { return x + y + z; }
    VEC3_IMPL(T,        dot, 0)(const vec3& other) const { return (*this * other).sum(); }
    
    VEC3_IMPL(fvec3,    norm, 1)(float d) const { float l = d / len(); return { x * l, y * l, z * l }; }
    
    VEC3_IMPL(vec3<T>,  lerp,         1)(const vec3& other, float t)     const { float r = 1 - t; return { x * r + t * other.x, y * r + t * other.y, z * r + t * other.z }; }
    VEC3_IMPL(vec3<T>&, lerp_to,      1)(const vec3& other, float t)           { *this = lerp(other, t); return *this; }
    VEC3_IMPL(vec3<T>,  towards,      1)(const vec3& other, float max_d) const { auto u = (other - *this).norm(max_d); return *this + u; }
    VEC3_IMPL(vec3<T>&, move_towards, 1)(const vec3& other, float max_d)       { auto u = (other - *this).norm(max_d); *this += u; return *this; }
    
    VEC3_IMPL(vec3<T>,  clamped, 1)() const { return clamp(ZERO(), ONE(), *this); }
    VEC3_IMPL(vec3<T>,  max,     0)(const vec3& a, const vec3& b) { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }
    VEC3_IMPL(vec3<T>,  min,     0)(const vec3& a, const vec3& b) { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }
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
     
#define IMPL_VEC3_OP(OP) VEC3_IMPL(vec3<T>,  operator OP,    0)(const vec3& other) const { return { (T)(x OP other.x), (T)(y OP other.y), (T)(z OP other.z) }; } /* NOLINT(bugprone-macro-parentheses) */ \
                         VEC3_IMPL(vec3<T>,  operator OP,    0)(T other)           const { return { (T)(x OP other),   (T)(y OP other),   (T)(z OP other)   }; } /* NOLINT(bugprone-macro-parentheses) */ \
                         VEC3_IMPL(vec3<T>&, operator OP##=, 0)(const vec3& other) { (T)(x OP##= other.x); (T)(y OP##= other.y); (T)(z OP##= other.z); return *this; } \
                         VEC3_IMPL(vec3<T>&, operator OP##=, 0)(T other)           { (T)(x OP##= other);   (T)(y OP##= other);   (T)(z OP##= other);   return *this; }

    IMPL_VEC3_OP(+)
    IMPL_VEC3_OP(-)
    IMPL_VEC3_OP(*)
    IMPL_VEC3_OP(/)

    VEC3_IMPL(vec3<T>,  operator%,  0)(const vec3& v) const { return { mod_t(x, v.x), mod_t(y, v.y), mod_t(z, v.z) }; }
    VEC3_IMPL(vec3<T>,  operator%,  0)(T v)           const { return { mod_t(x, v),   mod_t(y, v),   mod_t(z, v)   }; }
    VEC3_IMPL(vec3<T>&, operator%=, 0)(const vec3& v) { x = mod_t(x, v.x); y = mod_t(y, v.y); z = mod_t(z, v.z); return *this; }
    VEC3_IMPL(vec3<T>&, operator%=, 0)(T v)           { x = mod_t(x, v);   y = mod_t(y, v);   z = mod_t(z, v);   return *this; }

#undef IMPL_VEC3_OP

    VEC3_IMPL(, vec3, 0)(Direction3D dir, T scale) : x(0), y(0), z(0) {
        using enum Direction3D;
        if (dir == UNIT) {
            x = y = z = scale; return;
        }
        if (dir < 0 || dir > BACK) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? neg_t(scale) : scale;
    }

    VEC3_IMPL(, vec3, 0)(Corner3D cor, T scale) : x(0), y(0), z(0) {
        using enum Corner3D;
        if (cor < 0 || cor > BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? neg_t(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? neg_t(scale) : scale;
        y = (int)(cor & SIDE_BACK  ) ? neg_t(scale) : scale;
    }
    
#undef VEC3_IMPL
#pragma endregion // vec3
#pragma region Vec4
#define VEC4_IMPL(R, N, FF) GENERIC_T STDU_IF_ELSE(FF, (F_ONLY(R)), (R)) vec4<T>::N
    VEC4_IMPL(float,    len,      0)() const { return std::sqrtf((float)lensq()); }
    VEC4_IMPL(T,        lensq,    0)() const { return x*x + y*y + z*z + w*w; }
    VEC4_IMPL(float,    dist,     0)(const vec4& to) const { return (*this - to).len(); }
    VEC4_IMPL(T,        distsq,   0)(const vec4& to) const { return (*this - to).lensq(); }
    VEC4_IMPL(bool,     in_range, 0)(const vec4& other, T d) const { return distsq(other) <= d * d; }
    
    VEC4_IMPL(T,        sum, 0)()                  const { return x + y + z + w; }
    VEC4_IMPL(T,        dot, 0)(const vec4& other) const { return (*this * other).sum(); }
    
    VEC4_IMPL(fvec4,    norm, 1)(float d) const { float l = d / len(); return { x * l, y * l, z * l, w * l }; }
    
    VEC4_IMPL(vec4<T>,  lerp,         1)(const vec4& other, float t)     const { float r = 1 - t; return { x*r + t*other.x, y*r + t*other.y, z*r + t*other.z, w*r + t*other.w }; }
    VEC4_IMPL(vec4<T>&, lerp_to,      1)(const vec4& other, float t)           { *this = lerp(other, t); return *this; }
    VEC4_IMPL(vec4<T>,  towards,      1)(const vec4& other, float max_d) const { auto u = (other - *this).norm(max_d); return *this + u; }
    VEC4_IMPL(vec4<T>&, move_towards, 1)(const vec4& other, float max_d)       { auto u = (other - *this).norm(max_d); *this += u; return *this; }

    VEC4_IMPL(vec3<T>,  projected, 1)() const { return xyz() / w; }
    
    VEC4_IMPL(float,    angle, 1)(const vec4& other) const { return std::acos(dot(other) / (len() * other.len())); }
    
    VEC4_IMPL(vec4<T>,  clamped, 1)() const { return clamp(ZERO(), ONE(), *this); }
    VEC4_IMPL(vec4<T>,  max,     0)(const vec4& a, const vec4& b) { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) }; }
    VEC4_IMPL(vec4<T>,  min,     0)(const vec4& a, const vec4& b) { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) }; }
    VEC4_IMPL(vec4<T>,  clamp,   0)(const vec4& rmin, const vec4& rmax, const vec4& x) { return min(rmax, max(rmin, x)); }
    
    VEC4_IMPL(F_ONLY(colorf), color,   0)() const { return { x, y, z }; }
    VEC4_IMPL(B_ONLY(color),  color,   0)() const { return { x, y, z }; }
    
    VEC4_IMPL(vec2<T>, xy,  0)() const { return { x, y }; }
    VEC4_IMPL(vec3<T>, xyz, 0)() const { return { x, y, z }; }

#define IMPL_VEC4_OP(OP) VEC4_IMPL(vec4<T>,  operator OP,    0)(const vec4& other) const { return { (T)(x OP other.x), (T)(y OP other.y), (T)(z OP other.z), (T)(w OP other.w) }; } /* NOLINT(bugprone-macro-parentheses) */ \
                         VEC4_IMPL(vec4<T>,  operator OP,    0)(T other)           const { return { (T)(x OP other),   (T)(y OP other),   (T)(z OP other),   (T)(w OP other)   }; } /* NOLINT(bugprone-macro-parentheses) */ \
                         VEC4_IMPL(vec4<T>&, operator OP##=, 0)(const vec4& other) { (T)(x OP##= other.x); (T)(y OP##= other.y); (T)(z OP##= other.z); (T)(w OP##= other.w); return *this; } \
                         VEC4_IMPL(vec4<T>&, operator OP##=, 0)(T other)           { (T)(x OP##= other);   (T)(y OP##= other);   (T)(z OP##= other);   (T)(w OP##= other);   return *this; }

    IMPL_VEC4_OP(+)
    IMPL_VEC4_OP(-)
    IMPL_VEC4_OP(*)
    IMPL_VEC4_OP(/)

    VEC4_IMPL(vec4<T>,  operator%,  0)(const vec4& v) const { return { mod_t(x, v.x), mod_t(y, v.y), mod_t(z, v.z), mod_t(w, v.w) }; }
    VEC4_IMPL(vec4<T>,  operator%,  0)(T v)           const { return { mod_t(x, v),   mod_t(y, v),   mod_t(z, v),   mod_t(w, v)   }; }
    VEC4_IMPL(vec4<T>&, operator%=, 0)(const vec4& v) { x = mod_t(x, v.x); y = mod_t(y, v.y); z = mod_t(z, v.z); w = mod_t(w, v.w); return *this; }
    VEC4_IMPL(vec4<T>&, operator%=, 0)(T v)           { x = mod_t(x, v);   y = mod_t(y, v);   z = mod_t(z, v);   w = mod_t(w, v);   return *this; }

#undef IMPL_VEC4_OP

    VEC4_IMPL(, vec4, 0)(Direction4D dir, T scale) : x(0), y(0), z(0), w(0) {
        using enum Direction4D;
        if (dir == UNIT) {
            x = y = z = w = scale; return;
        }
        if (dir < 0 || dir > OUT) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? neg_t(scale) : scale;
    }

    VEC4_IMPL(, vec4, 0)(Corner4D cor, T scale) : x(0), y(0), z(0), w(0) {
        using enum Corner4D;
        if (cor < 0 || cor > OUTER_BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? neg_t(scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? neg_t(scale) : scale;
        z = (int)(cor & SIDE_BACK  ) ? neg_t(scale) : scale;
        w = (int)(cor & SIDE_OUTER ) ? neg_t(scale) : scale;
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

    template struct vec2<float>;
    template struct vec3<float>;
    template struct vec4<float>;
    template struct vec2<double>;
    template struct vec3<double>;
    template struct vec4<double>;
    template struct vec2<int>;
    template struct vec3<int>;
    template struct vec4<int>;
    template struct vec2<uint>;
    template struct vec3<uint>;
    template struct vec4<uint>;
    template struct vec2<uchar>;
    template struct vec3<uchar>;
    template struct vec4<uchar>;

#undef F_ONLY
#undef B_ONLY
#undef S_ONLY
#undef S_ONLY_U
}
