#pragma once
#include <type_traits>
#include <array>
#include <cmath>

#include "Constants.h"
#include "Option.h"
#include "Text.h"

#include "Utils/Iterator.h"
#include "Utils/Ref.h"

namespace Quasi::Math {
#pragma region Concepts and Decls
    namespace details {
        template <u32 N, class T> struct vecn_base;
    }

    template <u32 N, class T> struct VectorN : details::vecn_base<N, T> {
        std::array<T, N> elems;

        VectorN(T base = 0) { elems.fill(base); }
        template <class... R> VectorN(R... args)
        requires ((std::is_convertible_v<T, R> && ...) && sizeof...(R) == N) : elems { args... } {}

        auto tup() const {
            return [&]<uint... Is>(std::integer_sequence<uint, Is...>){
                return std::make_tuple(elems[Is]...);
            }(std::make_integer_sequence<uint, N> {});
        }

        static VectorN ZERO() { return { 0 }; }
        static VectorN ONE()  { return { 1 }; }
    };
    
    template <class> struct IsVectorType : std::false_type {};
    template <u32 N, class T> struct IsVectorType<VectorN<N, T>> : std::true_type {};
    
    template <class V>
    concept IVector = IsVectorType<V>::value;

    template <class T> using Unit    = VectorN<1, T>;
    template <class T> using Single  = VectorN<1, T>;
    template <class T> using Vector2 = VectorN<2, T>;
    template <class T> using Vector3 = VectorN<3, T>;
    template <class T> using Vector4 = VectorN<4, T>;
    using fVector2 = Vector2<float>;
    using fVector3 = Vector3<float>;
    using fVector4 = Vector4<float>;
    using dVector2 = Vector2<double>;
    using dVector3 = Vector3<double>;
    using dVector4 = Vector4<double>;
    using iVector2 = Vector2<int>;
    using iVector3 = Vector3<int>;
    using iVector4 = Vector4<int>;
    using uVector2 = Vector2<uint>;
    using uVector3 = Vector3<uint>;
    using uVector4 = Vector4<uint>;
    using bVector2 = Vector2<byte>;
    using bVector3 = Vector3<byte>;
    using bVector4 = Vector4<byte>;

    template <class> struct Complex;
    struct Quaternion;

    struct Transform2D;
    struct Transform3D;

    struct Color3;
    struct fColor3;
    struct fColor;
    struct Color;

    namespace details {
        template <class V> struct rect_origin_t;
        template <class V> struct rect_size_t;
    }
    template <u32, class> struct RectN;
    
#pragma endregion // concepts and decls

#pragma region Vector Swizzle Impl
#define VECTOR_SWIZZLING 1
#if VECTOR_SWIZZLING == 1
    template <class T, u32 D, u32 N>
    VectorN<N - 1, T> operator%(const VectorN<D, T>& vector, const char (&swizz)[N]) {
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            return VectorN<N - 1, T> { vector[VectorN<D, T>::params.find(swizz[I])]... };
        }(std::make_integer_sequence<u32, N - 1> {});
    }
#endif
#pragma endregion // vector swizzle impl

