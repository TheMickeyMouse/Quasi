#include "TextRenderer.h"

#include "Constants.h"

namespace Quasi::Graphics {
    void TextRenderer::CharQuad::MoveY(float y) {
        v0.Position.y += y;
        v1.Position.y += y;
        v2.Position.y += y;
        v3.Position.y += y;
    }
    void TextRenderer::CharQuad::MoveMinX(float x) {
        v0.Position.x += x;
        v2.Position.x += x;
    }
    void TextRenderer::CharQuad::MoveMaxX(float x) {
        v1.Position.x += x;
        v3.Position.x += x;
    }
    void TextRenderer::CharQuad::MoveTexMinX(float x) {
        v0.TextureCoord.x += x;
        v2.TextureCoord.x += x;
    }
    void TextRenderer::CharQuad::MoveTexMaxX(float x) {
        v1.TextureCoord.x += x;
        v3.TextureCoord.x += x;
    }
    float TextRenderer::CharQuad::Width() const {
        return v1.Position.x - v0.Position.x;
    }
    float TextRenderer::CharQuad::TextureWidth() const {
        return v1.TextureCoord.x - v0.TextureCoord.x;
    }

    TextRenderer::CharQuad* TextRenderer::Begin() {
        return (CharQuad*)textVertices.Data();
    }
    const TextRenderer::CharQuad* TextRenderer::Begin() const {
        return (const CharQuad*)textVertices.Data();
    }
    TextRenderer::CharQuad* TextRenderer::End() {
        return (CharQuad*)(textVertices.Data() + textVertices.Length());
    }
    const TextRenderer::CharQuad* TextRenderer::End() const {
        return (const CharQuad*)(textVertices.Data() + textVertices.Length());
    }
    TextRenderer::CharQuad& TextRenderer::CharAt(u32 index) {
        return Begin()[index];
    }
    const TextRenderer::CharQuad& TextRenderer::CharAt(u32 index) const {
        return Begin()[index];
    }

    void TextRenderer::Prepare() {
        scaleRatio = fontSize / font.fontSize; // scaling ratio
        lineSpacing = font.GetDefaultMetric().fontHeight.pointsf() * scaleRatio; // just a calculation (without * align.lineSpacing)
        spaceAdvance = align.GetAdvance(font.GetGlyphRect(' '), scaleRatio); // this gets used alot, just here

        totalHeight = lineSpacing * (((float)lineCount - 1) * align.lineSpacing + 1);
        pen = align.rect.corner(2);
        pen.y -= font.GetDefaultMetric().ascend.pointsf() * scaleRatio;
        pen.y -= align.IsVerticalJustified() ? 0 : align.GetYOff(totalHeight);
    }

    void TextRenderer::FixAlignX() { // quick function for reusing code, aligns current line
        if (align.IsAlignJustified())
            JustifyAlignX();
        else
            AlignX();
    }

    void TextRenderer::JustifyAlignX() {
        const float restWidth = 2 * align.GetXOff(lineWidth - align.letterSpacing.pointsf());
        if (lineWords.Length() < 2) return; // no need to align
        if (lastSpaceIndex == meshIndex) lineWords.Pop(); // check if this 'word' is necessary (is not empty or whitespace)
        const float space = restWidth / (float)(lineWords.Length() - 1); // amt of space between each word (accounts for existing spaces)
        for (u32 i = 1; i < lineWords.Length(); ++i) {
            const auto end = i == lineWords.Length() - 1 ? End() : Begin() + lineWords[i+1].index; // span of word
            const float xOff = space * (float)i;
            for (CharQuad* v = Begin() + lineWords[i].index; v != end; ++v) // get beginning of word
                v->MoveX(xOff);
        }
        lineWords.Clear(); // prepare for new line
        lineWords.Push({ meshIndex, 0.0f });
    }

