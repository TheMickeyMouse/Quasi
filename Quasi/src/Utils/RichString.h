#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Macros.h"

#include "Enum.h"
#include "Ref.h"

namespace Quasi::Text {
#define STYLE_SWITCH(N, S) (N, (0, "<" S ">", true, true))(N##_OFF, (0, "</" S ">", true, false))
    struct StyleData { // each 16 values represents 1 byte of memeory taken
        u32 byteEncodingSize;
        Str debugStr;
        bool isSwitch : 1 = false, isOn : 1 = false;

        QDefineEnum$(Style,
            STYLE_SWITCH(BOLD,          "b")
            STYLE_SWITCH(ITALIC,        "i")
            STYLE_SWITCH(UNDERLINE,     "u")
            STYLE_SWITCH(STRIKETHROUGH, "s")
            STYLE_SWITCH(SUBSCRIPT,     "sub")
            STYLE_SWITCH(SUPERSCRIPT,   "sup")
            STYLE_SWITCH(BLOCKQUOTE,    "bq")
            STYLE_SWITCH(ALT_TEXT,      "*alt")
            (FONT_TYPE,       (1, "<*font=\"{}\">",      false, false))
            (CODE_BLOCK,      (1, "<code *lang=\"{}\">", false, false))
            (ALIGNMENT,       (1, "<*align={}>",         false, false))
            (FONT_SIZE,       (2, "<*size={}>",          false, false))
            (TEXT_COLOR,      (4, "<*color={}>",         false, false))
            (HIGHLIGHT_COLOR, (4, "<*hcolor={}>",        false, false)),
        ARITH)
    };
#undef STYLE_SWITCH

    struct RichString {
        static constexpr char DELIMITER = (char)128;
        
        String rawString;
        Vec<byte> stylings;

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

    inline RichString operator ""_md(const char* markdown, usize size) {
        return RichString::ParseMarkdown(Str { markdown, size });
    }

    inline RichString operator ""_html(const char* html, usize size) {
        return RichString::ParseHtml(Str { html, size });
    }
}
