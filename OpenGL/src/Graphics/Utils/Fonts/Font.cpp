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
        constexpr int padding = 0;
        ivec2 pen      = { padding, 0 };

        constexpr int textureWidth = 512;
        uchar textureBuffer[textureWidth * textureWidth] = { 0 };
        glyphs.resize(127-32);
        for (uchar charCode = 32; charCode < 127; ++charCode) {
            const int error = FT_Load_Char(faceHandle, charCode, FT_LOAD_RENDER);
            if (error) {
                LOG("[ERR]: loading char with error code " << error);
                continue;  /* ignore errors */
            }

            if (pen.x + glyphHandle->bitmap.width + padding >= textureWidth) {
                pen.x = padding;
                pen.y += fontSize.px();
                if (pen.y >= textureWidth)
                    break;
            }

            // Font Height
            fontHeight = std::max(
                (faceHandle->size->metrics.ascender - faceHandle->size->metrics.descender) >> 6,
                (long)fontHeight.points());

            uchar* tbuff = textureBuffer + (textureWidth - 1 - pen.y) * textureWidth + pen.x;
            const uint width = glyphHandle->bitmap.width, height = glyphHandle->bitmap.rows;
            for (int y = 0; y < (int)height; ++y) {
                for (uint x = 0; x < width; ++x) {
                    tbuff[-y * textureWidth + x] =
                        glyphHandle->bitmap.buffer[y * width + x];
                }
            }

            Glyph& glyph = glyphs[charCode - 32];
            glyph.rect = fvec2 { pen.x, textureWidth - pen.y }.to(fvec2 { width, -(int)height }.as_size()) / textureWidth;
            glyph.advance = { (float)glyphHandle->advance.x / 64.0f, (float)glyphHandle->advance.y / 64.0f };
            glyph.offset  = { glyphHandle->bitmap_left, glyphHandle->bitmap_top };

            pen.x += (int)glyphHandle->bitmap.width + padding;
        }

        // Upload OpenGL Texture
        atlas = Texture(textureBuffer, textureWidth, textureWidth, false, GL_RED);
    }

    Font::Glyph& Font::GetGlyphRect(char c) {
        return glyphs[c - 32];
    }

    Mesh<VertexColorTexture3D> Font::RenderString(const std::string& str, PointPer64 size) {
        using namespace Maths;
        ivec2 pen = { 0, 0 };
        Mesh<VertexColorTexture3D> mesh = {};
        uint ti = 0;

        auto& vert = mesh.GetVertices();
        auto& ind  = mesh.GetIndices();
        const float scaleRatio = size / fontSize;

        // TODO: make this auto-detect texture width with packing alg
        constexpr float textureWidth = 512.0f;
        for (char glyph : str) {
            if (glyph == '\n') {
                pen.y -= (int)((float)fontHeight.px() * scaleRatio);
                pen.x = 0;
                continue;
            }
            if (glyph < 32 || glyph >= 127) continue;

            const Glyph& rect = GetGlyphRect(glyph);

            fvec2 size = rect.rect.size() * textureWidth;
            rect2f pos = rect.offset.as<float>().to(fvec2 { size.x, -size.y }.as_size()) * scaleRatio + pen;
            
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