    void TextRenderer::AlignX() {
        if (align.IsAlignLeft() && !align.IsCropX()) return; // slight optimization, align left is already by default rendered
            const float restWidth   = align.GetXOff(lineWidth - align.letterSpacing.pointsf());
            const bool shallCrop    = align.IsCropX();
            bool isVisible          = false;
            const bool isAllVisible = lineWidth <= align.rect.width();
            int visibleStart = ~(int)lastLineIndex, visibleEnd = ~(int)(meshIndex - 1); // store 'sign' as an indicator for 'undefined'
            for (u32 i = lastLineIndex; i < meshIndex; ++i) {
                CharAt(i).MoveX(restWidth); // offset all for centering
                if (isAllVisible || !shallCrop) continue; // if everything is visible or you dont need to crop just dont care
                if (!isVisible && visibleStart < 0 && CharAt(i).v1.Position.x >= align.rect.min.x) {
                    isVisible = true;
                    visibleStart = (int)i;
                    const float offset = align.rect.min.x - CharAt(i).v0.Position.x;
                    const float percentage = offset / CharAt(i).Width();
                    const float dTexX = CharAt(i).TextureWidth();
                    CharAt(i).MoveMinX(offset);
                    CharAt(i).MoveTexMinX(dTexX * percentage);
                } else if (isVisible && visibleEnd < 0 && CharAt(i).v1.Position.x >= align.rect.max.x) {
                    visibleEnd = (int)i;
                    const float offset = CharAt(i).v1.Position.x - align.rect.max.x;
                    const float percentage = offset / CharAt(i).Width();
                    const float dTexX = CharAt(i).TextureWidth();
                    CharAt(i).MoveMaxX(-offset);
                    CharAt(i).MoveTexMaxX(-dTexX * percentage);
                    break;
                }
            }
            if (shallCrop && !isAllVisible) {
                visibleStart ^= -(visibleStart < 0);
                visibleEnd ^= -(visibleEnd < 0);
                textVertices.Resize(4 * (visibleEnd + 1));
                textVertices.Erase((int)lastLineIndex * 4, (visibleStart - (int)lastLineIndex) * 4);
                // m - (vs - l) - (m - ve) -> m - vs + l - m + ve -> ve - vs + l
                const u32 removed = visibleStart - lastLineIndex + meshIndex - visibleEnd - 1;
                meshIndex -= removed;
                lastLineIndex -= removed;
            }
    }

    bool TextRenderer::WordWrap(float advance, IterOf<Str>& it, IterOf<Str> begin) {
        if (align.IsWordWrap() && lineWidth + advance > align.rect.width()) { // word wrapping (complex part over here)
            if (it == begin) return false;
            auto backUntilSpace       = it - 1; // new iterator for looping until we reach a space
            const float originalWidth = lineWidth; // keeps track if the word is too long and will always overflow without breaking the word
            do { // man the first time ive use do while
                lineWidth -= align.GetAdvance(font.GetGlyphRect(*backUntilSpace), scaleRatio); // dementia, remove line width with overflowed word
                if (lineWidth <= 0) { // if word is still too long, then abort and just render it on a single line
                    lineWidth = originalWidth; // reset width
                    return false; // abort
                }
                --backUntilSpace; // look back for spaces
            } while (*backUntilSpace != ' ');
            lineWidth -= spaceAdvance; // remember to remove the space as well
            if (align.IsAlignJustified()) lineWords.Pop(); // and delete this overflowed word

            textVertices.Resize(4 * lastSpaceIndex); // forget about mesh data; re-render the word
            meshIndex = lastSpaceIndex; // reset everything back to before the overflowed word
            lastSpaceIndex = 0;
            TriggerNewLine(); // make new line
            it = backUntilSpace; // reset to the character before the overflowed word, and now on new line
            return true; // go back and re-render the rest
        }
        return false;
    }

    void TextRenderer::FixAlignY() {
        if (align.IsVerticalJustified()) {
            if (!lineIndices) return; // no need to align lines NOLINT(cppcoreguidelines-avoid-goto, hicpp-avoid-goto)
            const float yOff = -align.GetYOff(lineSpacing * ((float)lineIndices.Length() * align.lineSpacing + 1))
                                * 2 / (float)lineIndices.Length(); // space between each line
            u32 lineNum = 0; // line number to keep track of offset
            for (u32 i = 0; i < meshIndex; ++i) {
                if (i >= (lineNum >= lineIndices.Length() ? U32_MAX : lineIndices[lineNum])) // last line doesnt exist, so index is at UINT32_MAX
                    ++lineNum; // increments line number if index surpasses next line index
                CharAt(i).MoveY(yOff * (float)lineNum); // offsets points accordingly
            }
        }
    }

