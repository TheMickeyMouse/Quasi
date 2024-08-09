#pragma once

#include "Type.h"
#include "Option.h"
#include "Macros.h"

namespace Quasi::Text {
    Option<String> ReadFile(Str fname);
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

    template <class N> requires std::integral<N> || std::floating_point<N>
    struct Parser<N> {
        static Option<N> Parse(Str str) {
            if (str.empty()) return nullptr;
            N number = 0;
            bool negate = false;
            if constexpr (!std::unsigned_integral<N>) {
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
                if constexpr (std::floating_point<N>) {
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

    namespace details {
        template <class T> constexpr Str t() { return Q_FUNC_NAME(); }
        constexpr usize t_startIdx = t<int>().find("int");
        constexpr usize t_totalSize = t<int>().length() - Q_STRLIT_LEN("int");
        // ReSharper disable once CppStaticAssertFailure
        static_assert(t_startIdx != Str::npos, "nameof type couldn't be found");

        template <auto V> constexpr Str v() { return Q_FUNC_NAME(); }
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