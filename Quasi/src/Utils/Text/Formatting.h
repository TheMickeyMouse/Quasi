#pragma once
#include "Utils/String.h"
#include "StringWriter.h"

namespace Quasi {
    struct CStr;
}

namespace Quasi::Text {
    template <class T> struct Formatter {};

    template <class T>
    usize FormatObjectTo(StringWriter output, const T& object, const typename Formatter<T>::FormatOptions options = {}) {
        return Formatter<T>::FormatTo(output, object, options);
    }

    template <class T>
    usize FormatObjectTo(StringWriter output, const T& object, Str options) {
        if constexpr (requires (Str x) { Formatter<T>::ConfigureOptions(x); })
            return Text::FormatObjectTo(output, object, Formatter<T>::ConfigureOptions(options));
        else return Formatter<T>::FormatTo(output, object, options);
    }

    template <class T>
    StringWriter& StringWriter::operator<<(const T& obj) { FormatObjectTo(*this, obj); return *this; }

    template <class... Ts>
    usize StringWriter::Write(Str fmt, const Ts& ...args) { return FormatTo(*this, fmt, args...); }

    // should only be used internally
    template <class T>
    usize FormatWriterPtr(StringWriter out, const void* anyOfT, Str opt) {
        return Text::FormatObjectTo(out, *Memory::UpcastPtr<T>(anyOfT), opt);
    }

    // should only be used internally
    usize FormatToDynamic(StringWriter output, Str fmt,
        const void* const argParams[], const FuncPtr<usize, StringWriter, const void*, Str> writerParams[], usize n);

    template <class... Ts>
    usize FormatDynamicTypesTo(StringWriter output, Str fmt, const void* argParams[]) {
        static constexpr FuncPtr<usize, StringWriter, const void*, Str> writerParams[] = {
            &Text::FormatWriterPtr<Ts>..., nullptr
        };
        return Text::FormatToDynamic(output, fmt, argParams, writerParams, sizeof...(Ts));
    }

    template <class... Ts>
    usize FormatTo(StringWriter output, Str fmt, const Ts&... args) {
        if constexpr (sizeof...(args) == 0) {
            output.Write(fmt);
            return fmt.Length();
        } else {
            const void* argParams[] = { (const void*)&args..., nullptr };
            return Text::FormatDynamicTypesTo<Ts...>(output, fmt, argParams);
        }
    }

    template <class T> struct WithFormatOptions {
        const T& subject;
        Formatter<T>::FormatOptions options;
    };

    template <class T>
    WithFormatOptions<RemQual<T>> SetFmtOptions(T&& subject, typename Formatter<T>::FormatOptions o) {
        return { (T&&)subject, std::move(o) };
    }

    template <class... Ts>
    String Format(Str fmt, const Ts&... args) {
        String s {};
        FormatTo(StringWriter::WriteTo(s), fmt, args...);
        return s;
    }

    // (?'char'.)?(?'align'[<^>])(?'len'[0-9]+)
    struct TextFormatOptions {
        usize targetLength = 0;
        enum Alignment : u8 { LEFT, CENTER, RIGHT } alignment = LEFT;
        char pad = ' ';
        bool escape = false;

        static TextFormatOptions Configure(Str opt);
    };

    template <>
    struct Formatter<Str> {
        using FormatOptions = TextFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return TextFormatOptions::Configure(opt); }
        static usize FormatTo(StringWriter sw, Str input, const FormatOptions& options);
        static usize FormatNoEscape(StringWriter sw, Str input, usize strlen, const FormatOptions& options);
    };

    template <>
    struct Formatter<char> {
        using FormatOptions = TextFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return TextFormatOptions::Configure(opt); }
        static usize FormatTo(StringWriter sw, char c, const FormatOptions& options);
    };

    template <> struct Formatter<CStr>   : Formatter<Str> {};
    template <> struct Formatter<StrMut> : Formatter<Str> {};
    template <> struct Formatter<String> : Formatter<Str> {};
    template <> struct Formatter<const char*> : Formatter<Str> {};
    template <> struct Formatter<char*>       : Formatter<Str> {};
    template <usize N> struct Formatter<const char[N]> : Formatter<Str> {};


    template <class T>
    struct Formatter<WithFormatOptions<T>> {
        using FormatOptions = Empty;
        static usize FormatTo(StringWriter sw, const WithFormatOptions<T>& fres, Empty) {
            return Text::FormatObjectTo(sw, fres.subject, fres.options);
        }
    };

    // aligning uses utf8-length instead
    struct Utf8Str : Str {};

    template <>
    struct Formatter<Utf8Str> : Formatter<Str> {
        static usize FormatTo(StringWriter sw, Utf8Str str, const TextFormatOptions& options) {
            return FormatNoEscape(sw, str, str.Utf8Length(), options);
        }
    };
}


#pragma region Extra Type Formattings
namespace Quasi::Text {
    template <> struct Formatter<void*> {
        static usize FormatTo(StringWriter sw, void* fres, Str);
    };

    template <class T> struct Formatter<Ref<T>> : Formatter<T> {};

    template <class T>
    struct Formatter<OptRef<T>> : Formatter<RemConst<T>> {
        using typename Formatter<RemConst<T>>::FormatOptions;
        static usize FormatTo(StringWriter sw, OptRef<T> optref, const FormatOptions& options) {
            if (optref) {
                sw.Write("Some(&"_str);
                const usize i = Formatter<RemConst<T>>::FormatTo(sw, *optref, options);
                sw.Write(')');
                return i + 7;
            } else { sw.Write("&None"_str); return 5; }
        }
    };

    template <class T>
    struct Formatter<Option<T>> : Formatter<T> {
        using typename Formatter<T>::FormatOptions;
        static usize FormatTo(StringWriter sw, const Option<T>& opt, const FormatOptions& options) {
            if (opt) {
                sw.Write("Some("_str);
                const usize i = Formatter<T>::FormatTo(sw, *opt, options);
                sw.Write(')');
                return i + 6;
            } else { sw.Write("None"_str); return 4; }
        }
    };

    template <class T>
    struct Formatter<Span<T>> : Formatter<RemConst<T>> {
        using typename Formatter<RemQual<T>>::FormatOptions;
        static usize FormatTo(StringWriter sw, Span<T> span, const FormatOptions& options) {
            usize len = 2;
            sw.Write('[');
            for (usize i = 0; i < span.Length(); ++i) {
                if (!i) len += sw.Write(", ");
                len += FormatObjectTo(sw, span[i], options);
            }
            sw.Write(']');
            return len;
        }
    };

    template <class T>          struct Formatter<Vec<T>>      : Formatter<Span<const T>> {};
    template <class T, usize N> struct Formatter<Array<T, N>> : Formatter<Span<const T>> {};

    template <class... Ts> struct Formatter<Tuple<Ts...>> {
        static usize FormatTo(StringWriter sw, const Tuple<Ts...>& tuple, Str fspec) {
            sw.Write('(');
            usize len = 2;
            tuple.Apply([&] (const auto& first, const auto&... args) {
                len += sw.Write(first);
                ([&] (const auto& x) { len += sw.Write(", ") + sw.Write(x); } (args), ...);
            });
            sw.Write(')');
            return len;
        }
    };
}
#pragma endregion