    void TextRenderer::ClipY(Math::fRect2D& pos, Math::fRect2D& tex, bool clipTop, bool clipBottom) const {
        if (clipTop && pos.max.y > align.rect.max.y) {
            const float offset = pos.max.y - align.rect.max.y;
            const float percentage = offset / pos.height();
            pos.max.y -= offset;
            tex.min.y += percentage * tex.height();
        }

        if (clipBottom && pos.min.y < align.rect.min.y) {
            const float offset = align.rect.min.y - pos.min.y;
            const float percentage = offset / pos.height();
            pos.min.y += offset;
            tex.max.y -= percentage * tex.height();
        }
    }

    void TextRenderer::PushCharQuad(const Math::fRect2D& pos, const Math::fRect2D& tex) {
        textVertices.Push({ pos.corner(0), tex.corner(0), 1.0f, Vertex::RENDER_TEXT }); // y flipped cuz opengl textures are flipped
        textVertices.Push({ pos.corner(1), tex.corner(1), 1.0f, Vertex::RENDER_TEXT });
        textVertices.Push({ pos.corner(2), tex.corner(2), 1.0f, Vertex::RENDER_TEXT }); // NOLINT(clang-diagnostic-xor-used-as-pow)
        textVertices.Push({ pos.corner(3), tex.corner(3), 1.0f, Vertex::RENDER_TEXT });
    }

    void TextRenderer::AddChar(IterOf<Str>& it, IterOf<Str> begin) {
        using namespace Math;
        const char glyph = *it;
        if (glyph == '\n') {
            TriggerNewLine(); // new line
        }
        if (glyph == ' ') { // space means new word
            TriggerSpace();
            return;
        }
        if (glyph < 32 || glyph >= 127) return; // bounds check for render-able character

        const bool wouldClipTop    = pen.y + font.GetDefaultMetric().fontHeight.pointsf() > align.rect.max.y,
                   wouldClipBottom = pen.y < align.rect.min.y,
                   isVisible       = align.rect.min.y <= pen.y + font.GetDefaultMetric().fontHeight.pointsf() && pen.y <= align.rect.max.y;
        
        if (align.IsCropY() && !isVisible) return;

        const Glyph& rect = font.GetGlyphRect(glyph); // the information for glyphs
        const float advance = align.GetAdvance(rect, scaleRatio); // pre-check if word will overflow
        if (align.IsLetterWrap() && lineWidth + advance > align.rect.width()) { // if too long then wrap this character
            TriggerNewLine(); // new line for wrapping
        }

        const fVector2 rsize = rect.rect.size() * font.textureSize.as<float>(); // real-scale size of the quad
        fRect2D texture = rect.rect;
        fRect2D pos = rect.offset.as<float>().to(fVector2 { rsize.x, -rsize.y }.as_size()) * scaleRatio + pen;

        if (align.IsCropY() && !align.IsVerticalJustified()) {
            ClipY(pos, texture, wouldClipTop, wouldClipBottom);
        }

        PushCharQuad(pos, texture);
        if (WordWrap(advance, it, begin)) return;
            
        ++meshIndex; // meshIndex is not always index, could encounter \n
        pen.x += advance; // move pen
        lineWidth += advance; // incr line width
        if (align.IsAlignJustified()) lineWords.Last().width += advance; // the current word also has to be updated}
    }

