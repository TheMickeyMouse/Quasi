#include "Font.h"

#include <numeric>

#include "Debugging.h"

namespace Graphics {
    Font::~Font() {
        FT_Done_Face(faceHandle);
    }

    void Font::Transfer(Font& dest, Font&& from) {
        dest.faceHandle = from.faceHandle;
        from.faceHandle = nullptr;
        
        dest.fontSize   = from.fontSize;
        dest.fontHeight = from.fontHeight;

        dest.glyphs = std::move(from.glyphs);
    }

    void Font::SetSize(PointPer64 charWidth, PointPer64 charHeight, uint dpi) {
        const int error = FT_Set_Char_Size(faceHandle, charWidth, charHeight == 0 ? charWidth : charHeight, dpi, dpi);
        fontSize = charWidth;
        if (error) LOG("[ERR]: font char size set with error code " << error);
    }

    void Font::RenderBitmap() {
        using namespace Maths;
        const FT_GlyphSlot glyphHandle = faceHandle->glyph;
        int pen = 0; // x value of pen

        constexpr int loadSDF = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF); // used for sdf rendering
        textureSize = 0;
        for (uchar charCode = 32; charCode < 127; ++charCode) { // loop through each character
            if (const int error = FT_Load_Char(faceHandle, charCode, loadSDF)) { // loads the sdf
                LOG("[ERR]: loading char with error code " << error);
                continue;  /* ignore errors */
            }

            textureSize.y = std::max(textureSize.y, glyphHandle->bitmap.rows); // y is guaranteed to fit the sdf
            textureSize.x += glyphHandle->bitmap.width; // packs sdfs together on the x axis
        }
        
