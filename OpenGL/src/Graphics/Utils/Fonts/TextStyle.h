#pragma once

namespace Quasi::Graphics {
    enum class FontStyle {
        NONE = 0,
        BOLD = 1 << 0,
        ITALIC = 1 << 1,
        BOLD_ITALIC = BOLD | ITALIC
    };

    enum TextStyle {
        NONE = 0,
        BOLD          = 1 << 0,
        ITALIC        = 1 << 1,
        UNDERLINE     = 1 << 2,
        STRIKETHROUGH = 1 << 3,
        SUBSCRIPT     = 1 << 4,
        SUPERSCRIPT   = 1 << 5,
        BLOCKQUOTE    = 1 << 6,
    };
    
    struct StyleState {
        TextStyle textStyling;
    };
}