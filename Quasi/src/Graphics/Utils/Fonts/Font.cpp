#include "Font.h"
#include "TextRenderer.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "Graphics/GLs/GLDebug.h"

namespace Quasi::Graphics {
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
        for (FaceHandle& faceHandle : faceHandles) {
            if (!faceHandle) continue;
            const int error = FT_Set_Char_Size(faceHandle.DataMut(), charWidth, charHeight == 0 ? charWidth : charHeight, dpi, dpi);
            GLLogger().Assert(!error, "Font Char size set with err code {}", error);
        }
    }

    void Font::RenderBitmap() {
        using namespace Math;
        int pen = 0; // x value of pen

        constexpr int loadSDF = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF); // used for sdf rendering
        constexpr int sdfExtrude = 16;

        textureSize = 0;
        Vec<u32> heights;
        heights.Reserve(faceHandles.Length());
        for (FaceHandle& faceHandle : faceHandles) {
            const FT_GlyphSlot glyphHandle = faceHandle->glyph;
            u32 y = 0;
            u32 x = 0;
            for (char charCode = 32; charCode < 127; ++charCode) { // loop through each character
                // new method: SDFs extrude 8 pixels in all directions, so you can load normally and then add 16
                // (no need to load twice!)
                if (const int error = FT_Load_Char(faceHandle.DataMut(), charCode, FT_LOAD_DEFAULT)) {
                    GLLogger().Error("Loading char with err code {}", error);
                    continue;  /* ignore errors */
                }

                y = std::max(y, glyphHandle->bitmap.rows); // y is guaranteed to fit the sdf
                x += glyphHandle->bitmap.width; // packs sdfs together on the x axis
            }
            y += sdfExtrude;
            x += (NUM_GLYPHS - 1 /* space */) * sdfExtrude;
            heights.Push(textureSize.y);
            textureSize.x = std::max(textureSize.x, x);
            textureSize.y += y;
        }

        Texture::SetPixelStore(PixelStoreParam::UNPACK_ALIGNMENT, 1);
        atlas = Texture::New(
            nullptr, { textureSize.x, textureSize.y },
            {
                .load = { .format = TextureFormat::RED, .internalformat = TextureIFormat::RGBA_8 },
            }
        ); // create blank texture
        atlas.Bind(); // set this texture to the active one
        glyphs.Resize(NUM_GLYPHS * faceHandles.Length()); // amt of glyphs-
        metrics.Reserve(faceHandles.Length());
        for (uint i = 0; i < faceHandles.Length(); ++i) {
            FaceHandle& faceHandle = faceHandles[i];
            const FT_GlyphSlot glyphHandle = faceHandle->glyph;
            metrics.Push({
                PointPer64::inP64(faceHandle->size->metrics.ascender - faceHandle->size->metrics.descender),
                PointPer64::inP64(faceHandle->size->metrics.ascender),
                PointPer64::inP64(faceHandle->size->metrics.descender)
            });
            for (char charCode = 32; charCode < 127; ++charCode) { // loop through again, this time drawing textures
                if (const int error = FT_Load_Char(faceHandle.DataMut(), charCode, loadSDF)) { // loads again
                    GLLogger().Error("Loading char with err code {}", error);
                    continue;
                }
            
                const uVector2 size = { glyphHandle->bitmap.width, glyphHandle->bitmap.rows }; // construct size in pixels of the texture
                atlas.SetSubTexture(glyphHandle->bitmap.buffer, uVector2 { pen, heights[i] }.to(size.as_size()), { .format = TextureFormat::RED }); // draw the sub texture

                Glyph& glyph = glyphs[charCode - 32 + i * NUM_GLYPHS]; // write rendering memory
                glyph.rect = fVector2 { pen, heights[i] }.to(size.asf().as_size()) / textureSize.asf(); // rect of texture in atlas
                glyph.advance = { (float)glyphHandle->advance.x / 64.0f, (float)glyphHandle->advance.y / 64.0f }; // pen move
                glyph.offset  = { glyphHandle->bitmap_left, glyphHandle->bitmap_top }; // offset from pen

                pen += (int)size.x; // moves pen
            }
            pen = 0;
        }
        Texture::SetPixelStore(PixelStoreParam::UNPACK_ALIGNMENT, 4);
    }

    const Glyph& Font::GetGlyphRect(char c, FontStyle style, u32 id) const {
        return glyphs[c - 32 + NUM_GLYPHS * (id * 4 + (u32)style)];
    }

    Mesh<Font::Vertex> Font::RenderText(Str string, PointPer64 size, const TextAlign& align) const {
        TextRenderer text { *this };
        text.SetAlign(align);
        text.SetFontSize(size);
        text.RenderText(string);

        Vec ind = Vec<TriIndices>::WithCap(text.textVertices.Length() / 2);
        for (u32 i = 0; i < ind.Capacity(); i += 2) {
            ind.Push(TriIndices { 0, 1, 2 } + i * 2);
            ind.Push(TriIndices { 1, 2, 3 } + i * 2);
        }
        
        return Mesh { std::move(text.textVertices), std::move(ind) }; // return the text mesh
    }

    Mesh<Font::Vertex> Font::RenderRichText(const Text::RichString& string, PointPer64 size, const TextAlign& align) const {
        TextRenderer text { *this };
        text.SetAlign(align);
        text.SetFontSize(size);
        text.RenderRichText(string);

        const usize iSize = text.textVertices.Length() / 2, back = text.bgIndices.Length();
        usize off = text.bgVertices.Length();
        text.bgIndices.Reserve(iSize);
        for (usize i = back; i < back + iSize; i += 2) {
            text.bgIndices.Push(TriIndices { 0, 1, 2 } + off);
            text.bgIndices.Push(TriIndices { 1, 2, 3 } + off);
            off += 4;
        }
        text.bgVertices.Reserve(text.textVertices.Length());
        text.bgVertices.Extend(text.textVertices);
        
        return Mesh { std::move(text.bgVertices), std::move(text.bgIndices) }; // return the text mesh
    }

    Font Font::LoadFile(CStr filename) {
        FT_Face face = nullptr;
        const int error = FT_New_Face(FontDevice::Library(), filename.Data(), 0, &face);
        if (!error) {
            Font font;
            font.SetDefaultFontStyle(face);
            return font;
        }
        
        if (error == FT_Err_Unknown_File_Format)
            GLLogger().Error("Font {} doesn't have valid format", filename);
        else
            GLLogger().Error("Font loaded with err code {}", error);
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
        
        GLLogger().Error("Font loaded with err code {}", error);
        return {};
    }

    void Font::ReserveFont() {
        faceHandles.ResizeExtraDefault(4);
    }

    void Font::AddFontStyle(u32 id, CStr filePath, FontStyle style) {
        FT_Face face;
        const int error = FT_New_Face(FontDevice::Library(), filePath.Data(), 0, &face);
        if (!error) {
            if (faceHandles.Length() <= (u32)id * 4 + (u32)style) faceHandles.ResizeDefault(id * 4 + (u32)style + 1);
            SetFontStyle(id, face, style);
        } else {
            GLLogger().QError$("Font loaded with err code {}", error);
        }
    }

    void Font::SetFontStyle(u32 id, FT_Face face, FontStyle style) {
        faceHandles[id << 2 | (u32)style].Replace(face);
    }
}
