#pragma once
#include <string>

#include "FontDevice.h"
#include "Mesh.h"
#include "NumTypes.h"
#include "Texture.h"
#include "Vector.h"

namespace Graphics {
    enum class StringFormatOptions : uchar {
        ALIGN_LEFT    = 1 << 0,
        ALIGN_RIGHT   = 1 << 1,
        ALIGN_JUSTIFY = ALIGN_LEFT | ALIGN_RIGHT,
        ALIGN_CENTER  = 0,

        VERTICAL_TOP     = 1 << 2,
        VERTICAL_BOTTOM  = 1 << 3,
        VERTICAL_JUSTIFY = VERTICAL_TOP | VERTICAL_BOTTOM,
        VERTICAL_CENTER  = 0,

        BOLD          = 1 << 4,
        ITALIC        = 1 << 5,
        UNDERLINE     = 1 << 6,
        STRIKETHROUGH = 1 << 7,
        
        DEFAULT = 0
    };
    STDU_IMPL_ENUM_OPERATORS(StringFormatOptions);
    using StrFmt = StringFormatOptions;
    
    class Font {
        FT_Face faceHandle = nullptr;
        PointPer64 fontSize = 0;
        PointPer64 fontHeight = 0;
    public: 
        struct Glyph {
            // internal coords
            Maths::rect2f rect;
            // render data
            Maths::fvec2 advance;
            Maths::ivec2 offset;
        };
    private:
        std::vector<Glyph> glyphs;
        Maths::uvec2 textureSize;
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
        OPENGL_API Mesh<VertexColorTexture3D> RenderString(
            const std::string& str, PointPer64 size, StrFmt fmt = StrFmt::DEFAULT /* TODO: add this */
        );

        Texture& GetTexture() { return atlas; }
        [[nodiscard]] const Texture& GetTexture() const { return atlas; }
        
        OPENGL_API static Font LoadFile (const std::string& filename);
        OPENGL_API static Font LoadBytes(const uchar* data, uint len);
    };
}
