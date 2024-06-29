#pragma once
#include "FontDevice.h"
#include "Math/Rect.h"

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

            WORD_WRAP   = 1 << 4,
            LETTER_WRAP = 1 << 5,

            CROP_X = 1 << 6,
            CROP_Y = 1 << 7,

            DEFAULT = 0
        };
        using AlignOptI = int;

        AlignOptI alignOptions = DEFAULT;
        Math::fRect2D rect;
        PointPer64 letterSpacing = 0_p64; // this is extra offset
        float lineSpacing = 1; // this is a multiplier

        TextAlign(const Math::fRect2D& rect) : rect(rect) {}

        TextAlign& Move(Math::fVector2 offset);

        TextAlign& SpaceOut(float line = 1, PointPer64 letter = 0_p64);
        TextAlign& Align(IList<AlignOptions> options);
        TextAlign& Align(IList<AlignOptI> options);
        TextAlign& Align(byte alignX, byte alignY, byte wrapping, bool cropX, bool cropY);
        TextAlign& ResetAlign();

        [[nodiscard]] bool IsAlignCenter()       const { return !(alignOptions & ALIGN_JUSTIFY); }
        [[nodiscard]] bool IsAlignLeft()         const { return  (alignOptions & ALIGN_LEFT) && !(alignOptions & ALIGN_RIGHT); }
        [[nodiscard]] bool IsAlignRight()        const { return !(alignOptions & ALIGN_LEFT) &&  (alignOptions & ALIGN_RIGHT); }
        [[nodiscard]] bool IsAlignJustified()    const { return  (alignOptions & ALIGN_JUSTIFY) == ALIGN_JUSTIFY; }
        [[nodiscard]] bool IsVerticalCenter()    const { return !(alignOptions & VERTICAL_JUSTIFY); }
        [[nodiscard]] bool IsVerticalTop()       const { return  (alignOptions & VERTICAL_TOP) && !(alignOptions & VERTICAL_BOTTOM); }
        [[nodiscard]] bool IsVerticalBottom()    const { return !(alignOptions & VERTICAL_TOP) &&  (alignOptions & VERTICAL_BOTTOM); }
        [[nodiscard]] bool IsVerticalJustified() const { return  (alignOptions & VERTICAL_JUSTIFY) == VERTICAL_JUSTIFY; }
        [[nodiscard]] bool IsWordWrap()          const { return  (alignOptions & WORD_WRAP);   }
        [[nodiscard]] bool IsLetterWrap()        const { return  (alignOptions & LETTER_WRAP); }
        [[nodiscard]] bool IsCropX()             const { return   alignOptions & CROP_X; }
        [[nodiscard]] bool IsCropY()             const { return   alignOptions & CROP_Y; }
        
        [[nodiscard]] float GetXOff(float width) const;
        [[nodiscard]] float GetYOff(float height) const;

        [[nodiscard]] float GetAdvance(const Glyph& glyph, float scaleRatio) const;
    };
}