    void TextRenderer::AddRichChar(const Text::RichString::Iter& it) {
        using namespace Math;
        const auto [glyph, style] = *it;
        if (glyph == '\n') {
            TriggerNewLine(); // new line
        }
        if (glyph == ' ') { // space means new word
            TriggerSpace();
            return;
        }
        if (glyph < 32 || glyph >= 127) return; // bounds check for render-able character

        const bool isMono = style.codeLang;
        const int fntID = isMono ? Font::MONOSPACE_FONT_ID : Font::DEFAULT_FONT_ID;
        const FontStyle fntStyle = (FontStyle)(style.styleFlags & 3);
        const Font::FontMetrics& metric = font.GetMetric(fntID, fntStyle);

        const bool wouldClipTop    = pen.y + metric.fontHeight.pointsf() > align.rect.max.y,
                   wouldClipBottom = pen.y < align.rect.min.y,
                   isVisible       = align.rect.min.y <= pen.y + metric.fontHeight.pointsf() && pen.y <= align.rect.max.y;
        
        if (align.IsCropY() && !isVisible) return;

        const Glyph& rect = font.GetGlyphRect(glyph, fntStyle, fntID); // the information for glyphs
        const float advance = align.GetAdvance(rect, scaleRatio); // pre-check if word will overflow
        if (align.IsLetterWrap() && lineWidth + advance > align.rect.width()) { // if too long then wrap this character
            TriggerNewLine(); // new line for wrapping
        }

        const fVector2 rsize = rect.rect.size() * font.textureSize.as<float>(); // real-scale size of the quad
        fRect2D texture = rect.rect;
        fRect2D pos = rect.offset.asf().to(fVector2 { rsize.x, -rsize.y }.as_size()) * scaleRatio + pen;

        if (align.IsCropY() && !align.IsVerticalJustified()) {
            ClipY(pos, texture, wouldClipTop, wouldClipBottom);
        }
        
        PushCharQuad(pos, texture);
        // TODO: if (WordWrap(advance, it, begin)) return;
            
        ++meshIndex; // meshIndex is not always index, could encounter \n
        pen.x += advance; // move pen
        lineWidth += advance; // incr line width
        if (align.IsAlignJustified()) lineWords.Last().width += advance; // the current word also has to be updated}
    }

    void TextRenderer::TriggerNewLine() {
        pen.y -= lineSpacing * align.lineSpacing; // moves pen to new square
        pen.x = align.rect.min.x;

        FixAlignX(); // align the line
            
        if (align.IsVerticalJustified() && lineWidth > 0) // add current index to line indices
            lineIndices.Push(meshIndex);
            
        lastLineIndex = meshIndex; // same as above
        lineWidth = 0; // reset new line width 
    }

    void TextRenderer::TriggerSpace() {
        if (align.IsAlignJustified() && lastSpaceIndex < meshIndex) // check if is justify and word is not empty
            lineWords.Push({ meshIndex, 0.0f });
        pen.x += spaceAdvance; // moves pen
        lineWidth += spaceAdvance; // increments line's width
        lastSpaceIndex = meshIndex; // update index
    }

    void TextRenderer::RenderText(Str string) {
        using namespace Math;
        lineCount = (u32)(std::ranges::count(string, '\n') + 1); // line count for vertical alignment
        Prepare();
        
        if (align.IsAlignJustified()) lineWords.Push({ 0, 0.0f }); // add new beginning 'word'

        for (auto it = string.begin(); it != string.end(); ++it) { // loop each char in string (keep in mind not ranged for loop)
            AddChar(it, string.begin());
        }
        FixAlignX(); // dont forget to fix the last line
        FixAlignY();
    }

