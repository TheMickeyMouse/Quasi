#pragma once
#include "Func.h"
#include "Hash.h"

namespace Quasi {
#pragma region Indexing
    template <usize N> struct GetTupleElement {
        template <class T, class... Rest>
        using Result = GetTupleElement<N - 1>::template DeferResult<Rest...>;
        template <class T, class... Rest>
        static Result<T, Rest...> GetDeferResult;
        template <class... Ts> using DeferResult = decltype(GetDeferResult<Ts...>);
    };

    template <> struct GetTupleElement<0> {
        template <class T, class...> using Result = T;
        template <class T, class... Ts> static T GetDeferResult;
        template <class... Ts> using DeferResult = decltype(GetDeferResult<Ts...>);
    };

    template <usize N, class... Types>
    using TupleElement = GetTupleElement<N>::template Result<Types...>;

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

    template <char... Digits> ParseStaticIndex<Digits...> operator ""_th() { return {}; }
    // for 1_st
    template <char... Digits> ParseStaticIndex<Digits...> operator ""_st() { return {}; }
    // for 2_nd
    template <char... Digits> ParseStaticIndex<Digits...> operator ""_nd() { return {}; }
    // for 3_rd
    template <char... Digits> ParseStaticIndex<Digits...> operator ""_rd() { return {}; }
#pragma endregion

    template <class...> struct Tuple;

    template <class T, class... Ts> struct Tuple<T, Ts...> {
        T first;
        [[no_unique_address]]
        Tuple<Ts...> rest;
    public:
        Tuple() = default;
        Tuple(T first, Ts... rest) : first((T&&)first), rest((Ts&&)rest...) {}

        template <usize N>
        const TupleElement<N, T, Ts...>& Get() const {
            if constexpr (N == 0) return first;
            else return rest.template Get<N - 1>();
        }
        template <usize N>
        TupleElement<N, T, Ts...>& Get() {
            if constexpr (N == 0) return first;
            else return rest.template Get<N - 1>();
        }

        // structured binding support
        template <usize N> const TupleElement<N, T, Ts...>& get() const { return Get<N>(); }
        template <usize N> TupleElement<N, T, Ts...>& get() { return Get<N>(); }

        template <usize N> const TupleElement<N, T, Ts...>& operator[](StaticIndex<N>) const { return Get<N>(); }
        template <usize N>       TupleElement<N, T, Ts...>& operator[](StaticIndex<N>)       { return Get<N>(); }

        template <class U, class... Us>
        void TieTo(U& out, Us&... outs) const {
            out = first;
            rest.TieTo(outs...);
        }
        void TieMoveTo(T& out, Ts&... outs) {
            out = std::move(first);
            rest.TieMoveTo(outs...);
        }

        template <FnArgs<const T&, const Ts&...> M>
        FuncResult<M, const T&, const Ts&...> Apply(M&& map) const {
            return rest.Apply([&] (const Ts&... args) { return map(first, args...); });
        }
        template <FnArgs<T&, Ts&...> M>
        FuncResult<M, T&, Ts&...> ApplyMut(M&& map) {
            return rest.ApplyMut([&] (Ts&... args) { return map(first, args...); });
        }

        template <class... Us>
        Tuple<T, Ts..., Us...> Join(Tuple<Us...> tup) const& {
            return Apply([&] (const T& arg, const Ts&... args) {
                return tup.ApplyMut([&] (Us&... args2) {
                    return Tuple<T, Ts..., Us...> { arg, args..., std::move(args2)... };
                });
            });
        }
        template <class... Us>
        Tuple<T, Ts..., Us...> Join(Tuple<Us...> tup) && {
            return Apply([&] (T& arg, Ts&... args) {
                return tup.ApplyMut([&] (Us&... args2) {
                    return Tuple<T, Ts..., Us...> { std::move(arg), std::move(args)..., std::move(args2)... };
                });
            });
        }

        Hashing::Hash GetHashCode() const {
            Hashing::Hash h = Hashing::HashObject(first);
            h = Hashing::HashCombine(h, rest.GetHashCode());
            return h;
        }

        template <class... Us> friend struct Tuple;
    };

    template <> struct Tuple<> {
        Tuple() = default;

        template <usize> void Get() const = delete;
        template <usize> void get() const = delete;
        void operator[](auto) const = delete;
        void TieTo() const {}
        void TieMoveTo() const {}

        template <FnArgs<> M> FuncResult<M> Apply(M&& map) const { return map(); }
        template <FnArgs<> M> FuncResult<M> ApplyMut(M&& map)    { return map(); }
        template <class Tup> Tup Join(Tup tup) const { return std::move(tup); }

        Hashing::Hash GetHashCode() const { return Hashing::EmptyHash(); }
    };

    template <class... Ts>
    Tuple(Ts...) -> Tuple<Ts...>;
}

// structured binding support
template <class... Ts>
struct std::tuple_size<Quasi::Tuple<Ts...>> { static constexpr Quasi::usize value = sizeof...(Ts); };
template <Quasi::usize N, class... Ts>
struct std::tuple_element<N, Quasi::Tuple<Ts...>> { using type = Quasi::TupleElement<N, Ts...>; };