#pragma region Vector Util Fn
    namespace details {
        template <class T> struct floating_of {};
        template <class T> requires std::is_arithmetic_v<T> struct floating_of<T> {
            using type = std::common_type_t<T, float>;
        };

        template <IVector T> struct floating_of<T> {
            using type = VectorN<T::dimension, typename floating_of<typename T::scalar>::type>;
        };
        template <class T> using floating_of_t = typename floating_of<T>::type;

        template <class T, class U> T cast(U val) { return (T)val; }

        template <class T, class U, class F, u32... I>
        void rangecopy(T& out, const U& in, F fn, std::integer_sequence<u32, I...>) {
            // ReSharper disable once CppDFAUnusedValue, CppDFAUnreadVariable
            Empty _ = { (out[I] = fn(in[I]))... };
        }

        template <u32 N, class From, class To>
        VectorN<N, To> typecast(const VectorN<N, From>& vec) {
            VectorN<N, To> out;
            rangecopy(out, vec, cast<To, From>, std::make_integer_sequence<u32, N> {});
            return out;
        }

        template <u32 From, uint To, class T>
        VectorN<To, T> sizecast(const VectorN<From, T>& vec) {
            VectorN<To, T> out;
            rangecopy(out, vec, std::identity {}, std::make_integer_sequence<u32, std::min(From, To)> {});
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

        template <u32 N, class T, class F>
        VectorN<N, decltype(F{}(T{}, T{}))> operate(F f, const VectorN<N, T>& a, const VectorN<N, T>& b, int = 0 /* here to explicitly say for no recursion*/) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                return VectorN<N, decltype(F{}(T{}, T{}))> { f(a[I], b[I])... };
            }(std::make_integer_sequence<u32, N> {});
        }

        template <u32 N, class T, class F>
        VectorN<N, decltype(F{}(T{}, T{}))> operate(F f, const VectorN<N, T>& a, std::convertible_to<T> auto b, int = 0) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                return VectorN<N, decltype(F{}(T{}, T{}))> { f(a[I], b)... };
            }(std::make_integer_sequence<u32, N> {});
        }

        template <u32 N, u32 M, class T, class U, class F>
        VectorN<std::max(N, M), decltype(F{}(T{}, U{}))> operate(F f, const VectorN<N, T>& a, const VectorN<M, U>& b, int = 0) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                return VectorN<std::max(N, M), decltype(F{}(T{}, U{}))> { f(I < N ? a[I] : 0, I < M ? b[I] : 0)... };
            }(std::make_integer_sequence<u32, N> {});
        }

        template <class F, class T, class U> requires requires { operate(F {}, U {}, T {}, int {} /* specify to not choose this function */); }
        auto operate(F f, const T& a, const U& b) { return operate(f, b, a, int {}); }

        template <u32 N, class T, class F>
        void operate_inplace(F f, VectorN<N, T>& a, const VectorN<N, T>& b, int = 0 /* here to explicitly say for no recursion*/) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                Empty _ = { (a[I] = f(a[I], b[I]))... };
            }(std::make_integer_sequence<u32, N> {});
        }

        template <u32 N, class T, class F>
        void operate_inplace(F f, VectorN<N, T>& a, std::convertible_to<T> auto b, int = 0) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                Empty _ = { (a[I] = f(a[I], b))... };
            }(std::make_integer_sequence<u32, N> {});
        }

        template <u32 N, u32 M, class T, class U, class F>
        requires (M <= N && std::is_convertible_v<decltype(F{}(T{}, U{})), T>)
        void operate_inplace(F f, VectorN<N, T>& a, const VectorN<M, U>& b, int = 0 /* here to explicitly say for no recursion*/) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                Empty _ = { (a[I] = f(a[I], I < M ? b[I] : 0))... };
            }(std::make_integer_sequence<u32, N> {});
        }

        template <u32 N, class T, class F>
        T accum(F f, const VectorN<N, T>& v, T base) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                Empty _ = { (base = f(base, v[I]))... };
                return base;
            }(std::make_integer_sequence<u32, N> {});
        }

        template <u32 N, class T>
        VectorN<N, T> negate(const VectorN<N, T>& vec) {
            return [&]<u32... I>(std::integer_sequence<u32, I...>) {
                return VectorN<N, T> { -vec[I]... };
            }(std::make_integer_sequence<u32, N> {});
        }
    }
#pragma endregion

#define NODISC [[nodiscard]]

