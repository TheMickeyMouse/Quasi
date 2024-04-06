#pragma once
#include <type_traits>
#include <format>
#include <array>

#include "Corner.h"
#include "Direction.h"
#include "NumTypes.h"

#include <stdu/types.h>
#include <stdu/ref.h>

namespace Maths {
#pragma region Concepts and Decls
    template <uint N, class T> struct vecn_base;
    template <uint N, class T> struct vecn : vecn_base<N, T> {
        std::array<T, N> elems;

        vecn(T base = 0) { elems.fill(base); }
        template <class... R> vecn(R... args)
        requires ((std::is_convertible_v<T, R> && ...) && sizeof...(R) == N) : elems { args... } {}

        [[nodiscard]] auto tup() const {
            return [&]<uint... Is>(std::integer_sequence<uint, Is...>){
                return std::make_tuple(elems[Is]...);
            }(std::make_integer_sequence<uint, N> {});
        }

        static vecn ZERO() { return { 0 }; }
        static vecn ONE()  { return { 1 }; }
    };
    
    template <class> struct is_vec_t : std::false_type {};
    template <uint N, class T> struct is_vec_t<vecn<N, T>> : std::true_type {};
    
    template <class V>
    concept vec_t = is_vec_t<V>::value;

    template <class T> using vec1   = vecn<1, T>;
    template <class T> using unit   = vecn<1, T>;
    template <class T> using single = vecn<1, T>;
    template <class T> using vec2 = vecn<2, T>;
    template <class T> using vec3 = vecn<3, T>;
    template <class T> using vec4 = vecn<4, T>;
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

    template <class V> struct _rect_origin_inbetween_;
    template <class V> struct _rect_size_inbetween_;
    template <uint, class> struct rect;
    
#pragma endregion // concepts and decls

#pragma region Vector Swizzle Impl
#define VECTOR_SWIZZLING 1
#if VECTOR_SWIZZLING == 1
    namespace swizz {
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
                packed = this->into_id(args[0]);
                N = n - 1;
                for (uchar i = 1; i < N; ++i) packed |= this->into_id(args[i]) << (2 * i);
            }

            constexpr uchar operator[](uchar i) const {
                return 3 & (packed >> (i * 2));
            }
        };

        // swizzles a vecD<T> into a vec{S.N}<T> with reflection data V and of swizzle S
        template <class T, uchar D, vector_swizzle_data<D> V, swizzle<D, V> S>
        // ReSharper disable once CppNotAllPathsReturnValue
        typename vecn<S.N, T>::type swizzle_impl(const T* v) {
            if constexpr (S.N == 2) return { v[S[0]], v[S[1]] };
            if constexpr (S.N == 3) return { v[S[0]], v[S[1]], v[S[2]] };
            if constexpr (S.N == 4) return { v[S[0]], v[S[1]], v[S[2]], v[S[3]] };
            if constexpr (S.N < 2 || S.N > 4) return {};
        }
    };
#endif
#pragma endregion // vector swizzle impl

