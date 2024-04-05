#include "Font.h"
#include "TextRenderer.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "GLDebug.h"

namespace Graphics {
    void FontDeleter::operator()(FT_FaceRec_* ptr) const {
        if (ptr) FT_Done_Face(ptr);
    }

    void Font::Transfer(Font& dest, Font&& from) {
        dest.faceHandles = std::move(from.faceHandles);
        
        dest.fontSize = from.fontSize;
        dest.metrics  = std::move(from.metrics);

        dest.glyphs = std::move(from.glyphs);
    }

    void Font::SetSize(PointPer64 charWidth, PointPer64 charHeight, uint dpi) {
        fontSize = charWidth;
        for (const FaceHandle& faceHandle : faceHandles) {
            if (!faceHandle) continue;
            const int error = FT_Set_Char_Size(faceHandle.get(), charWidth, charHeight == 0 ? charWidth : charHeight, dpi, dpi);
            GLLogger().AssertFmt(!error, {"Font Char size set with err code {}"}, error);
        }
    }

    void Font::RenderBitmap() {
        using namespace Maths;
        int pen = 0; // x value of pen

        constexpr int loadSDF = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF); // used for sdf rendering
        constexpr int sdfExtrude = 16;

        textureSize = 0;
        std::vector<uint> heights;
        heights.reserve(faceHandles.size());
        for (const FaceHandle& faceHandle : faceHandles) {
            const FT_GlyphSlot glyphHandle = faceHandle->glyph;
            uint y = 0;
            uint x = 0;
            for (char charCode = 32; charCode < 127; ++charCode) { // loop through each character
                // new method: SDFs extrude 8 pixels in all directions, so you can load normally and then add 16
                // (no need to load twice!)
                if (const int error = FT_Load_Char(faceHandle.get(), charCode, FT_LOAD_DEFAULT)) { 
                    GLLogger().Error({"Loading char with err code {}"}, error);
                    continue;  /* ignore errors */
                }

                y = std::max(y, glyphHandle->bitmap.rows); // y is guaranteed to fit the sdf
                x += glyphHandle->bitmap.width; // packs sdfs together on the x axis
            }
            y += sdfExtrude;
            x += (NUM_GLYPHS - 1 /* space */) * sdfExtrude;
            heights.push_back(textureSize.y);
            textureSize.x = std::max(textureSize.x, x);
            textureSize.y += y;
        }

