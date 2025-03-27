#pragma once
#include "Numeric.h"

namespace Quasi {
#pragma region Indexing
    template <usize N> struct GetTupleElement {
        template <class T, class U, class V, class W, class... Rest>
        using Result = typename GetTupleElement<N - 4>::template Result<Rest...>;
    };

    template <> struct GetTupleElement<0> {
        template <class T, class...> using Result = T;
    };
    template <> struct GetTupleElement<1> {
        template <class T, class U, class...> using Result = U;
    };
    template <> struct GetTupleElement<2> {
        template <class T, class U, class V, class...> using Result = V;
    };
    template <> struct GetTupleElement<3> {
        template <class T, class U, class V, class W, class...> using Result = W;
    };

    template <usize N, class... Types>
    using TupleElement = typename GetTupleElement<N>::template Result<Types...>;

    namespace ComptimeInt {
        struct DigitAccum {
            usize n = 0;

            constexpr DigitAccum operator+(char x) const {
                return { '0' <= x && x <= '9' ? n * 10 + (x - '0') : n };
            }
        };

        template <char... Digits>
        static constexpr usize _ReadNum = (DigitAccum { 0 } + ... + Digits).n;
    }

    template <usize N> struct StaticIndex { static constexpr usize Index = N; };
    template <char... Digits> using ParseStaticIndex = StaticIndex<ComptimeInt::_ReadNum<Digits...> - 1>;

    template <char... Digits> ParseStaticIndex<Digits...> operator ""_th() {
        return ParseStaticIndex<Digits...> {};
    }
    // for 1_st
    template <char... Digits> ParseStaticIndex<Digits...> operator ""_st() {
        return ParseStaticIndex<Digits...> {};
    }
    // for 2_nd
    template <char... Digits> ParseStaticIndex<Digits...> operator ""_nd() {
        return ParseStaticIndex<Digits...> {};
    }
    // for 3_rd
    template <char... Digits> ParseStaticIndex<Digits...> operator ""_rd() {
        return ParseStaticIndex<Digits...> {};
    }
#pragma endregion

    template <usize, class... T> struct TupleLeaf { T leaf; };

    template <usize N, class T, class... Ts> struct TupleImpl : TupleLeaf<N, T>, TupleImpl<N + 1, Ts...> {
        TupleImpl() = default;
        TupleImpl(T first, Ts... args) : TupleLeaf<N, T>(std::move(first)), TupleImpl<N + 1, Ts...>(std::move(args)...) {}
    };
    template <usize N, class T> struct TupleImpl<N, T> : TupleLeaf<N, T> {
        TupleImpl() = default;
        TupleImpl(T first) : TupleLeaf<N, T>(std::move(first)) {}
    };

    template <class... Ts> struct Tuple : TupleImpl<0, Ts...> {
        using TupleImpl<0, Ts...>::TupleImpl;

        template <usize N>
        const TupleElement<N, Ts...>& Get() const {
            return static_cast<const TupleLeaf<N, TupleElement<N, Ts...>>*>(this)->node;
        }
        template <usize N>
        TupleElement<N, Ts...>& Get() {
            return static_cast<TupleLeaf<N, TupleElement<N, Ts...>>*>(this)->node;
        }

        // structured binding support
        template <usize N> const TupleElement<N, Ts...>& get() const { return Get<N>(); }
        template <usize N> TupleElement<N, Ts...>& get() { return Get<N>(); }

        template <usize N> const TupleElement<N, Ts...>& operator[](StaticIndex<N>) const { return Get<N>(); }
        template <usize N>       TupleElement<N, Ts...>& operator[](StaticIndex<N>)       { return Get<N>(); }

        template <class... Us>
        void TieTo(Us&... outs) const {
            [&]<usize... Is>(IntSeq<Is...>) {
                ((outs = Us(Get<Is>())), ...);
            }(IntRangeSeq<sizeof...(Ts)> {});
        }
        void TieMoveTo(Ts&... outs) {
            [&]<usize... Is>(IntSeq<Is...>) {
                ((outs = std::move(Get<Is>())), ...);
            }(IntRangeSeq<sizeof...(Ts)> {});
        }
    };

    template <class... Ts>
    Tuple(Ts...) -> Tuple<Ts...>;
}

namespace Quasi::Hashing {
    enum Hash : usize;

    template <class... Ts>
    struct Hasher<Tuple<Ts...>> {
        Hash operator()(const Tuple<Ts...>& tup) const {
            Hash h {};
            [&]<usize... Is>(IntSeq<Is...>) {
                ((h = HashCombine(h, HashObject(tup.template Get<Is>()))), ...);
            }(IntRangeSeq<sizeof...(Ts)> {});
            return h;
        }
    };
}

// structured binding support
template <class... Ts>
struct std::tuple_size<Quasi::Tuple<Ts...>> { static constexpr Quasi::usize value = sizeof...(Ts); };