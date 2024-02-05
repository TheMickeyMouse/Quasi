#pragma once
namespace Maths {
    template <int N, class T> struct rect;
    
    template <class V> struct _rect_origin_inbetween_ {
        static auto _scalar_type_t() { if constexpr (is_vec_v<V>) return typename V::scalar {}; else return V {}; }
        using scalar = decltype(_scalar_type_t());
        static constexpr int _dimension_num() { if constexpr (is_vec_v<V>) return V::dimension; else return 1; }
        static constexpr int dimension = _dimension_num();
        using rect_t = rect<dimension, scalar>;
        V pos;
        rect_t rect(V size) const;
    };

    template <class V> struct _rect_size_inbetween_ { V pos; };

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
    } \
    template <class U> rect& operator OP##=(U v) { return *this = *this + v; }

    template <class T>
    _rect_size_inbetween_<T> as_size(T val) {
        return { val };
    }

    template <class T>
    _rect_origin_inbetween_<T> as_origin(T val) {
        return { val };
    }

    template <class T>
    T min_t(T a, T b) {
        if constexpr (is_vec_v<T>) return T::min(a, b);
        else return std::min(a, b);
    }

    template <class T>
    T max_t(T a, T b) {
        if constexpr (is_vec_v<T>) return T::max(a, b);
        else return std::max(a, b);
    }
    
    template <int N, class T>
    struct rect {
        static int constexpr dimension = N;
        static bool constexpr is1D = N == 1;
        using scalar = T;
        using vec = std::conditional_t<is1D, T, typename vecn<N, T>::type>;
        vec min, max;

        rect() : min(0), max(0) {}
        rect(T min, T max) requires is1D : min(min), max(max) {}
        rect(T minX, T maxX, T minY, T maxY) requires (N == 2) : min(minX, minY), max(maxX, maxY) {}
        rect(T minX, T maxX, T minY, T maxY, T minZ, T maxZ) requires (N == 3) : min(minX, minY, minZ), max(maxX, maxY, maxZ) {}
        rect(T minX, T maxX, T minY, T maxY, T minZ, T maxZ, T minW, T maxW) requires (N == 4) : min(minX, minY, minZ, minW), max(maxX, maxY, maxZ, maxW) {}
        rect(const vec& min, const vec& max) requires !is1D : min(min), max(max) {}
        rect(const vec& min, const _rect_size_inbetween_<vec>& size) : min(min), max(min + size.pos) {}
        rect(const _rect_origin_inbetween_<vec>& origin, const vec& size) { *this = origin.rect(size); }

        bool operator==(const rect& other) const { return min == other.min && max == other.max; }
        bool operator[](const int i) const { return corner(i); }
        
        RECT_OP(add, +)
        RECT_OP(sub, -)
        RECT_OP(mul, *)
        RECT_OP(div, /)
        
        scalar n_distance(int n) const { return max[n] - min[n]; }
        scalar width()    const { return n_distance(0); }
        scalar height()   const requires (N >= 2) { return n_distance(1); }
        scalar depth()    const requires (N >= 3) { return n_distance(2); }
        scalar duration() const requires (N >= 4) { return n_distance(3); }

        scalar n_volume() const {
            static_assert(2 <= N && N <= 4, "invalid dimension");
            if constexpr (N == 1) return width();
            else if constexpr (N == 2) return width() * height();
            else if constexpr (N == 3) return width() * height() * depth();
            else if constexpr (N == 4) return width() * height() * depth() * duration();
            else return 0;
        }
        scalar area()        const requires (N == 2) { return n_volume(); }
        scalar volume()      const requires (N == 3) { return n_volume(); }
        scalar hypervolume() const requires (N == 4) { return n_volume(); }
        
        vec size()   const { return  max - min; }
        vec center() const { return (max + min) / 2; }
        vec corner(int i) const {
            if constexpr (is1D) return i ? max : min;
            else {
                int b = 0;
                return min.apply(
                    [&b, i](scalar x, scalar y) { return 1 << b++ & i ? y : x; },
                    max);
            }
        } // each bit is a y/n decision on min or max (0 = min, 1 is max)

        rect& correct() { vec m = min; min = Maths::min_t(min, max); max = Maths::max_t(m, max); return *this; } // fixes min max errors
        rect corrected() { return { Maths::min_t(min, max), Maths::max_t(min, max) }; }
        
        bool contains(const rect& other) const { return min < other.min && other.max < max; }
        bool contains(const vec&  other) const { return min < other && other < max; }
        
        rect& offset  (const vec& off) { max += off; min += off; return *this; }
        rect  offseted(const vec& off) const { return { min + off, max + off }; }

        rect expand(const rect& other) const { return { Maths::min_t(min, other.min), Maths::max_t(max, other.max) }; }
        rect shrink(const rect& other) const { return { Maths::min_t(min, other.min), Maths::max_t(max, other.max) }; }

        rect inset(T radius)          const { return { min + radius, max - radius }; }
        rect inset(const vec& radius) const { return { min + radius, max - radius }; }
        rect extrude(T radius)          const { return { min - radius, max + radius }; }
        rect extrude(const vec& radius) const { return { min - radius, max + radius }; }

        rect_iter<rect> begin() const;
        rect_iter<rect> end() const;
    };
#undef RECT_OP
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

    // template <class V> rect<_rect_origin_inbetween_<V>::dimension, std::conditional_t<is_vec_v<V>, typename V::scalar, V>>
    // _rect_origin_inbetween_<V>::rect(V size) const {
    //     return { pos + size / 2, pos - size / 2 };
    // }

    template <class V> typename _rect_origin_inbetween_<V>::rect_t
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