#pragma region Vector Util Fn
    namespace vecops {
        template <class T, class U> T cast(U val) { return (T)val; }

        template <class T, class U, class F, uint... I>
        void rangecopy(T& out, const U& in, F fn, std::integer_sequence<uint, I...>) {
            // ReSharper disable once CppDFAUnusedValue, CppDFAUnreadVariable
            stdu::empty _ = { (out[I] = fn(in[I]))... };
        }

        template <uint N, class From, class To>
        vecn<N, To> typecast(const vecn<N, From>& vec) {
            vecn<N, To> out;
            rangecopy(out, vec, cast<To, From>, std::make_integer_sequence<uint, N> {});
            return out;
        }

        template <uint From, uint To, class T>
        vecn<To, T> sizecast(const vecn<From, T>& vec) {
            vecn<To, T> out;
            rangecopy(out, vec, std::identity {}, std::make_integer_sequence<uint, std::min(From, To)> {});
            return out;
        }

        struct add { auto operator()(auto a, auto b) { return a + b; } };
        struct sub { auto operator()(auto a, auto b) { return a - b; } };
        struct mul { auto operator()(auto a, auto b) { return a * b; } };
        struct div { auto operator()(auto a, auto b) { return a / b; } };
        struct mod {
            auto operator()(auto a, auto b) {
                if constexpr (std::is_floating_point_v<decltype(a)> || std::is_floating_point_v<decltype(b)>)
                    return std::fmod(a, b);
                else return a % b;
            }
        };
        struct cneg { auto operator()(auto v) { if constexpr (std::is_signed_v<decltype(v)>) return -v; else return 0; } };

        struct eq  { bool operator()(auto a, auto b) { return a == b; } };
        struct neq { bool operator()(auto a, auto b) { return a != b; } };
        struct lt  { bool operator()(auto a, auto b) { return a <  b; } };
        struct le  { bool operator()(auto a, auto b) { return a <= b; } };
        struct ge  { bool operator()(auto a, auto b) { return a >= b; } };
        struct gt  { bool operator()(auto a, auto b) { return a >  b; } };

        struct min { auto operator()(auto a, auto b) { return a < b ? a : b; } };
        struct max { auto operator()(auto a, auto b) { return a < b ? b : a; } };

        template <uint N, class T, class F>
        vecn<N, decltype(F{}(T{}, T{}))> operate(F f, const vecn<N, T>& a, const vecn<N, T>& b, int = 0 /* here to explicitly say for no recursion*/) {
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                return vecn<N, decltype(F{}(T{}, T{}))> { f(a[I], b[I])... };
            }(std::make_integer_sequence<uint, N> {});
        }

        template <uint N, class T, class F>
        vecn<N, decltype(F{}(T{}, T{}))> operate(F f, const vecn<N, T>& a, std::convertible_to<T> auto b, int = 0) {
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                return vecn<N, decltype(F{}(T{}, T{}))> { f(a[I], b)... };
            }(std::make_integer_sequence<uint, N> {});
        }

        template <uint N, uint M, class T, class U, class F>
        vecn<N, decltype(F{}(T{}, U{}))> operate(F f, const vecn<N, T>& a, const vecn<M, U>& b, int = 0) {
            constexpr uint NM = std::max(N, M);
            vecn<NM, T> ca = vecn<NM, T>::ZERO();
            vecn<NM, U> cb = vecn<NM, U>::ZERO();
            rangecopy(ca, a, std::identity {}, std::make_integer_sequence<uint, N> {});
            rangecopy(cb, b, std::identity {}, std::make_integer_sequence<uint, M> {});
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                return vecn<N, decltype(F{}(T{}, U{}))> { f(a[I], b[I])... };
            }(std::make_integer_sequence<uint, N> {});
        }

        template <class F, class T, class U> requires requires { operate(F {}, U {}, T {}, int {} /* specify to not choose this function */); }
        auto operate(F f, const T& a, const U& b) { return operate(f, b, a, int {}); }

        template <uint N, class T, class F>
        void operate_inplace(F f, vecn<N, T>& a, const vecn<N, T>& b, int = 0 /* here to explicitly say for no recursion*/) {
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                stdu::empty _ = { (a[I] = f(a[I], b[I]))... };
            }(std::make_integer_sequence<uint, N> {});
        }

        template <uint N, class T, class F>
        void operate_inplace(F f, vecn<N, T>& a, std::convertible_to<T> auto b, int = 0) {
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                stdu::empty _ = { (a[I] = f(a[I], b))... };
            }(std::make_integer_sequence<uint, N> {});
        }

        template <uint N, uint M, class T, class U, class F>
        requires (M <= N && std::is_convertible_v<decltype(F{}(T{}, U{})), T>)
        void operate_inplace(F f, vecn<N, T>& a, const vecn<M, U>& b, int = 0 /* here to explicitly say for no recursion*/) {
            vecn<N, U> cb = vecn<N, U>::ZERO();
            rangecopy(cb, b, std::identity {}, std::make_integer_sequence<uint, N> {});
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                stdu::empty _ = { (a[I] = f(a[I], cb[I]))... };
            }(std::make_integer_sequence<uint, N> {});
        }

        template <uint N, class T, class F>
        T accum(F f, const vecn<N, T>& v, T base) {
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                stdu::empty _ = { (base = f(base, v[I]))... };
                return base;
            }(std::make_integer_sequence<uint, N> {});
        }

        template <uint N, class T>
        vecn<N, T> negate(const vecn<N, T>& vec) {
            return [&]<uint... I>(std::integer_sequence<uint, I...>) {
                return vecn<N, T> { -vec[I]... };
            }(std::make_integer_sequence<uint, N> {});
        }
    }