#pragma region Vector Base
    struct RandomGenerator;

    template <u32 N, class T>
    struct details::vecn_base {
    public:
        using scalar = T;
        using vect = VectorN<N, T>;
        static constexpr u32 size() { return N; }
        static constexpr u32 dimension = N;

        static constexpr bool traits_float  = std::is_floating_point_v<T>,
                              traits_signed = std::is_signed_v<T>;
    public:
        using float_type = floating_of_t<T>;

        static vect from_span(Span<const T> span) {
            vect out;
            details::rangecopy(out, span, std::identity {}, std::make_integer_sequence<u32, N> {});
            return out;
        }

        static vect from_direction(u32 directionID, T scale) {
            vect val;
            val[directionID >> 1] = directionID & 1 ? cneg {}(scale) : scale;
            return val;
        }

        static vect from_corner(u32 cornerID, T scale) {
            vect val;
            for (u32 i = 0; i < N; ++i) val[i] = cornerID & (1 << i) ? scale : cneg {}(scale);
            return val;
        }

        T* begin() { return (T*)this; }
        T* end()   { return (T*)this + N; }
        NODISC const T* begin()  const { return (const T*)this; }
        NODISC const T* cbegin() const { return (const T*)this; }
        NODISC const T* end()    const { return (const T*)this + N; }
        NODISC const T* cend()   const { return (const T*)this + N; }

        T& at_unchecked(usize i) { return *(begin() + i); }
        NODISC const T& at_unchecked(usize i) const { return *(begin() + i); }
        T& operator[](usize i) { return at_unchecked(i); }
        NODISC const T& operator[](usize i) const { return at_unchecked(i); }

        Ref<T> at(usize i) { return i < N ? at_unchecked(i) : nullptr; }
        NODISC Ref<const T> at(usize i) const { return i < N ? at_unchecked(i) : nullptr; }

        T& first() { return at_unchecked(0); }
        T& last()  { return at_unchecked(N - 1); }
        NODISC const T& first() const { return at_unchecked(0); }
        NODISC const T& last()  const { return at_unchecked(N - 1); }

        NODISC VectorN<N + 1, T> extend(const T& ex) const {
            VectorN<N + 1, T> base;
            details::rangecopy(base, as_vec(), std::identity {}, std::make_integer_sequence<u32, N> {});
            base.last() = ex;
            return base;
        }

        NODISC VectorN<N - 1, T> shrink() const { return (VectorN<N - 1, T>)as_vec(); }

        vect& as_vec() { return *(vect*)this; }
        NODISC const vect& as_vec() const { return *(const vect*)this; }

        template <class U> NODISC operator VectorN<N, U>() const { return details::typecast<N, T, U>(as_vec()); }
        template <u32 M>   NODISC operator VectorN<M, T>() const { return details::sizecast<N, M, T>(as_vec()); }
        template <class U> NODISC VectorN<N, U> as() const { return (VectorN<N, U>)as_vec(); }
        NODISC VectorN<N, float_type> asf() const { return as<float_type>(); }

        NODISC auto operator+() const { return as_vec(); }
        NODISC auto operator-() const requires traits_signed { return details::negate(as_vec()); }
        NODISC auto operator+(const auto& v) const { return details::operate(add {}, as_vec(), v); }
        NODISC auto operator-(const auto& v) const { return details::operate(sub {}, as_vec(), v); }
        NODISC auto operator*(const auto& v) const { return details::operate(mul {}, as_vec(), v); }
        NODISC auto operator/(const auto& v) const { return details::operate(div {}, as_vec(), v); }
        NODISC auto operator%(const auto& v) const { return details::operate(mod {}, as_vec(), v); }
        VectorN<N, T>& operator+=(const auto& v) {  details::operate_inplace(add {}, as_vec(), v); return as_vec(); }
        VectorN<N, T>& operator-=(const auto& v) {  details::operate_inplace(sub {}, as_vec(), v); return as_vec(); }
        VectorN<N, T>& operator*=(const auto& v) {  details::operate_inplace(mul {}, as_vec(), v); return as_vec(); }
        VectorN<N, T>& operator/=(const auto& v) {  details::operate_inplace(div {}, as_vec(), v); return as_vec(); }
        VectorN<N, T>& operator%=(const auto& v) {  details::operate_inplace(mod {}, as_vec(), v); return as_vec(); }

        NODISC bool eq(const vect& other) const { return as_vec() == other; }
        NODISC bool neq(const vect& other) const { return !eq(other); }
        NODISC std::strong_ordering  ord_cmp(const vect& other) const { return as_vec()->tup() <=> other.tup(); }
        NODISC std::partial_ordering abs_cmp(const vect& other) const {
            return as_vec() == other ? std::partial_ordering::equivalent :
                   as_vec() <  other ? std::partial_ordering::less :
                   as_vec() >  other ? std::partial_ordering::greater :
                   std::partial_ordering::unordered;
        }
        NODISC bool iszero() const { return eq(as_vec().ZERO()); }

        NODISC bool operator==(const auto& v) const { return details::operate(details::eq  {}, as_vec(), v).all(); }
        NODISC bool operator!=(const auto& v) const { return details::operate(details::neq {}, as_vec(), v).all(); }
        NODISC bool operator< (const auto& v) const { return details::operate(details::lt  {}, as_vec(), v).all(); }
        NODISC bool operator<=(const auto& v) const { return details::operate(details::le  {}, as_vec(), v).all(); }
        NODISC bool operator>=(const auto& v) const { return details::operate(details::ge  {}, as_vec(), v).all(); }
        NODISC bool operator> (const auto& v) const { return details::operate(details::gt  {}, as_vec(), v).all(); }

        NODISC float_type len() const { return std::sqrt((float_type)lensq()); }
        NODISC T          lensq() const { return dot(as_vec()); }
        NODISC float_type dist(const vect& to) const { return (as_vec() - to).len(); }
        NODISC float_type distsq(const vect& to) const { return (as_vec() - to).lensq(); }
        NODISC bool       in_range(const vect& other, T d) const { return dist(other) <= d; }
        NODISC vect       norm() const { return as_vec() / len(); }
        NODISC vect       norm(float d) const { return norm() * d; }
        NODISC vect       safe_norm() const { return lensq() <= EPSILON * EPSILON ? 0 : norm(); }

        NODISC T sum() const { return details::accum(add {}, as_vec(), (T)0); }
        NODISC T dot(const vect& other) const { return (as_vec() * other).sum(); }

        NODISC VectorN<N, float_type> lerp(const vect& other, float t) const { return as_vec() + (other - as_vec()) * t; }
        vect& lerp_to(const vect& other, float t) requires traits_float { return as_vec() = lerp(other, t); }
        NODISC VectorN<N, float_type> towards(const vect& other, float max_d) const { vect u = (other - *this).norm(); return *this + u * max_d; }
        vect& move_towards(const vect& other, float max_d) requires traits_float { return as_vec() = towards(other, max_d); }

        NODISC float_type angle(const vect& other) { return std::acos(dot(other) / (len() * other.len())); }
        NODISC vect slerp(const vect& other, float t) {
            const float_type theta = angle(other);
            return (std::sin((1 - t) * theta) * as_vec() + std::sin(t * theta) * other) / std::sin(theta);
        }

        NODISC vect clamped() const { return clamp(as_vec().ZERO(), as_vec().ONE(), as_vec()); }
        vect& clamp() { return as_vec() = clamped(); }
        NODISC VectorN<N, float_type> len_clamped() const { return as_vec() / std::max(1, len()); }
        vect& len_clamp() { return as_vec() = len_clamped(); }
        NODISC static vect max(const vect& a, const vect& b) { return details::operate(details::max {}, a, b); }
        NODISC static vect min(const vect& a, const vect& b) { return details::operate(details::min {}, a, b); }
        NODISC static vect max(const Collection<vect> auto& nums) { vect m = { std::numeric_limits<T>::lowest() }; for (const auto& v : nums) { m = max(m, v); } return m; }
        NODISC static vect min(const Collection<vect> auto& nums) { vect m = { std::numeric_limits<T>::max()    }; for (const auto& v : nums) { m = min(m, v); } return m; }
        NODISC static vect clamp(const RectN<N, T>& r, const vect& x);

        NODISC vect map(const RectN<N, T>& input, const RectN<N, T>& output) const;

        NODISC rect_origin_t<vect> as_origin() const;
        NODISC rect_size_t<vect> as_size() const;
        NODISC RectN<N, T> to(const vect& other) const;
        NODISC RectN<N, T> to(const rect_size_t<vect>& other) const;
        NODISC bool is_in(const RectN<N, T>& region) const;

        NODISC bool all() const requires std::is_same_v<T, bool> { return details::accum([](bool a, bool b) { return a && b; }, as_vec(), true); }
        NODISC bool any() const requires std::is_same_v<T, bool> { return details::accum([](bool a, bool b) { return a || b; }, as_vec(), true); }

        static vect random(RandomGenerator& rg, const RectN<N, T>& range);
        static vect random_on_unit(RandomGenerator& rg) requires traits_float;
        static vect random_in_unit(RandomGenerator& rg) requires traits_float;

        static Option<vect> parse(Str string, Str sep, Str beg, Str end, Fn<Option<T>, Str> auto elemParser) {
            if (!string.starts_with(beg) || !string.starts_with(end)) return nullptr;

            string = string.substr(beg.size(), string.size() - beg.size() - end.size());
            u32 splittings[N - 1] {};
            for (u32 i = 0; i < N - 1; ++i) {
                const auto it = string.find(sep, i ? splittings[i - 1] + sep.size() : 0);
                if (it == Str::npos) return nullptr;
                splittings[i] = it;
            }
            if (string.find(sep, splittings[N - 2] + sep.size()) != Str::npos) return nullptr;

            vect vector;
            for (u32 i = 0; i < N; ++i) {
                Option<T> val = elemParser(string.substr(
                    i ? splittings[i - 1] + sep.size() : 0,
                    i == N - 1 ? Str::npos : splittings[i] - (i ? splittings[i - 1] + sep.size() : 0)
                ));
                if (!val) return nullptr;
                vector[i] = val.Unwrap();
            }
            return vector;
        }
        static Option<vect> parse(Str string, Str sep, Str beg, Str end) { return parse(string, sep, beg, end, Text::Parse<T>); }
    };

    template <u32 N, class T> auto operator+(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec + val; }
    template <u32 N, class T> auto operator-(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return VectorN<N, T> { val } - vec; }
    template <u32 N, class T> auto operator*(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec * val; }
    template <u32 N, class T> auto operator/(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return VectorN<N, T> { val } / vec; }
    template <u32 N, class T> auto operator%(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return VectorN<N, T> { val } % vec; }

    template <u32 N, class T> auto operator==(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec == val; }
    template <u32 N, class T> auto operator!=(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec != val; }
    template <u32 N, class T> auto operator< (std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec >  val; }
    template <u32 N, class T> auto operator<=(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec >= val; }
    template <u32 N, class T> auto operator>=(std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec <= val; }
    template <u32 N, class T> auto operator> (std::convertible_to<T> auto val, const VectorN<N, T>& vec) requires (!IVector<decltype(val)>) { return vec <  val; }
