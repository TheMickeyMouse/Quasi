#include "Text.h"

#include <algorithm>
#include <fstream>

#include "Logger.h"

namespace Quasi::Text {
    Option<String> ReadFile(Str fname) {
        if (std::ifstream in { fname.data(), std::ios::in | std::ios::binary }) {
            String contents;
            in.seekg(0, std::ios::end);
            contents.resize((uint)in.tellg());
            // ill be honest idk what the compiler wants from me
            in.seekg(0, std::ios::beg);
            in.read(contents.data(), (long long)contents.size());
            in.close();
            return contents;
        }
        return nullptr;
    }

    bool WriteFile(Str fname, Str contents) {
        if (std::ofstream out { fname.data() }) {
            out << contents;
            return true;
        }
        return false;
    }

    bool ExistsFile(Str fname) {
        return std::ifstream { fname.data() }.good();
    }

    Tuple<Str, Str> SplitDirectory(Str fname) {
        const usize pos = fname.find_last_of("\\/");
        return String::npos == pos ?
            Tuple { Str {}, fname } :
            Tuple { fname.substr(0, pos),
                    fname.substr(pos + 1) };
    }

    String ToLower(Str string) {
        String lower;
        lower.resize(string.size());
        std::ranges::transform(string, lower.begin(), [](const char c) { return std::tolower(c); });
        return lower;
    }

    String ToUpper(Str string) {
        String upper;
        upper.resize(string.size());
        std::ranges::transform(string, upper.begin(), [](const char c) { return std::toupper(c); });
        return upper;
    }

    String Escape(Str string) {
        String ss;
        ss.reserve(string.size() + 2);
        ss += '"';
        for (const char c : string) {
            if (c == '"') {
                ss += "\\\"";
            } else if (isprint(c)) {
                ss += c;
            } else {
                // filter \\, \n, \r, \t, \", \b
                ss += '\\';
                switch(c) {
                    case '\\': ss += '\\'; break;
                    case '\t': ss += 't';  break;
                    case '\r': ss += 'r';  break;
                    case '\n': ss += 'n';  break;
                    case '\b': ss += 'b';  break;
                    default:
                        constexpr char hexdig[17] = "0123456789ABCDEF";
                    ss += 'x';
                    ss += hexdig[c >> 4];
                    ss += hexdig[c & 0xF];
                }
            }
        }
        ss += '"';
        return ss;
    }

    Option<String> Unescape(Str string) {
        String ss;
        ss.reserve(string.size());

        for (size_t i = 0; i < string.length(); ++i) {
            if (string[i] == '\\') {
                if (i + 1 >= string.length()) return nullptr;
                switch (string[i + 1]) {
                    case 'n':  ss += '\n'; break;
                    case 't':  ss += '\t'; break;
                    case 'r':  ss += '\r'; break;
                    case '\\': ss += '\\'; break;
                    case 'b':  ss += '\b'; break;
                    // Add more cases as needed
                    default: return nullptr;
                }
                ++i;
            } else {
                ss += string[i];
            }
        }

        return ss;
    }

    String AutoIndent(Str text) {
        static constexpr Str INDENT_INCR = "{[", INDENT_DECR = "}]";
        static constexpr auto NOT_FOUND = Str::npos;

        String ss;
        u32 indent = 0;
        for (usize i = 0; i < text.size() - 1; ++i) {
            const char c = text[i];
            if (INDENT_INCR.find(text[i]) != NOT_FOUND) {
                ++indent;
            }
            if (INDENT_DECR.find(text[i + 1]) != NOT_FOUND) {
                Debug::Assert(indent > 0, "unindenting past level 0");
                --indent;
            }
            if (c == '\n') {
                ss += '\n';
                for (u32 j = 0; j < indent; ++j) ss += ' ';
                continue;
            }
            ss += c;
        }
        ss += text.back();
        return ss;
    }

    String Quote(Str txt) {
        return std::format("\"{}\"", txt);
    }
}
