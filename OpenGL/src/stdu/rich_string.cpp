// ReSharper disable CppClangTidyCppcoreguidelinesAvoidGoto
#include "rich_string.h"

#include <format>

#include "Graphics/Utils/Fonts/Font.h"

namespace stdu {
    bool matches(std::string::const_iterator iter, std::string::const_iterator end, const char* matchExpr) {
        const int len = (int)std::strlen(matchExpr);
        if (end - iter < len) return false;
        for (auto it = iter; it < iter + len; ++it, ++matchExpr) {
            if (*it != *matchExpr) return false;
        }
        return true;
    }

    std::string::const_iterator find(
        std::string::const_iterator iter, std::string::const_iterator end,
        const char* matchExpr) {
        while (iter != end && !matches(iter, end, matchExpr)) ++iter;
        return iter;
    }

    uint rich_string::sizeof_style(style s) { return (uint)((s & 127) >> 4); }
    bool rich_string::is_switch(style s) { return s < 9; }
    bool rich_string::is_state(style s, bool state) { return (bool)(s & style::SWITCH) != state; }
    rich_string::style rich_string::set_state(style s, bool state) { return s & 127 | (!state << 7); }
    
    rich_string rich_string::parse_markdown(const std::string& markdown) {
        // *           (N)-> IS SPECIAL CHAR ----IS WHITESPACE ------> [ {V} IS '*' (N)               (Y)TOGGLE BOLD
        // * START -> IS '\'                      (N)->START LINE      [       (Y)---+----> IS NEXT CHAR '*' |-----------v
        // *   ^       (Y)-> ADD NEXT CHAR --v          = TRUE         [ {V} IS '~,~'                 (N)TOGGLE ITALIC   |
        // *   ^-----------------------------|                         [       (Y)-------TOGGLE STRIKETHROUGH v          |
        // *   ^  [ { } REPLACE '---' -> HORIZ_LINE (ALT TEXT '-')   ] [ { } IS '>' && START LINE         |   |          |
        // *   ^  [ { } IF '-' && START LINE -> LIST (ALT TEXT '\t.' ] [       (Y)-------TOGGLE QUOTE UNTIL NEWLINE v    |
        // *   ^  [ { } IF '+' && START LINE -> LIST (NO ALT TEXT)   ] [ { } IS '```' && START LINE       |   |     |    |
        // *   ^  [ { } IF '- [( |x)]' -> CHECKBOX (ALT TEXT '#/v'   ] [       (Y)-------TOGGLE CODE BLOCK && SET LANG v |
        // *   ^  [ { } IF '<...>' -> PARSE TAG (UNSUCCESSFUL -> RAW)] [ { } IS '`' && NOT AFTER          |   |     |  | |
        // *   ^  [ { } IF '\n\n' -> NEW PARA (ALT TEXT '\n')        ] [      (Y)-------TOGGLE CODE BLOCK && LANG=0 v  | |
        // *   ^-------------------------------[GO BACK TO START]-----------------------------------------<---<-----<--<-|
        // *   
        // *   =============[ MARKDOWN PARSING CONTROL FLOW ]=====================================================
        
        enum State {
            NONE, LITERAL, SKIP
        };
        enum StyleFlags {
            NO_FLAGS = 0,
            BOLD_FLAG = 1 << 1,
            ITALIC_FLAG = 1 << 2,
            STRIKE_FLAG = 1 << 3,
            QUOTE_FLAG = 1 << 4, 
            MONOSPACE_FLAG = 1 << 5,
        };
        
        int state = NONE, styleFlags = NO_FLAGS;
        bool isStartLine = true;
        rich_string str;
        const auto end = markdown.end();
        for (auto it = markdown.begin(); it != markdown.end(); ++it) {
            const char c = *it;

            if (state == SKIP) {
                state = NONE;
                continue;
            }
            
            if (state == LITERAL) {
                state = NONE;
                if (c == '\n') {
                    str.append('\\');
                    goto end;
                }
                goto rawChar;
            }

            if (state == NONE) {
                if (c == '\\') {
                    state = LITERAL;
                    goto end;
                }

                if (matches(it, end, "\n\n")) {
                    ++it;
                    goto newLine;
                }

                if (c == '`') {
                    auto searchIt = find(it + 1, end, "`");
                    if (searchIt == markdown.end()) {
                        state = LITERAL;
                        goto rawChar;
                    }
                    str.add_tag(style::CODE_BLOCK, { 1 });
                    str.append(it + 1, searchIt);
                    str.add_tag(style::CODE_BLOCK, { 0 });
                    it = searchIt;
                    goto end;
                }

                if (c == '*') { // try bold or italic
                    const uchar boldOrItalic = it + 1 < markdown.end() ? it[1] == '*' : 255;
                    // 0 -> italic, 1 -> bold, 255 -> no need
                    if (boldOrItalic == 255) goto rawChar;
                    const bool isSet = styleFlags & 2 << (1 - boldOrItalic);
                    if (!isSet) {
                        auto searchIt = find(it + 1 + boldOrItalic, end, boldOrItalic ? "**" : "*");
                        if (searchIt == markdown.end()) {
                            state = boldOrItalic ? SKIP : LITERAL;
                            goto rawChar;
                        }
                    }
                    state = boldOrItalic ? SKIP : NONE;
                    str.add_tag(boldOrItalic ? style::BOLD : style::ITALIC, !isSet);
                    styleFlags ^= 2 << (1 - boldOrItalic);
                    goto end;
                }

                if (matches(it, end, "~~")) {
                    const bool isSet = styleFlags & STRIKE_FLAG;
                    if (!isSet) {
                        auto searchIt = find(it + 2, end, "~~");
                        if (searchIt == markdown.end()) {
                            state = LITERAL;
                            goto rawChar;
                        }
                    }
                    state = SKIP;
                    str.add_tag(style::STRIKETHROUGH, !isSet);
                    styleFlags ^= STRIKE_FLAG;
                    goto end;
                }

                if (c == '>' && isStartLine) {
                    str.add_tag(style::BLOCKQUOTE, true);
                    styleFlags |= QUOTE_FLAG;
                }

                if (matches(it, end, "<br>")) {
                    str.append('\n');
                    it += 3;
                    goto end;
                }
            }

            rawChar:
            str.append(c);
            goto end;

            newLine:
            isStartLine = true;
            str.add_tag(style::ALT_TEXT, true);
            str.append('\n');
            str.add_tag(style::ALT_TEXT, false);
            goto end;

            end:
            if (!std::isspace(c)) {
                isStartLine = false;
                if (styleFlags & QUOTE_FLAG) {
                    styleFlags &= ~QUOTE_FLAG;
                    str.add_tag(style::BLOCKQUOTE, false, -1);
                }
            }
        }

        if (styleFlags & BOLD_FLAG)   str.add_tag(style::BOLD,          false);
        if (styleFlags & ITALIC_FLAG) str.add_tag(style::ITALIC,        false);
        if (styleFlags & STRIKE_FLAG) str.add_tag(style::STRIKETHROUGH, false);
        if (styleFlags & QUOTE_FLAG)  str.add_tag(style::BLOCKQUOTE,    false);

        return str;
    }

