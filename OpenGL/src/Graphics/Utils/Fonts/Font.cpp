#include "Font.h"

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
        int pen = 0;

        textureSize = 0;
        for (uchar charCode = 32; charCode < 127; ++charCode) {
            const uint glyphIdx = FT_Get_Char_Index(faceHandle, charCode);
            if (const int error = FT_Load_Glyph(faceHandle, glyphIdx, FT_LOAD_DEFAULT)) {
                LOG("[ERR]: loading char with error code " << error);
                continue;  /* ignore errors */
            }

            textureSize.y = std::max(textureSize.y, glyphHandle->bitmap.rows);
            textureSize.x += glyphHandle->bitmap.width;
        }
        
        std::unique_ptr<uchar[]> textureBuffer(new uchar[textureSize.x * textureSize.y] { 0 });
        
        glyphs.resize(127-32);
        for (uchar charCode = 32; charCode < 127; ++charCode) {
            const int error = FT_Load_Char(faceHandle, charCode, FT_LOAD_RENDER);
            if (error) {
                LOG("[ERR]: loading char with error code " << error);
                continue;  /* ignore errors */
            }

            // Font Height
            fontHeight = std::max(
                (faceHandle->size->metrics.ascender - faceHandle->size->metrics.descender) >> 6,
                (long)fontHeight.points());

            uchar* tbuff = textureBuffer.get() + (textureSize.y - 1) * textureSize.x + pen;
            const uint width = glyphHandle->bitmap.width, height = glyphHandle->bitmap.rows;
            for (int y = 0; y < (int)height; ++y) {
                for (uint x = 0; x < width; ++x) {
                    tbuff[-y * textureSize.x + x] =
                        glyphHandle->bitmap.buffer[y * width + x];
                }
            }

            Glyph& glyph = glyphs[charCode - 32];
            glyph.rect = fvec2 { pen, textureSize.y }.to(fvec2 { width, -(int)height }.as_size()) / textureSize;
            glyph.advance = { (float)glyphHandle->advance.x / 64.0f, (float)glyphHandle->advance.y / 64.0f };
            glyph.offset  = { glyphHandle->bitmap_left, glyphHandle->bitmap_top };

            pen += (int)width;
        }

        // Upload OpenGL Texture
        atlas = Texture(textureBuffer.get(), textureSize.x, textureSize.y, false, GL_RED, 1);
    }

    Font::Glyph& Font::GetGlyphRect(char c) {
        return glyphs[c - 32];
    }

    Mesh<VertexColorTexture3D> Font::RenderString(const std::string& str, PointPer64 size, StrFmt fmt) {
        using namespace Maths;
        ivec2 pen = { 0, 0 };
        Mesh<VertexColorTexture3D> mesh = {};
        uint ti = 0;

        auto& vert = mesh.GetVertices();
        auto& ind  = mesh.GetIndices();
        const float scaleRatio = size / fontSize;

        // TODO: make this auto-detect texture width with packing alg
        for (char glyph : str) {
            if (glyph == '\n') {
                pen.y -= (int)((float)fontHeight.px() * scaleRatio);
                pen.x = 0;
                continue;
            }
            if (glyph < 32 || glyph >= 127) continue;

            const Glyph& rect = GetGlyphRect(glyph);

            const fvec2 rsize = rect.rect.size() * textureSize;
            rect2f pos = rect.offset.as<float>().to(fvec2 { rsize.x, -rsize.y }.as_size()) * scaleRatio + pen;
            
            vert.push_back({ pos.corner(0), 1, rect.rect.corner(0), 0 });
            vert.push_back({ pos.corner(1), 1, rect.rect.corner(1), 0 });
            vert.push_back({ pos.corner(2), 1, rect.rect.corner(2), 0 });
            vert.push_back({ pos.corner(3), 1, rect.rect.corner(3), 0 });

            ind.push_back(TriIndices { 0, 1, 2 } + ti * 4);
            ind.push_back(TriIndices { 1, 2, 3 } + ti * 4);
            
            ++ti; // ti is not always index, could encounter \n
            pen += rect.advance * scaleRatio;
        }

        return mesh;
    }

    Font Font::LoadFile(const std::string& filename) {
        Font font;
        int error = FT_New_Face(FontDevice::Library(), filename.c_str(), 0, &font.faceHandle);
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
