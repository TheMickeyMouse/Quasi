#pragma once
#include <random>
#include <chrono>

#include "Vector.h"

namespace Quasi::Math {
    // man why doesnt c++ just have a standard random library thats actually easy to use
    struct RandomGenerator {
        inline static std::seed_seq seeder {
            std::random_device {}(),
            (u32)std::chrono::steady_clock::now().time_since_epoch().count()
        };
        std::mt19937 device { GetSeed() };

        template <class T> using int_dist_t = std::uniform_int_distribution<T>;
        template <class T> using real_dist_t = std::uniform_real_distribution<T>;
        using bool_dist_t = std::bernoulli_distribution;

        RandomGenerator() = default;

        static std::seed_seq& GetSeed() { return seeder; }
        void SetSeed(const uint val) { device.seed(val); }
        template <class Sq> void SetSeed(Sq& newSeeder) { device.seed(newSeeder()); }
        void Reseed() { device.seed(seeder); }

        void Discard(std::integral auto num) { device.discard((u64)num); }
        u32 GetRaw() { return device(); }

        template <std::integral I> I Get(
            I min = std::numeric_limits<I>::min(),
            I max = std::numeric_limits<I>::max()) { return GetIncl(min, (I)(max - 1)); }
        template <std::integral I> I GetIncl(
            I min = std::numeric_limits<I>::min(),
            I max = std::numeric_limits<I>::max()) { return int_dist_t<I> { min, max } (device); }

        template <std::floating_point F> F Get(F min = 0, F max = 1)
        { return real_dist_t<F> { min, max } (device); }
        template <std::floating_point F> F GetIncl(F min = 0, F max = 1)
        { return getf(min, std::nextafter(max, std::numeric_limits<F>::max())); }

        char Get    (const char min, const char max) { return (char)Get<i16>(min, max); }
        char GetIncl(const char min, const char max) { return (char)Get<i16>(min, (i16)(max + 1)); }
        byte Get    (const byte min, const byte max) { return (byte)Get<u16>(min, max); }
        byte GetIncl(const byte min, const byte max) { return (byte)Get<u16>(min, (u16)max + 1); }

        template <class N, class M>
        requires std::is_arithmetic_v<N> && std::is_arithmetic_v<M> && (!std::is_same_v<std::remove_cvref_t<N>, std::remove_cvref_t<M>>)
        std::common_type_t<N, M> Get(N min, M max) {
            using common_t = std::common_type_t<N, M>;
            return Get<common_t>((common_t)min, (common_t)max);
        }
        template <class N, class M>
        requires std::is_arithmetic_v<N> && std::is_arithmetic_v<M> && (!std::is_same_v<std::remove_cvref_t<N>, std::remove_cvref_t<M>>)
        std::common_type_t<N, M> GetIncl(N min, M max) {
            using common_t = std::common_type_t<N, M>;
            return GetIncl<common_t>((common_t)min, (common_t)max);
        }

        bool GetBool(std::floating_point auto probability) { return bool_dist_t { probability } (device); }

        template <class F> auto Get(F f) -> decltype(f(0)) { return f(device); }

        template <std::input_iterator It> It Choose(It beg, It end) {
            const auto size = std::distance(beg, end);
            using diff_t = typename std::iterator_traits<It>::difference_type;
            return size ? beg + Get<diff_t>(0, size) : end;
        }

        template <class T>
        T Choose(IList<T> ilist) { return *Choose(ilist.begin(), ilist.end()); }

        template <ArrayLike C> ArrayElement<C>& Choose(C& arr) { return *Choose(std::begin(arr), std::end(arr)); }
        template <ArrayLike C> const ArrayElement<C>& choose(const C& arr) { return *Choose(std::begin(arr), std::end(arr)); }

        template <class T, template <typename> class C = Vec> C<T> Generate(T min, T max, usize size) {
            C<T> arr;
            arr.reserve(size);
            for (usize i = 0; i < size; ++i) arr.emplace_back(Get(min, max));
            return arr;
        }

        template <class T, usize N> Array<T, N> Generate(T min, T max) {
            Array<T, N> arr;
            for (usize i = 0; i < N; ++i) arr[i] = Get(min, max);
            return arr;
        }

        template <class F, template <typename> class C = Vec>
        auto Gen(F f, std::size_t size) -> C<decltype(f(0))> {
            C<decltype(f(0))> arr;
            arr.reserve(size);
            for (usize i = 0; i < size; ++i) arr.emplace_back(Get(f));
            return arr;
        }

        template <class F, usize N> auto Gen(F f) -> Array<decltype(f(0)), N> {
            Array<decltype(f(0)), N> arr;
            for (usize i = 0; i < N; ++i) arr[i] = Get(f);
            return arr;
        }

        template <class T> T& Choose(Span<T> span) { return span[Get<usize>(0, span.size())]; }
    };

    template <u32 N, class T>
    typename details::vecn_base<N, T>::vect details::vecn_base<N, T>::random(RandomGenerator& rg, const RectN<N, T>& range) {
        return [&]<u32... Is>(std::integer_sequence<u32, Is...>) {
            return VectorN<N, T> { rg.Get(range.min[Is], range.max[Is])... };
        }(std::make_integer_sequence<u32, N> {});
    }

    template <u32 N, class T>
    typename details::vecn_base<N, T>::vect details::vecn_base<N, T>::random_on_unit(RandomGenerator& rg) requires traits_float {
        return random(rg, { -1, 1 }).norm(); // also uniform
    }

    template <u32 N, class T>
    typename details::vecn_base<N, T>::vect details::vecn_base<N, T>::random_in_unit(RandomGenerator& rg) requires traits_float {
        return random_on_unit(rg) * std::pow(rg.Get((T)0, (T)1), (T)1 / (T)N); // uniform distribution
    }
}
