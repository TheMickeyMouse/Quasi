#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "macros.h"

#include "enum_utils.h"
#include "NumTypes.h"
#include <core.h>

namespace stdu {
    bool matches(std::string::const_iterator iter, std::string::const_iterator end, const char* matchExpr);
    std::string::const_iterator find(std::string::const_iterator iter, std::string::const_iterator end, const char* matchExpr);

    struct rich_string {
        enum class style : uchar;
        static constexpr char DELIMITER = (char)128;
        
        std::string rawString;
        std::vector<uchar> stylings;

#define STYLE_SWITCH(N, V) N = (V), N##_ON = (N), N##_OFF = N##_ON ^ SWITCH
        enum class style : uchar { // each 16 values represents 1 byte of memeory taken
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
        
        static STDU_ENUM_TOSTR(style, style_tostr,
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

        OPENGL_API static uint sizeof_style(style s);
        OPENGL_API static bool is_switch(style s);
        OPENGL_API static bool is_state(style s, bool state);
        OPENGL_API static style set_state(style s, bool state);

        OPENGL_API static rich_string parse_markdown(const std::string& markdown);
        OPENGL_API static rich_string parse_html(const std::string& html);

        std::string& raw_string() { return rawString; }
        [[nodiscard]] const std::string& raw_string() const { return rawString; }

        void append(const char c) { rawString += c; }
        template <class It> void append(It begin, It end) { rawString += std::string_view { begin, end }; }
        OPENGL_API void add_tag(style s, std::initializer_list<uchar> data, int off = 0);
        OPENGL_API void add_tag(style s, bool state, int off = 0);

        [[nodiscard]] OPENGL_API uint lines() const;

        [[nodiscard]] OPENGL_API std::string debug_rawstr() const;

        struct const_iter;
        struct rich_char;
        struct style_state;
        
        [[nodiscard]] OPENGL_API const_iter begin() const;
        [[nodiscard]] OPENGL_API const_iter end() const;
    };

    enum class code_lang {
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

#define MAP(VALUE, FIRST, ...)  { FIRST, code_lang::VALUE } STDU_IF(STDU_HAS_ARGS(__VA_ARGS__),, MAP2(VALUE, __VA_ARGS__))
#define MAP2(VALUE, FIRST, ...) { FIRST, code_lang::VALUE } STDU_IF(STDU_HAS_ARGS(__VA_ARGS__),, MAP3(VALUE, __VA_ARGS__))
#define MAP3(VALUE, FIRST, ...) { FIRST, code_lang::VALUE } STDU_IF(STDU_HAS_ARGS(__VA_ARGS__),, MAP (VALUE, __VA_ARGS__))

    static std::unordered_map<std::string, code_lang> lang_mapping = {
        MAP(C, "c"),
        MAP(CPP, "c++", "cpp", "cplusplus"),
        MAP(CSHARP, "c#", "csharp"),
        MAP(JAVASCRIPT, "js", "javascript"),
        MAP(JAVA, "java"),
        MAP(HTML, "html"),
        MAP(PYTHON, "py", "python"),
        MAP(LUA, "lua"),
        MAP(GO_LANG, "go", "golang", "go_lang"),
        MAP(RUST, "rust"),
    };

#undef MAP
#undef MAP2
#undef MAP3

    struct rich_string::style_state {
        uchar styleFlags = 0, alignment = 0;
        uchar codeLang = 0;
        uchar color[4] = { 0, 0, 0, 255 }, highlight[4] = { 0, 0, 0, 0 };
        uchar font = 0;
        short fontSizeP64 = 0; // big endian, aka 0x12'34 and not 0x34'12

        OPENGL_API void add_state(style s, const uchar* data);
    };

    struct rich_string::rich_char {
        char c;
        style_state style;
    };

    struct rich_string::const_iter {
        const rich_string* str;
        const char* iter;
        style_state currentState;
        uint styleOffset = 0;

        bool operator==(const const_iter& other) const { return iter == other.iter; }
        OPENGL_API rich_char operator*() const;
        OPENGL_API const_iter& operator++();
    };
    
    STDU_IMPL_ENUM_OPERATORS(rich_string::style);

    inline rich_string operator ""_md(const char* markdown, std::size_t size) {
        return rich_string::parse_markdown(std::string { markdown, size });
    }

    inline rich_string operator ""_html(const char* html, std::size_t size) {
        return rich_string::parse_html(std::string { html, size });
    }
}
