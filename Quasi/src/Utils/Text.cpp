#include "Text.h"

#include <fstream>

#include "CStr.h"
#include "Debug/Logger.h"
#include "Text/Num.h"

namespace Quasi::Text {
    Option<String> ReadFile(CStr fname) {
        // standard C implementation
        std::FILE *fp = std::fopen(fname.Data(), "r");
        if (fp) {
            std::fseek(fp, 0, SEEK_END);
            const usize length = std::ftell(fp);
            // ownership is transfered at return
            char* const contentsRaw = (char*)Memory::AllocateRaw(length * sizeof(char));

            std::rewind(fp);
            std::fread(contentsRaw, 1, length, fp);
            std::fclose(fp);
            return String::Compose(contentsRaw, length, length);
        }
        return nullptr;
    }

    Option<String> ReadFileBinary(CStr fname) {
        std::FILE *fp = std::fopen(fname.Data(), "rb");
        if (fp) {
            std::fseek(fp, 0, SEEK_END);
            const usize length = std::ftell(fp);
            // ownership is transfered at return
            char* const contentsRaw = (char*)Memory::AllocateRaw(length * sizeof(char));

            std::rewind(fp);
            std::fread(contentsRaw, 1, length, fp);
            std::fclose(fp);
            return String::Compose(contentsRaw, length, length);
        }
        return nullptr;
    }

    bool WriteFile(CStr fname, Str contents) {
        if (std::ofstream out { fname.Data() }) {
            out.write(contents.Data(), (isize)contents.Length());
            return true;
        }
        return false;
    }

    bool ExistsFile(CStr fname) {
        return std::ifstream { fname.Data() }.good();
    }

    Tuple<Str, Str> SplitDirectory(Str fname) {
        const auto [pos, _] = fname.RevFindOneOf("\\/");
        return pos ?
            Tuple { Str::Empty(), fname } :
            Tuple { fname.Substr(0, pos),
                    fname.Substr(pos + 1) };
    }

    String AutoIndent(Str text) {
        String ss;
        u32 indent = 0;
        for (usize i = 0; i < text.Length() - 1; ++i) {
            const char c = text[i];
            if (text[i] == '{' || text[i] == '[') {
                ++indent;
            }
            if (text[i + 1] == '}' || text[i + 1] == ']') {
                Debug::AssertMsg(indent > 0, "unindenting past level 0");
                --indent;
            }
            if (c == '\n') {
                ss += '\n';
                for (u32 j = 0; j < indent; ++j) ss += ' ';
                continue;
            }
            ss += c;
        }
        ss += text.Last();
        return ss;
    }

    ColoredStr Dye(ConsoleColor col, Str txt) {
        return { col, txt };
    }

    usize Formatter<ConsoleColor>::FormatTo(StringWriter output, ConsoleColor c, Empty) {
        output.Write("\x1B["_str);
        if (IsBold(c))
            output.Write("1;"_str);

        char regbuf[3] = { '\0', '\0', 'm' };
        u16 byte2 = Regular(c);
        byte2 += (byte2 * 103 >> 9) * (256 - 10);
        Memory::WriteU16(byte2, regbuf);

        output.Write(Str::Slice(regbuf, 3));
        return 2 + (IsBold(c) ? 2 : 0) + 3;
    }

    usize Formatter<ColoredStr>::FormatTo(StringWriter output, const ColoredStr& cstr, const FormatOptions& options) {
        usize padLen = options.targetLength - cstr.text.Length();
        padLen &= -(padLen <= options.targetLength); // fun bithacks that turn negative numbers into 0
        const usize left = padLen * (usize)options.alignment / 2;

        return output.WriteRepeat(options.pad, left) +
               FormatObjectTo(output, cstr.color) +
               output.Write(cstr.text) +
               output.Write("\x1B[0m"_str) +
               output.WriteRepeat(options.pad, padLen - left);
    }

    template struct Formatter<ConsoleColor>;
    template struct Formatter<ColoredStr>;
}
