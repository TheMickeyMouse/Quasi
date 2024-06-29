#include "TextAlign.h"

#include "Font.h"

namespace Quasi::Graphics {
    TextAlign& TextAlign::Move(Math::fVector2 offset) {
        rect += offset;
        return *this;
    }

    TextAlign& TextAlign::SpaceOut(float line, PointPer64 letter) {
        letterSpacing = letter;
        lineSpacing = line;
        return *this;
    }

    TextAlign& TextAlign::Align(IList<AlignOptions> options) {
        for (const AlignOptions opt : options)
            alignOptions |= opt;
        return *this;
    }

    TextAlign& TextAlign::Align(IList<AlignOptI> options) {
        for (const AlignOptI opt : options)
            alignOptions |= opt;
        return *this;
    }

    TextAlign& TextAlign::Align(byte alignX, byte alignY, byte wrapping, bool cropX, bool cropY) {
        alignOptions = alignX | (alignY << 2) | (wrapping << 4) | (cropX << 6) | (cropY << 7);
        return *this;
    }

    TextAlign& TextAlign::ResetAlign() {
        alignOptions = DEFAULT;
        return *this;
    }

    float TextAlign::GetXOff(float width) const {
        const float emptySpace = rect.width() - width;
        return emptySpace * (float)(!(alignOptions & ALIGN_LEFT) + !!(alignOptions & ALIGN_RIGHT)) / 2;
    }

    float TextAlign::GetYOff(float height) const {
        const float emptySpace = rect.height() - height;
        return emptySpace * (float)(!(alignOptions & VERTICAL_TOP) + !!(alignOptions & VERTICAL_BOTTOM)) / 2;
    }

    float TextAlign::GetAdvance(const Glyph& glyph, float scaleRatio) const {
        return glyph.advance.x * scaleRatio + letterSpacing.pointsf();
    }
}