#pragma endregion

#define NODISC [[nodiscard]]

#pragma region Vector Base
    template <uint N, class T>
    struct vecn_base {
    public:
        using scalar = T;
        using vect = vecn<N, T>;
        static constexpr uint size() { return N; }
        static constexpr uint dimension = N;

        static constexpr bool traits_float  = std::is_floating_point_v<T>,
                              traits_signed = std::is_signed_v<T>;

    private:
        static auto float_type_helper() {
            if constexpr (std::is_arithmetic_v<T>) return std::common_type_t<float, T> {};
            else if constexpr (vec_t<T>) return vecn<T::dimension, std::common_type_t<typename T::scalar, float>> {};
            else return nullptr;
        }
    public:
        using float_type = decltype(float_type_helper());

        static vect from_span(std::span<const T> span) {
            vect out;
            vecops::rangecopy(out, span, std::identity {}, std::make_integer_sequence<uint, N> {});
            return out;
        }

        T* begin() { return (T*)this; }
        T* end()   { return (T*)this + N; }
        NODISC const T* begin()  const { return (const T*)this; }
        NODISC const T* cbegin() const { return (const T*)this; }
        NODISC const T* end()    const { return (const T*)this + N; }
        NODISC const T* cend()   const { return (const T*)this + N; }

        T& at_unchecked(uint i) { return *(begin() + i); }
        NODISC const T& at_unchecked(uint i) const { return *(begin() + i); }
        T& operator[](uint i) { return at_unchecked(i); }
        NODISC const T& operator[](uint i) const { return at_unchecked(i); }

        stdu::ref<T> at(uint i) { return i < N ? at_unchecked(i) : nullptr; }
        NODISC stdu::ref<const T> at(uint i) const { return i < N ? at_unchecked(i) : nullptr; }

        T& first() { return operator[](0); }
        T& last()  { return operator[](N - 1); }
        NODISC const T& first() const { return operator[](0); }
        NODISC const T& last()  const { return operator[](N - 1); }

        NODISC vecn<N + 1, T> extend(const T& ex) const {
            vecn<N + 1, T> base;
            vecops::rangecopy(base, as_vec(), std::identity {}, std::make_integer_sequence<uint, N> {});
            base.last() = ex;
            return base;
        }

        NODISC vecn<N - 1, T> shrink() const { return (vecn<N - 1, T>)as_vec(); }

        vect& as_vec() { return *(vect*)this; }
        NODISC const vect& as_vec() const { return *(const vect*)this; }

        template <class U> NODISC operator vecn<N, U>() const { return vecops::typecast<N, T, U>(as_vec()); }
        template <uint M>  NODISC operator vecn<M, T>() const { return vecops::sizecast<N, M, T>(as_vec()); }
        template <class U> NODISC vecn<N, U> as() const { return (vecn<N, U>)as_vec(); }
        NODISC vecn<N, float_type> asf() const { return as<float_type>(); }

        NODISC auto operator+() const { return as_vec(); }
        NODISC auto operator-() const requires traits_signed { return vecops::negate(as_vec()); }
        template <class V> NODISC auto operator+(const V& v) const { return vecops::operate(vecops::add {}, as_vec(), v); }
        template <class V> NODISC auto operator-(const V& v) const { return vecops::operate(vecops::sub {}, as_vec(), v); }
        template <class V> NODISC auto operator*(const V& v) const { return vecops::operate(vecops::mul {}, as_vec(), v); }
        template <class V> NODISC auto operator/(const V& v) const { return vecops::operate(vecops::div {}, as_vec(), v); }
        template <class V> NODISC auto operator%(const V& v) const { return vecops::operate(vecops::mod {}, as_vec(), v); }
        template <class V> vecn<N, T>& operator+=(const V& v) { vecops::operate_inplace(vecops::add {}, as_vec(), v); return as_vec(); }
        template <class V> vecn<N, T>& operator-=(const V& v) { vecops::operate_inplace(vecops::sub {}, as_vec(), v); return as_vec(); }
        template <class V> vecn<N, T>& operator*=(const V& v) { vecops::operate_inplace(vecops::mul {}, as_vec(), v); return as_vec(); }
        template <class V> vecn<N, T>& operator/=(const V& v) { vecops::operate_inplace(vecops::div {}, as_vec(), v); return as_vec(); }
        template <class V> vecn<N, T>& operator%=(const V& v) { vecops::operate_inplace(vecops::mod {}, as_vec(), v); return as_vec(); }

        NODISC bool eq(const vect& other) const { return as_vec() == other; }
        NODISC bool neq(const vect& other) const { return !eq(other); }
        NODISC std::strong_ordering  ord_cmp(const vect& other) const { return as_vec()->tup() <=> other.tup(); }
        NODISC std::partial_ordering abs_cmp(const vect& other) const {
            return as_vec() == other ? std::partial_ordering::equivalent :
                   as_vec() <  other ? std::partial_ordering::less :
                   as_vec() >  other ? std::partial_ordering::greater :
                   std::partial_ordering::unordered;
        }
        NODISC bool iszero() const { return eq(as_vec()->ZERO()); }

        template <class V> NODISC bool operator==(const V& v) const { return vecops::operate(vecops::eq  {}, as_vec(), v).all(); }
        template <class V> NODISC bool operator!=(const V& v) const { return vecops::operate(vecops::neq {}, as_vec(), v).all(); }
        template <class V> NODISC bool operator< (const V& v) const { return vecops::operate(vecops::lt  {}, as_vec(), v).all(); }
        template <class V> NODISC bool operator<=(const V& v) const { return vecops::operate(vecops::le  {}, as_vec(), v).all(); }
        template <class V> NODISC bool operator>=(const V& v) const { return vecops::operate(vecops::ge  {}, as_vec(), v).all(); }
        template <class V> NODISC bool operator> (const V& v) const { return vecops::operate(vecops::gt  {}, as_vec(), v).all(); }

        NODISC float_type len() const { return std::sqrt((float_type)lensq()); }
        NODISC T          lensq() const { return dot(as_vec()); }
        NODISC float_type dist(const vect& to) const { return (as_vec() - to).len(); }
        NODISC float_type distsq(const vect& to) const { return (as_vec() - to).lensq(); }
        NODISC bool       in_range(const T& other, T d) const { return dist(other) <= d; }
        NODISC vect       norm() const { return as_vec() / len(); }
        NODISC vect       norm(float d) const { return norm() * d; }

        NODISC T sum() const { return vecops::accum(vecops::add {}, as_vec(), (T)0); }
        NODISC T dot(const vect& other) const { return (as_vec() * other).sum(); }

        NODISC vecn<N, float_type> lerp(const vect& other, float t) const { return as_vec() + (other - as_vec()) * t; }
        vect& lerp_to(const vect& other, float t) requires traits_float { return as_vec() = lerp(other, t); }
        NODISC vecn<N, float_type> towards(const vect& other, float max_d) const { vect u = (other - *this).norm(); return *this + u * max_d; }
        vect& move_towards(const vect& other, float max_d) requires traits_float { return as_vec() = towards(other, max_d); }

        NODISC vect clamped() const { return clamp(as_vec().ZERO(), as_vec().ONE(), as_vec()); }
        vect& clamp() { return as_vec() = clamped(); }
        NODISC vecn<N, float> len_clamped() const { return as_vec() / std::max(1, len()); }
        vect& len_clamp() { return as_vec() = len_clamped(); }
        NODISC static vect max(const vect& a, const vect& b) { return vecops::operate(vecops::max {}, a, b); }
        NODISC static vect min(const vect& a, const vect& b) { return vecops::operate(vecops::min {}, a, b); }
        NODISC static vect clamp(const rect<N, T>& r, const vect& x);

        NODISC _rect_origin_inbetween_<vect> as_origin() const;
        NODISC _rect_size_inbetween_<vect> as_size() const;
        NODISC rect<N, T> to(const vect& other) const;
        NODISC rect<N, T> to(const _rect_size_inbetween_<vect>& other) const;
        NODISC bool is_in(const rect<N, T>& region) const;

        NODISC bool all() const requires std::is_same_v<T, bool> { return vecops::accum([](bool a, bool b) { return a && b; }, as_vec(), true); }
        NODISC bool any() const requires std::is_same_v<T, bool> { return vecops::accum([](bool a, bool b) { return a || b; }, as_vec(), true); }

        static vect random(struct random_gen& rg, const rect<N, T>& range);
        static vect random_on_unit(random_gen& rg) requires traits_float;
        static vect random_in_unit(random_gen& rg) requires traits_float;
    };

    template <uint N, class T> auto operator+(std::convertible_to<T> auto val, const vecn<N, T>& vec) requires (!vec_t<decltype(val)>) { return vec + val; }
    template <uint N, class T> auto operator-(std::convertible_to<T> auto val, const vecn<N, T>& vec) requires (!vec_t<decltype(val)>) { return vecn<N, T> { val } - vec; }
    template <uint N, class T> auto operator*(std::convertible_to<T> auto val, const vecn<N, T>& vec) requires (!vec_t<decltype(val)>) { return vec * val; }
    template <uint N, class T> auto operator/(std::convertible_to<T> auto val, const vecn<N, T>& vec) requires (!vec_t<decltype(val)>) { return vecn<N, T> { val } / vec; }
    template <uint N, class T> auto operator%(std::convertible_to<T> auto val, const vecn<N, T>& vec) requires (!vec_t<decltype(val)>) { return vecn<N, T> { val } % vec; }
