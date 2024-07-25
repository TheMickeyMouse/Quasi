#pragma once
#include "Macros.h"
#include "Type.h"

#define Q_MATCH(...)         if (const auto& $ = __VA_ARGS__; true) { if (false)
#define Q_CASE(...)          } else if ($ __VA_ARGS__) {
#define Q_WITH               == Quasi::Matching::Never Q_RPAREN() Q_WITH_CASE Q_LPAREN()
#define Q_WITH_CASE(M, ...)  } else if (M($, __VA_ARGS__)) {
#define Q_LET                == Quasi::Matching::Never Q_RPAREN() Q_LET_CASE Q_LPAREN()
#define Q_LET_CASE(N, ...)   } else if (const auto& N) {
#define Q_OTHERWISE          } else

#ifdef Q_EXT_MATCH_SYNTAX
#define match(...) Q_MATCH(__VA_ARGS__)
#define when(...)  Q_UNARY(Q_CASE(__VA_ARGS__))
#define with       Q_WITH
#define let        Q_LET
#define otherwise  Q_OTHERWISE

#define in with Quasi::Matching::In,
#define orwhen  || $
#define andwhen && $
#else
#warning Extended Match syntax has been disabled (define Q_EXT_MATCH_SYNTAX to enable), some syntax features wont work properly
#endif

namespace Quasi::Matching {
    inline struct NeverType {
        constexpr bool operator==(auto&) const { return false; }
    } Never;

    template <class T>
    bool In(const T& value, CollectionOf<T> auto& list) {
        if constexpr (requires { { list.contains(value) } -> std::same_as<bool>; })
            return list.contains(value);
        else if constexpr (requires { { list.find(value) } -> std::same_as<decltype(list.begin())>; }) {
            const auto it = list.find(value);
            return it == list.end() ? false : true;
        } else {
            const auto it = std::find(std::begin(list), std::end(list), value);
            return it == std::end(list) ? false : true;
        }
    }

    template <class T>
    bool In(const T& value, IList<T> list) {
        return In(value, Span { list });
    }
}