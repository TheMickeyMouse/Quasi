#pragma once
#include "Macros.h"
#include "Type.h"

#define Q_MATCH(...)         if (const auto& $ = __VA_ARGS__; true) { if (false)
#define Q_CASE(...)          Q_IF(Q_HAS_ARGS(__VA_ARGS__), } else if ($ __VA_ARGS__) {)
#define Q_WITH               ) Q_WITH_CASE (
#define Q_WITH_CASE(M, ...)  } else if (M($, __VA_ARGS__)) {
#define Q_OTHERWISE          } else

#ifdef Q_EXT_MATCH_SYNTAX
#define match(...) Q_MATCH(__VA_ARGS__)
#define when(...)  Q_CASE(__VA_ARGS__)
#define with       Q_WITH
#define otherwise  Q_OTHERWISE

#define in with Quasi::Matching::In,
#define orwhen  || $
#define andwhen && $
#else
#warning Extended Match syntax has been disabled (define Q_EXT_MATCH_SYNTAX to enable), some syntax features wont work properly
#endif

namespace Quasi::Matching {
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
}