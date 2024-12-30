#pragma once
#include <string>
#include <freetype/freetype.h>

#include "Mesh.h"
#include "TextAlign.h"
#include "TextStyle.h"
#include "Textures/Texture.h"

#define Q_USER_FONTS R"(C:\Users\User\AppData\Local\Microsoft\Windows\Fonts\)"
#define Q_WIN_FONTS R"(C:\Windows\Fonts\)"

struct FT_FaceRec_;

namespace Quasi::Text { struct RichString; }

namespace Quasi::Graphics {
    struct FontDeleter { void operator()(FT_FaceRec_* ptr) const; };
    using FaceHandle = Box<FT_FaceRec_, FontDeleter>;

    struct Glyph {
        // internal coords
        Math::fRect2D rect;
        // render data
        Math::fVector2 advance;
        Math::iVector2 offset;
    };

    class Font {
        static constexpr int DEFAULT_FONT_ID = 0, MONOSPACE_FONT_ID = 1;
        Vec<FaceHandle> faceHandles;
        PointPer64 fontSize = 0;
    public:
        struct FontMetrics {
            PointPer64 fontHeight, ascend, descend;
        };

        struct Vertex {
            Math::fVector2 Position;
            Math::fVector2 TextureCoord;
            Math::fColor Color;
            int RenderType;
            static constexpr int RENDER_TEXT = 1, RENDER_FILL = 0;

            QuasiDefineVertex$(Vertex, 2D, (Position, PosTf)(TextureCoord)(Color)(RenderType));
        };
    private:
        static constexpr u32 NUM_GLYPHS = 127 - 32;
        Vec<Glyph> glyphs;
        Vec<FontMetrics> metrics;
        Math::uVector2 textureSize;
        Texture atlas;
    public:
        Font() { faceHandles.Push({ nullptr }); }
        Font(FT_FaceRec_* fHand) { faceHandles.Push(FaceHandle::Own(fHand)); }
        ~Font() = default;

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        static void Transfer(Font& dest, Font&& from);
        Font(Font&& f) noexcept { Transfer(*this, std::move(f)); }
        Font& operator=(Font&& f) noexcept { Transfer(*this, std::move(f)); return *this; }

        void SetSize(PointPer64 charWidth, PointPer64 charHeight = 0 /* use width instead */, u32 dpi = FontDevice::DPI());
        void RenderBitmap();

        const Glyph& GetGlyphRect(char c, FontStyle style = FontStyle::NONE, u32 id = 0) const;
        Mesh<Vertex> RenderText(
            Str string, PointPer64 size,
            const TextAlign& align = { { 0, INFINITY } }
        ) const;

        Mesh<Vertex> RenderRichText(
            const Text::RichString& string, PointPer64 size,
            const TextAlign& align = { { 0, INFINITY } }
        ) const;
        
        Texture& GetTexture() { return atlas; }
        const Texture& GetTexture() const { return atlas; }
        
        static Font LoadFile (Str filename);
        static Font LoadBytes(const byte* data, u32 len);

        FT_FaceRec_* DefaultFontUnchecked(FontStyle style = FontStyle::NONE) { return &*faceHandles[(u32)style]; }
        FT_FaceRec_* GetFontUnchecked(u32 id = 0, FontStyle style = FontStyle::NONE) { return &*faceHandles[(id << 2) | (u32)style]; }
        FT_FaceRec_* DefaultFont(FontStyle style = FontStyle::NONE) { return DefaultFontUnchecked(style) ? DefaultFontUnchecked(style) : DefaultFontUnchecked(); }
        FT_FaceRec_* GetFont(u32 id = 0, FontStyle style = FontStyle::NONE) { return GetFontUnchecked(id, style) ? GetFontUnchecked(id, style) : DefaultFont(style); }

        void ReserveFont();
        void AddDefaultFontStyle(Str filePath, FontStyle style = FontStyle::NONE) { AddFontStyle(DEFAULT_FONT_ID, filePath, style); }
        void AddMonoFontStyle(Str filePath, FontStyle style = FontStyle::NONE) { AddFontStyle(MONOSPACE_FONT_ID, filePath, style); }
        void AddFontStyle(u32 id, Str filePath, FontStyle style = FontStyle::NONE);
        void SetDefaultFontStyle(FT_FaceRec_* face, FontStyle style = FontStyle::NONE) { SetFontStyle(DEFAULT_FONT_ID, face, style); }
        void SetMonoFontStyle(FT_FaceRec_* face, FontStyle style = FontStyle::NONE) { SetFontStyle(MONOSPACE_FONT_ID, face, style); }
        void SetFontStyle(u32 id, FT_FaceRec_* face, FontStyle style = FontStyle::NONE);

        const FontMetrics& GetMetric(u32 id, FontStyle style = FontStyle::NONE) const { return metrics[id * 4 + (u32)style]; }
        const FontMetrics& GetDefaultMetric(FontStyle style = FontStyle::NONE) const { return GetMetric(DEFAULT_FONT_ID, style); }
        const FontMetrics& GetMonoMetric(FontStyle style = FontStyle::NONE) const { return GetMetric(MONOSPACE_FONT_ID, style); }

        friend struct TextRenderer;
    };
}
