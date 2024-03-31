#pragma once
#include <charconv>
#include <optional>
#include <type_traits>
#include <limits>

namespace stdu {
    template <class T, T Null>
    struct parse_num_t {
        T impl(std::string_view v) requires std::is_arithmetic_v<T> {
            T out;
            const auto res = std::from_chars(v.data(), v.data() + v.length(), out);
            return res.ec == std::errc::invalid_argument ? Null : out;
        }
    };

    template <class T>
    struct parse_num_opt {
        std::optional<T> impl(std::string_view v) {
            T out;
            const auto res = std::from_chars(v.data(), v.data() + v.length(), out);
            return res.ec == std::errc::invalid_argument ? std::nullopt : std::optional<T> { out };
        }
    };

    template <class T> struct null;
    template <std::integral I> struct null<I> : std::integral_constant<I, ~0> { }; // essentially -1
    template <std::floating_point F> struct null<F> { static constexpr F value = NAN; };
    template <class T> struct null<std::optional<T>> { static constexpr std::optional<T> value = std::nullopt; };

    template <class T>
    std::optional<T> parse_num(std::string_view v) {
        return parse_num_opt<T> {}.impl(v);
    }

    template <class T, T Null>
    T parse_num_unchecked(std::string_view v) {
        return parse_num_t<T, Null> {}.impl(v);
    }

    inline int parse_int(std::string_view v) { return parse_num_unchecked<int, -1>(v); }
    inline short parse_i16(std::string_view v) { return parse_num_unchecked<short, -1>(v); }
    inline long long parse_i64(std::string_view v) { return parse_num_unchecked<long long, -1>(v); }
    inline unsigned int parse_uint(std::string_view v) { return parse_num_unchecked<uint, ~0U>(v); }
    inline unsigned short parse_u16(std::string_view v) { return parse_num_unchecked<ushort, (ushort)~0U>(v); }
    inline unsigned long long parse_u64(std::string_view v) { return parse_num_unchecked<uint64, ~0ULL>(v); }
    inline float parse_float(std::string_view v) { return parse_num_unchecked<float, std::numeric_limits<float>::quiet_NaN()>(v); }
    inline double parse_f64(std::string_view v) { return parse_num_unchecked<double, std::numeric_limits<double>::quiet_NaN()>(v); }
    inline long double parse_f128(std::string_view v) { return parse_num_unchecked<long double, std::numeric_limits<long double>::quiet_NaN()>(v); }
}