        Texture::SetPixelStore(PixelStoreParam::UNPACK_ALIGNMENT, 1);
        atlas = Texture(
            nullptr, { textureSize.x, textureSize.y },
            {
                .load = { .format = TextureFormat::RED, .internalformat = TextureIFormat::RGBA_8 },
            }
        ); // create blank texture
        atlas.Bind(); // set this texture to the active one
        glyphs.resize(NUM_GLYPHS * faceHandles.size()); // amt of glyphs-
        metrics.reserve(faceHandles.size());
        for (uint i = 0; i < faceHandles.size(); ++i) {
            const FaceHandle& faceHandle = faceHandles[i];
            const FT_GlyphSlot glyphHandle = faceHandle->glyph;
            metrics.emplace_back(
                PointPer64::inP64(faceHandle->size->metrics.ascender - faceHandle->size->metrics.descender),
                PointPer64::inP64(faceHandle->size->metrics.ascender),
                PointPer64::inP64(faceHandle->size->metrics.descender)
            );
            for (char charCode = 32; charCode < 127; ++charCode) { // loop through again, this time drawing textures
                if (const int error = FT_Load_Char(faceHandle.get(), charCode, loadSDF)) { // loads again
                    GLLogger().Error({"Loading char with err code {}"}, error);
                    continue;
                }
            
                const uvec2 size = { glyphHandle->bitmap.width, glyphHandle->bitmap.rows }; // construct size in pixels of the texture
                atlas.SetSubTexture(glyphHandle->bitmap.buffer, uvec2 { pen, heights[i] }.to(size.as_size()), { .format = TextureFormat::RED }); // draw the sub texture

                Glyph& glyph = glyphs[charCode - 32 + i * NUM_GLYPHS]; // write rendering memory
                glyph.rect = fvec2 { pen, heights[i] }.to(size.asf().as_size()) / textureSize.asf(); // rect of texture in atlas
                glyph.advance = { (float)glyphHandle->advance.x / 64.0f, (float)glyphHandle->advance.y / 64.0f }; // pen move
                glyph.offset  = { glyphHandle->bitmap_left, glyphHandle->bitmap_top }; // offset from pen

                pen += (int)size.x; // moves pen
            }
            pen = 0;
        }
        Texture::SetPixelStore(PixelStoreParam::UNPACK_ALIGNMENT, 4);
    }

    const Glyph& Font::GetGlyphRect(char c, FontStyle style, int id) const {
        return glyphs[c - 32 + NUM_GLYPHS * (id * 4 + (int)style)];
    }

    Mesh<Font::Vertex> Font::RenderText(const std::string& string, PointPer64 size, const TextAlign& align) const {
        TextRenderer text { *this };
        text.SetAlign(align);
        text.SetFontSize(size);
        text.RenderText(string);

        std::vector<TriIndices> ind { text.textVertices.size() / 2 };
        for (uint i = 0; i < ind.size(); i += 2) {
            ind[i]     = TriIndices { 0, 1, 2 } + i * 2;
            ind[i + 1] = TriIndices { 1, 2, 3 } + i * 2;
        }
        
        return Mesh { std::move(text.textVertices), std::move(ind) }; // return the text mesh
    }

    Mesh<Font::Vertex> Font::RenderRichText(const stdu::rich_string& string, PointPer64 size, const TextAlign& align) const {
        TextRenderer text { *this };
        text.SetAlign(align);
        text.SetFontSize(size);
        text.RenderRichText(string);

        const usize iSize = text.textVertices.size() / 2, back = text.bgIndices.size();
        usize off = text.bgVertices.size();
        text.bgIndices.reserve(back + iSize);
        for (usize i = back; i < back + iSize; i += 2) {
            text.bgIndices.push_back(TriIndices { 0, 1, 2 } + off);
            text.bgIndices.push_back(TriIndices { 1, 2, 3 } + off);
            off += 4;
        }
        text.bgVertices.reserve(text.bgVertices.size() + text.textVertices.size());
        text.bgVertices.insert(text.bgVertices.end(), text.textVertices.begin(), text.textVertices.end());
        
        return Mesh { std::move(text.bgVertices), std::move(text.bgIndices) }; // return the text mesh
    }

    Font Font::LoadFile(const std::string& filename) {
        FT_Face face = nullptr;
        const int error = FT_New_Face(FontDevice::Library(), filename.c_str(), 0, &face);
        if (!error) {
            Font font;
            font.SetDefaultFontStyle(face);
            return font;
        }
        
        if (error == FT_Err_Unknown_File_Format)
            GLLogger().Error({"Font {} doesn't have valid format"}, filename);
        else
            GLLogger().Error({"Font loaded with err code {}"}, error);
        return {};
    }

    Font Font::LoadBytes(const uchar* data, uint len) {
        FT_Face face;
        const int error = FT_New_Memory_Face(FontDevice::Library(), data, (long)len, 0, &face);
        if (!error) {
            Font font;
            font.SetDefaultFontStyle(face);
            return font;
        }
        
        GLLogger().Error({"Font loaded with err code {}"}, error);
        return {};
    }

    void Font::ReserveFont() {
        faceHandles.resize(faceHandles.size() + 4);
    }

    void Font::AddFontStyle(int id, const std::string& filePath, FontStyle style) {
        FT_Face face;
        const int error = FT_New_Face(FontDevice::Library(), filePath.c_str(), 0, &face);
        if (!error) {
            if (faceHandles.size() <= (uint)id * 4 + (uint)style) faceHandles.resize(id * 4 + (int)style + 1);
            SetFontStyle(id, face, style);
        } else {
            GLLogger().Error({"Font loaded with err code {}"}, error);
        }
    }

    void Font::SetFontStyle(int id, FT_Face face, FontStyle style) {
        faceHandles[id << 2 | (int)style].reset(face);
    }
}
