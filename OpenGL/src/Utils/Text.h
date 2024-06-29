#pragma once
#include <source_location>

#include "Type.h"
#include "Option.h"
#include "Macros.h"

namespace Quasi::Text {
    Option<String> ReadFile(Str fname);
    Tuple<Str, Str> SplitDirectory(Str fname);

    String ToLower(Str string), ToUpper(Str string);
    String Escape(Str string);
    Option<String> Unescape(Str string);

    namespace details {
        template <class T>
        struct parser_t {
            static Option<T> impl(Str) { return nullptr; }
        };
    }

    template <class T>
    Option<T> Parse(Str str) {
        return details::parser_t<T>::impl(str);
    }

    template <class N> requires std::integral<N> || std::floating_point<N>
    struct details::parser_t<N> {
        static Option<N> impl(Str str) {
            N number;
            if (auto [end, ec] = std::from_chars(str.begin(), str.end(), number);
                ec != std::errc {} || end != str.end())
                return nullptr;
            return number;
        }
    };

    template <>
    struct details::parser_t<String> {
        static Option<String> impl(Str str) {
            if (str.starts_with('"') && str.ends_with('"')) {
                return Unescape(str.substr(1, str.length() - 2));
            }
            return String { str };
        }
    };

    template <class R, class F = std::identity>
    String ArrStr(R&& vect, Str sep = ", ", F&& t = {});

    String AutoIndent(Str text);
    String Quote(Str txt);

    namespace details {
        template <class T> constexpr Str t() { return std::source_location::current().function_name(); }
        constexpr usize t_startIdx = t<int>().find("int");
        constexpr usize t_totalSize = t<int>().length() - Q_STRLIT_LEN("int");
        // ReSharper disable once CppStaticAssertFailure
        static_assert(t_startIdx != Str::npos, "nameof type couldn't be found");

        template <auto V> constexpr Str v() { return std::source_location::current().function_name(); }
        constexpr usize v_startIdx = v<0>().find('0');
        constexpr usize v_totalSize = v<0>().length() - 1;
        // ReSharper disable once CppStaticAssertFailure
        static_assert(v_startIdx != Str::npos, "nameof value couldn't be found");
    }

    template <class T> constexpr Str TypeName() {
        return details::t<T>().substr(details::t_startIdx, details::t<T>().length() - details::t_totalSize);
    }

    template <auto V> constexpr Str ValueName() {
        return details::v<V>().substr(details::v_startIdx, details::v<V>().length() - details::v_totalSize);
    }

    template <usize N>
    struct FixedString  {
        char buf[N + 1]{};
        constexpr FixedString(const char* s) {
            for (unsigned i = 0; i != N; ++i) buf[i] = s[i];
        }
        constexpr operator const char*() const { return buf; }
        static constexpr usize SIZE = N;
    };
    template <usize N> FixedString(const char (&)[N]) -> FixedString<N - 1>;
}

template <class R, class F>
Quasi::String Quasi::Text::ArrStr(R&& vect, Str sep, F&& t) {
    auto it = std::begin(vect);
    if (it == std::end(vect)) return "";
    String ss;
    ss += t(*it);
    ++it;
    for (; it != std::end(vect); ++it) {
        ss += sep;
        ss += t(*it);
    }
    return ss;
}