#pragma once
#include <string>

#include "FontDevice.h"
#include "Mesh.h"
#include "NumTypes.h"
#include "Texture.h"
#include "Vector.h"

namespace Graphics {
    class Font {
        FT_Face faceHandle = nullptr;
        PointPer64 fontSize = 0;
        PointPer64 fontHeight = 0;
        
        struct Glyph {
            // internal coords
            Maths::rect2f rect;
            // render data
            Maths::fvec2 advance;
            Maths::ivec2 offset;
        };

        std::vector<Glyph> glyphs;
        Texture atlas;
    public:
        Font() {}
        Font(FT_Face fHand) : faceHandle(fHand) {}
        OPENGL_API ~Font();

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        OPENGL_API static void Transfer(Font& dest, Font&& from);
        Font(Font&& f) noexcept { Transfer(*this, std::move(f)); }
        Font& operator=(Font&& f) noexcept { Transfer(*this, std::move(f)); return *this; }

        OPENGL_API void SetSize(PointPer64 charWidth, PointPer64 charHeight = 0 /* use width instead */, uint dpi = FontDevice::DPI());
        OPENGL_API void RenderBitmap();

        OPENGL_API Glyph& GetGlyphRect(char c);
        OPENGL_API Mesh<VertexColorTexture3D> RenderString(const std::string& str, PointPer64 size);

        Texture& GetTexture() { return atlas; }
        [[nodiscard]] const Texture& GetTexture() const { return atlas; }
        
        OPENGL_API static Font LoadFile (const std::string& filename);
        OPENGL_API static Font LoadBytes(const uchar* data, uint len);
    };
}
