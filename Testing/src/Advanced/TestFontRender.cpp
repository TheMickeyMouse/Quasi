#include "TestFontRender.h"
#include "Utils/RichString.h"
#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "GLs/Texture.h"

namespace Test {
    void TestFontRender::OnInit(Graphics::GraphicsDevice& gdevice) {
        render = gdevice.CreateNewRender<Vertex>(1024, 1024);

        render.UseShaderFromFile(RES("shader.vert"), RES("shader.frag"));
        render.SetProjection(projection);

        font = Graphics::Font::LoadFile(RES("arial.ttf"));
        font.AddDefaultFontStyle(RES("arialbd.ttf"), Graphics::FontStyle::BOLD);
        font.AddDefaultFontStyle(RES("ariali.ttf"),  Graphics::FontStyle::ITALIC);
        font.AddDefaultFontStyle(RES("arialbi.ttf"), Graphics::FontStyle::BOLD_ITALIC);
        font.AddMonoFontStyle(RES("JetBrainsMono-Regular.ttf"));
        font.AddMonoFontStyle(RES("JetBrainsMono-Bold.ttf"),       Graphics::FontStyle::BOLD);
        font.AddMonoFontStyle(RES("JetBrainsMono-Italic.ttf"),     Graphics::FontStyle::ITALIC);
        font.AddMonoFontStyle(RES("JetBrainsMono-BoldItalic.ttf"), Graphics::FontStyle::BOLD_ITALIC);

        font.SetSize(48);
        font.RenderBitmap();

        const float aspect = font.GetTexture().Size().AspectRatio();
        Vec<Vertex> atlVertices = Vec<Vertex>::New({
            { { -100.0f * aspect, -100.0f }, 1, { 0.0f, 1.0f }, 1 },
            { { +100.0f * aspect, -100.0f }, 1, { 1.0f, 1.0f }, 1 },
            { { +100.0f * aspect, +100.0f }, 1, { 1.0f, 0.0f }, 1 },
            { { -100.0f * aspect, +100.0f }, 1, { 0.0f, 0.0f }, 1 },
        });
        
        Vec<Graphics::Triplet> atlIndices = Vec<Graphics::Triplet>::New({
            { 0, 1, 2 },
            { 2, 3, 0 }
        });
        
        meshAtlas = Graphics::Mesh(std::move(atlVertices), std::move(atlIndices));

        const Math::fColor bgColor = Math::fColor::Better::Black();
        Vec<Vertex> bgVertices = Vec<Vertex>::New({
            { { -200.0f, -200.0f }, bgColor, { 0.0f, 0.0f }, 0 },
            { { +200.0f, -200.0f }, bgColor, { 0.0f, 0.0f }, 0 },
            { { +200.0f, +200.0f }, bgColor, { 0.0f, 0.0f }, 0 },
            { { -200.0f, +200.0f }, bgColor, { 0.0f, 0.0f }, 0 },
        });
        
        Vec<Graphics::Triplet> bgIndices = Vec<Graphics::Triplet>::New({
            { 0, 1, 2 },
            { 2, 3, 0 }
        });

        meshBg = Graphics::Mesh(std::move(bgVertices), std::move(bgIndices));
    }

    void TestFontRender::OnRender(Graphics::GraphicsDevice& gdevice) {
        render.SetCamera(transform.As3D().TransformMatrix());

        auto& vert = meshBg.vertices;
        vert[0].Position = textBox.Corner({ false, false });
        vert[1].Position = textBox.Corner({ true,  false });
        vert[2].Position = textBox.Corner({ true,  true  }); // yes this is correct
        vert[3].Position = textBox.Corner({ false, true  });

        if (!showAtlas) {
            const Graphics::TextAlign alignment =
                Graphics::TextAlign { textBox }
                .Align({ alignX, alignY << 2, wrapMethod << 4, cropX << 6, cropY << 7 })
                .SpaceOut(lineSpace, Graphics::PointPer64::inP64((int)(letterSpace * 64.0f)));

            meshStr = (useMarkdown ?
                font.RenderRichText(Text::RichString::ParseMarkdown(string), fontSize, alignment) :
                font.RenderText(string, fontSize, alignment)
            ).GeometryConvert(QGLCreateBlueprint$(Vertex, (
                in (Position, Color, RenderType, TextureCoord),
                out (Position) = Position;,
                out (Color) = RenderType ? color : Color;,
                out (TextureCoordinate) = TextureCoord;,
                out (isText) = RenderType;
            )));
        }
        render.Draw({ &meshBg, showAtlas ? &meshAtlas : &meshStr }, Graphics::UseArgs({
            { "u_font",           font.GetTexture(), 0 },
            { "u_thickness",      thickness },
            { "u_softness",       softness },
            { "u_shadowColor",    shadowColor },
            { "u_shadowSoftness", shadowSoftness },
            { "u_shadowOffset",   shadowOffset },
        }));
    }

    void TestFontRender::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditTransform("Transform", transform);

        ImGui::EditString("String", string);
        ImGui::Checkbox("Parse as Markdown", &useMarkdown);
        
        ImGui::EditColor ("Color",       color);
        ImGui::EditScalar("Font Size",   fontSize,  1,    fRange { 4, 48 });
        ImGui::EditScalar("Thickness",   thickness, 0.01, fRange { 1 });
        ImGui::EditScalar("Softness",    softness,  0.01, fRange { 1 });

        ImGui::EditVector("Text Bottom Left", textBox.min, 1, Math::fRect2D { -300, 300 });
        ImGui::EditVector("Text Top Right",   textBox.max, 1, Math::fRect2D { -300, 300 });

        ImGui::Combo("Alignment X", &alignX, "Center\0Left\0Right\0Justified\0\0");
        ImGui::Combo("Alignment Y", &alignY, "Center\0Top\0Bottom\0Justified\0\0");
        ImGui::Combo("Wrap Mode", &wrapMethod, "None\0Break Sentence\0Break Words\0\0");
        ImGui::Checkbox("Crop X", &cropX); ImGui::SameLine(); ImGui::Checkbox("Crop Y", &cropY);
        ImGui::EditScalar("Letter Spacing", letterSpace, 0.01, fRange { -10, 10 });
        ImGui::EditScalar("Line   Spacing", lineSpace,   0.01, fRange { -1, 2 });

        if (ImGui::CollapsingHeader("Shadow Properties")) {
            ImGui::EditColor ("Shadow Color",    shadowColor);
            ImGui::EditScalar("Shadow Softness", shadowSoftness, 0.01, fRange { 1 });
            ImGui::EditVector("Shadow Offset",   shadowOffset, 0.1, Math::fRect2D { -10, 10 });
        }

        if (ImGui::Button(showAtlas ? "Hide Atlas" : "Show Atlas")) {
            showAtlas ^= true;
        }

        if (showAtlas) {
            const float x = meshAtlas.vertices[1].Position.x;
            const auto& glyph = font.GetGlyphRect(string.Last());
            meshAtlas.SetTransform(Math::Transform2D::Translate({ x * (1 - 2 * glyph.rect.Center().x), 0 }));
        }
    }

    void TestFontRender::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        render.Destroy();
    }
}
