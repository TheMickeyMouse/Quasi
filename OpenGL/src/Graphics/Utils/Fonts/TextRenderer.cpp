#include "TextRenderer.h"


namespace Graphics {
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

    void TextRenderer::Prepare() {
        scaleRatio = fontSize / font.fontSize; // scaling ratio
        lineSpacing = font.fontHeight.pointsf() * scaleRatio; // just a calculation (without * align.lineSpacing)
        spaceAdvance = align.GetAdvance(font, ' ', scaleRatio); // this gets used alot, just here
        
        totalHeight = lineSpacing * (((float)lineCount - 1) * align.lineSpacing + 1);
        pen = align.rect.corner(2);
        pen.y -= lineSpacing;
        pen.y -= align.IsVerticalJustified() ? 0 : align.GetYOff(totalHeight);
    }

    TextRenderer::CharQuad* TextRenderer::Begin() {
        return (CharQuad*)vertices.data();
    }
    const TextRenderer::CharQuad* TextRenderer::Begin() const {
        return (const CharQuad*)vertices.data();
    }
    TextRenderer::CharQuad* TextRenderer::End() {
        return (CharQuad*)(vertices.data() + vertices.size());
    }
    const TextRenderer::CharQuad* TextRenderer::End() const {
        return (const CharQuad*)(vertices.data() + vertices.size());
    }
    TextRenderer::CharQuad& TextRenderer::CharAt(uint index) {
        return Begin()[index];
    }
    const TextRenderer::CharQuad& TextRenderer::CharAt(uint index) const {
        return Begin()[index];
    }

    void TextRenderer::FixAlignX() { // quick function for reusing code, aligns current line
        if (align.IsAlignJustified())
            JustifyAlignX();
        else
            AlignX();
    }

    void TextRenderer::JustifyAlignX() {
        const float restWidth = 2 * align.GetXOff(lineWidth - align.letterSpacing.pointsf());
        if (lineWords.size() < 2) return; // no need to align
        if (lastSpaceIndex == meshIndex) lineWords.pop_back(); // check if this 'word' is necessary (is not empty or whitespace)
        const float space = restWidth / (float)(lineWords.size() - 1); // amt of space between each word (accounts for existing spaces)
        for (uint i = 1; i < lineWords.size(); ++i) {
            const auto end = i == lineWords.size() - 1 ? End() : Begin() + lineWords[i+1].index; // span of word
            const float xOff = space * (float)i;
            for (CharQuad* v = Begin() + lineWords[i].index; v != end; ++v) // get beginning of word
                v->MoveX(xOff);
        }
        lineWords.clear(); // prepare for new line
        lineWords.emplace_back(meshIndex, 0.0f);
    }

    void TextRenderer::AlignX() {
        if (align.IsAlignLeft() && !align.IsCropX()) return; // slight optimization, align left is already by default rendered
            const float restWidth   = align.GetXOff(lineWidth - align.letterSpacing.pointsf());
            const bool shallCrop    = align.IsCropX();
            bool isVisible          = false;
            const bool isAllVisible = lineWidth <= align.rect.width();
            int visibleStart = ~(int)lastLineIndex, visibleEnd = ~(int)(meshIndex - 1); // store 'sign' as an indicator for 'undefined'
            for (uint i = lastLineIndex; i < meshIndex; ++i) {
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
                vertices.resize(4 * (visibleEnd + 1));
                vertices.erase(vertices.begin() + (int)lastLineIndex * 4, vertices.begin() + visibleStart * 4);
                // m - (vs - l) - (m - ve) -> m - vs + l - m + ve -> ve - vs + l
                const uint removed = visibleStart - lastLineIndex + meshIndex - visibleEnd - 1;
                meshIndex -= removed;
                lastLineIndex -= removed;
            }
    }

    bool TextRenderer::WordWrap(float advance, std::string::const_iterator& it,
                                               std::string::const_iterator  begin) {
        if (align.IsWordWrap() && lineWidth + advance > align.rect.width()) { // word wrapping (complex part over here)
            if (it == begin) return false;  // NOLINT(cppcoreguidelines-avoid-goto, hicpp-avoid-goto)
            auto backUntilSpace       = it - 1; // new iterator for looping until we reach a space
            const float originalWidth = lineWidth; // keeps track if the word is too long and will always overflow without breaking the word
            do { // man the first time ive use do while
                lineWidth -= align.GetAdvance(font, *backUntilSpace, scaleRatio); // dementia, remove line width with overflowed word
                if (lineWidth <= 0) { // if word is still too long, then abort and just render it on a single line
                    lineWidth = originalWidth; // reset width
                    return false; // abort
                }
                --backUntilSpace; // look back for spaces
            } while (*backUntilSpace != ' ');
            lineWidth -= spaceAdvance; // remember to remove the space as well
            if (align.IsAlignJustified()) lineWords.pop_back(); // and delete this overflowed word 

            vertices.resize(4 * lastSpaceIndex); // forget about mesh data; re-render the word
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
            if (lineIndices.empty()) return; // no need to align lines NOLINT(cppcoreguidelines-avoid-goto, hicpp-avoid-goto)
            const float yOff = -align.GetYOff(lineSpacing * ((float)lineIndices.size() * align.lineSpacing + 1))
                                * 2 / (float)lineIndices.size(); // space between each line
            uint lineNum = 0; // line number to keep track of offset
            for (uint i = 0; i < meshIndex; ++i) {
                if (i >= (lineNum >= lineIndices.size() ? UINT32_MAX : lineIndices[lineNum])) // last line doesnt exist, so index is at UINT32_MAX
                    ++lineNum; // increments line number if index surpasses next line index
                CharAt(i).MoveY(yOff * (float)lineNum); // offsets points accordingly
            }
        }
    }

