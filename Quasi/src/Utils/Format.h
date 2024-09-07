#pragma once
#include <concepts>
#include <cmath>

#include "Func.h"
#include "Text.h"

namespace Quasi {
    template <class T>
    struct Option;
}

namespace Quasi::Text {
    struct StringOutput {
        String& outref;
        usize position;

        static StringOutput From(String& string) {
            return { .outref = string, .position = string.size() };
        }

        StringOutput& Move(const usize i = 1) {
            position += i;
            if (position >= outref.size()) {
                outref.resize(position);
            }
            return *this;
        }
        StringOutput& operator++() { return Move(); }
        StringOutput& operator+=(usize n) { return Move(n); }

        [[nodiscard]] char& Curr() const { return outref[position]; }
        [[nodiscard]] char& operator*() const { return Curr(); }

        void Put(const char c, const usize count = 1) {
            for (u32 i = 0; i < count; ++i) {
                outref += c;
            }
        }
        void operator()(const char c, const usize n = 1) { Put(c, n); }

        void Put(const Str s) {
            outref += s;
            position = outref.size();
        }
        void operator()(const Str s) { Put(s); }
    };

    struct FormatString {
        Str rawstr;
        struct SubstitudeData {
            u32 position, skipPosition, index;
            Str specifier;
        };
        Vec<SubstitudeData> subs;
        Vec<u32> brackSkips;

        static FormatString From(Str fstr);
    };

    template <class T> struct Formatter {};

    void CheckFormatOption(Str tname, bool success, Str spec);
    template <class T> void DynFormatFunc(const void* data, Str spec, StringOutput output) {
        const T& value = *(const T*)data;
        Formatter<T> fmtr;
        CheckFormatOption(Text::TypeName<T>(), fmtr.AddOption(spec), spec);
        fmtr.FormatTo(value, output);
    }

    template <class T>
    void FormatOnto(StringOutput output, const T& value) {
        Formatter<T> fmtr {};
        fmtr.FormatTo(value, output);
    }

    template <class... Ts>
    void FormatOnto(StringOutput out, Str fmt, const Ts& ...args) {
        const FormatString f = FormatString::From(fmt);

        const void* argAnys[] = { &args... };
        void(*formatters[])(const void*, Str, StringOutput) = { &DynFormatFunc<Ts>... };

        u32 brackI = 0, subI = 0, lastPos = 0;
        while (brackI < f.brackSkips.size() && subI < f.subs.size()) {
            const u32 brackSkip = f.brackSkips[brackI],
                      specPos   = f.subs[subI].position,
                    & until     = std::min(brackSkip, specPos);
            out(fmt.substr(lastPos, until - lastPos));
            if (&until == &brackSkip) {
                out(fmt[brackSkip]);
                ++brackI;
                lastPos = brackSkip + 2;
            } else {
                const auto& sub = f.subs[subI];
                const u32 i = sub.index;
                formatters[i](argAnys[i], sub.specifier, out);
                ++subI;
                lastPos = sub.skipPosition + 1;
            }
        }
        for (; brackI < f.brackSkips.size(); ++brackI) {
            const u32 b = f.brackSkips[brackI];
            out(fmt.substr(lastPos, b - lastPos + 1));
            lastPos = b + 2;
        }
        for (; subI < f.subs.size(); ++subI) {
            const auto& s = f.subs[subI];
            out(fmt.substr(lastPos, s.position - lastPos));
            formatters[s.index](argAnys[s.index], s.specifier, out);
            lastPos = s.skipPosition + 1;
        }
        out(fmt.substr(lastPos));
    }

    template <class T>
    String Format(const T& value) {
        String str;
        FormatOnto(StringOutput::From(str), value);
        return str;
    }

    template <class... Ts>
    String Format(Str fmt, const Ts& ...args) {
        String str;
        FormatOnto(StringOutput::From(str), fmt, args...);
        return str;
    }

    template <class T = Str>
    struct PadWith {
        enum { LEFT, MIDDLE, RIGHT } align = LEFT;
        u32 pad = 0;
        char fill = ' ';
        T string;
    };