        atlas = Texture(nullptr, textureSize.x, textureSize.y, true, GL_RED, 1); // create blank texture
        atlas.SetActive(0); // set this texture to the active one
        glyphs.resize(127-32); // amt of glyphs
        for (uchar charCode = 32; charCode < 127; ++charCode) { // loop through again, this time drawing textures
            if (const int error = FT_Load_Char(faceHandle, charCode, loadSDF)) { // loads again
                LOG("[ERR]: loading char with error code " << error);
                continue;
            }
            
            fontHeight = PointPer64::inP64(std::max( // this is the y value for new line, definition:
                (faceHandle->size->metrics.ascender - faceHandle->size->metrics.descender),
                (long)fontHeight.pointsPer64()));
            
            const uvec2 size = { glyphHandle->bitmap.width, glyphHandle->bitmap.rows }; // construct size in pixels of the texture
            atlas.SetSubTexture(glyphHandle->bitmap.buffer, uvec2 { pen, 0 }.to(size.as_size()), GL_RED); // draw the sub texture

            Glyph& glyph = glyphs[charCode - 32]; // write rendering memory
            glyph.rect = fvec2 { pen, 0 }.to(size.as<float>().as_size()) / textureSize; // rect of texture in atlas
            glyph.advance = { (float)glyphHandle->advance.x / 64.0f, (float)glyphHandle->advance.y / 64.0f }; // pen move
            glyph.offset  = { glyphHandle->bitmap_left, glyphHandle->bitmap_top }; // offset from pen

            pen += (int)size.x; // moves pen
        }
    }

    const Font::Glyph& Font::GetGlyphRect(char c) const {
        return glyphs[c - 32];
    }

    Mesh<Font::Vertex> Font::RenderString(const std::string& string, PointPer64 size, const StringAlign& align) const {
        // im gonna try and comment this cuz this is pretty complicated
        using namespace Maths;
        const float scaleRatio = size / fontSize; // scaling ratio
        const float lineSpacing = fontHeight.pointsf() * scaleRatio; // just a calculation (without * align.lineSpacing)
        const float spaceAdvance = align.GetAdvance(*this, ' ', scaleRatio); // this gets used alot, just here
        fvec2 pen = align.rect.corner(2); // the current pen position (for drawing)
        pen.y -= lineSpacing; // move pen down 1 line
        uint meshIndex = 0; // mesh index for keeping track of vertices
        const uint lineCount = std::ranges::count(string, '\n') + 1; //line count for vertical alignment
        const float totalHeight = lineSpacing * ((float)(lineCount - 1) * align.lineSpacing + 1);
        const float restHeight = align.GetYOff(totalHeight);
        pen.y -= align.IsVerticalJustified() ? 0 : restHeight;

        uint lastLineIndex = 0, lastSpaceIndex = 0; // history, last line for alignment and last space for word wrapping
        struct Word { uint index; float width; }; // (aka letters with spaces at front and back, also keeps track of width)
        std::vector<Word> lineWords; // the words on current line (only used for justify x)
        std::vector<uint> lineIndices; // starting index for each line (only used for justify y)
        float lineWidth = 0; // current line width, for centering
        const bool isJustifyX = align.IsAlignJustified(), isJustifyY = align.IsVerticalJustified();

        const int wrapMode = align.IsWordWrap() + (align.IsLetterWrap() << 1); // 0 if no wrap, 1 if word wrap, and 2 if break words
        
        if (isJustifyX) lineWords.emplace_back(0, 0.0f); // add new beginning 'word'

        std::vector<Vertex> vert; // for meshes

        const auto fixAlignX = [&, isJustifyX] { // quick function for reusing code, aligns current line
            const float restWidth = align.GetXOff(lineWidth - align.letterSpacing.pointsf()); // half the empty space width (is doubled if alignment is right) 
            if (isJustifyX) {
                if (lineWords.size() < 2) return; // no need to align
                if (lastSpaceIndex == meshIndex) lineWords.pop_back(); // check if this 'word' is necessary (is not empty or whitespace)
                const float space = restWidth * 2 / (float)(lineWords.size() - 1); // amt of space between each word (accounts for existing spaces)
                for (uint i = 1; i < lineWords.size(); ++i) {
                    const Vertex* end = i == lineWords.size() - 1 ? &vert.back() + 1 : vert.data() + lineWords[i+1].index * 4; // span of word
                    for (Vertex* v = vert.data() + lineWords[i].index * 4; v < end; ++v) { // get beginning of word
                        v->Position.x += space * (float)i; // increment for justify x
                    }
                }
                lineWords.clear(); // prepare for new line
                lineWords.emplace_back(meshIndex, 0.0f);
            } else {
                if (align.IsAlignLeft() && !align.IsCropX()) return; // slight optimization, align left is already by default rendered
                const bool shallCrop    = align.IsCropX();
                bool isVisible          = false;
                const bool isAllVisible = lineWidth <= align.rect.width();
                int visibleStart = ~(int)lastLineIndex, visibleEnd = ~(int)(meshIndex - 1); // store 'sign' as an indicator for 'undefined'
                for (uint i = lastLineIndex; i < meshIndex; ++i) {
                    vert[i * 4 + 0].Position.x += restWidth;
                    vert[i * 4 + 1].Position.x += restWidth;
                    vert[i * 4 + 2].Position.x += restWidth;
                    vert[i * 4 + 3].Position.x += restWidth; // offset all for centering
                    if (isAllVisible || !shallCrop) continue; // if everything is visible or you dont need to crop just dont care
                    if (!isVisible && visibleStart < 0 && vert[i * 4 + 1].Position.x >= align.rect.min.x) {
                        isVisible = true;
                        visibleStart = (int)i;
                        const float offset = vert[i * 4].Position.x - align.rect.min.x;
                        const float percentage = -offset / (vert[i * 4 + 1].Position.x - vert[i * 4].Position.x);
                        const float dTexX = vert[i * 4 + 1].TextureCoord.x - vert[i * 4].TextureCoord.x;
                        vert[i * 4 + 0].Position.x -= offset;
                        vert[i * 4 + 2].Position.x -= offset;
                        vert[i * 4 + 0].TextureCoord.x += dTexX * percentage;
                        vert[i * 4 + 2].TextureCoord.x += dTexX * percentage;
                    } else if (isVisible && visibleEnd < 0 && vert[i * 4 + 1].Position.x >= align.rect.max.x) {
                        visibleEnd = (int)i;
                        const float offset = vert[i * 4 + 1].Position.x - align.rect.max.x;
                        const float percentage = offset / (vert[i * 4 + 1].Position.x - vert[i * 4].Position.x);
                        const float dTexX = vert[i * 4 + 1].TextureCoord.x - vert[i * 4].TextureCoord.x;
                        vert[i * 4 + 1].Position.x -= offset;
                        vert[i * 4 + 3].Position.x -= offset;
                        vert[i * 4 + 1].TextureCoord.x -= dTexX * percentage;
                        vert[i * 4 + 3].TextureCoord.x -= dTexX * percentage;
                        break;
                    }
                }
                if (shallCrop && !isAllVisible) {
                    visibleStart ^= -(visibleStart < 0);
                    visibleEnd ^= -(visibleEnd < 0);
                    vert.resize(4 * (visibleEnd + 1));
                    vert.erase(vert.begin() + (int)lastLineIndex * 4, vert.begin() + visibleStart * 4);
                    // m - (vs - l) - (m - ve) -> m - vs + l - m + ve -> ve - vs + l
                    const uint removed = visibleStart - lastLineIndex + meshIndex - visibleEnd - 1;
                    meshIndex -= removed;
                    lastLineIndex -= removed;
                }
            }
        };

        const auto triggerNewLine = [&] { // function called to add a new line
            pen.y -= lineSpacing * align.lineSpacing; // moves pen to new square
            pen.x = align.rect.min.x;

            fixAlignX(); // align the line
            
            if (isJustifyY && lineWidth > 0) // add current index to line indices
                lineIndices.push_back(meshIndex);
            
            lastLineIndex = meshIndex; // same as above
            lineWidth = 0; // reset new line width 
        };
        
        for (auto it = string.begin(); it != string.end(); ++it) { // loop each char in string (keep in mind not ranged for loop)
            const char glyph = *it;
            if (glyph == '\n') {
                triggerNewLine(); // new line
            }
            if (glyph == ' ') { // space means new word
                if (isJustifyX && lastSpaceIndex < meshIndex) // check if is justify and word is not empty
                    lineWords.emplace_back(meshIndex, 0.0f);
                pen.x += spaceAdvance; // moves pen
                lineWidth += spaceAdvance; // increments line's width
                lastSpaceIndex = meshIndex; // update index
                continue;
            }
            if (glyph < 32 || glyph >= 127) continue; // bounds check for render-able characters

            const Glyph& rect = GetGlyphRect(glyph); // the information for glyphs
            
            const float advance = align.GetAdvance(*this, glyph, scaleRatio); // pre-check if word will overflow
            if (wrapMode == 2 && lineWidth + advance > align.rect.width()) { // if too long then wrap this character
                triggerNewLine(); // new line for wrapping
            }

            const bool wouldClipTop    = pen.y + fontHeight.pointsf() > align.rect.max.y,
                       wouldClipBottom = pen.y < align.rect.min.y,
                       isVisible       = align.rect.min.y <= pen.y + fontHeight.pointsf() && pen.y <= align.rect.max.y;

            if (align.IsCropY() && !isVisible) continue;

            const fvec2 rsize = rect.rect.size() * textureSize; // real-scale size of the quad
            rect2f texture = rect.rect;
            rect2f pos = rect.offset.as<float>().to(fvec2 { rsize.x, -rsize.y }.as_size()) * scaleRatio + pen;

            if (align.IsCropY() && !align.IsVerticalJustified()) {
                if (wouldClipTop && pos.max.y > align.rect.max.y) {
                    const float offset = pos.max.y - align.rect.max.y;
                    const float percentage = offset / pos.height();
                    pos.max.y -= offset;
                    texture.min.y += percentage * texture.height();
                }

                if (wouldClipBottom && pos.min.y < align.rect.min.y) {
                    const float offset = align.rect.min.y - pos.min.y;
                    const float percentage = offset / pos.height();
                    pos.min.y += offset;
                    texture.max.y -= percentage * texture.height();
                }
            }
            
            // ^^ gets relative offset, constructs a quad with side lengths of rsize, then scaled and moved to pen
            vert.push_back({ pos.corner(0), texture.corner(0 ^ 2 /* flip Y with xor */) }); // y flipped cuz opengl textures are flipped
            vert.push_back({ pos.corner(1), texture.corner(1 ^ 2 /* flip Y with xor */) });
            // ReSharper disable once CppIdenticalOperandsInBinaryExpression
            vert.push_back({ pos.corner(2), texture.corner(2 ^ 2 /* flip Y with xor */) }); // NOLINT(clang-diagnostic-xor-used-as-pow)
            vert.push_back({ pos.corner(3), texture.corner(3 ^ 2 /* flip Y with xor */) });

            if (wrapMode == 1 && lineWidth + advance > align.rect.width()) { // word wrapping (complex part over here)
                auto backUntilSpace       = it - 1; // new iterator for looping until we reach a space
                const float originalWidth = lineWidth; // keeps track if the word is too long and will always overflow without breaking the word
                while (*backUntilSpace != ' ') {
                    lineWidth -= align.GetAdvance(*this, *backUntilSpace, scaleRatio); // dementia, remove line width with overflowed word
                    if (lineWidth <= 0) { // if word is still too long, then abort and just render it on a single line
                        lineWidth = originalWidth; // reset width
                        goto escape; // abort
                    }
                    --backUntilSpace; // look back for spaces
                }
                lineWidth -= spaceAdvance; // remember to remove the space as well
                if (isJustifyX) lineWords.pop_back(); // and delete this overflowed word 

                vert.resize(4 * lastSpaceIndex); // forget about mesh data; re-render the word
                meshIndex = lastSpaceIndex; // reset everything back to before the overflowed word
                lastSpaceIndex = 0;
                triggerNewLine(); // make new line
                it = backUntilSpace; // reset to the character before the overflowed word, and now on new line
                continue; // go back and re-render the rest
            }

            escape: // abort destination
            ++meshIndex; // meshIndex is not always index, could encounter \n
            pen.x += advance; // move pen
            lineWidth += advance; // incr line width
            if (isJustifyX) lineWords.back().width += advance; // the current word also has to be updated
        }
        fixAlignX(); // dont forget to fix the last line
        
        if (isJustifyY) {
            if (lineIndices.empty()) goto finish; // no need to align lines  // NOLINT(cppcoreguidelines-avoid-goto, hicpp-avoid-goto)
            const float yOff = -restHeight * 2 / (float)lineIndices.size(); // space between each line
            uint lineNum = 0; // line number to keep track of offset
            for (uint i = 0; i < meshIndex; ++i) {
                if (i >= (lineNum >= lineIndices.size() ? UINT32_MAX : lineIndices[lineNum])) // last line doesnt exist, so index is at UINT32_MAX
                    ++lineNum; // increments line number if index surpasses next line index
                vert[i * 4 + 0].Position.y += yOff * (float)lineNum; // offsets points accordingly
                vert[i * 4 + 1].Position.y += yOff * (float)lineNum;
                vert[i * 4 + 2].Position.y += yOff * (float)lineNum;
                vert[i * 4 + 3].Position.y += yOff * (float)lineNum;
            }
        }

        // [&] { // runs a sub-function so i can use 'return', used for aligning lines vertically
        //     const float totalHeight = lineSpacing * ((float)(lineCount - 1) * align.lineSpacing + 1);
        //     const float restHeight = align.GetYOff(totalHeight);
        //     // ^^ calculates half empty space on y axis, with the amt of used space being N * lineSpacing + (N-1) * in-betweens 
        //     if (isJustifyY) {
        //         if (lineIndices.empty()) return; // no need to align lines
        //         const float yOff = -restHeight * 2 / (float)lineIndices.size(); // space between each line
        //         uint lineNum = 0; // line number to keep track of offset
        //         for (uint i = 0; i < meshIndex; ++i) {
        //             if (i >= (lineNum >= lineIndices.size() ? UINT32_MAX : lineIndices[lineNum])) // last line doesnt exist, so index is at UINT32_MAX
        //                 ++lineNum; // increments line number if index surpasses next line index
        //             vert[i * 4 + 0].Position.y += yOff * (float)lineNum; // offsets points accordingly
        //             vert[i * 4 + 1].Position.y += yOff * (float)lineNum;
        //             vert[i * 4 + 2].Position.y += yOff * (float)lineNum;
        //             vert[i * 4 + 3].Position.y += yOff * (float)lineNum;
        //         }
        //     } else {
        //         if (align.IsVerticalTop() && !align.IsCropY()) return; // no need to center if top aligned, because by default its rendering on top
        //         const bool shallCrop    = align.IsCropY();
        //         bool isVisible          = false;
        //         const bool isAllVisible = totalHeight <= align.rect.height();
        //         int visibleStart = ~0, visibleEnd = ~(int)(meshIndex - 1); // store 'sign' as an indicator for 'undefined'
        //         for (uint i = 0; i < meshIndex; ++i) {
        //             vert[i * 4 + 0].Position.y -= restHeight;
        //             vert[i * 4 + 1].Position.y -= restHeight;
        //             vert[i * 4 + 2].Position.y -= restHeight;
        //             vert[i * 4 + 3].Position.y -= restHeight; // rest height is automatically doubled if is bottom aligned, center all text
        //             if (isAllVisible || !shallCrop) continue;
        //             if (!isVisible && visibleStart < 0 && vert[i * 4 + 2].Position.y <= align.rect.max.y) {
        //                 isVisible = true;
        //                 for (uint j = i; j < meshIndex; ++j) {
        //                     if (vert[j * 4 + 2].Position.y <= align.rect.max.y)
        //                         break;
        //                     const float offset = vert[j * 4 + 2].Position.y - align.rect.max.y;
        //                     const float percentage = offset / (vert[j * 4 + 2].Position.y - vert[j * 4].Position.y);
        //                     const float dTexY = vert[j * 4 + 2].TextureCoord.y - vert[j * 4].TextureCoord.y;
        //                     vert[j * 4 + 2].Position.y -= offset;
        //                     vert[j * 4 + 3].Position.y -= offset;
        //                     vert[j * 4 + 2].TextureCoord.y -= dTexY * percentage;
        //                     vert[j * 4 + 3].TextureCoord.y -= dTexY * percentage;
        //                 }
        //                 visibleStart = (int)i;
        //             } else if (isVisible && visibleEnd < 0 && vert[i * 4].Position.y <= align.rect.min.y) {
        //                 for (uint j = i; j < meshIndex; ++j) {
        //                     if (vert[j * 4].Position.y < align.rect.min.y) {
        //                         visibleEnd = (int)j;
        //                         break;
        //                     }
        //                     const float offset = align.rect.min.y - vert[j * 4].Position.y;
        //                     const float percentage = offset / (vert[j * 4 + 2].Position.y - vert[j * 4].Position.y);
        //                     const float dTexY = vert[j * 4 + 2].TextureCoord.y - vert[j * 4].TextureCoord.y;
        //                     vert[j * 4 + 0].Position.y += offset;
        //                     vert[j * 4 + 1].Position.y += offset;
        //                     vert[j * 4 + 0].TextureCoord.y += dTexY * percentage;
        //                     vert[j * 4 + 1].TextureCoord.y += dTexY * percentage;
        //                 }
        //                 break;
        //             }
        //         }
        //         if (shallCrop && !isAllVisible) {
        //             visibleStart ^= -(visibleStart < 0);
        //             visibleEnd ^= -(visibleEnd < 0);
        //             vert.resize(4 * visibleEnd);
        //             vert.erase(vert.begin(), vert.begin() + visibleStart * 4);
        //         }
        //     }
        // }();
        finish:

        std::vector<TriIndices> ind { vert.size() / 2 };
        for (uint i = 0; i < ind.size(); i += 2) {
            ind[i]     = TriIndices { 0, 1, 2 } + i * 2;
            ind[i + 1] = TriIndices { 1, 2, 3 } + i * 2;
        }
        
        return Mesh { std::move(vert), std::move(ind) }; // return the text mesh
    }

    Font Font::LoadFile(const std::string& filename) {
        Font font;
        const int error = FT_New_Face(FontDevice::Library(), filename.c_str(), 0, &font.faceHandle);
        if (!error) return font;
        
        if (error == FT_Err_Unknown_File_Format)
            LOG("[ERR]: font " << filename << " doesn't have valid format");
        else
            LOG("[ERR]: font load with error code " << error);
        return {};
    }

    Font Font::LoadBytes(const uchar* data, uint len) {
        Font font;
        const int error = FT_New_Memory_Face(FontDevice::Library(), data, (long)len, 0, &font.faceHandle);
        if (!error) return font;
        
        LOG("[ERR]: font load with error code " << error);
        return {};
    }
}
