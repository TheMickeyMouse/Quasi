#include "TestFontRender.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Keyboard.h"
#include "Graphics/Utils/Fonts/Font.h"

namespace Test {
    void TestFontRender::OnInit(Graphics::GraphicsDevice& gdevice) {
        render = gdevice.CreateNewRender<Vertex>(100, 50);

        render.UseShader(
            "#shader vertex\n"
            "#version 330 core\n"
            "layout(location = 0) in vec4 position;\n"
            "layout(location = 1) in vec4 color;\n"
            "layout(location = 2) in vec2 texCoord;\n"
            "out vec4 v_color;\n"
            "out vec2 v_texCoord;\n"
            "uniform mat4 u_projection;\n"
            "uniform mat4 u_view;\n"
            "void main(){\n"
            "    gl_Position = u_projection * u_view * position;\n"
            "    v_color = color;\n"
            "    v_texCoord = texCoord;\n"
            "}\n"
            "#shader fragment\n"
            "#version 330 core\n"
            "layout(location = 0) out vec4 color;\n"
            "in vec4 v_color;\n"
            "in vec2 v_texCoord;\n"
            "uniform sampler2D u_font;\n"
            "void main(){\n"
            "    color = v_color;\n"
            "    color.a = texture(u_font, v_texCoord).r;\n"
            "}"
        );
        render.SetProjection(projection);
        
        font = Graphics::Font::LoadFile(R"(C:\Windows\Fonts\arial.ttf)");

        font.SetSize(48);
        font.RenderBitmap();
        font.GetTexture().Bind(0);

        Vertex vertices[] = { 
            { { -100.0f, -100.0f, 0 }, 1, { 0.0f, 0.0f } },
            { { +100.0f, -100.0f, 0 }, 1, { 1.0f, 0.0f } },
            { { +100.0f, +100.0f, 0 }, 1, { 1.0f, 1.0f } },
            { { -100.0f, +100.0f, 0 }, 1, { 0.0f, 1.0f } },
        };
        
        Graphics::TriIndices indices[] = {
            { 0, 1, 2 },
            { 2, 3, 0 }
        };
        
        meshAtlas = Graphics::Mesh(
            std::vector(vertices, vertices + 4),
            std::vector(indices, indices + 2)
        );
    }

    void TestFontRender::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);
        Maths::mat3D mat = Maths::mat3D::transform(modelTranslation,
                                                   modelScale,
                                                   modelRotation);
        render.SetCamera(mat);
        render.GetShader().Bind();
        render.GetShader().SetUniform1I("u_font", 0);

        meshStr = font.RenderString(string, 48)
            .Convert<Vertex>([](const VertexColorTexture3D& v) {
                return Vertex { v.Position, v.Color, v.TextureCoordinate };
            });
        meshStr.Bind(render);
        
        //LOG(mat);
        meshStr.ApplyMaterial(&Vertex::Color, color);
        
        render.ResetData();
        if (showAtlas) render.AddNewMeshes(&meshAtlas, 1);
        render.Render();
    }

    void TestFontRender::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);
        
        ImGui::DragFloat3("Translation", modelTranslation.begin());
        ImGui::DragFloat3("Scale",       modelScale.begin(), 0.1f);
        ImGui::DragFloat3("Rotation",    modelRotation.begin(), 0.03f);
        ImGui::ColorEdit4("Color",       color.begin());

        ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace, IO::Keyboard.KeyPressed(IO::Key::BACKSPACE) && string.size() > 1);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, IO::Keyboard.KeyPressed(IO::Key::ENTER));

        ImGui::InputTextMultiline("String", &string);

        if (ImGui::Button(showAtlas ? "Hide Atlas" : "Show Atlas")) {
            showAtlas ^= true;
        }
    }

    void TestFontRender::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        render.Destroy();
    }
}