#pragma endregion

#pragma region Vector Impl
#pragma region Vec1
    template <class T>
    struct VectorN<1, T> : public details::vecn_base<1, T> {
        using base = details::vecn_base<1, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = VectorN<dimension, float_type>;
        T x;

        VectorN(T x = 0) : x(x) {}
        VectorN(std::convertible_to<T> auto x) : x((T)x) {}

        static constexpr Str params = "x";

        NODISC const T& value() const { return x; }
        T& value() { return x; }
        NODISC operator const T&() const { return value(); }
        operator T&() { return value(); }

        static VectorN ZERO() { return {  0 }; }
        static VectorN ONE()  { return { +1 }; }
    };
#pragma endregion // Vec1
#pragma region Vec2
    template <class T>
    struct VectorN<2, T> : public details::vecn_base<2, T> {
        using base = details::vecn_base<2, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = VectorN<dimension, float_type>;
        T x; T y;

        VectorN(T s = 0) : x(s), y(s) {}
        VectorN(T x, T y) : x(x), y(y) {}
        VectorN(std::convertible_to<T> auto x, std::convertible_to<T> auto y) : x((T)x), y((T)y) {}

        static constexpr Str params = "xy";

        static VectorN RIGHT()                       { return { +1,  0 }; }
        static VectorN LEFT() requires traits_signed { return { -1,  0 }; }
        static VectorN UP()                          { return {  0, +1 }; }
        static VectorN DOWN() requires traits_signed { return {  0, -1 }; }
        static VectorN ZERO()                        { return {  0,  0 }; }
        static VectorN ONE()                         { return { +1, +1 }; }

        NODISC float_type zcross(const VectorN& other) const;

        NODISC float_type slope() const;

        NODISC float_type angle() const;
        NODISC float_type angle_signed(const VectorN& other) const;
        NODISC float_vec polar() const;
        static float_vec from_polar(T r, T theta);

        NODISC VectorN perpend() const requires traits_signed;

        NODISC float_vec rotated(float angle) const;
        NODISC float_vec rotated(float angle, const VectorN& origin) const;
        NODISC float_vec rotated_by(const Complex<T>& rotation) const;
        NODISC VectorN projected(const VectorN& axis) const;
        NODISC VectorN reflected(const VectorN& normal) const;
        NODISC VectorN transformed_by(const Transform2D& transform) const;

        VectorN& rotate(float angle)                        requires traits_float { return *this = rotated(angle); }
        VectorN& rotate(float angle, const VectorN& origin) requires traits_float { return *this = rotated(angle, origin); }
        VectorN& rotate_by(const Complex<T>& rotation) requires traits_float;
        VectorN& reflect(const VectorN& normal) { return *this = reflected(normal); }
        VectorN& project(const VectorN& axis)   { return *this = projected(axis); }
        VectorN& transform_by(const Transform2D& transform);

        NODISC Vector3<T> with_z(T z) const { return { x, y, z }; }

        NODISC std::tuple<T, T> tup() const { return { x, y }; }

        static VectorN unit_x(T x) { return { x, 0 }; }
        static VectorN unit_y(T y) { return { 0, y }; }
    };

    template <class T> typename VectorN<2, T>::float_type VectorN<2, T>::zcross(const VectorN& other) const { return x * other.y - y * other.x; }
    template <class T> typename VectorN<2, T>::float_type VectorN<2, T>::slope() const { return (float_type)y / (float_type)x; }
    template <class T> typename VectorN<2, T>::float_type VectorN<2, T>::angle() const { return std::atan2((float_type)y, (float_type)x); }
    template <class T> typename VectorN<2, T>::float_type VectorN<2, T>::angle_signed(const VectorN& other) const { return std::atan2((float_type)(y * other.x - x * other.y), (float_type)this->dot(other)); }
    template <class T> typename VectorN<2, T>::float_vec VectorN<2, T>::polar() const { return { this->len(), angle() }; }
    template <class T> typename VectorN<2, T>::float_vec VectorN<2, T>::from_polar(T r, T theta) { return { r * std::cos(theta), r * std::sin(theta) }; }

    template <class T> VectorN<2, T> VectorN<2, T>::perpend() const requires traits_signed { return { y, -x }; } /* perpendicular vector (rotated 90 deg), (0, 1) -> (1, 0) */
    template <class T> typename VectorN<2, T>::float_vec VectorN<2, T>::rotated(float angle) const {
        float_type sin = std::sin(angle), cos = std::cos(angle);
        return { x * cos - y * sin, x * sin + y * cos };
    }
    template <class T> typename VectorN<2, T>::float_vec VectorN<2, T>::rotated(float angle, const VectorN& origin) const { return (*this - origin).rotated(angle) + (float_vec)origin; }
    template <class T> VectorN<2, T> VectorN<2, T>::reflected(const VectorN& normal) const { return *this - 2 * this->dot(normal) * normal; }
    template <class T> VectorN<2, T> VectorN<2, T>::projected(const VectorN& axis) const { return axis * this->dot(axis) / axis.lensq(); }
