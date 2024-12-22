#pragma once

#include "Vector.h"

namespace Quasi::Math {
    template <u32 N, class T> struct RectN;

    namespace details {
        template <class V> struct rect_origin_t {
            using scalar = typename V::scalar;
            static constexpr u32 dimension = V::dimension;
            using rect_t = RectN<dimension, scalar>;
            V pos;
            rect_t rect(const V& size) const;
        };

        template <class V> struct rect_size_t { V pos; };
    }

    template <class> struct IsRectType : std::false_type {};
    template <u32 N, class T> struct IsRectType<RectN<N, T>> : std::true_type {};
    template <class T> concept RectLike = IsRectType<T>::value;

    template <RectLike> struct RectIter;

#define RECT_OP(OP) \
    template <class U> auto operator OP(U v) const { \
        static_assert(std::is_arithmetic_v<U> || IVector<U>, "rect::operator" #OP " not supported"); \
        return RectN<N, decltype((min OP v).x)> { min OP v, max OP v }; \
    } \
    template <class U> RectN& operator OP##=(U v) { return *this = *this OP v; }

#define NODISC [[nodiscard]]

    namespace details {
        template <class T>
        rect_size_t<T> as_size(const T& val) {
             return { val };
         }

        template <class T>
        rect_origin_t<T> as_origin(const T& val) {
            return { val };
        }
    }
    
    template <u32 N, class T>
    struct RectN {
        static u32 constexpr dimension = N;
        static bool constexpr is1D = N == 1;
        using scalar = T;
        using vec = VectorN<N, T>;
        vec min, max;

        RectN() : min(0), max(0) {}
        RectN(T min, T max) requires is1D : min(min), max(max) {}
        RectN(T minX, T maxX, T minY, T maxY) requires (N == 2) : min(minX, minY), max(maxX, maxY) {}
        RectN(T minX, T maxX, T minY, T maxY, T minZ, T maxZ) requires (N == 3) : min(minX, minY, minZ), max(maxX, maxY, maxZ) {}
        RectN(T minX, T maxX, T minY, T maxY, T minZ, T maxZ, T minW, T maxW) requires (N == 4) : min(minX, minY, minZ, minW), max(maxX, maxY, maxZ, maxW) {}
        RectN(const vec& min, const vec& max) requires (!is1D) : min(min), max(max) {}
        RectN(const vec& min, const details::rect_size_t<vec>& size) : min(min), max(min + size.pos) {}
        RectN(const details::rect_origin_t<vec>& origin, const vec& size) { *this = origin.rect(size); }

        static RectN empty() { return {}; }
        static RectN whole() { return { std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max() }; }
        static RectN unrange() { return { std::numeric_limits<T>::max(), std::numeric_limits<T>::lowest() }; }
        static RectN at(const vec& point) { return { point, point }; }
        static RectN over(const Collection<vec> auto& nums);

        NODISC bool operator==(const RectN& other) const { return min == other.min && max == other.max; }
        NODISC vec operator[](const usize i) const { return corner(i); }
        
        RECT_OP(+)
        RECT_OP(-)
        RECT_OP(*)
        RECT_OP(/)
        
        NODISC scalar n_distance(int n) const { return max[n] - min[n]; }
        NODISC scalar width()    const { return n_distance(0); }
        NODISC scalar height()   const requires (N >= 2) { return n_distance(1); }
        NODISC scalar depth()    const requires (N >= 3) { return n_distance(2); }
        NODISC scalar duration() const requires (N >= 4) { return n_distance(3); }

        NODISC scalar n_volume() const {
            static_assert(1 <= N && N <= 4, "invalid dimension");
            if constexpr (N == 1) return width();
            else if constexpr (N == 2) return width() * height();
            else if constexpr (N == 3) return width() * height() * depth();
            else if constexpr (N == 4) return width() * height() * depth() * duration();
            else return 0;
        }
        NODISC scalar area()        const requires (N == 2) { return n_volume(); }
        NODISC scalar volume()      const requires (N == 3) { return n_volume(); }
        NODISC scalar hypervolume() const requires (N == 4) { return n_volume(); }
        
        NODISC vec size()   const { return (vec)(max - min); }
        NODISC vec center() const { return (vec)((max + min) / 2); }
        NODISC vec corner(usize i) const {
            vec result;
            for (uint b = 0; b < N; ++b) {
                result[b] = i & (1 << b) ? max[b] : min[b];
            }
            return result;
        } // each bit is a y/n decision on min or max (0 = min, 1 is max)

        NODISC vec clamp(const vec& val) const { return vec::max(vec::min(val, max), min); }

        RectN& correct() { vec m = min; min = vec::min(min, max); max = vec::max(m, max); return *this; } // fixes min max errors
        NODISC RectN corrected() const { return { vec::min(min, max), vec::max(min, max) }; }
        
        NODISC bool contains(const RectN& other) const { return min < other.min && other.max < max; }
        NODISC bool contains(const vec&   other) const { return min < other && other < max; }
        
        RectN& offset(const vec& off) { max += off; min += off; return *this; }
        NODISC RectN offseted(const vec& off) const { return { (vec)(min + off), (vec)(max + off) }; }

        NODISC RectN expand(const RectN& other)     const { return { vec::min(min, other.min), vec::max(max, other.max) }; }
        NODISC RectN expand_until(const vec& other) const { return { vec::min(min, other),     vec::max(max, other)     }; }
        NODISC RectN intersect(const RectN& other)  const { return { vec::max(min, other.min), vec::min(max, other.max) }; }

        NODISC RectN inset  (T radius)          const { return { (vec)(min + radius), (vec)(max - radius) }; }
        NODISC RectN inset  (const vec& radius) const { return { (vec)(min + radius), (vec)(max - radius) }; }
        NODISC RectN extrude(T radius)          const { return { (vec)(min - radius), (vec)(max + radius) }; }
        NODISC RectN extrude(const vec& radius) const { return { (vec)(min - radius), (vec)(max + radius) }; }

        NODISC bool overlaps(const RectN& other) const { return (min < other.max) && (max > other.min); }
        NODISC vec  overlap (const RectN& other) const { return vec::max(max - other.min, other.max - min); }

        NODISC RectN<1, scalar> xrange() const requires (N >= 1) { return { min.x, max.x }; }
        NODISC RectN<1, scalar> yrange() const requires (N >= 2) { return { min.y, max.y }; }
        NODISC RectN<1, scalar> zrange() const requires (N >= 3) { return { min.z, max.z }; }
        NODISC RectN<1, scalar> wrange() const requires (N >= 4) { return { min.w, max.w }; }

        NODISC RectIter<RectN> begin() const;
        NODISC RectIter<RectN> end() const;
    };

    template <class T, u32 D, u32 N>
    RectN<N - 1, T> operator%(const RectN<D, T>& rect, const char (&swizz)[N]) {
        return { rect.min % swizz, rect.max % swizz };
    }

#undef RECT_OP
    template <RectLike R>
    struct RectIter {
        using value_t = typename R::vec;
        const R* rect;
        value_t curr;

        RectIter& incr() {
            ++curr[0];
            u32 i = 0;
            while (i < R::dimension - 1) {
                if (curr[i] >= rect->max[i]) {
                    curr[i] = rect->min[i];
                    ++curr[++i];
                    continue;
                }
                return *this;
            }
            return *this;
        }

        NODISC bool operator==(const RectIter& other) const { return rect == other.rect && curr == other.curr; }
        RectIter& operator++() { return incr(); }
        NODISC const value_t& operator*() const { return curr; }
    };

    template <class V> typename details::rect_origin_t<V>::rect_t
    details::rect_origin_t<V>::rect(const V& size) const {
        return { (V)(pos + size / 2), (V)(pos - size / 2) };
    }

    template <u32 N, class T> RectN<N, T> RectN<N, T>::over(const Collection<VectorN<N, T>> auto& nums) {
        RectN r = unrange();
        for (const auto& v : nums) r = r.expand_until(v);
        return r;
    }

    template <u32 N, class T> RectIter<RectN<N, T>> RectN<N, T>::begin() const {
        return { this, min };
    }

    template <u32 N, class T> RectIter<RectN<N, T>> RectN<N, T>::end() const {
        return { this, max };
    }

    template <u32 N, class T>
    typename details::vecn_base<N, T>::vect details::vecn_base<N, T>::clamp(const RectN<N, T>& r, const vect& x) {
        return r.clamp(x);
    }

    template <u32 N, class T>
    bool details::vecn_base<N, T>::is_in(const RectN<N, T>& region) const {
        return region.min <= as_vec() && as_vec() <= region.max;
    }

    template <u32 N, class T>
    typename details::vecn_base<N, T>::vect details::vecn_base<N, T>::map(const RectN<N, T>& input, const RectN<N, T>& output) const {
        return (as_vec() - input.min) / input.size() * output.size() + output.min;
    }

    template <u32 N, class T> details::rect_origin_t<typename details::vecn_base<N, T>::vect> details::vecn_base<N, T>::as_origin() const { return { as_vec() }; }
    template <u32 N, class T> details::rect_size_t<typename details::vecn_base<N, T>::vect> details::vecn_base<N, T>::as_size() const { return { as_vec() }; }
    template <u32 N, class T> RectN<N, T> details::vecn_base<N, T>::to(const vect& other) const { return { as_vec(), other }; }
    template <u32 N, class T> RectN<N, T> details::vecn_base<N, T>::to(const rect_size_t<vect>& other) const { return { as_vec(), other }; }

    template <class T> using Range  = RectN<1, T>;
    template <class T> using Rect2D = RectN<2, T>;
    template <class T> using Rect3D = RectN<3, T>;
    template <class T> using Rect4D = RectN<4, T>;
    using iRange = Range<int>;
    using uRange = Range<uint>;
    using bRange = Range<byte>;
    using zRange = Range<usize>;
    using fRange = Range<float>;
    using dRange = Range<double>;
    using fRect2D = Rect2D<float>;
    using fRect3D = Rect3D<float>;
    using fRect4D = Rect4D<float>;
    using dRect2D = Rect2D<double>;
    using dRect3D = Rect3D<double>;
    using dRect4D = Rect4D<double>;
    using iRect2D = Rect2D<int>;
    using iRect3D = Rect3D<int>;
    using iRect4D = Rect4D<int>;
    using uRect2D = Rect2D<uint>;
    using uRect3D = Rect3D<uint>;
    using uRect4D = Rect4D<uint>;
    using bRect2D = Rect2D<byte>;
    using bRect3D = Rect3D<byte>;
    using bRect4D = Rect4D<byte>;

    template struct RectN<1, int>;
    template struct RectN<1, uint>;
    template struct RectN<1, byte>;
    template struct RectN<1, usize>;
    template struct RectN<1, float>;
    template struct RectN<1, double>;
    template struct RectN<2, float>;
    template struct RectN<3, float>;
    template struct RectN<4, float>;
    template struct RectN<2, double>;
    template struct RectN<3, double>;
    template struct RectN<4, double>;
    template struct RectN<2, int>;
    template struct RectN<3, int>;
    template struct RectN<4, int>;
    template struct RectN<2, uint>;
    template struct RectN<3, uint>;
    template struct RectN<4, uint>;
    template struct RectN<2, byte>;
    template struct RectN<3, byte>;
    template struct RectN<4, byte>;
}
