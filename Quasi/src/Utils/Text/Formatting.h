#pragma once
#include "Utils/String.h"
#include "StringWriter.h"

namespace Quasi::Text {
    template <class T> struct Formatter;

    template <class T>
    usize FormatObjectTo(StringWriter output, const T& object, const typename Formatter<T>::FormatOptions options = {}) {
        return Formatter<T>::FormatTo(output, object, options);
    }

    template <class T>
    usize FormatObjectTo(StringWriter output, const T& object, Str options) {
        return Text::FormatObjectTo(output, object, Formatter<T>::ConfigureOptions(options));
    }

    // should only be used internally
    template <class T>
    usize FormatWriterPtr(StringWriter out, const void* anyOfT, Str opt) {
        return Text::FormatObjectTo(out, *Memory::UpcastPtr<T>(anyOfT), opt);
    }

    // should only be used internally
    usize FormatToDynamic(StringWriter output, Str fmt,
        const void* argParams[], FuncPtr<void, StringWriter, const void*, Str> writerParams[], usize n);

    template <class... Ts>
    usize FormatDynamicTypesTo(StringWriter output, Str fmt, const void* argParams[]) {
        static constexpr FuncPtr<usize, StringWriter, const void*, Str> writerParams[] = {
            &Text::FormatWriterPtr<Ts>...
        };
        return Text::FormatToDynamic(output, fmt, argParams, writerParams, sizeof...(Ts));
    }

    template <class... Ts>
    usize FormatTo(StringWriter output, Str fmt, Ts&&... args) {
        const void* argParams[] = { (const void*)&args... };
        return Text::FormatDynamicTypesTo<Ts...>(output, fmt, argParams);
    }

    template <class T> struct WithFormatOptions {
        T subject;
        typename Formatter<T>::FormatOptions options;
    };

    template <class T>
    WithFormatOptions<RemQual<T>> SetFmtOptions(T&& subject, typename Formatter<T>::FormatOptions o) {
        return { (T&&)subject, std::move(o) };
    }

    template <class T> struct FormatResultLeaf {
        T leaf;
    };

    // intermediate result, lazily evaluated
    template <class... Ts>
    struct FormatResult : FormatResultLeaf<Ts>... {
        Str fmtStr;

        String ToString() const {
            String output;
            WriteTo(StringWriter::WriteTo(output));
            return output;
        }

        usize WriteTo(StringWriter output) const {
            const void* argParams[] = { static_cast<FormatResultLeaf<Ts>>(this)->leaf... };
            return Text::FormatDynamicTypesTo<Ts...>(output, fmtStr, argParams);
        }

        operator String() const { return ToString(); }
    };

    template <class... Ts>
    FormatResult<Ts...> Format(Str fmt, Ts&&... args) {
        return { (Ts&&)args..., fmt };
    }

    template <class... Ts> usize StringWriter::Write(const FormatResult<Ts...>& fmtRes) {
        return fmtRes.WriteTo(*this);
    }

    // (?'char'.)?(?'align'[<^>])(?'len'[0-9]+)
    struct TextFormatOptions {
        usize targetLength = 0;
        enum Alignment { LEFT, CENTER, RIGHT } alignment = LEFT;
        char pad = ' ';
        bool escape = false;

        static TextFormatOptions Configure(Str opt);
    };

    template <>
    struct Formatter<Str> {
        using FormatOptions = TextFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return TextFormatOptions::Configure(opt); }
        static usize FormatTo(StringWriter sw, Str input, const FormatOptions& options);
        static usize FormatNoEscape(StringWriter sw, Str input, const FormatOptions& options);
    };

    template <>
    struct Formatter<char> {
        using FormatOptions = TextFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return TextFormatOptions::Configure(opt); }
        static usize FormatTo(StringWriter sw, char c, const FormatOptions& options);
    };

    template <> struct Formatter<StrMut> : Formatter<Str> { using Formatter<Str>::FormatOptions; };
    template <> struct Formatter<String> : Formatter<Str> { using Formatter<Str>::FormatOptions; };
    template <usize N> struct Formatter<const char[N]> : Formatter<Str> { using Formatter<Str>::FormatOptions; };

    template <class T>
    struct Formatter<WithFormatOptions<T>> {
        using FormatOptions = Empty;
        static usize FormatTo(StringWriter sw, const WithFormatOptions<T>& fres, Empty) {
            return Text::FormatObjectTo(sw, fres.subject, fres.options);
        }
    };

    template <class... Ts>
    struct Formatter<FormatResult<Ts...>> {
        using FormatOptions = TextFormatOptions;

        static FormatOptions ConfigureOptions(Str opt) { return TextFormatOptions::Configure(opt); }

        static usize FormatTo(StringWriter sw, const FormatResult<Ts...>& fres, const FormatOptions& options) {
            if (options.alignment == TextFormatOptions::LEFT) {
                usize len = 0;
                if (options.escape) {
                    len = fres.ToString().WriteEscape(sw);
                } else
                    len = fres.WriteTo(sw);

                if (options.targetLength > len) {
                    sw.WriteRepeat(options.pad, options.targetLength - len);
                    return options.targetLength;
                }
                return len;
            } else {
                const String result = fres.ToString();
                return FormatObjectTo(sw, result, options);
            }
        }
    };
}
