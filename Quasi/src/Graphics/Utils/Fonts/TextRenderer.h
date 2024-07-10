#pragma once
#include <vector>
#include "Utils/RichString.h"

#include "Font.h"
#include "TextAlign.h"
#include "Vector.h"

namespace Quasi::Graphics {
    struct TextRenderer {
        struct Word {
            u32 index;
            float width;
        };
        
        const Font& font;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
        float scaleRatio = 1;
        float lineSpacing = 0, spaceAdvance = 0;
        Math::fVector2 pen;
        u32 meshIndex = 0;
        u32 lineCount = 0;
        float totalHeight = 1;

        u32 lastLineIndex = 0, lastSpaceIndex = 0;
        Vec<Word> lineWords;
        Vec<u32> lineIndices;
        float lineWidth = 0;

        PointPer64 fontSize = 0;

        TextAlign align = { { 0, INFINITY } };
        using Vertex = Font::Vertex;
        Vec<Vertex> textVertices, bgVertices;
        Vec<TriIndices> bgIndices;
        
        struct CharQuad {
            Vertex v0, v1, v2, v3;

            void MoveX(float x) { MoveMinX(x); MoveMaxX(x); }
            void MoveY(float y);
            void MoveMinX(float x);
            void MoveMaxX(float x);
            void MoveTexMinX(float x);
            void MoveTexMaxX(float x);
            [[nodiscard]] float Width() const;
            [[nodiscard]] float TextureWidth() const;
        };

        TextRenderer(const Font& fnt) : font(fnt) {}

        void SetAlign(const TextAlign& txtAlign) { align = txtAlign; }
        void SetFontSize(PointPer64 fSize) { fontSize = fSize; }
        void Prepare();

                            CharQuad* Begin();
        [[nodiscard]] const CharQuad* Begin() const;
                            CharQuad* End();
        [[nodiscard]] const CharQuad* End() const;
                            CharQuad& CharAt(u32 index);
        [[nodiscard]] const CharQuad& CharAt(u32 index) const;


        void FixAlignX();
        void JustifyAlignX();
        void AlignX();
        bool WordWrap(float advance, IterOf<Str>& it, IterOf<Str> begin);
        
        void FixAlignY();
        void ClipY(Math::fRect2D& pos, Math::fRect2D& tex,
                   bool clipTop, bool clipBottom) const;
        
        void PushCharQuad(const Math::fRect2D& pos, const Math::fRect2D& tex);
        void AddChar(IterOf<Str>& it, IterOf<Str> begin);
        void AddRichChar(const Text::RichString::Iter& it);
        
        void TriggerNewLine();
        void TriggerSpace();
        
        void RenderText(Str string);
        void RenderRichText(const Text::RichString& string);

        void AddRoundedRect(const Math::fRect2D& region, float roundRadius, const Math::fColor& color);

        friend class Font;
    };
}