    void TextRenderer::RenderRichText(const Text::RichString& string) {
        using namespace Math;
        lineCount = string.Lines(); // line count for vertical alignment
        Prepare();
        
        if (align.IsAlignJustified()) lineWords.Push({ 0, 0.0f }); // add new beginning 'word'

        Vec<fRange> monos;
        u32 monoNum = 0;
        const float monoPadding = fontSize.pointsf() * 0.33f;
        auto it = string.begin();
        for (; it != string.end(); ++it) { // loop each char in string (keep in mind not ranged for loop)
            if (it.currentState.codeLang && monos.Length() == monoNum) {
                monos.Push({ pen.x, INF });
                pen.x += monoPadding;
            } else if (!it.currentState.codeLang && monos.Length() != monoNum) {
                pen.x += monoPadding;
                monos.Last().max = pen.x;
                ++monoNum;
            }
            if (*it.iter == '\n') {
                if (it.currentState.codeLang) {
                    pen.x += monoPadding;
                    monos.Last().max = pen.x;
                }
                const float restWidth = align.GetXOff(lineWidth - align.letterSpacing.pointsf());
                for (const fRange monoSpan : monos) {
                    const auto [_, ascend, descent] = font.GetDefaultMetric();
                    AddRoundedRect(
                        { monoSpan.min + restWidth, monoSpan.max + restWidth,
                          pen.y + descent.pointsf() * scaleRatio, pen.y + ascend.pointsf() * scaleRatio },
                        fontSize.pointsf() * 0.3f, fColor::from_hex("27303d")
                    );
                }
                monos.Clear();
                monoNum = 0;
            }
            AddRichChar(it);
        }
        FixAlignX(); // dont forget to fix the last line
        FixAlignY();

        if (!it.currentState.codeLang && monos.Length() != monoNum) {
            pen.x += monoPadding;
            monos.Last().max = pen.x;
        }
        const float restWidth = align.GetXOff(lineWidth - align.letterSpacing.pointsf());
        for (const fRange monoSpan : monos) {
            const auto [_, ascend, descent] = font.GetDefaultMetric();
            AddRoundedRect(
                { monoSpan.min + restWidth, monoSpan.max + restWidth,
                  pen.y + descent.pointsf() * scaleRatio, pen.y + ascend.pointsf() * scaleRatio },
                fontSize.pointsf() * 0.3f, fColor::from_hex("27303d")
            );
        }
    }

    void TextRenderer::AddRoundedRect(const Math::fRect2D& region, float roundRadius, const Math::fColor& color) {
        using namespace Math;
        // rectangle
        const fVector2 y = fVector2::unit_y(roundRadius);
        constexpr int renderType = Vertex::RENDER_FILL;
        const u32 off = (u32)bgVertices.Length();
        bgVertices.Push({ region.corner(0) + y, 0.0f, color, renderType });
        bgVertices.Push({ region.corner(1) + y, 0.0f, color, renderType });
        bgVertices.Push({ region.corner(2) - y, 0.0f, color, renderType });
        bgVertices.Push({ region.corner(3) - y, 0.0f, color, renderType });

        const fVector2 x = fVector2::unit_x(roundRadius);
        bgVertices.Push({ region.corner(2) + x,     0.0f, color, renderType });
        bgVertices.Push({ region.corner(2) + x - y, 0.0f, color, renderType });
        bgVertices.Push({ region.corner(3) - x,     0.0f, color, renderType });
        bgVertices.Push({ region.corner(3) - x - y, 0.0f, color, renderType });

        bgVertices.Push({ region.corner(0) + x,     0.0f, color, renderType });
        bgVertices.Push({ region.corner(0) + x + y, 0.0f, color, renderType });
        bgVertices.Push({ region.corner(1) - x,     0.0f, color, renderType });
        bgVertices.Push({ region.corner(1) - x + y, 0.0f, color, renderType });

        bgIndices.Push(TriIndices { 0,  1,  2 } + off);
        bgIndices.Push(TriIndices { 1,  2,  3 } + off);
        bgIndices.Push(TriIndices { 4,  5,  6 } + off);
        bgIndices.Push(TriIndices { 5,  6,  7 } + off);
        bgIndices.Push(TriIndices { 8,  9, 10 } + off);
        bgIndices.Push(TriIndices { 9, 10, 11 } + off);

        constexpr int cuts = 3, map = 0x00'01'03'02; // just flips 3 with 2
        constexpr float angle = HALF_PI / (float)cuts;
        u32 ind = 12 + off;
        for (int corner = 0; corner < 4; ++corner) {
            fVector2 origin = region.inset(roundRadius).corner(corner);
            bgVertices.Push({ origin, 0.0f, color, renderType });
            for (int i = 0; i < cuts; ++i) {
                bgVertices.Push({
                    fVector2::from_polar(roundRadius,
                        angle * (float)i + HALF_PI * (float)(map >> corner * 8 & 255))
                    + origin, 0.0f, color, renderType });
                bgIndices.Push({ ind, ind + 1 + i, ind + 2 + i });
            }
            bgVertices.Push({
                fVector2::from_polar(roundRadius,
                    HALF_PI * (float)(1 + (map >> corner * 8 & 255)))
                + origin, 0.0f, color, renderType });
            ind += cuts + 2;
        }
    }
}
