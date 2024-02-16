#pragma once
#include <optional>
#include <ranges>

#include "Vector.h"

#include "stdu/parsing.h"

namespace Maths {
    template <class> struct parser;

    namespace detail {
        using namespace std::literals;

        inline std::string_view DEFAULT_COMMA_SEPARATOR = ", "sv,
                                DEFAULT_BRACKET_BEGIN = "("sv,
                                DEFAULT_BRACKET_CLOSE = ")"sv;
    }

    template <vec_t T>
    struct parser<T> {
        using scalar_t = typename T::scalar;
        template <class F>
        static std::optional<T> impl(
            const std::string_view v,
            F&& cparser,
            const std::string_view sep = detail::DEFAULT_COMMA_SEPARATOR,
            const std::string_view beg = detail::DEFAULT_BRACKET_BEGIN,
            const std::string_view end = detail::DEFAULT_BRACKET_CLOSE)
        {
            if (!v.starts_with(beg) || !v.starts_with(end)) return std::nullopt;

            auto split = std::views::split(v.substr(beg.length(), v.length() - beg.length() - end.length()), sep);
            T vec;
            int ptr = 0;
            for (const auto val : split) {
                if (ptr >= T::dimension) return std::nullopt;

                std::optional<scalar_t> t = cparser(std::string_view(val.begin(), val.end()));
                if (!t.has_value()) return std::nullopt;

                vec[ptr++] = t.value();
            }
            if (ptr != T::dimension) return std::nullopt;
            return vec;
        }
    };

    template <class T, int N>
    std::optional<typename vecn<N, T>::type> parse_vec(
        const std::string_view v,
        const std::string_view sep = detail::DEFAULT_COMMA_SEPARATOR,
        const std::string_view beg = detail::DEFAULT_BRACKET_BEGIN,
        const std::string_view end = detail::DEFAULT_BRACKET_CLOSE)
    {
        return parser<typename vecn<N, T>::type>::impl(v, stdu::parse_num<T>, sep, beg, end);
    }

    template <class T, int N, class F>
    std::optional<typename vecn<N, T>::type> parse_vec(
        const std::string_view v,
        F&& cparser,
        const std::string_view sep = detail::DEFAULT_COMMA_SEPARATOR,
        const std::string_view beg = detail::DEFAULT_BRACKET_BEGIN,
        const std::string_view end = detail::DEFAULT_BRACKET_CLOSE)
    {
        return parser<typename vecn<N, T>::type>::impl(v, cparser, sep, beg, end);
    }
}
