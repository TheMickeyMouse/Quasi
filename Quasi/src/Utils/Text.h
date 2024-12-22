#pragma once

#include "Type.h"
#include "Option.h"
#include "Macros.h"

namespace Quasi::Text {
    Option<String> ReadFile(Str fname);
    Option<String> ReadFileBinary(Str fname);
    bool WriteFile(Str fname, Str contents);
    bool ExistsFile(Str fname);

    Tuple<Str, Str> SplitDirectory(Str fname);

    String ToLower(Str string), ToUpper(Str string);
    String Escape(Str string);
    Option<String> Unescape(Str string);

    template <class T>
    struct Parser {
        static Option<T> Parse(Str) { return nullptr; }
    };

    template <class T>
    Option<T> Parse(Str str) {
        return Parser<T>::Parse(str);
    }

    template <Numeric N>
    struct Parser<N> {
        static Option<N> Parse(Str str) {
            if (str.empty()) return nullptr;
            N number = 0;
            bool negate = false;
            if constexpr (!Unsigned<N>) {
                if (str[0] == '-') {
                    negate = true;
                    str = str.substr(1);
                } else if (str[0] == '+')
                    str = str.substr(1);
            } else if (str[0] == '+') {
                str = str.substr(1);
            }
            if (str.empty()) return nullptr;

            for (u32 i = 0; i < str.size(); ++i) {
                if (std::isdigit(str[i])) {
                    number *= (N)10;
                    number += (N)(str[i] - '0');
                    continue;
                }
                if constexpr (Floating<N>) {
                    if (str[i] == '.') {
                        N sub = 0, pwOfTen = (N)0.1;
                        for (u32 j = i + 1; j < str.size(); ++j) {
                            if (!std::isdigit(str[j])) return nullptr;
                            sub += pwOfTen * (N)(str[j] - '0');
                            pwOfTen *= (N)0.1;
                        }
                        return (negate ? (N)-1 : (N)1) * (number + sub);
                    }
                }
                return nullptr;
            }
            return negate ? -number : number;
        }
    };

    template <>
    struct Parser<String> {
        static Option<String> Parse(Str str) {
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

    // adapted from https://stackoverflow.com/a/59522794/19968422
    namespace details {
        template <class T> constexpr Str t() { return Q_FUNC_NAME(); }
        constexpr usize T_START_IDX = t<int>().find("int");
        constexpr usize T_TOTAL_SIZE = t<int>().length() - Q_STRLIT_LEN("int");
        // ReSharper disable once CppStaticAssertFailure
        static_assert(T_START_IDX != Str::npos, "nameof type couldn't be found");

        template <auto V> constexpr Str v() { return Q_FUNC_NAME(); }
        constexpr usize V_START_IDX = v<0>().find('0');
        constexpr usize V_TOTAL_SIZE = v<0>().length() - 1;
        // ReSharper disable once CppStaticAssertFailure
        static_assert(V_START_IDX != Str::npos, "nameof value couldn't be found");
    }

    template <class T> constexpr Str TypeName() {
        return details::t<T>().substr(details::T_START_IDX, details::t<T>().length() - details::T_TOTAL_SIZE);
    }

    template <auto V> constexpr Str ValueName() {
        return details::v<V>().substr(details::V_START_IDX, details::v<V>().length() - details::V_TOTAL_SIZE);
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