    void TextRenderer::ClipY(Maths::rect2f& pos, Maths::rect2f& tex, bool clipTop, bool clipBottom) const {
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

    void TextRenderer::PushCharQuad(const Maths::rect2f& pos, const Maths::rect2f& tex) {
        vertices.push_back({ pos.corner(0), tex.corner(0 ^ 2 /* flip Y with xor */) }); // y flipped cuz opengl textures are flipped
        vertices.push_back({ pos.corner(1), tex.corner(1 ^ 2 /* flip Y with xor */) });
        // ReSharper disable once CppIdenticalOperandsInBinaryExpression
        vertices.push_back({ pos.corner(2), tex.corner(2 ^ 2 /* flip Y with xor */) }); // NOLINT(clang-diagnostic-xor-used-as-pow)
        vertices.push_back({ pos.corner(3), tex.corner(3 ^ 2 /* flip Y with xor */) });
    }

    void TextRenderer::TriggerNewLine() {
        pen.y -= lineSpacing * align.lineSpacing; // moves pen to new square
        pen.x = align.rect.min.x;

        FixAlignX(); // align the line
            
        if (align.IsVerticalJustified() && lineWidth > 0) // add current index to line indices
            lineIndices.push_back(meshIndex);
            
        lastLineIndex = meshIndex; // same as above
        lineWidth = 0; // reset new line width 
    }

    void TextRenderer::TriggerSpace() {
        if (align.IsAlignJustified() && lastSpaceIndex < meshIndex) // check if is justify and word is not empty
            lineWords.emplace_back(meshIndex, 0.0f);
        pen.x += spaceAdvance; // moves pen
        lineWidth += spaceAdvance; // increments line's width
        lastSpaceIndex = meshIndex; // update index
    }

    std::vector<Font::Vertex> TextRenderer::RenderText(const std::string& string) {
        using namespace Maths;
        lineCount = std::ranges::count(string, '\n') + 1; //line count for vertical alignment
        Prepare();
            
        bool wouldClipTop    = pen.y + font.fontHeight.pointsf() > align.rect.max.y,
             wouldClipBottom = pen.y < align.rect.min.y,
             isVisible       = align.rect.min.y <= pen.y + font.fontHeight.pointsf() && pen.y <= align.rect.max.y;
        if (align.IsAlignJustified()) lineWords.emplace_back(0, 0.0f); // add new beginning 'word'

        for (auto it = string.begin(); it != string.end(); ++it) { // loop each char in string (keep in mind not ranged for loop)
            const char glyph = *it;
            if (glyph == '\n') {
                TriggerNewLine(); // new line
                wouldClipTop    = pen.y + font.fontHeight.pointsf() > align.rect.max.y;
                wouldClipBottom = pen.y < align.rect.min.y;
                isVisible       = align.rect.min.y <= pen.y + font.fontHeight.pointsf() && pen.y <= align.rect.max.y;
            }
            if (glyph == ' ') { // space means new word
                TriggerSpace();
                continue;
            }
            if (glyph < 32 || glyph >= 127) continue; // bounds check for render-able characters
            
            if (align.IsCropY() && !isVisible) continue;

            const Font::Glyph& rect = font.GetGlyphRect(glyph); // the information for glyphs
            const float advance = align.GetAdvance(font, glyph, scaleRatio); // pre-check if word will overflow
            if (align.IsLetterWrap() && lineWidth + advance > align.rect.width()) { // if too long then wrap this character
                TriggerNewLine(); // new line for wrapping
            }

            const fvec2 rsize = rect.rect.size() * font.textureSize; // real-scale size of the quad
            rect2f texture = rect.rect;
            rect2f pos = rect.offset.as<float>().to(fvec2 { rsize.x, -rsize.y }.as_size()) * scaleRatio + pen;

            if (align.IsCropY() && !align.IsVerticalJustified()) {
                ClipY(pos, texture, wouldClipTop, wouldClipBottom);
            }

            PushCharQuad(pos, texture);
            if (WordWrap(advance, it, string.begin())) continue;
            
            ++meshIndex; // meshIndex is not always index, could encounter \n
            pen.x += advance; // move pen
            lineWidth += advance; // incr line width
            if (align.IsAlignJustified()) lineWords.back().width += advance; // the current word also has to be updated
        }
        FixAlignX(); // dont forget to fix the last line
        FixAlignY();
        
        return vertices;
    }
}
