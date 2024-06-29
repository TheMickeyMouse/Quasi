#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Macros.h"

#include "Enum.h"
#include "Ref.h"


namespace Quasi::Text {
    struct RichString {
        enum class Style : byte;
        static constexpr char DELIMITER = (char)128;
        
        String rawString;
        Vec<byte> stylings;

#define STYLE_SWITCH(N, V) N = (V), N##_ON = (N), N##_OFF = N##_ON ^ SWITCH
        enum class Style : byte { // each 16 values represents 1 byte of memeory taken
            NONE = 0,
            SWITCH = 1 << 7,

            STYLE_SWITCH(BOLD, 1),
            STYLE_SWITCH(ITALIC, 2),
            STYLE_SWITCH(UNDERLINE, 3),
            STYLE_SWITCH(STRIKETHROUGH, 4),
            STYLE_SWITCH(SUBSCRIPT, 5),
            STYLE_SWITCH(SUPERSCRIPT, 6),
            STYLE_SWITCH(BLOCKQUOTE, 7),
            STYLE_SWITCH(ALT_TEXT, 8),

            FONT_TYPE = 16,
            CODE_BLOCK = 17,
            ALIGNMENT = 18,
            
            FONT_SIZE = 32,

            TEXT_COLOR = 64,
            HIGHLIGHT_COLOR = 65,
        };
#undef STYLE_SWITCH
        
        static Q_ENUM_TOSTR(Style, StyleToStr,
            (BOLD_ON,          "<b>")   (BOLD_OFF,   "</b>")
            (ITALIC_ON,        "<i>")   (ITALIC_OFF, "</i>")
            (UNDERLINE_ON,     "<u>")   (UNDERLINE_OFF,     "</u>")
            (STRIKETHROUGH_ON, "<s>")   (STRIKETHROUGH_OFF, "</s>")
            (SUBSCRIPT_ON,     "<sub>") (SUBSCRIPT_OFF,     "</sub>")
            (SUPERSCRIPT_ON,   "<sup>") (SUPERSCRIPT_OFF,   "</sup>")
            (BLOCKQUOTE_ON,    "<bq>")  (BLOCKQUOTE_OFF,    "</bq>")
            (ALT_TEXT_ON,      "<_alt>")(ALT_TEXT_OFF,      "</_alt>")
            (FONT_TYPE,        "<p style=\"font-family:{}\">")
            (CODE_BLOCK,       "<code _lang={}>")
            (ALIGNMENT,        "<p style=\"_text-align:{}\">")
            (FONT_SIZE,        "<p style=\"font-size:{}/64\">")
            (TEXT_COLOR,       "<p style=\"color:rgba({}, {}, {}, {})\">")
            (HIGHLIGHT_COLOR,  "<p style=\"background-color:rgba({}, {}, {}, {})\">"), "\\")

        static uint SizeOfStyle(Style s);
        static bool IsSwitch(Style s);
        static bool IsState(Style s, bool state);
        static Style SetState(Style s, bool state);

        static bool Matches(IterOf<Str> iter, IterOf<Str> end, Str matchExpr);
        static IterOf<Str> Find(IterOf<Str> iter, IterOf<Str> end, Str matchExpr);
        static RichString ParseMarkdown(Str markdown);
        static RichString ParseHtml(Str html);

        String& RawString() { return rawString; }
        [[nodiscard]] const String& RawString() const { return rawString; }

        void Append(const char c) { rawString += c; }
        template <class It> void Append(It begin, It end) { rawString += Str { begin, end }; }
        void AddTag(Style s, IList<byte> data, int off = 0);
        void AddTag(Style s, bool state, int off = 0);

        [[nodiscard]] uint Lines() const;

        [[nodiscard]] String DebugRawstr() const;

        struct Iter;
        struct RichChar;
        struct StyleState;
        
        [[nodiscard]] Iter begin() const;
        [[nodiscard]] Iter end() const;

        enum class CodeLang {
            C,
            CPP,
            CSHARP,
            JAVASCRIPT,
            JAVA,
            HTML,
            PYTHON,
            LUA,
            GO_LANG,
            RUST
        };

#define MAP(VALUE, FIRST, ...)  { FIRST, CodeLang::VALUE } Q_IF(Q_HAS_ARGS(__VA_ARGS__),, MAP2(VALUE, __VA_ARGS__))
#define MAP2(VALUE, FIRST, ...) { FIRST, CodeLang::VALUE } Q_IF(Q_HAS_ARGS(__VA_ARGS__),, MAP3(VALUE, __VA_ARGS__))
#define MAP3(VALUE, FIRST, ...) { FIRST, CodeLang::VALUE } Q_IF(Q_HAS_ARGS(__VA_ARGS__),, MAP (VALUE, __VA_ARGS__))

        inline static Map<Str, CodeLang> LanguageMapping = {
            MAP(C, "c"),
            MAP(CPP, "c++", "cpp"),
            MAP(CSHARP, "c#", "csharp"),
            MAP(JAVASCRIPT, "js", "javascript"),
            MAP(JAVA, "java"),
            MAP(HTML, "html"),
            MAP(PYTHON, "py", "python"),
            MAP(LUA, "lua"),
            MAP(GO_LANG, "go", "golang"),
            MAP(RUST, "rust"),
        };

#undef MAP
#undef MAP2
#undef MAP3
    };

    struct RichString::StyleState {
        byte styleFlags = 0, alignment = 0;
        byte codeLang = 0;
        byte color[4] = { 0, 0, 0, 255 }, highlight[4] = { 0, 0, 0, 0 };
        byte font = 0;
        i16 fontSizeP64 = 0; // big endian, aka 0x12'34 and not 0x34'12

        void AddState(Style s, const byte* data);
    };

    struct RichString::RichChar {
        char c;
        StyleState style;
    };

    struct RichString::Iter {
        Ref<const RichString> str;
        const char* iter;
        StyleState currentState;
        uint styleOffset = 0;

        bool operator==(const Iter& other) const { return iter == other.iter; }
        RichChar operator*() const;
        Iter& operator++();
    };
    
    Q_IMPL_ENUM_OPERATORS(RichString::Style);

    inline RichString operator ""_md(const char* markdown, usize size) {
        return RichString::ParseMarkdown(Str { markdown, size });
    }

    inline RichString operator ""_html(const char* html, usize size) {
        return RichString::ParseHtml(Str { html, size });
    }
}