    template <class T = Str> PadWith<T> PadLeft  (const T& s, u32 pad, char fill = ' ') { return { PadWith<T>::LEFT,   pad, fill, s }; }
    template <class T = Str> PadWith<T> PadMiddle(const T& s, u32 pad, char fill = ' ') { return { PadWith<T>::MIDDLE, pad, fill, s }; }
    template <class T = Str> PadWith<T> PadRight (const T& s, u32 pad, char fill = ' ') { return { PadWith<T>::RIGHT,  pad, fill, s }; }

#pragma region Formattings
    template <class T>
    concept IFormatter = requires (const T& t) {
        { Formatter(t) } -> std::same_as<T>;
    };

    template <std::integral I>
    struct Formatter<I> {
        enum Base { BINARY, OCTAL, DECIMAL, HEX, CAPITAL_HEX } radix = DECIMAL;
        enum Sign { NOSIGN, POSITIVE } signMode = NOSIGN;
        u32 padding = 0;
        bool useZeroPad = false;

        bool AddOption(Str arguments) {
            // :[+][b|o|d|x|X][0][#num(pad)]
            if (arguments.empty()) return true;

            if (arguments[0] == '+') {
                signMode = POSITIVE;
                arguments = arguments.substr(1);
            }
            if (arguments.empty()) return true;

            switch (std::tolower(arguments[0])) {
                case 'x': radix = HEX;         arguments = arguments.substr(1); break;
                case 'X': radix = CAPITAL_HEX; arguments = arguments.substr(1); break;
                case 'd': radix = DECIMAL;     arguments = arguments.substr(1); break;
                case 'b': radix = BINARY;      arguments = arguments.substr(1); break;
                case 'o': radix = OCTAL;       arguments = arguments.substr(1); break;
                default:;
            }
            if (arguments.empty()) return true;

            if (arguments[0] == '0') {
                useZeroPad = true;
                arguments = arguments.substr(1);
            }
            if (arguments.empty()) return true;

            if (const auto zeropad = Text::Parse<u32>(arguments)) {
                padding = *zeropad;
                return true;
            } else {
                return false;
            }
        }

        static std::make_unsigned_t<I> Abs(I num) {
            using U = std::make_unsigned_t<I>;
            if constexpr (std::is_unsigned_v<I>)
                return num;
            else return num < 0 ? (U)-num : (U)num;
        }

        void FormatTo(I number, StringOutput output) const {
            u32 preciseDigits = 0;
            constexpr u32 POWERS_OF_TEN[] = {
                1, 10, 100, 1'000, 10'000, 100'000,
                1'000'000, 10'000'000, 100'000'000, 1'000'000'000
            };
            switch (radix) {
                case BINARY: preciseDigits =  std::bit_width(Abs(number));          break;
                case OCTAL:  preciseDigits = (std::bit_width(Abs(number)) + 2) / 3; break;
                case DECIMAL:
                    preciseDigits =
                        std::lower_bound(POWERS_OF_TEN, std::end(POWERS_OF_TEN), Abs(number) - 1) - POWERS_OF_TEN;
                    preciseDigits = std::max(1u, preciseDigits);
                    break;
                case HEX:
                case CAPITAL_HEX: preciseDigits = (std::bit_width(Abs(number)) + 3) / 4; break;
            }

            const u32 zeros = preciseDigits < padding ?
                (padding - preciseDigits - (number < 0 || signMode == POSITIVE)) : 0;
            output(useZeroPad ? '0' : ' ', zeros);
            if (number < 0 || signMode == POSITIVE)
                output(number < 0 ? '-' : '+');

            const I base = 0x10'10'0A'08'02 >> (radix * 8) & 0xFF;
            I digits[8 * sizeof(I)] {};
            I num = Abs(number);
            u32 i = 0;
            for (; num; num /= base)
                digits[i++] = num % base;
            while (i--) {
                const char c = digits[i] >= 10 ? (radix == CAPITAL_HEX ? 'A' : 'a') - 10 + digits[i] : ('0' + digits[i]);
                output(c);
            }
        }
    };
    template <std::floating_point F>
    struct Formatter<F> {
        enum Mode { DEFAULT, SCIENTIFIC, SCI_CAPS } mode = DEFAULT;
        char decimalPoint = '.';
        bool showPositiveSign = false;
        bool properDenormals = true;
        u32 precision = 3, padding = 0;
        bool useZeroPad = false, useZeroPrec = false;

