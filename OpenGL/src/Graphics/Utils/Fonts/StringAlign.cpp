#include "StringAlign.h"

#include "Font.h"

namespace Graphics {
    StringAlign& StringAlign::Move(Maths::fvec2 offset) {
        rect += offset;
        return *this;
    }

    StringAlign& StringAlign::SpaceOut(float line, PointPer64 letter) {
        letterSpacing = letter;
        lineSpacing = line;
        return *this;
    }

    StringAlign& StringAlign::Align(std::initializer_list<AlignOptions> options) {
        for (const AlignOptions opt : options)
            alignOptions |= opt;
        return *this;
    }

    StringAlign& StringAlign::Align(std::initializer_list<AlignOptI> options) {
        for (const AlignOptI opt : options)
            alignOptions |= opt;
        return *this;
    }

    StringAlign& StringAlign::ResetAlign() {
        alignOptions = DEFAULT;
        return *this;
    }

    float StringAlign::GetXOff(float width) const {
        const float emptySpace = rect.width() - width;
        return emptySpace * (float)(!(alignOptions & ALIGN_LEFT) + !!(alignOptions & ALIGN_RIGHT)) / 2;
    }

    float StringAlign::GetYOff(float height) const {
        const float emptySpace = rect.height() - height;
        return emptySpace * (float)(!(alignOptions & VERTICAL_TOP) + !!(alignOptions & VERTICAL_BOTTOM)) / 2;
    }

    float StringAlign::GetAdvance(const Font& font, char c, float scaleRatio) const {
        return font.GetGlyphRect(c).advance.x * scaleRatio + (float)letterSpacing.points();
    }
}