    rich_string rich_string::parse_html(const std::string& html) {
        return {};
    }

    void rich_string::add_tag(style s, std::initializer_list<uchar> data, int off) {
        rawString.insert(rawString.end() + off, DELIMITER);
        stylings.push_back((uchar)s);
        for (uchar byte : data) stylings.push_back(byte);
    }

    void rich_string::add_tag(style s, bool state, int off) {
        rawString.insert(rawString.end() + off, DELIMITER);
        stylings.push_back((uchar)((state ? 0 : (uchar)style::SWITCH) | s));
    }

    uint rich_string::lines() const {
        return std::ranges::count(rawString, '\n') + 1;
    }

    std::string rich_string::debug_rawstr() const {
        std::string debug;
        debug.reserve(rawString.size());
        const uchar* stylePtr                = stylings.data();
        std::string::const_iterator lastSpan = rawString.begin();
        for (auto it = rawString.begin(); it != rawString.end(); ++it) {
            if (*it == DELIMITER) {
                debug += std::string_view { lastSpan, it };
                const style s = (style)*stylePtr;
                switch (sizeof_style(s)) {
                    case 0: debug += style_tostr(s); break;
                    case 1: debug += std::vformat(style_tostr(s), std::make_format_args(stylePtr[1])); break;
                    case 2: debug += std::vformat(style_tostr(s), std::make_format_args(stylePtr[1], stylePtr[2])); break;
                    case 4: debug += std::vformat(style_tostr(s), std::make_format_args(stylePtr[1], stylePtr[2], stylePtr[3], stylePtr[4])); break;
                    default: break;
                }
                stylePtr += sizeof_style(s) + 1;
                lastSpan = it + 1;
            }
        }
        debug += std::string_view { lastSpan, rawString.end() };
        return debug;
    }

    rich_string::const_iter rich_string::begin() const {
        return ++const_iter { .str = this, .iter = rawString.data() - 1 };
    }
    rich_string::const_iter rich_string::end() const {
        return { .str = this, .iter = rawString.data() + rawString.size() };
    }

    void rich_string::style_state::add_state(style s, const uchar* data) {
        switch (sizeof_style(s)) {
            case 0:
                styleFlags = (uchar)(styleFlags & ~(1 << (s - 1)) | (is_state(s, true) << s - 1));
            return;
            case 1: switch (s) {  // NOLINT(clang-diagnostic-switch-enum)
                case style::FONT_TYPE: font = *data; break;
                case style::CODE_BLOCK: codeLang = *data; break;
                case style::ALIGNMENT: alignment = *data; break;
                default:;
            }
            return;
            case 2:
                if (s == style::FONT_SIZE) fontSizeP64 = (short)((data[0] << 8) + data[1]); break;
            case 4: switch (s) {  // NOLINT(clang-diagnostic-switch-enum)
                case style::TEXT_COLOR:      std::copy_n(data, 4, color);     break;
                case style::HIGHLIGHT_COLOR: std::copy_n(data, 4, highlight); break;
                default:;
            }
            default:;
        }
    }

    rich_string::rich_char rich_string::const_iter::operator*() const {
        return { .c = *iter, .style = currentState };
    }

    rich_string::const_iter& rich_string::const_iter::operator++() {
        ++iter;
        for (; *iter == DELIMITER; ++iter) {
            const uchar* begin = str->stylings.data() + styleOffset;
            currentState.add_state((style)*begin, begin + 1);
            styleOffset += sizeof_style((style)*begin) + 1;
        }
        return *this;
    }
}
