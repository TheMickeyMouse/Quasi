#pragma once
#include <vector>
#include "stdu/rich_string.h"

#include "Font.h"
#include "TextAlign.h"
#include "Vector.h"

namespace Graphics {
    struct TextRenderer {
        struct Word {
            uint index;
            float width;
        };
        
        const Font& font;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
        float scaleRatio = 1;
        float lineSpacing = 0, spaceAdvance = 0;
        Maths::fvec2 pen;
        uint meshIndex = 0;
        uint lineCount = 0;
        float totalHeight = 1;

        uint lastLineIndex = 0, lastSpaceIndex = 0;
        std::vector<Word> lineWords;
        std::vector<uint> lineIndices;
        float lineWidth = 0;

        PointPer64 fontSize = 0;

        TextAlign align = { { 0, INFINITY } };
        using Vertex = Font::Vertex;
        std::vector<Vertex> textVertices, bgVertices;
        std::vector<TriIndices> bgIndices;
        
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
                            CharQuad& CharAt(uint index);
        [[nodiscard]] const CharQuad& CharAt(uint index) const;


        void FixAlignX();
        void JustifyAlignX();
        void AlignX();
        bool WordWrap(float advance, std::string::const_iterator& it,
                                     std::string::const_iterator  begin);
        
        void FixAlignY();
        void ClipY(Maths::rect2f& pos, Maths::rect2f& tex,
                   bool clipTop, bool clipBottom) const;
        
        void PushCharQuad(const Maths::rect2f& pos, const Maths::rect2f& tex);
        void AddChar(std::string::const_iterator& it, std::string::const_iterator begin);
        void AddRichChar(const stdu::rich_string::const_iter& it);
        
        void TriggerNewLine();
        void TriggerSpace();
        
        std::vector<Font::Vertex> RenderText(const std::string& string);
        std::vector<Font::Vertex> RenderRichText(const stdu::rich_string& string);

        void AddRoundedRect(const Maths::rect2f& region, float roundRadius, const Maths::colorf& color);

        friend class Font;
    };
}