        bool AddOption(Str arguments) {
            // :[+][e|E][0#num(pad)][.|, 0#num(prec)][!]
            if (arguments.empty()) return true;

            if (arguments[0] == '+') {
                showPositiveSign = true;
                arguments = arguments.substr(1);
            }
            if (arguments.empty()) return true;

            if (std::tolower(arguments[0]) == 'e') {
                mode = arguments[0] == 'e' ? SCIENTIFIC : SCI_CAPS;
                arguments = arguments.substr(1);
            }

            if (arguments.empty()) return true;

            if (arguments[0] == '0') {
                padding = useZeroPad;
                arguments = arguments.substr(1);
            }
            if (arguments.empty()) return true;

            if (arguments.back() == '!') {
                properDenormals = true;
                arguments = arguments.substr(0, arguments.size() - 1);
                if (arguments.empty()) return true;
            }

            if (const usize decimalPt = std::min(arguments.find('.'), arguments.find(',')); decimalPt != Str::npos) {
                decimalPoint = arguments[decimalPoint];
                const auto zeropad  = Text::Parse<u32>(arguments.substr(0, decimalPt));
                if (arguments[decimalPt + 1] == '0') {
                    useZeroPrec = true;
                }
                const auto zeroprec = Text::Parse<u32>(arguments.substr(decimalPt + 1 + useZeroPrec));
                if (!zeropad || !zeroprec) return false;
                padding = *zeropad;
                precision = *zeroprec;
                return true;
            }

            if (const auto zeropad = Text::Parse<u32>(arguments)) {
                padding = *zeropad;
                return true;
            } else {
                return false;
            }
        }

        void FormatTo(F number, StringOutput output) const {
            if (std::isnan(number))
                return output(properDenormals ? "NaN" : "nan");
            if (std::isinf(number)) {
                if (showPositiveSign && number > 0) {
                    output('+');
                } else if (number < 0) {
                    output('-');
                }
                return output(properDenormals ? "Infinity" : "inf");
            }
            if (mode == SCIENTIFIC || mode == SCI_CAPS) {
                const F log = std::log(number),
                        powerOf10 = std::pow(10, std::floor(log));
                F       mantisa   = number / powerOf10;
                output('0' + (u32)mantisa);
                output(decimalPoint);
                for (u32 i = 0; i < precision; ++i) {
                    mantisa -= std::floor(mantisa);
                    mantisa *= (F)10;
                    output('0' + (u32)mantisa);
                }
                output(mode == SCI_CAPS ? 'E' : 'e');
                FormatOnto(output, (u32)log);
                return;
            }
            const u32 log = (u32)std::log(number);
            F x = number;
            for (u32 i = log; i--;) {
                const F rem = x / std::pow((F)10, (F)i);
                output('0' + (u32)rem);
                x -= std::floor(rem);
            }

            output(decimalPoint);
            for (u32 i = 1; i <= precision; ++i) {
                const F fract = x * std::pow((F)10, (F)i);
                output('0' + (u32)fract);
                x -= std::floor(fract);
            }
        }
    };
    template <>
    struct Formatter<char> {
        bool AddOption(Str) { return true; }
        void FormatTo(char c, StringOutput output) const {
            output(c);
        }
    };

    template <>
    struct Formatter<Str> {
        enum { LEFT, RIGHT, MID } align = LEFT;
        u32 pad = 0;
        char fill = ' ';

        bool AddOption(Str arguments) {
            // :[< | > | ^][#chr(fill)] #num(pad)
            if (arguments.empty()) return true;

            switch(arguments[0]) {
                case '<': { align = LEFT;  arguments = arguments.substr(1); break; }
                case '>': { align = RIGHT; arguments = arguments.substr(1); break; }
                case '^': { align = MID;   arguments = arguments.substr(1); break; }
                default:;
            }

            if (arguments.empty()) return false;
            if (arguments.size() == 1) { pad = arguments[0] - '0'; return true; }
            fill = arguments[0];
            const auto p = Text::Parse<u32>(arguments.substr(1));
            if (!p) return false;
            pad = *p;
            return true;
        }

