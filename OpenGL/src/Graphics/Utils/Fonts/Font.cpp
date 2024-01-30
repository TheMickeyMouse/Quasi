#include "Font.h"
#include "TextRenderer.h"

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

    Mesh<Font::Vertex> Font::RenderText(const std::string& string, PointPer64 size, const TextAlign& align) const {
        TextRenderer text { *this };
        text.SetAlign(align);
        text.SetFontSize(size);
        text.RenderText(string);

        std::vector<TriIndices> ind { text.vertices.size() / 2 };
        for (uint i = 0; i < ind.size(); i += 2) {
            ind[i]     = TriIndices { 0, 1, 2 } + i * 2;
            ind[i + 1] = TriIndices { 1, 2, 3 } + i * 2;
        }
        
        return Mesh { std::move(text.vertices), std::move(ind) }; // return the text mesh
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
