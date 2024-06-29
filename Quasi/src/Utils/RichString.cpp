// ReSharper disable CppClangTidyCppcoreguidelinesAvoidGoto
#include "RichString.h"

#include <format>
#include <algorithm>


namespace Quasi::Text {
    bool RichString::Matches(IterOf<Str> iter, IterOf<Str> end, Str matchExpr) {
        if (end - iter < matchExpr.size()) return false;
        const char* m = matchExpr.data();
        for (auto it = iter; it < iter + matchExpr.size(); ++it, ++m) {
            if (*it != *m) return false;
        }
        return true;
    }

    IterOf<Str> RichString::Find(IterOf<Str> iter, IterOf<Str> end, Str matchExpr) {
        while (iter != end && !Matches(iter, end, matchExpr)) ++iter;
        return iter;
    }

    uint RichString::SizeOfStyle(Style s) { return (uint)((s & 127) >> 4); }
    bool RichString::IsSwitch(Style s) { return s < 9; }
    bool RichString::IsState(Style s, bool state) { return (bool)(s & Style::SWITCH) != state; }
    RichString::Style RichString::SetState(Style s, bool state) { return s & 127 | (!state << 7); }
    
    RichString RichString::ParseMarkdown(Str markdown) {
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
        RichString str;
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
                    str.Append('\\');
                    goto end;
                }
                goto rawChar;
            }

            if (state == NONE) {
                if (c == '\\') {
                    state = LITERAL;
                    goto end;
                }

                if (Matches(it, end, "\n\n")) {
                    ++it;
                    goto newLine;
                }

                if (c == '`') {
                    auto searchIt = Find(it + 1, end, "`");
                    if (searchIt == markdown.end()) {
                        state = LITERAL;
                        goto rawChar;
                    }
                    str.AddTag(Style::CODE_BLOCK, { 1 });
                    str.Append(it + 1, searchIt);
                    str.AddTag(Style::CODE_BLOCK, { 0 });
                    it = searchIt;
                    goto end;
                }

                if (c == '*') { // try bold or italic
                    const byte boldOrItalic = it + 1 < markdown.end() ? it[1] == '*' : 255;
                    // 0 -> italic, 1 -> bold, 255 -> no need
                    if (boldOrItalic == 255) goto rawChar;
                    const bool isSet = styleFlags & 2 << (1 - boldOrItalic);
                    if (!isSet) {
                        auto searchIt = Find(it + 1 + boldOrItalic, end, boldOrItalic ? "**" : "*");
                        if (searchIt == markdown.end()) {
                            state = boldOrItalic ? SKIP : LITERAL;
                            goto rawChar;
                        }
                    }
                    state = boldOrItalic ? SKIP : NONE;
                    str.AddTag(boldOrItalic ? Style::BOLD : Style::ITALIC, !isSet);
                    styleFlags ^= 2 << (1 - boldOrItalic);
                    goto end;
                }

                if (Matches(it, end, "~~")) {
                    const bool isSet = styleFlags & STRIKE_FLAG;
                    if (!isSet) {
                        auto searchIt = Find(it + 2, end, "~~");
                        if (searchIt == markdown.end()) {
                            state = LITERAL;
                            goto rawChar;
                        }
                    }
                    state = SKIP;
                    str.AddTag(Style::STRIKETHROUGH, !isSet);
                    styleFlags ^= STRIKE_FLAG;
                    goto end;
                }

                if (c == '>' && isStartLine) {
                    str.AddTag(Style::BLOCKQUOTE, true);
                    styleFlags |= QUOTE_FLAG;
                }

                if (Matches(it, end, "<br>")) {
                    str.Append('\n');
                    it += 3;
                    goto end;
                }
            }

            rawChar:
            str.Append(c);
            goto end;

            newLine:
            isStartLine = true;
            str.AddTag(Style::ALT_TEXT, true);
            str.Append('\n');
            str.AddTag(Style::ALT_TEXT, false);
            goto end;

            end:
            if (!std::isspace(c)) {
                isStartLine = false;
                if (styleFlags & QUOTE_FLAG) {
                    styleFlags &= ~QUOTE_FLAG;
                    str.AddTag(Style::BLOCKQUOTE, false, -1);
                }
            }
        }

        if (styleFlags & BOLD_FLAG)   str.AddTag(Style::BOLD,          false);
        if (styleFlags & ITALIC_FLAG) str.AddTag(Style::ITALIC,        false);
        if (styleFlags & STRIKE_FLAG) str.AddTag(Style::STRIKETHROUGH, false);
        if (styleFlags & QUOTE_FLAG)  str.AddTag(Style::BLOCKQUOTE,    false);

        return str;
    }

    RichString RichString::ParseHtml(Str html) {
        // TODO: add plz
        return {};
    }

    void RichString::AddTag(Style s, IList<byte> data, int off) {
        rawString.insert(rawString.end() + off, DELIMITER);
        stylings.push_back((byte)s);
        for (byte byte : data) stylings.push_back(byte);
    }

    void RichString::AddTag(Style s, bool state, int off) {
        rawString.insert(rawString.end() + off, DELIMITER);
        stylings.push_back((byte)((state ? 0 : (byte)Style::SWITCH) | s));
    }

    uint RichString::Lines() const {
        return (uint)(std::ranges::count(rawString, '\n') + 1);
    }

    String RichString::DebugRawstr() const {
        String debug;
        debug.reserve(rawString.size());
        const byte*    stylePtr = stylings.data();
        IterOf<String> lastSpan = rawString.begin();
        for (auto it = rawString.begin(); it != rawString.end(); ++it) {
            if (*it == DELIMITER) {
                debug += Str { lastSpan, it };
                const Style s = (Style)*stylePtr;
                switch (SizeOfStyle(s)) {
                    case 0: debug += StyleToStr(s); break;
                    case 1: debug += std::vformat(StyleToStr(s), std::make_format_args(stylePtr[1])); break;
                    case 2: debug += std::vformat(StyleToStr(s), std::make_format_args(stylePtr[1], stylePtr[2])); break;
                    case 4: debug += std::vformat(StyleToStr(s), std::make_format_args(stylePtr[1], stylePtr[2], stylePtr[3], stylePtr[4])); break;
                    default: break;
                }
                stylePtr += SizeOfStyle(s) + 1;
                lastSpan = it + 1;
            }
        }
        debug += Str { lastSpan, rawString.end() };
        return debug;
    }

    RichString::Iter RichString::begin() const {
        return ++Iter { .str = *this, .iter = rawString.data() - 1 };
    }
    RichString::Iter RichString::end() const {
        return { .str = *this, .iter = rawString.data() + rawString.size() };
    }

    void RichString::StyleState::AddState(Style s, const byte* data) {
        switch (SizeOfStyle(s)) {
            case 0:
                styleFlags = (byte)(styleFlags & ~(1 << (s - 1)) | (IsState(s, true) << s - 1));
            return;
            case 1: switch (s) {  // NOLINT(clang-diagnostic-switch-enum)
                case Style::FONT_TYPE: font = *data; break;
                case Style::CODE_BLOCK: codeLang = *data; break;
                case Style::ALIGNMENT: alignment = *data; break;
                default:;
            }
            return;
            case 2:
                if (s == Style::FONT_SIZE) fontSizeP64 = (short)((data[0] << 8) + data[1]); break;
            case 4: switch (s) {  // NOLINT(clang-diagnostic-switch-enum)
                case Style::TEXT_COLOR:      std::copy_n(data, 4, color);     break;
                case Style::HIGHLIGHT_COLOR: std::copy_n(data, 4, highlight); break;
                default:;
            }
            default:;
        }
    }

    RichString::RichChar RichString::Iter::operator*() const {
        return { .c = *iter, .style = currentState };
    }

    RichString::Iter& RichString::Iter::operator++() {
        ++iter;
        for (; *iter == DELIMITER; ++iter) {
            const byte* begin = str->stylings.data() + styleOffset;
            currentState.AddState((Style)*begin, begin + 1);
            styleOffset += SizeOfStyle((Style)*begin) + 1;
        }
        return *this;
    }
}
