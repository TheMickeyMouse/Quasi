#pragma once
#include <random>
#include <chrono>

#include "Quaternion.h"
#include "Complex.h"
#include "Constants.h"

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
        void SetSeed(const u32 val) { device.seed(val); }
        template <class Sq> void SetSeed(Sq& newSeeder) { device.seed(newSeeder()); }
        void Reseed() { device.seed(seeder); }

        void Discard(Integer auto num) { device.discard((u64)num); }
        u32 GetRaw() { return device(); }

        template <Integer I> I Get(
            I min = std::numeric_limits<I>::min(),
            I max = std::numeric_limits<I>::max()) { return GetIncl(min, (I)(max - 1)); }
        template <Integer I> I GetIncl(
            I min = std::numeric_limits<I>::min(),
            I max = std::numeric_limits<I>::max()) { return int_dist_t<I> { min, max } (device); }

        template <Floating F> F Get(F min = 0, F max = 1)
        { return real_dist_t<F> { min, max } (device); }
        template <Floating F> F GetIncl(F min = 0, F max = 1)
        { return getf(min, std::nextafter(max, std::numeric_limits<F>::max())); }

        template <Floating F> F GetLogarithmic(F min = 0, F max = 1)
        { return std::log(Get(std::exp(min), std::exp(max))); }
        template <Floating F> F GetLogarithmicIncl(F min = 0, F max = 1)
        { return std::log(GetIncl(std::exp(min), std::exp(max))); }

        template <Floating F> F GetExponential(F min = 0, F max = 1)
        { return std::exp(Get(std::log(min), std::log(max))); }
        template <Floating F> F GetExponentialIncl(F min = 0, F max = 1)
        { return std::exp(GetIncl(std::log(min), std::log(max))); }

        char Get    (const char min, const char max) { return (char)Get<i16>(min, max); }
        char GetIncl(const char min, const char max) { return (char)Get<i16>(min, (i16)(max + 1)); }
        byte Get    (const byte min, const byte max) { return (byte)Get<u16>(min, max); }
        byte GetIncl(const byte min, const byte max) { return (byte)Get<u16>(min, (u16)max + 1); }

        template <class N, class M>
        requires Numeric<N> && Numeric<M> && DistantTo<N, M>
        Common<N, M> Get(N min, M max) {
            using T = Common<N, M>;
            return Get<T>((T)min, (T)max);
        }
        template <class N, class M>
        requires Numeric<N> && Numeric<M> && DistantTo<N, M>
        Common<N, M> GetIncl(N min, M max) {
            using T = Common<N, M>;
            return GetIncl<T>((T)min, (T)max);
        }

        bool GetBool(std::floating_point auto probability = 0.5f) { return bool_dist_t { probability } (device); }

        template <class F> auto Get(F f) -> decltype(f(0)) { return f(device); }

        template <class T>
        T Choose(IList<T> ilist) { return Choose(Spans::FromIList(ilist)); }

        template <ContinuousCollectionAny C>
        CollectionItem<C>&       Choose(C& arr)       { return arr[Get(0, arr.Length())]; }
        template <ContinuousCollectionAny C>
        const CollectionItem<C>& Choose(const C& arr) { return arr[Get(0, arr.Length())]; }

        template <class T, template <typename> class C = Vec> C<T> Generate(T min, T max, usize size) {
            C arr = C<T>::WithCap(size);
            for (usize i = 0; i < size; ++i) arr.Push(Get(min, max));
            return arr;
        }

        template <class T, usize N> Array<T, N> Generate(T min, T max) {
            Array<T, N> arr;
            for (usize i = 0; i < N; ++i) arr[i] = Get(min, max);
            return arr;
        }

        template <class F, template <typename> class C = Vec>
        auto Gen(F f, usize size) -> C<decltype(f(0))> {
            C arr = C<decltype(f(0))>::WithCap(size);
            for (usize i = 0; i < size; ++i) arr.Push(Get(f));
            return arr;
        }

        template <class F, usize N> auto Gen(F f) -> Array<decltype(f(0)), N> {
            Array<decltype(f(0)), N> arr;
            for (usize i = 0; i < N; ++i) arr[i] = Get(f);
            return arr;
        }

        template <class T> T& Choose(Span<T> span) { return span[Get<usize>(0, span.Length())]; }
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

    template <class T> Complex<T> Complex<T>::random_rot(RandomGenerator& rg) {
        return rotate(rg.Get(0, TAU));
    }

    inline Quaternion Quaternion::random_rot(RandomGenerator& rg) {
        const auto [u, v, w] = fVector3::random(rg, { 0, 1 });
        return { std::sqrt(1 - u) * std::sin(TAU * v), std::sqrt(1 - u) * std::cos(TAU * v),
                 std::sqrt(u)     * std::sin(TAU * w), std::sqrt(u)     * std::sin(TAU * w) };
    }
}
