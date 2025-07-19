#pragma once
#include "FontDevice.h"
#include "Utils/Math/Rect.h"

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

        TextAlign& Move(Math::fv2 offset);

        TextAlign& SpaceOut(float line = 1, PointPer64 letter = 0_p64);
        TextAlign& Align(IList<AlignOptions> options);
        TextAlign& Align(IList<AlignOptI> options);
        TextAlign& Align(byte alignX, byte alignY, byte wrapping, bool cropX, bool cropY);
        TextAlign& ResetAlign();

        bool IsAlignCenter()       const { return !(alignOptions & ALIGN_JUSTIFY); }
        bool IsAlignLeft()         const { return  (alignOptions & ALIGN_LEFT) && !(alignOptions & ALIGN_RIGHT); }
        bool IsAlignRight()        const { return !(alignOptions & ALIGN_LEFT) &&  (alignOptions & ALIGN_RIGHT); }
        bool IsAlignJustified()    const { return  (alignOptions & ALIGN_JUSTIFY) == ALIGN_JUSTIFY; }
        bool IsVerticalCenter()    const { return !(alignOptions & VERTICAL_JUSTIFY); }
        bool IsVerticalTop()       const { return  (alignOptions & VERTICAL_TOP) && !(alignOptions & VERTICAL_BOTTOM); }
        bool IsVerticalBottom()    const { return !(alignOptions & VERTICAL_TOP) &&  (alignOptions & VERTICAL_BOTTOM); }
        bool IsVerticalJustified() const { return  (alignOptions & VERTICAL_JUSTIFY) == VERTICAL_JUSTIFY; }
        bool IsWordWrap()          const { return  (alignOptions & WORD_WRAP);   }
        bool IsLetterWrap()        const { return  (alignOptions & LETTER_WRAP); }
        bool IsCropX()             const { return   alignOptions & CROP_X; }
        bool IsCropY()             const { return   alignOptions & CROP_Y; }
        
        float GetXOff(float width) const;
        float GetYOff(float height) const;

        float GetAdvance(const Glyph& glyph, float scaleRatio) const;
    };
}