#pragma endregion
#pragma region Vec3
    template <class T>
    struct VectorN<3, T> : public details::vecn_base<3, T> {
        using base = details::vecn_base<3, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = VectorN<dimension, float_type>;
        T x; T y; T z;

        VectorN(T s = 0) : x(s), y(s), z(s) {}
        VectorN(T x, T y, T z) : x(x), y(y), z(z) {}
        VectorN(std::convertible_to<T> auto x, std::convertible_to<T> auto y,
             std::convertible_to<T> auto z) : x((T)x), y((T)y), z((T)z) {}

        static constexpr Str params = "xyz";

        static VectorN RIGHT()                       { return {  1,  0,  0 }; }
        static VectorN LEFT() requires traits_signed { return { -1,  0,  0 }; }
        static VectorN UP()                          { return {  0,  1,  0 }; }
        static VectorN DOWN() requires traits_signed { return {  0, -1,  0 }; }
        static VectorN FRONT()                       { return {  0,  0,  1 }; }
        static VectorN BACK() requires traits_signed { return {  0,  0, -1 }; }
        static VectorN ZERO()                        { return {  0,  0,  0 }; }
        static VectorN ONE()                         { return {  1,  1,  1 }; }

        NODISC VectorN cross(const VectorN& other) const;

        NODISC float_type angle_signed(const VectorN& other, const VectorN& normal) const;

        NODISC float_type yaw() const;
        NODISC float_type pitch() const;
        NODISC float_vec spheric() const;
        static float_vec from_spheric(T r, T yaw, T pitch);

        NODISC float_vec rotated_by(const Quaternion& rotation) const;
        NODISC Vector2<float_type> projected() const;
        NODISC VectorN reflected(const VectorN& normal) const;
        NODISC VectorN transformed_by(const Transform3D& transform) const;

        VectorN& rotate_by(const Quaternion& rotation) requires traits_float;
        VectorN& reflect(const VectorN& normal) { return *this = reflected(normal); }
        VectorN& transform_by(const Transform3D& transform);

        NODISC auto to_color(T alpha = 1) const -> std::conditional_t<traits_float, fColor, Color>;
        NODISC auto to_color3() const -> std::conditional_t<traits_float, fColor3, Color3>;

        NODISC Vector2<T> xy() const { return { x, y }; }
        NODISC Vector4<T> with_w(T w = 1) const { return { x, y, z, w }; }

        NODISC std::tuple<T, T, T> tup() const { return { x, y, z }; }

        static VectorN unit_x(T x) { return { x, 0, 0 }; }
        static VectorN unit_y(T y) { return { 0, y, 0 }; }
        static VectorN unit_z(T z) { return { 0, 0, z }; }
    };

    template <class T> Vector3<T> VectorN<3, T>::cross(const VectorN& other) const { return { (T)(y * other.z - z * other.y), (T)(z * other.x - x * other.z), (T)(x * other.y - y * other.x) }; }

    template <class T> typename VectorN<3, T>::float_type VectorN<3, T>::yaw() const { return std::atan2((float_type)x, (float_type)z); }
    template <class T> typename VectorN<3, T>::float_type VectorN<3, T>::pitch() const { return std::asin((float_type)y / this->len()); }
    template <class T> typename VectorN<3, T>::float_vec VectorN<3, T>::spheric() const { return { this->len(), yaw(), pitch() }; }
    template <class T> typename VectorN<3, T>::float_vec VectorN<3, T>::from_spheric(T r, T yaw, T pitch) { return (float_type)r * float_vec { std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch) }; }

    template <class T> typename VectorN<3, T>::float_type VectorN<3, T>::angle_signed(const VectorN& other, const VectorN& normal) const { return std::atan2((float_type)cross(other).dot(normal), (float_type)this->dot(other)); }

    template <class T> Vector2<typename VectorN<3, T>::float_type> VectorN<3, T>::projected() const { return (Vector2<float_type>)xy() / (float_type)z; }
    template <class T> Vector3<T> VectorN<3, T>::reflected(const VectorN& normal) const { return *this - 2 * this->dot(normal) * normal; }
