#pragma once
#include <iostream>
#include <format>

#include "stdu/enum_utils.h"

namespace Debug {
    enum class ConsoleColor {
        RESET = 0,

        BLACK   = 0, H_BLACK   = 60,
        RED     = 1, H_RED     = 61,
        GREEN   = 2, H_GREEN   = 62,
        YELLOW  = 3, H_YELLOW  = 63,
        BLUE    = 4, H_BLUE    = 64,
        MAGENTA = 5, H_MAGENTA = 65,
        CYAN    = 6, H_CYAN    = 66,
        GRAY    = 7, H_GRAY    = 67,

        FG_BLACK    = 30, BG_BLACK    = 40,
        FG_RED      = 31, BG_RED      = 41,
        FG_GREEN    = 32, BG_GREEN    = 42,
        FG_YELLOW   = 33, BG_YELLOW   = 43,
        FG_BLUE     = 34, BG_BLUE     = 44,
        FG_MAGENTA  = 35, BG_MAGENTA  = 45,
        FG_CYAN     = 36, BG_CYAN     = 46,
        FG_GRAY     = 37, BG_GRAY     = 47,
        HFG_BLACK   = 90, HBG_BLACK   = 100,
        HFG_RED     = 91, HBG_RED     = 101,
        HFG_GREEN   = 92, HBG_GREEN   = 102,
        HFG_YELLOW  = 93, HBG_YELLOW  = 103,
        HFG_BLUE    = 94, HBG_BLUE    = 104,
        HFG_MAGENTA = 95, HBG_MAGENTA = 105,
        HFG_CYAN    = 96, HBG_CYAN    = 106,
        HFG_GRAY    = 97, HBG_GRAY    = 107,

        HIGHLIGHT = 60,
        BACKGROUND = 40,
        FOREGROUND = 30,

        BOLD_FLAG = 128,
        REGULAR_MASK = 127
    };
    STDU_IMPL_ENUM_OPERATORS(ConsoleColor);

    constexpr char ANSI_ESCAPE_CHAR = '\x1B';

    constexpr ConsoleColor Highlight(ConsoleColor color) { return color + ConsoleColor::HIGHLIGHT; }
    constexpr ConsoleColor BgColor(ConsoleColor color) { return color + ConsoleColor::BACKGROUND; }
    constexpr ConsoleColor FgColor(ConsoleColor color) { return color + ConsoleColor::FOREGROUND; }
    constexpr ConsoleColor HBgColor(ConsoleColor color) { return BgColor(Highlight(color)); }
    constexpr ConsoleColor HFgColor(ConsoleColor color) { return FgColor(Highlight(color)); }

    constexpr bool IsBold(ConsoleColor color) { return (color & ConsoleColor::BOLD_FLAG) != 0; }
    constexpr ConsoleColor Bold(ConsoleColor color) { return color | ConsoleColor::BOLD_FLAG; }
    constexpr ConsoleColor Regular(ConsoleColor color) { return color & ConsoleColor::REGULAR_MASK; }

    struct ColoredText {
        ConsoleColor color;
        std::string string;
    };

    inline ColoredText Dye(const std::string& s, ConsoleColor color) { return { color, s }; }

    inline std::ostream& operator<<(std::ostream& out, ConsoleColor color);
    inline std::ostream& operator<<(std::ostream& out, const ColoredText& text);
}

namespace std {
    template <>
    struct formatter<Debug::ConsoleColor> : formatter<std::string> {
        auto format(Debug::ConsoleColor c, std::format_context& ctx) const {
            return formatter<std::string>::format(
                std::format("{}[{}{}m", Debug::ANSI_ESCAPE_CHAR,
                    IsBold(c) ? "1;" : "", (int)Regular(c)), ctx);
        }
    };

    template <>
    struct formatter<Debug::ColoredText> : formatter<std::string> {
        auto format(const Debug::ColoredText& t, std::format_context& ctx) const {
            return formatter<std::string>::format(std::format("{}{}{}", t.color, t.string, Debug::ConsoleColor::RESET), ctx);
        }
    };
}

namespace Debug {
    inline std::ostream& operator<<(std::ostream& out, ConsoleColor color);
}