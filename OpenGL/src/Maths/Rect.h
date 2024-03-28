#pragma once

#include "Vector.h"

namespace Maths {
    template <uint N, class T> struct rect;
    
    template <class V> struct _rect_origin_inbetween_ {
        using scalar = typename V::scalar;
        static constexpr uint dimension = V::dimension;
        using rect_t = rect<dimension, scalar>;
        V pos;
        [[nodiscard]] rect_t rect(V size) const;
    };

    template <class V> struct _rect_size_inbetween_ { V pos; };

    template <class> struct is_rect_t : std::false_type {};
    template <uint N, class T> struct is_rect_t<rect<N, T>> : std::true_type {};
    template <class T> concept rect_t = is_rect_t<T>::value;

    template <rect_t> struct rect_iter;

#define RECT_OP(OP) \
    template <class U> auto operator OP(U v) const { \
        static_assert(std::is_arithmetic_v<U> || vec_t<U>, "rect::operator" #OP " not supported"); \
        return rect<N, decltype((min OP v).x)> { min OP v, max OP v }; \
    } \
    template <class U> rect& operator OP##=(U v) { return *this = *this + v; }

#define NODISC [[nodiscard]]

    template <class T>
    _rect_size_inbetween_<T> as_size(const T& val) {
        return { val };
    }

    template <class T>
    _rect_origin_inbetween_<T> as_origin(const T& val) {
        return { val };
    }
    
    template <uint N, class T>
    struct rect {
        static uint constexpr dimension = N;
        static bool constexpr is1D = N == 1;
        using scalar = T;
        using vec = vecn<N, T>;
        vec min, max;

        rect() : min(0), max(0) {}
        rect(T min, T max) requires is1D : min(min), max(max) {}
        rect(T minX, T maxX, T minY, T maxY) requires (N == 2) : min(minX, minY), max(maxX, maxY) {}
        rect(T minX, T maxX, T minY, T maxY, T minZ, T maxZ) requires (N == 3) : min(minX, minY, minZ), max(maxX, maxY, maxZ) {}
        rect(T minX, T maxX, T minY, T maxY, T minZ, T maxZ, T minW, T maxW) requires (N == 4) : min(minX, minY, minZ, minW), max(maxX, maxY, maxZ, maxW) {}
        rect(const vec& min, const vec& max) requires !is1D : min(min), max(max) {}
        rect(const vec& min, const _rect_size_inbetween_<vec>& size) : min(min), max(min + size.pos) {}
        rect(const _rect_origin_inbetween_<vec>& origin, const vec& size) { *this = origin.rect(size); }

        NODISC bool operator==(const rect& other) const { return min == other.min && max == other.max; }
        NODISC vec operator[](const int i) const { return corner(i); }
        
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
        NODISC vec corner(int i) const {
            vec result;
            for (uint b = 0; b < N; ++b) {
                result[b] = i & (1 << b) ? max[b] : min[b];
            }
            return result;
        } // each bit is a y/n decision on min or max (0 = min, 1 is max)

        NODISC vec clamp(const vec& val) const { return vec::max(vec::min(val, max), min); }

        rect& correct() { vec m = min; min = vec::min(min, max); max = vec::max(m, max); return *this; } // fixes min max errors
        NODISC rect corrected() const { return { vec::min(min, max), vec::max(min, max) }; }
        
        NODISC bool contains(const rect& other) const { return min < other.min && other.max < max; }
        NODISC bool contains(const vec&  other) const { return min < other && other < max; }
        
        rect& offset(const vec& off) { max += off; min += off; return *this; }
        NODISC rect offseted(const vec& off) const { return { (vec)(min + off), (vec)(max + off) }; }

        NODISC rect expand(const rect& other) const { return { vec::min(min, other.min), vec::max(max, other.max) }; }
        NODISC rect shrink(const rect& other) const { return { vec::min(min, other.min), vec::max(max, other.max) }; }

        NODISC rect inset(T radius)          const { return { (vec)(min + radius), (vec)(max - radius) }; }
        NODISC rect inset(const vec& radius) const { return { (vec)(min + radius), (vec)(max - radius) }; }
        NODISC rect extrude(T radius)          const { return { (vec)(min - radius), (vec)(max + radius) }; }
        NODISC rect extrude(const vec& radius) const { return { (vec)(min - radius), (vec)(max + radius) }; }

        NODISC bool overlaps(const rect& other) const { return (min < other.max) && (max > other.min); }

        NODISC rect_iter<rect> begin() const;
        NODISC rect_iter<rect> end() const;
    };
#undef RECT_OP
    template <rect_t R>
    struct rect_iter {
        using value_t = typename R::vec;
        const R* rect;
        value_t curr;

        rect_iter& incr() {
            ++curr[0];
            int i = 0;
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

        bool operator==(const rect_iter& other) const { return rect == other.rect && curr == other.curr; }
        rect_iter& operator++() { return incr(); }
        const value_t& operator*() const { return curr; }
    };

    // template <class V> rect<_rect_origin_inbetween_<V>::dimension, std::conditional_t<is_vec_v<V>, typename V::scalar, V>>
    // _rect_origin_inbetween_<V>::rect(V size) const {
    //     return { pos + size / 2, pos - size / 2 };
    // }

    template <class V> typename _rect_origin_inbetween_<V>::rect_t
    _rect_origin_inbetween_<V>::rect(V size) const {
        return { (V)(pos + size / 2), (V)(pos - size / 2) };
    }

    template <uint N, class T> rect_iter<rect<N, T>> rect<N, T>::begin() const {
        return { this, min };
    }

    template <uint N, class T> rect_iter<rect<N, T>> rect<N, T>::end() const {
        return { this, max };
    }

    template <uint N, class T>
    typename vecn_base<N, T>::vec_t vecn_base<N, T>::clamp(const rect<N, T>& r, const vec_t& x) {
        return r.clamp(x);
    }

    template <uint N, class T>
    bool vecn_base<N, T>::is_in(const rect<N, T>& region) const {
        return region.min <= as_vec() && as_vec() <= region.max;
    }

    template <uint N, class T> _rect_origin_inbetween_<typename vecn_base<N, T>::vec_t> vecn_base<N, T>::as_origin() const { return { as_vec() }; }
    template <uint N, class T> _rect_size_inbetween_<typename vecn_base<N, T>::vec_t> vecn_base<N, T>::as_size() const { return { as_vec() }; }
    template <uint N, class T> rect<N, T> vecn_base<N, T>::to(const vec_t& other) const { return { as_vec(), other }; }
    template <uint N, class T> rect<N, T> vecn_base<N, T>::to(const _rect_size_inbetween_<vec_t>& other) const { return { as_vec(), other }; }

    template <class T> using range = rect<1, T>;
    template <class T> using rect2 = rect<2, T>;
    template <class T> using rect3 = rect<3, T>;
    template <class T> using rect4 = rect<4, T>;
    using irange = range<int>;
    using urange = range<uint>;
    using brange = range<uchar>;
    using rangez = range<usize>;
    using rangef = range<float>;
    using ranged = range<double>;
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

    template struct rect<1, int>;
    template struct rect<1, uint>;
    template struct rect<1, uchar>;
    template struct rect<1, usize>;
    template struct rect<1, float>;
    template struct rect<1, double>;
    template struct rect<2, float>;
    template struct rect<3, float>;
    template struct rect<4, float>;
    template struct rect<2, double>;
    template struct rect<3, double>;
    template struct rect<4, double>;
    template struct rect<2, int>;
    template struct rect<3, int>;
    template struct rect<4, int>;
    template struct rect<2, uint>;
    template struct rect<3, uint>;
    template struct rect<4, uint>;
    template struct rect<2, uchar>;
    template struct rect<3, uchar>;
    template struct rect<4, uchar>;
}