#pragma endregion
#pragma region Vec4
    template <class T>
    struct VectorN<4, T> : details::vecn_base<4, T> {
        using base = details::vecn_base<4, T>;
        using base::dimension, base::traits_float, base::traits_signed;
        using float_type = typename base::float_type;
        using float_vec = VectorN<dimension, float_type>;
        T x; T y; T z; T w;

        VectorN(T s = 0, T w = 0) : x(s), y(s), z(s), w(w) {}
        VectorN(T x, T y, T z, T w = 1) : x(x), y(y), z(z), w(w) {}
        VectorN(std::convertible_to<T> auto x, std::convertible_to<T> auto y,
                std::convertible_to<T> auto z, std::convertible_to<T> auto w = 1) : x((T)x), y((T)y), z((T)z), w((T)w) {}

        static constexpr Str params = "xyzw";

        static constexpr VectorN RIGHT()                       { return {  1,  0,  0,  0 }; }
        static constexpr VectorN LEFT() requires traits_signed { return { -1,  0,  0,  0 }; }
        static constexpr VectorN UP()                          { return {  0,  1,  0,  0 }; }
        static constexpr VectorN DOWN() requires traits_signed { return {  0, -1,  0,  0 }; }
        static constexpr VectorN FRONT()                       { return {  0,  0,  1,  0 }; }
        static constexpr VectorN BACK() requires traits_signed { return {  0,  0, -1,  0 }; }
        static constexpr VectorN IN()                          { return {  0,  0,  0,  1 }; }
        static constexpr VectorN OUT()  requires traits_signed { return {  0,  0,  0, -1 }; }
        static constexpr VectorN ZERO()                        { return {  0,  0,  0,  0 }; }
        static constexpr VectorN ONE()                         { return {  1,  1,  1,  1 }; }

        NODISC Vector3<T> xyz() const { return { x, y, z }; }
        NODISC Vector2<T> xy() const { return { x, y }; }

        NODISC Vector3<float_type> projected() const;

        NODISC auto to_color() const -> std::conditional_t<traits_float, fColor, Color>;

        NODISC std::tuple<T, T, T, T> tup() const { return { x, y, z, w }; }

        static VectorN unit_x(T x) { return { x, 0, 0, 0 }; }
        static VectorN unit_y(T y) { return { 0, y, 0, 0 }; }
        static VectorN unit_z(T z) { return { 0, 0, z, 0 }; }
        static VectorN unit_w(T w) { return { 0, 0, 0, w }; }
    };

    template <class T> Vector3<typename VectorN<4, T>::float_type> VectorN<4, T>::projected() const { return (Vector3<float_type>)xyz() / (float_type)w; }
#pragma endregion
#pragma endregion
#undef NODISC
}

#pragma region Formatting
#include "Format.h"
namespace Quasi::Text {
    template <u32 N, class T>
    struct Formatter<Math::VectorN<N, T>> : Formatter<Array<T, N>> {
        void WriteElement(T x, char s, StringOutput output) const {
            for (const char c : this->elemFormat) {
                if (c == '#') {
                    output(s);
                    continue;
                }
                if (c == '$') {
                    FormatOnto(output, x);
                    continue;
                }
                output(c);
            }
        }
        void FormatTo(const Math::VectorN<N, T>& vec, StringOutput output) {
            output(this->brack);
            for (u32 i = 0; i < N; ++i) {
                WriteElement(vec[i], "xyzw"[i], output);
                if (i < N - 1 || this->trailingComma)
                    output(this->seperator);
            }
            output(this->brack + 1);
        }
    };
}
#pragma endregion
