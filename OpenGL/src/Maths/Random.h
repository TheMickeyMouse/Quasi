#pragma once
#include <random>
#include <chrono>

#include "NumTypes.h"
#include "opengl.h"

namespace Maths {
    // man why doesnt c++ just have a standard random library thats actually easy to use
    struct random_gen {
        inline static std::seed_seq seeder {
            std::random_device {}(),
            (uint)std::chrono::steady_clock::now().time_since_epoch().count()
        };
        std::mt19937 device { get_seed() };

        template <class T> using int_dist_t = std::uniform_int_distribution<T>;
        template <class T> using real_dist_t = std::uniform_real_distribution<T>;
        using bool_dist_t = std::bernoulli_distribution;

        random_gen() = default;

        static std::seed_seq& get_seed() { return seeder; }
        void set_seed(const uint val) { device.seed(val); }
        template <class Sq> void set_seed(Sq& seeder) { device.seed(seeder()); }
        void reseed() { device.seed(seeder); }

        void discard(std::integral auto num) { device.discard((uint64)num); }
        uint get_raw() { return device(); }

        template <std::integral I> I geti(
            I min = std::numeric_limits<I>::min(),
            I max = std::numeric_limits<I>::max()) { return geti_inclusive(min, (I)(max - 1)); }
        template <std::integral I> I geti_inclusive(
            I min = std::numeric_limits<I>::min(),
            I max = std::numeric_limits<I>::max()) { return int_dist_t<I> { min, max } (device); }

        template <std::floating_point F> F getf(F min = 0, F max = 1)
        { return real_dist_t<F> { min, max } (device); }
        template <std::floating_point F> F getf_inclusive(F min = 0, F max = 1)
        { return getf(min, std::nextafter(max, std::numeric_limits<F>::max())); }

        char getb(const char min, const char max) { return (char)geti<short>(min, max); }
        char getb_inclusive(const char min, const char max) { return (char)geti<short>(min, (short)(max + 1)); }
        uchar getb(const uchar min, const uchar max) { return (uchar)geti<uint16>(min, max); }
        uchar getb_inclusive(const uchar min, const uchar max) { return (uchar)geti<uint16>(min, (uint16)max + 1); }

        template <class N, class M> requires std::is_arithmetic_v<N> && std::is_arithmetic_v<M>
        std::common_type_t<N, M> getn(N min, M max) {
            using common_t = std::common_type_t<N, M>;
            if constexpr (std::is_integral_v<common_t>) return geti<common_t>((common_t)min, (common_t)max);
            else return getf<common_t>((common_t)min, (common_t)max);
        }
        template <class N, class M> requires std::is_arithmetic_v<N> && std::is_arithmetic_v<M>
        std::common_type_t<N, M> getn_inclusive(N min, M max) {
            using common_t = std::common_type_t<N, M>;
            if constexpr (std::is_integral_v<common_t>) return geti_inclusive<common_t>((common_t)min, (common_t)max);
            else return getf_inclusive<common_t>((common_t)min, (common_t)max);
        }

        bool getprob(std::floating_point auto probability) { return bool_dist_t { probability } (device()); }

        template <class F> auto get(F f) -> decltype(f(0)) { return f(device); }

        template <std::input_iterator It> It choose(It beg, It end) {
            const auto size = std::distance(beg, end);
            using diff_t = typename std::iterator_traits<It>::difference_type;
            return size ? beg + geti<diff_t>(0, size) : end;
        }

        template <class T>
        T choose(std::initializer_list<T> ilist) { return *choose(ilist.begin(), ilist.end()); }

        template <class C> typename C::value_type& choose(C& arr)
        { return *choose(std::begin(arr), std::end(arr)); }
        template <class C> const typename C::value_type& choose(const C& arr)
        { return *choose(std::begin(arr), std::end(arr)); }

        template <class T, template <typename> class C = std::vector> C<T> gen(T min, T max, std::size_t size) {
            C<T> arr;
            arr.reserve(size);
            for (std::size_t i = 0; i < size; ++i) arr.emplace_back(getn(min, max));
            return arr;
        }

        template <class T, std::size_t N> std::array<T, N> gen(T min, T max) {
            std::array<T, N> arr;
            for (std::size_t i = 0; i < N; ++i) arr[i] = getn(min, max);
            return arr;
        }

        template <class F, template <typename> class C = std::vector>
        auto gen(F f, std::size_t size) -> C<decltype(f(0))> {
            C<decltype(f(0))> arr;
            arr.reserve(size);
            for (std::size_t i = 0; i < size; ++i) arr.emplace_back(get(f));
            return arr;
        }

        template <class F, std::size_t N> auto gen(F f) -> std::array<decltype(f(0)), N> {
            std::array<decltype(f(0)), N> arr;
            for (std::size_t i = 0; i < N; ++i) arr[i] = get(f);
            return arr;
        }

        template<typename T, std::size_t N>
        static T* get(T (&arr)[N]) {
            return arr + geti<std::size_t>(0, N);
        }
    };
}