        void FormatTo(Str str, StringOutput output) const {
            const u32 len = str.size();
            switch (align) {
                case LEFT: {
                    output(str);
                    output(fill, len >= pad ? 0 : pad - len);
                    break;
                }
                case RIGHT: {
                    output(fill, len >= pad ? 0 : pad - len);
                    output(str);
                    break;
                }
                case MID: {
                    if (len >= pad) {
                        output(str);
                        break;
                    }
                    const u32 space = pad - len;
                    output(fill, space / 2);
                    output(str);
                    output(fill, space - space / 2);
                    break;
                }
            }
        }
    };

    template <>
    struct Formatter<String> : Formatter<Str> {};
    template <>
    struct Formatter<const char*> : Formatter<Str> {};
    template <>
    struct Formatter<char*> : Formatter<Str> {};
    template <>
    struct Formatter<const char[]> : Formatter<Str> {};
    template <>
    struct Formatter<char[]> : Formatter<Str> {};

    template <class T>
    struct Formatter<T*> : Formatter<usize> {
        bool AddOption(Str) { radix = CAPITAL_HEX; useZeroPad = true; padding = sizeof(usize) * 8 / 4; return true; }
        void FormatTo(T* ptr, StringOutput output) {
            if constexpr (std::is_void_v<T>) {
                output("<ptr to 0x");
                Formatter<usize>::FormatTo(reinterpret_cast<usize>(ptr), output);
                output(">");
            } else {
                output("<");
                output(Text::TypeName<T>());
                output("* to 0x");
                Formatter<usize>::FormatTo(reinterpret_cast<usize>(ptr), output);
                output(">");
            }
        }
    };

    template <CollectionLike R>
    struct Formatter<R> {
        char brack = '[';
        Str elemFormat = "$";
        Str seperator = ", ";
        bool trailingComma = false;
        bool AddOption(Str args) {
            // :[( | [ | {][#str(fmt)][) | ] | }][#str(sep)][+|*]
            if (args.empty()) return true;
            if (const char trail = args.back(); trail == '+' || trail == '*') {
                trailingComma = trail == '*';
                args = args.substr(0, args.size() - 1);
                if (args.empty()) return true;
            }
            const char open = args[0];
            if (const usize close = args.rfind((char)(open + 1)); close == Str::npos) return false;
            else {
                seperator = args.substr(close + 1);
                args = args.substr(0, close + 1);
            }
            brack = args[0];
            elemFormat = args.substr(1, args.size() - 1);
            return true;
        }
        void WriteElement(const ArrayElement<R>& x, usize i, StringOutput output) const {
            for (const char c : elemFormat) {
                if (c == '#') {
                    FormatOnto(output, i);
                    continue;
                }
                if (c == '$') {
                    FormatOnto(output, x);
                    continue;
                }
                output(c);
            }
        }
        void FormatTo(const R& range, StringOutput output) const {
            output(brack);
            usize i = 0;
            auto next = range.begin();
            ++next;
            for (auto it = range.begin(); it != range.end(); it = next, ++next) {
                WriteElement(*it, i, output);
                if (trailingComma || next != range.end())
                    output(seperator);
                ++i;
            }
            output(brack + 1);
        }
    };

    template <class T>
    struct Formatter<Option<T>> : Formatter<T> {
        void FormatTo(const Option<T>& option, StringOutput output) const {
            if (option) {
                output("Some {");
                Formatter<T>::FormatTo(*option, output);
                output("}");
            } else
                output("None {}");
        }
    };

    template <class T>
    struct Formatter<PadWith<T>> : Formatter<T> {
        void FormatTo(const PadWith<T>& padded, StringOutput output) const {
            if (padded.align == PadWith<T>::LEFT) {
                const usize i = output.position;
                FormatOnto(output, padded.string);
                if (output.position - i < padded.pad)
                    output(padded.fill, padded.pad - (output.position - i));
            } else if (padded.align == PadWith<T>::RIGHT) {
                String s;
                FormatOnto(StringOutput::From(s), padded.pad);
                if (s.size() < padded.pad)
                    output(padded.fill, padded.pad - s.size());
                output(s);
            } else {
                String s;
                FormatOnto(StringOutput::From(s), padded.pad);
                if (s.size() >= padded.pad) {
                    output(s);
                    return;
                }
                const usize space = padded.pad - s.size();
                output(padded.fill, space / 2);
                output(s);
                output(padded.fill, space - space / 2);
            }
        }
    };
#pragma endregion
}
