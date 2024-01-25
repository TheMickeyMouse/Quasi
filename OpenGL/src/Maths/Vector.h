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

    template <vec_t V> struct _rect_origin_inbetween_;
    template <vec_t V> struct _rect_size_inbetween_;
    template <int, class> struct rect;
    
    template <class T> using rect2 = rect<2, T>;
    template <class T> using rect3 = rect<3, T>;
    template <class T> using rect4 = rect<4, T>;
    using rect2f = rect2<float>;
    using rect3f = rect3<float>;
    using rect4f = rect4<float>;
    using rect2d = rect2<double>;
    using rect3d = rect3<double>;
    using rect4d = rect4<double>;
    using rect2i = rect2<int>;
    using rect3i = rect3<int>;
    using rect4i = rect4<int>;
    using rect2u = rect2<uint>;
    using rect3u = rect3<uint>;
    using rect4u = rect4<uint>;
    using rect2b = rect2<uchar>;
    using rect3b = rect3<uchar>;
    using rect4b = rect4<uchar>;
    
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

#define ARITH(T, U) stdu::arithmetic_t<T, U>
#define ARITH_T(T, U, O) typename ARITH(T, U)::O##_t
#define ARITH_DO(U, M) ARITH(scalar, U)::M
    
#define VEC_OP(V, M) \
    template <class U> auto M(U v) const { \
        static_assert(std::is_arithmetic_v<U> || is_vec_v<U>, #V "::" #M " not supported"); /* NOLINT(bugprone-macro-parentheses) */ \
        if constexpr (std::is_arithmetic_v<U>) return apply([=](scalar x){ return ARITH_DO(U, M)(x, v); }); /* NOLINT(bugprone-macro-parentheses) */ \
        else if constexpr (is_vec_v<U>) return apply(ARITH_DO(SCALAR_T(U), M), v); /* NOLINT(bugprone-macro-parentheses) */ \
    }
    
#define VEC_OP_OTHER(V, M) \
    template <class U, class T> \
    requires (std::is_arithmetic_v<U> && requires (U u, V<T> t) { t M u; }) /* NOLINT(bugprone-macro-parentheses) */ \
    auto operator M(U u, const V<T>& t) { return t M u; }

#define VEC_CMP(V, M) \
    template <class U> bool M(U v) const { \
        static_assert(std::is_arithmetic_v<U> || is_vec_v<U>, #V "::" #M " not supported"); /* NOLINT(bugprone-macro-parentheses) */ \
        if constexpr (std::is_arithmetic_v<U>) return apply([=](scalar x){ return ARITH_DO(U, M)(x, v); }).all(); /* NOLINT(bugprone-macro-parentheses) */ \
        else if constexpr (is_vec_v<U>) return apply(ARITH_DO(SCALAR_T(U), M), v).all(); /* NOLINT(bugprone-macro-parentheses) */ \
    }
    
#define VEC_CMP_OTHER(V, M, N_M) \
    template <class U, class T> \
    requires (std::is_arithmetic_v<U> && requires (U u, V<T> t) { t M u; }) /* NOLINT(bugprone-macro-parentheses) */ \
    bool operator M(U u, const V<T>& t) { return t N_M u; }
    
#define VEC2DEF(NAME, SCALAR, _X, _Y, DEF_OP, DEF_CMP, CEXPR, ...) \
    struct NAME { \
        using scalar = SCALAR; \
        static constexpr int dimension = 2; \
        \
        SCALAR _X, _Y; \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR s = 0) : _X(s), _Y(s) {} \
        STDU_IF(CEXPR, constexpr) NAME (SCALAR _X, SCALAR _Y) : _X(_X), _Y(_Y) {} \
        \
        STDU_IF(CEXPR, constexpr) SCALAR operator[] (uint i) const { return ((const SCALAR*)this)[i]; } /* NOLINT(bugprone-macro-parentheses) */ \
        \
        STDU_IF(DEF_OP, \
        VEC_OP(vec2, add); \
        VEC_OP(vec2, sub); \
        VEC_OP(vec2, mul); \
        VEC_OP(vec2, div); \
        VEC_OP(vec2, mod); \
        \
        template <class U> auto  operator+ (U v) const { return add(v); } \
        template <class U> NAME& operator+=(U v)       { return *this = add(v); } \
        template <class U> auto  operator- (U v) const { return sub(v); } \
        template <class U> NAME& operator-=(U v)       { return *this = sub(v); } \
        template <class U> auto  operator* (U v) const { return mul(v); } \
        template <class U> NAME& operator*=(U v)       { return *this = mul(v); } \
        template <class U> auto  operator/ (U v) const { return div(v); } \
        template <class U> NAME& operator/=(U v)       { return *this = div(v); } \
        template <class U> auto  operator% (U v) const { return mod(v); } \
        template <class U> NAME& operator%=(U v)       { return *this = mod(v); } \
        \
        NAME operator+() const { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
        S_ONLY(NAME) operator-() const { return { -_X, -_Y }; } /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        \
        STDU_IF(DEF_CMP, \
        VEC_CMP(vec2, lt); \
        VEC_CMP(vec2, le); \
        VEC_CMP(vec2, eq); \
        VEC_CMP(vec2, ge); \
        VEC_CMP(vec2, gt); \
        VEC_CMP(vec2, ne); \
        \
        template <class U> bool operator <(U v) const { return lt(v); } \
        template <class U> bool operator<=(U v) const { return le(v); } \
        template <class U> bool operator==(U v) const { return eq(v); } \
        template <class U> bool operator>=(U v) const { return ge(v); } \
        template <class U> bool operator >(U v) const { return gt(v); } \
        template <class U> bool operator!=(U v) const { return ne(v); } \
        ) \
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
    }; \
    STDU_IF(DEF_OP, \
        VEC_OP_OTHER(vec2, +) \
        VEC_OP_OTHER(vec2, -) \
        VEC_OP_OTHER(vec2, *) \
        VEC_OP_OTHER(vec2, /) \
        VEC_OP_OTHER(vec2, %) \
    ) \
    STDU_IF(DEF_CMP, \
        VEC_CMP_OTHER(vec2,  <,  >) \
        VEC_CMP_OTHER(vec2, <=, >=) \
        VEC_CMP_OTHER(vec2, ==, ==) \
        VEC_CMP_OTHER(vec2, >=, <=) \
        VEC_CMP_OTHER(vec2,  >,  <) \
        VEC_CMP_OTHER(vec2, !=, !=) \
    )

#define VEC3DEF(NAME, SCALAR, _X, _Y, _Z, DEF_OP, DEF_CMP, CEXPR, ...) \
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
        VEC_OP(vec3, add); \
        VEC_OP(vec3, sub); \
        VEC_OP(vec3, mul); \
        VEC_OP(vec3, div); \
        VEC_OP(vec3, mod); \
        \
        template <class U> auto  operator+ (U v) const { return add(v); } \
        template <class U> NAME& operator+=(U v)       { return *this = add(v); } \
        template <class U> auto  operator- (U v) const { return sub(v); } \
        template <class U> NAME& operator-=(U v)       { return *this = sub(v); } \
        template <class U> auto  operator* (U v) const { return mul(v); } \
        template <class U> NAME& operator*=(U v)       { return *this = mul(v); } \
        template <class U> auto  operator/ (U v) const { return div(v); } \
        template <class U> NAME& operator/=(U v)       { return *this = div(v); } \
        template <class U> auto  operator% (U v) const { return mod(v); } \
        template <class U> NAME& operator%=(U v)       { return *this = mod(v); } \
        \
        NAME operator+() const { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
        S_ONLY(NAME) operator-() const { return { -_X, -_Y, -_Z }; } /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        \
        STDU_IF(DEF_CMP, \
        VEC_CMP(vec3, lt); \
        VEC_CMP(vec3, le); \
        VEC_CMP(vec3, eq); \
        VEC_CMP(vec3, ge); \
        VEC_CMP(vec3, gt); \
        VEC_CMP(vec3, ne); \
        \
        template <class U> bool operator <(U v) const { return lt(v); } \
        template <class U> bool operator<=(U v) const { return le(v); } \
        template <class U> bool operator==(U v) const { return eq(v); } \
        template <class U> bool operator>=(U v) const { return ge(v); } \
        template <class U> bool operator >(U v) const { return gt(v); } \
        template <class U> bool operator!=(U v) const { return ne(v); } \
        ) \
        \
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
    }; \
    STDU_IF(DEF_OP, \
        VEC_OP_OTHER(vec3, +) \
        VEC_OP_OTHER(vec3, -) \
        VEC_OP_OTHER(vec3, *) \
        VEC_OP_OTHER(vec3, /) \
        VEC_OP_OTHER(vec3, %) \
    ) \
    STDU_IF(DEF_CMP, \
        VEC_CMP_OTHER(vec3,  <,  >) \
        VEC_CMP_OTHER(vec3, <=, >=) \
        VEC_CMP_OTHER(vec3, ==, ==) \
        VEC_CMP_OTHER(vec3, >=, <=) \
        VEC_CMP_OTHER(vec3,  >,  <) \
        VEC_CMP_OTHER(vec3, !=, !=) \
    )

#define VEC4DEF(NAME, SCALAR, _X, _Y, _Z, _W, DEF_OP, DEF_CMP, DEF_W_VAL, CEXPR, ...) \
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
        VEC_OP(vec4, add); \
        VEC_OP(vec4, sub); \
        VEC_OP(vec4, mul); \
        VEC_OP(vec4, div); \
        VEC_OP(vec4, mod); \
        \
        template <class U> auto  operator+ (U v) const { return add(v); } \
        template <class U> NAME& operator+=(U v)       { return *this = add(v); } \
        template <class U> auto  operator- (U v) const { return sub(v); } \
        template <class U> NAME& operator-=(U v)       { return *this = sub(v); } \
        template <class U> auto  operator* (U v) const { return mul(v); } \
        template <class U> NAME& operator*=(U v)       { return *this = mul(v); } \
        template <class U> auto  operator/ (U v) const { return div(v); } \
        template <class U> NAME& operator/=(U v)       { return *this = div(v); } \
        template <class U> auto  operator% (U v) const { return mod(v); } \
        template <class U> NAME& operator%=(U v)       { return *this = mod(v); } \
        \
        NAME operator+() const { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
        S_ONLY(NAME) operator-() const { return { -_X, -_Y, -_Z, -_W }; } /* NOLINT(bugprone-macro-parentheses) */ \
        ) \
        \
        STDU_IF(DEF_CMP, \
        VEC_CMP(vec4, lt); \
        VEC_CMP(vec4, le); \
        VEC_CMP(vec4, eq); \
        VEC_CMP(vec4, ge); \
        VEC_CMP(vec4, gt); \
        VEC_CMP(vec4, ne); \
        \
        template <class U> bool operator <(U v) const { return lt(v); } \
        template <class U> bool operator<=(U v) const { return le(v); } \
        template <class U> bool operator==(U v) const { return eq(v); } \
        template <class U> bool operator>=(U v) const { return ge(v); } \
        template <class U> bool operator >(U v) const { return gt(v); } \
        template <class U> bool operator!=(U v) const { return ne(v); } \
        ) \
        \
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
    }; \
    STDU_IF(DEF_OP, \
    VEC_OP_OTHER(vec4, +) \
    VEC_OP_OTHER(vec4, -) \
    VEC_OP_OTHER(vec4, *) \
    VEC_OP_OTHER(vec4, /) \
    VEC_OP_OTHER(vec4, %) \
    ) \
    STDU_IF(DEF_CMP, \
    VEC_CMP_OTHER(vec4,  <,  >) \
    VEC_CMP_OTHER(vec4, <=, >=) \
    VEC_CMP_OTHER(vec4, ==, ==) \
    VEC_CMP_OTHER(vec4, >=, <=) \
    VEC_CMP_OTHER(vec4,  >,  <) \
    VEC_CMP_OTHER(vec4, !=, !=) \
    )

#define BOOL_ONLY(T) template <class S = scalar> std::enable_if_t<std::is_same_v<S, bool>, T>
#define B_ONLY(T) template <class S = scalar> std::enable_if_t<std::is_same_v<S, uchar>, T>
#define S_ONLY_U(U) std::conditional_t<std::is_signed_v<scalar>, U, stdu::empty>
#define F_ONLY(T) template <class S = scalar> std::enable_if_t<std::is_floating_point_v<S>, T>
#define COMMON_VEC_MATH(T) \
    float len() const; \
    scalar lensq() const; \
    float dist(R(T) to) const; \
    auto distsq(R(T) to) const; \
    bool in_range(R(T) other, scalar d) const; \
    \
    scalar sum() const; \
    \
    auto dot(R(T) other) const; \
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
    static T clamp(R(T) rmin, R(T) rmax, R(T) x); \
    \
    _rect_origin_inbetween_<T> as_origin() const; \
    _rect_size_inbetween_<T> as_size() const; \
    auto to(const T& other) const; \
    auto to(const _rect_size_inbetween_<T>& other) const; \
    bool is_in(const rect<dimension, scalar>& region) const; \
    \
    BOOL_ONLY(bool) all() const; \
    BOOL_ONLY(bool) any() const; \
    \
    template <class F, class... Ts> \
    auto apply(F func, const T<Ts>&... args) const -> T<decltype(func((scalar)0, ((Ts)0)...))>;
    
#pragma endregion // templated vectors
    
#pragma region Vec2
    template <class T>
    VEC2DEF(vec2, T, x, y, 1, 1, 1, \
        vec2(Direction2D dir, T scale = 1); \
        vec2(Corner2D dir,    T scale = 1); \
        vec2(stdu::convertible_to<T> auto x, stdu::convertible_to<T> auto y) : x((T)x), y((T)y) {} \
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
    )
#pragma endregion // vec2
#pragma region Vec3
    template <class T>
    VEC3DEF(vec3, T, x, y, z, 1, 1, 1,
        vec3(Direction3D dir, T scale = 1); \
        vec3(Corner3D dir,    T scale = 1); \
        vec3(stdu::convertible_to<T> auto x, stdu::convertible_to<T> auto y, \
             stdu::convertible_to<T> auto z) : x((T)x), y((T)y), z((T)z) {} \
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
        vec4(stdu::convertible_to<T> auto x, stdu::convertible_to<T> auto y, \
             stdu::convertible_to<T> auto z, stdu::convertible_to<T> auto w = 1) : x((T)x), y((T)y), z((T)z), w((T)w) {} \
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
    STDU_IF(HAS_A, constexpr T(const char (&hex)[1+8+1]);) \
    \
    bool eq(const T& other) const; \
    bool loose_eq(const T& other) const; \
    bool neq(const T& other) const; \
    bool operator==(const T& other) const; \
    \
    OPENGL_API T neg() const; \
    OPENGL_API T lerp(const T& other, float t) const; \
    OPENGL_API T mul(const T& other) const; \
    OPENGL_API T screen(const T& other) const; \
    OPENGL_API T overlay(const T& other) const; \
    STDU_IF(HAS_A, OPENGL_API without_alpha_t mul_alpha() const;) \
    \
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
    STDU_IF(HAS_A, OPENGL_API without_alpha_t rgb() const;) \
    OPENGL_API STDU_IF_ELSE(HAS_A, (T), (with_alpha_t)) with_alpha(scalar alpha = (C)) const; \
    OPENGL_API STDU_IF_ELSE(HAS_A, (T), (with_alpha_t)) rgb1() const; \
    \
    OPENGL_API \
    STDU_IF_ELSE(HAS_A, (operator without_alpha_t() const;), (operator with_alpha_t() const;)) \
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
    
    VEC3DEF(color3f, float, r, g, b,    0, 0,      1, OPENGL_API operator color3 () const; using with_alpha_t    = colorf;  COLOR_COMMON(color3f, 0, 1.0f));
    VEC3DEF(color3,  uchar, r, g, b,    0, 0,      1, OPENGL_API operator color3f() const; using with_alpha_t    = color;   COLOR_COMMON(color3,  0, 255));
    VEC4DEF(colorf,  float, r, g, b, a, 0, 0, 1,   1, OPENGL_API operator color  () const; using without_alpha_t = color3f; COLOR_COMMON(colorf,  1, 1.0f));
    VEC4DEF(color,   uchar, r, g, b, a, 0, 0, 255, 1, OPENGL_API operator colorf () const; using without_alpha_t = color3;  COLOR_COMMON(color,   1, 255));


#pragma endregion // color

#undef VEC_OP
#undef VEC_OP_OTHER
#undef VEC_CMP
#undef VEC_CMP_OTHER
#undef COLOR_COMMON
#undef VEC2DEF
#undef VEC3DEF
#undef VEC4DEF
#undef SCALAR_T
#undef F_ONLY
#undef B_ONLY
#undef BOOL_ONLY
#undef S_ONLY
#undef S_ONLY_U
#undef COMMON_VEC_MATH
#undef R
#pragma endregion // declaration

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

#undef ARITH_DO
#undef ARITH
#undef ARITH_T
    
#undef F_ONLY
#undef B_ONLY
#undef S_ONLY
#undef S_ONLY_U
}

#define VECTOR_IMPL
#include "VectorImpl.h"
#undef VECTOR_IMPL