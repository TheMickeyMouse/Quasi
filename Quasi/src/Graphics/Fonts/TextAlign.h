#pragma once
#include "Font.h"

namespace Quasi::Graphics {
    struct Glyph;

    struct TextAlign {
        enum AlignOptions {
            ALIGN_LEFT    = 1 << 0,
            ALIGN_RIGHT   = 1 << 1,
            ALIGN_JUSTIFY = ALIGN_LEFT | ALIGN_RIGHT,
            ALIGN_CENTER  = 0,

            VERTICAL_TOP     = 1 << 2,
            VERTICAL_BOTTOM  = 1 << 3,
            VERTICAL_JUSTIFY = VERTICAL_TOP | VERTICAL_BOTTOM,
            VERTICAL_CENTER  = 0,

            WORD_WRAP = 1 << 4,

            DEFAULT = 0
        };

        int alignment = DEFAULT;
        Math::fv2 rect = { Math::Infinity };
        float letterSpacing = 0; // this is extra offset
        float lineSpacing = 1; // this is a multiplier

        float GetAdvance(const Glyph& glyph, float scaleRatio) const {
            return glyph.advance.x * scaleRatio + letterSpacing;
        }
    };
}