#pragma endregion

#pragma region Vector Impl
    namespace types {
        template <uint N, class T> using ft = typename vecn_base<N, T>::float_type;
        template <uint N, class T> using fv = typename vecn<N, T>::float_vec;
    }

#pragma region Vec1
    template <class T>
    struct vecn<1, T> : public vecn_base<1, T> {
        using base = vecn_base<1, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = vecn<dimension, float_type>;
        T x;

        vecn(T x = 0) : x(x) {}
        vecn(stdu::convertible_to<T> auto x) : x((T)x) {}

        static constexpr swizz::vector_swizzle_data<dimension> params = "x";
        template <auto S>
        vecn<S.N, T> swizzle() const {
            return swizz::swizzle_impl<T, dimension, params, swizz::swizzle<dimension, params> { S }>(this->begin());
        }

        [[nodiscard]] std::string str() const { return std::format("(x: {})", x); }

        NODISC const T& value() const { return x; }
        T& value() { return x; }
        NODISC operator const T&() const { return value(); }
        operator T&() { return value(); }

        static vecn ZERO() { return {  0 }; }
        static vecn ONE()  { return { +1 }; }
    };
#pragma endregion // Vec1
#pragma region Vec2
    template <class T>
    struct vecn<2, T> : public vecn_base<2, T> {
        using base = vecn_base<2, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = vecn<dimension, float_type>;
        T x; T y;

        vecn(T s = 0) : x(s), y(s) {}
        vecn(T x, T y) : x(x), y(y) {}
        explicit vecn(Direction2D dir, T scale = 1);
        explicit vecn(Corner2D cor, T scale = 1);
        vecn(stdu::convertible_to<T> auto x, stdu::convertible_to<T> auto y) : x((T)x), y((T)y) {}

        static constexpr swizz::vector_swizzle_data<dimension> params = "xy";
        template <auto S>
        vecn<S.N, T> swizzle() const {
            return swizz::swizzle_impl<T, dimension, params, swizz::swizzle<dimension, params> { S }>(this->begin());
        }

        [[nodiscard]] std::string str() const { return std::format("(x: {}, y: {})", x, y); }

        static vecn RIGHT()                       { return { +1,  0 }; }
        static vecn LEFT() requires traits_signed { return { -1,  0 }; }
        static vecn UP()                          { return {  0, +1 }; }
        static vecn DOWN() requires traits_signed { return {  0, -1 }; }
        static vecn ZERO()                        { return {  0,  0 }; }
        static vecn ONE()                         { return { +1, +1 }; }

        NODISC float_type slope() const;

        NODISC float_type angle() const;
        NODISC float_type angle(const vecn& other) const;
        NODISC float_type angle_signed(const vecn& other) const;
        NODISC float_vec polar() const;
        static float_vec from_polar(T r, T theta);

        NODISC vecn perpend() const requires traits_signed;

        NODISC float_vec rotated(float angle) const;
        NODISC float_vec rotated(float angle, const vecn& origin) const;
        NODISC vecn projected(const vecn& axis) const;
        NODISC vecn reflected(const vecn& normal) const;
        vecn& rotate(float angle)                     requires traits_float { return *this = rotated(angle); }
        vecn& rotate(float angle, const vecn& origin) requires traits_float { return *this = rotated(angle, origin); }
        vecn& reflect(const vecn& normal) { return *this = reflected(normal); }
        vecn& project(const vecn& axis)   { return *this = projected(axis); }

        NODISC vec3<T> with_z(T z) const { return { x, y, z }; }

        NODISC std::tuple<T, T> tup() const { return { x, y }; }

        static vecn unit_x(T x) { return { x, 0 }; }
        static vecn unit_y(T y) { return { 0, y }; }
    };

    template <class T> types::ft<2, T> vecn<2, T>::slope() const { return (float_type)y / (float_type)x; }
    template <class T> types::ft<2, T> vecn<2, T>::angle() const { return std::atan2((float_type)y, (float_type)x); }
    template <class T> types::ft<2, T> vecn<2, T>::angle(const vecn& other) const { return std::acos((float_type)this->dot(other) / (this->len() * other.len())); }
    template <class T> types::ft<2, T> vecn<2, T>::angle_signed(const vecn& other) const { return std::atan2((float_type)(y * other.x - x * other.y), (float_type)this->dot(other)); }
    template <class T> types::fv<2, T> vecn<2, T>::polar() const { return { this->len(), angle() }; }
    template <class T> types::fv<2, T> vecn<2, T>::from_polar(T r, T theta) { return { r * std::cos(theta), r * std::sin(theta) }; }

    template <class T> vec2<T> vecn<2, T>::perpend() const requires traits_signed { return { y, -x }; } /* perpendicular vector (rotated 90 deg), (0, 1) -> (1, 0) */
    template <class T> types::fv<2, T> vecn<2, T>::rotated(float angle) const {
        types::ft<2, T> sin = std::sin(angle), cos = std::cos(angle);
        return { x * cos - y * sin, x * sin + y * cos };
    }
    template <class T> types::fv<2, T> vecn<2, T>::rotated(float angle, const vecn& origin) const { return (*this - origin).rotated(angle) + (float_vec)origin; }
    template <class T> vecn<2, T> vecn<2, T>::reflected(const vecn& normal) const { return *this - 2 * this->dot(normal) * normal; }
    template <class T> vecn<2, T> vecn<2, T>::projected(const vecn& axis) const { return axis * this->dot(axis) / axis.lensq(); }

    template <class T> vecn<2, T>::vecn(Direction2D dir, T scale) : x(0), y(0) {
        using enum Direction2D;
        if (dir == UNIT) {
            x = y = scale; return;
        }
        if (dir < 0 || dir > DOWN) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? vecops::cneg {} (scale) : scale;
    }

    template <class T> vecn<2, T>::vecn(Corner2D cor, T scale) : x(0), y(0) {
        using enum Corner2D;
        if (cor < 0 || cor > BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? vecops::cneg {} (scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? vecops::cneg {} (scale) : scale;
    }
#pragma endregion
#pragma region Vec3
    template <class T>
    struct vecn<3, T> : public vecn_base<3, T> {
        using base = vecn_base<3, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = vecn<dimension, float_type>;
        T x; T y; T z;

        vecn(T s = 0) : x(s), y(s), z(s) {}
        vecn(T x, T y, T z) : x(x), y(y), z(z) {}
        explicit vecn(Direction3D dir, T scale = 1);
        explicit vecn(Corner3D cor,    T scale = 1);
        vecn(stdu::convertible_to<T> auto x, stdu::convertible_to<T> auto y,
             stdu::convertible_to<T> auto z) : x((T)x), y((T)y), z((T)z) {}

        static constexpr swizz::vector_swizzle_data<dimension> params = "xyz";
        template <auto S>
        vecn<S.N, T> swizzle() const {
            return swizz::swizzle_impl<T, dimension, params, swizz::swizzle<dimension, params> { S }>(this->begin());
        }

        [[nodiscard]] std::string str() const { return std::format("(x: {}, y: {}, z: {})", x, y, z); }

        static vecn RIGHT()                       { return {  1,  0,  0 }; }
        static vecn LEFT() requires traits_signed { return { -1,  0,  0 }; }
        static vecn UP()                          { return {  0,  1,  0 }; }
        static vecn DOWN() requires traits_signed { return {  0, -1,  0 }; }
        static vecn FRONT()                       { return {  0,  0,  1 }; }
        static vecn BACK() requires traits_signed { return {  0,  0, -1 }; }
        static vecn ZERO()                        { return {  0,  0,  0 }; }
        static vecn ONE()                         { return {  1,  1,  1 }; }

        NODISC vecn cross(const vecn& other) const;

        NODISC float_type angle(const vecn& other) const;
        NODISC float_type angle_signed(const vecn& other, const vecn& normal) const;

        NODISC float_type yaw() const;
        NODISC float_type pitch() const;
        NODISC float_vec spheric() const;
        static float_vec from_spheric(T r, T yaw, T pitch);

        NODISC vec2<float_type> projected() const;
        NODISC vecn reflected(const vecn& normal) const;
        vecn& reflect(const vecn& normal) { return *this = reflected(normal); }

        NODISC auto to_color(T alpha = 1) const -> std::conditional_t<traits_float, colorf,  color>;
        NODISC auto to_color3() const -> std::conditional_t<traits_float, color3f, color3>;

        NODISC vec2<T> xy() const { return { x, y }; }
        NODISC vec4<T> with_w(T w = 1) const { return { x, y, z, w }; }

        NODISC std::tuple<T, T, T> tup() const { return { x, y, z }; }

        static vecn unit_x(T x) { return { x, 0, 0 }; }
        static vecn unit_y(T y) { return { 0, y, 0 }; }
        static vecn unit_z(T z) { return { 0, 0, z }; }
    };

    template <class T> vec3<T> vecn<3, T>::cross(const vecn& other) const { return { (T)(y * other.z - z * other.y), (T)(z * other.x - x * other.z), (T)(x * other.y - y * other.x) }; }

    template <class T> types::ft<3, T> vecn<3, T>::yaw() const { return std::atan2((float_type)x, (float_type)z); }
    template <class T> types::ft<3, T> vecn<3, T>::pitch() const { return std::asin((float_type)y / this->len()); }
    template <class T> types::fv<3, T> vecn<3, T>::spheric() const { return { this->len(), yaw(), pitch() }; }
    template <class T> types::fv<3, T> vecn<3, T>::from_spheric(T r, T yaw, T pitch) { return (float_type)r * float_vec { std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch) }; }

    template <class T> types::ft<3, T> vecn<3, T>::angle(const vecn& other) const { return std::acos((float_type)this->dot(other) / (this->len() * other.len())); }
    template <class T> types::ft<3, T> vecn<3, T>::angle_signed(const vecn& other, const vecn& normal) const { return std::atan2((float_type)cross(other).dot(normal), (float_type)this->dot(other)); }

    template <class T> vec2<types::ft<3, T>> vecn<3, T>::projected() const { return (vec2<float_type>)xy() / (float_type)z; }
    template <class T> vec3<T> vecn<3, T>::reflected(const vecn& normal) const { return *this - 2 * this->dot(normal) * normal; }

    template <class T> vecn<3, T>::vecn(Direction3D dir, T scale) : x(0), y(0), z(0) {
        using enum Direction3D;
        if (dir == UNIT) {
            x = y = z = scale; return;
        }
        if (dir < 0 || dir > BACK) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? vecops::cneg {} (scale) : scale;
    }

    template <class T> vecn<3, T>::vecn(Corner3D cor, T scale) : x(0), y(0), z(0) {
        using enum Corner3D;
        if (cor < 0 || cor > BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? vecops::cneg {} (scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? vecops::cneg {} (scale) : scale;
        z = (int)(cor & SIDE_BACK  ) ? vecops::cneg {} (scale) : scale;
    }
#pragma endregion
#pragma region Vec4
    template <class T>
    struct vecn<4, T> : vecn_base<4, T> {
        using base = vecn_base<4, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = vecn<dimension, float_type>;
        T x; T y; T z; T w;

        vecn(T s = 0, T w = 0) : x(s), y(s), z(s), w(w) {}
        vecn(T x, T y, T z, T w = 1) : x(x), y(y), z(z), w(w) {}
        explicit vecn(Direction4D dir, T scale = 1);
        explicit vecn(Corner4D    cor, T scale = 1);
        vecn(stdu::convertible_to<T> auto x, stdu::convertible_to<T> auto y,
             stdu::convertible_to<T> auto z, stdu::convertible_to<T> auto w = 1) : x((T)x), y((T)y), z((T)z), w((T)w) {}

        static constexpr swizz::vector_swizzle_data<dimension> params = "xyzw";
        template <auto S>
        vecn<S.N, T> swizzle() const {
            return swizz::swizzle_impl<T, dimension, params, swizz::swizzle<dimension, params> { S }>(this->begin());
        }

        [[nodiscard]] std::string str() const { return std::format("(x: {}, y: {}, z: {}, w: {})", x, y, z, w); }

        static constexpr vecn RIGHT()                       { return {  1,  0,  0,  0 }; }
        static constexpr vecn LEFT() requires traits_signed { return { -1,  0,  0,  0 }; }
        static constexpr vecn UP()                          { return {  0,  1,  0,  0 }; }
        static constexpr vecn DOWN() requires traits_signed { return {  0, -1,  0,  0 }; }
        static constexpr vecn FRONT()                       { return {  0,  0,  1,  0 }; }
        static constexpr vecn BACK() requires traits_signed { return {  0,  0, -1,  0 }; }
        static constexpr vecn IN()                          { return {  0,  0,  0,  1 }; }
        static constexpr vecn OUT()  requires traits_signed { return {  0,  0,  0, -1 }; }
        static constexpr vecn ZERO()                        { return {  0,  0,  0,  0 }; }
        static constexpr vecn ONE()                         { return {  1,  1,  1,  1 }; }

        NODISC float_type angle(const vecn& other) const;

        NODISC vec3<T> xyz() const { return { x, y, z }; }
        NODISC vec2<T> xy() const { return { x, y }; }

        NODISC vec3<float_type> projected() const;

        NODISC auto to_color() const -> std::conditional_t<traits_float, colorf, color>;

        NODISC std::tuple<T, T, T, T> tup() const { return { x, y, z, w }; }

        static vecn unit_x(T x) { return { x, 0, 0, 0 }; }
        static vecn unit_y(T y) { return { 0, y, 0, 0 }; }
        static vecn unit_z(T z) { return { 0, 0, z, 0 }; }
        static vecn unit_w(T w) { return { 0, 0, 0, w }; }
    };

    template <class T> types::ft<4, T> vecn<4, T>::angle(const vecn& other) const { return std::acos((float_type)this->dot(other) / (this->len() * other.len())); }
    template <class T> vec3<types::ft<4, T>> vecn<4, T>::projected() const { return (vec3<float_type>)xyz() / (float_type)w; }

    template <class T> vecn<4, T>::vecn(Direction4D dir, T scale) : x(0), y(0), z(0), w(0) {
        using enum Direction4D;
        if (dir == UNIT) {
            x = y = z = w = scale; return;
        }
        if (dir < 0 || dir > OUT) return;
        (&x)[(int)(dir >> 1)] = (int)(dir & 1) ? vecops::cneg {} (scale) : scale;
    }

    template <class T> vecn<4, T>::vecn(Corner4D cor, T scale) : x(0), y(0), z(0), w(0) {
        using enum Corner4D;
        if (cor < 0 || cor > OUTER_BACK_BOTTOM_LEFT) return;
        x = (int)(cor & SIDE_LEFT  ) ? vecops::cneg {} (scale) : scale;
        y = (int)(cor & SIDE_BOTTOM) ? vecops::cneg {} (scale) : scale;
        z = (int)(cor & SIDE_BACK  ) ? vecops::cneg {} (scale) : scale;
        w = (int)(cor & SIDE_OUTER ) ? vecops::cneg {} (scale) : scale;
    }
#pragma endregion
#pragma endregion

#undef NODISC
}