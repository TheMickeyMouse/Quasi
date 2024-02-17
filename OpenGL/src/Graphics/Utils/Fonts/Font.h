#pragma once
#include <string>

#include "Mesh.h"
#include "NumTypes.h"
#include "TextAlign.h"
#include "TextStyle.h"
#include "Texture.h"
#include "Vector.h"

#define GL_USER_FONTS R"(C:\Users\User\AppData\Local\Microsoft\Windows\Fonts\)"
#define GL_WIN_FONTS R"(C:\Windows\Fonts\)"

namespace stdu {
    struct rich_string;
}

namespace Graphics {
    struct FontDeleter {
        FontDeleter() = default;
        OPENGL_API void operator()(FT_FaceRec_* ptr) const;
    };

    using FaceHandle = std::unique_ptr<FT_FaceRec, FontDeleter>;

    struct Glyph {
        // internal coords
        Maths::rect2f rect;
        // render data
        Maths::fvec2 advance;
        Maths::ivec2 offset;
    };

    class Font {
        static constexpr int DEFAULT_FONT_ID = 0, MONOSPACE_FONT_ID = 1;
        std::vector<FaceHandle> faceHandles;
        PointPer64 fontSize = 0;
    public:
        struct FontMetrics {
            PointPer64 fontHeight, ascend, descend;
        };

        struct Vertex {
            Maths::fvec3 Position;
            Maths::fvec2 TextureCoord;
            Maths::colorf Color;
            int RenderType;
            static constexpr int RENDER_TEXT = 1, RENDER_FILL = 0;

            GL_VERTEX_T(Vertex);
            GL_VERTEX_FIELD((Position)(TextureCoord)(Color)(RenderType));
            GL_VERTEX_TRANSFORM_FIELDS((Position))
        };
    private:
        static constexpr uint NUM_GLYPHS = 127 - 32;
        std::vector<Glyph> glyphs;
        std::vector<FontMetrics> metrics;
        Maths::uvec2 textureSize;
        Texture atlas;
    public:
        Font() { faceHandles.emplace_back(nullptr); }
        Font(FT_Face fHand) { faceHandles.push_back(FaceHandle(fHand)); }
        ~Font() = default;

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        OPENGL_API static void Transfer(Font& dest, Font&& from);
        Font(Font&& f) noexcept { Transfer(*this, std::move(f)); }
        Font& operator=(Font&& f) noexcept { Transfer(*this, std::move(f)); return *this; }

        OPENGL_API void SetSize(PointPer64 charWidth, PointPer64 charHeight = 0 /* use width instead */, uint dpi = FontDevice::DPI());
        OPENGL_API void RenderBitmap();

        OPENGL_API [[nodiscard]] const Glyph& GetGlyphRect(char c, FontStyle style = FontStyle::NONE, int id = 0) const;
        OPENGL_API Mesh<Vertex> RenderText(
            const std::string& string, PointPer64 size,
            const TextAlign& align = { { 0, INFINITY } }
        ) const;

        OPENGL_API Mesh<Vertex> RenderRichText(
            const stdu::rich_string& string, PointPer64 size,
            const TextAlign& align = { { 0, INFINITY } }
        ) const;
        
        Texture& GetTexture() { return atlas; }
        [[nodiscard]] const Texture& GetTexture() const { return atlas; }
        
        OPENGL_API static Font LoadFile (const std::string& filename);
        OPENGL_API static Font LoadBytes(const uchar* data, uint len);

        FT_Face DefaultFontUnchecked(FontStyle style = FontStyle::NONE) const { return faceHandles[(int)style].get(); }
        FT_Face GetFontUnchecked(int id = 0, FontStyle style = FontStyle::NONE) const { return faceHandles[(id << 2) | (int)style].get(); }
        FT_Face DefaultFont(FontStyle style = FontStyle::NONE) const { return DefaultFontUnchecked(style) ? DefaultFontUnchecked(style) : DefaultFontUnchecked(); }
        FT_Face GetFont(int id = 0, FontStyle style = FontStyle::NONE) const { return GetFontUnchecked(id, style) ? GetFontUnchecked(id, style) : DefaultFont(style); }

        OPENGL_API void ReserveFont();
        void AddDefaultFontStyle(const std::string& filePath, FontStyle style = FontStyle::NONE) { AddFontStyle(DEFAULT_FONT_ID, filePath, style); }
        void AddMonoFontStyle(const std::string& filePath, FontStyle style = FontStyle::NONE) { AddFontStyle(MONOSPACE_FONT_ID, filePath, style); }
        OPENGL_API void AddFontStyle(int id, const std::string& filePath, FontStyle style = FontStyle::NONE);
        void SetDefaultFontStyle(FT_Face face, FontStyle style = FontStyle::NONE) { SetFontStyle(DEFAULT_FONT_ID, face, style); }
        void SetMonoFontStyle(FT_Face face, FontStyle style = FontStyle::NONE) { SetFontStyle(MONOSPACE_FONT_ID, face, style); }
        OPENGL_API void SetFontStyle(int id, FT_Face face, FontStyle style = FontStyle::NONE);

        [[nodiscard]] const FontMetrics& GetMetric(int id, FontStyle style = FontStyle::NONE) const { return metrics[id * 4 + (int)style]; }
        [[nodiscard]] const FontMetrics& GetDefaultMetric(FontStyle style = FontStyle::NONE) const { return GetMetric(DEFAULT_FONT_ID, style); }
        [[nodiscard]] const FontMetrics& GetMonoMetric(FontStyle style = FontStyle::NONE) const { return GetMetric(MONOSPACE_FONT_ID, style); }

        friend struct TextRenderer;
    };
}
