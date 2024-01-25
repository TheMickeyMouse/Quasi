#pragma once
namespace Maths {
    template <vec_t V> struct _rect_origin_inbetween_ {
        V pos;
        rect<V::dimension, typename V::scalar> rect(V size) const;
    };

    template <vec_t V> struct _rect_size_inbetween_ { V pos; };

    template <class> struct is_rect_t : std::false_type {};
    template <int N, class T> struct is_rect_t<rect<N, T>> : std::true_type {};
    template <class T> constexpr bool is_rect_v = is_rect_t<T>::value;
    template <class T> concept rect_t = is_rect_v<T>;

    template <rect_t> struct rect_iter;

#define ARITH(T, U) stdu::arithmetic_t<T, U>
#define ARITH_T(T, U, O) typename ARITH(T, U)::O##_t

#define RECT_OP(M, OP) \
    template <class U> auto operator OP(U v) const { \
        static_assert(std::is_arithmetic_v<U> || is_vec_v<U>, "rect::" #M  " not supported"); \
        if constexpr (std::is_arithmetic_v<U>) return rect<N, ARITH_T(T, U, M)> { min OP v, max OP v }; \
        else if constexpr (is_vec_v<U>) return rect<N, ARITH_T(T, typename U::scalar, M)> { min OP v, max OP v }; \
    }
    
    template <int N, class T>
    struct rect { 
        using vec = typename vecn<N, T>::type;
        using scalar = typename vec::scalar;
        static int constexpr dimension = N;
        vec min, max;

        rect() : min(0), max(0) {}
        rect(const vec& min, const vec& max) : min(min), max(max) {}
        rect(const vec& min, const _rect_size_inbetween_<vec>& size) : min(min), max(min + size.pos) {}
        rect(const _rect_origin_inbetween_<vec>& origin, const vec& size) { *this = origin.rect(size); }

        bool operator==(const rect& other) const { return min == other.min && max == other.max; }
        bool operator[](const int i) const { return corner(i); }
        
        RECT_OP(add, +)
        RECT_OP(sub, -)
        RECT_OP(mul, *)
        RECT_OP(div, /)
        
        vec size()   const { return  max - min; }
        vec center() const { return (max + min) / 2; }
        vec corner(int i) const {
            int b = 0;
            return min.apply(
                [&b, i](scalar x, scalar y) { return 1 << b++ & i ? y : x; },
                max);
        } // each bit is a y/n decision on min or max (0 = min, 1 is max)

        rect& correct() { vec m = min; min = vec::min(min, max); max = vec::max(m, max); return *this; } // fixes min max errors
        rect corrected() { return { vec::min(min, max), vec::max(min, max) }; }
        
        bool contains(const rect& other) const { return min < other.min && other.max < max; }
        bool contains(const vec&  other) const { return min < other && other < max; }
        
        rect& offset  (const vec& off) { max += off; min += off; return *this; }
        rect  offseted(const vec& off) const { return { min + off, max + off }; }

        rect expand(const rect& other) const { return { std::min(min, other.min), std::max(max, other.max) }; }
        rect shrink(const rect& other) const { return { std::min(min, other.min), std::max(max, other.max) }; }

        rect_iter<rect> begin() const;
        rect_iter<rect> end() const;
    };
#undef ARITH
#undef ARITH_T
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

    template <vec_t V> rect<V::dimension, typename V::scalar>
    _rect_origin_inbetween_<V>::rect(V size) const {
        return { pos + size / 2, pos - size / 2 };
    }

    template <int N, class T> rect_iter<rect<N, T>> rect<N, T>::begin() const {
        return { this, min };
    }

    template <int N, class T> rect_iter<rect<N, T>> rect<N, T>::end() const {
        return { this, max };
    }
}
