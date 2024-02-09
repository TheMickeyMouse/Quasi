#include "TestPostProcessing.h"

#include "imgui.h"
#include "Mesh.h"
#include "Quad.h"

namespace Test {
    void TestPostProcessing::OnInit(Graphics::GraphicsDevice& gdevice) {
        Test::OnInit(gdevice);

        scene = gdevice.CreateNewRender<VertexColor3D>(72, 108);
        postProcessingQuad = gdevice.CreateNewRender<VertexTexture2D>(4, 2);

        cubes.reserve(9);

        constexpr float s = 0.5f;
        cubes.push_back(Graphics::MeshUtils::CubeMesh(0, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_GRAY()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1,  1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_WHITE()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1,  1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_YELLOW()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1, -1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_PURPLE()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1, -1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_RED()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1,  1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_CYAN()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1,  1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_LIME()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1, -1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_BLUE()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1, -1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_BLACK()));

        scene.BindMeshes(cubes);
        scene.UseShader(Graphics::Shader::StdColored);
        scene.SetProjection(projection);

        const auto [winX, winY] = gdevice.GetWindowSize();
        fbo = Graphics::FrameBuffer { 0 };
        fbo.Bind();

        renderResult = Graphics::Texture {
            nullptr, (uint)winX, (uint)winY,
            true, Graphics::TextureFormat::RGB, (Graphics::TextureInternalFormat)Graphics::TextureFormat::RGB
        };

        fbo.Attach(renderResult);

        depthStencilAttachment = Graphics::RenderBuffer {
            Graphics::TextureInternalFormat::DEPTH_24_STENCIL_8, gdevice.GetWindowSize()
        };

        fbo.Bind();
        fbo.Attach(depthStencilAttachment, Graphics::AttachmentType::DEPTH_STENCIL);
        fbo.Complete();
        fbo.Unbind();

        screenQuad = Graphics::Primitives::Quad { 0, Maths::fvec3::RIGHT(), Maths::fvec3::UP() }
                    .IntoMesh<VertexTexture2D>(
                        [](Maths::fvec3 v) -> Maths::fvec2 { return v.xy(); }, &VertexTexture2D::Position)
                    .Convert<VertexTexture2D>([](const VertexTexture2D& v) {
                        return VertexTexture2D { v.Position, (v.Position + 1) * 0.5f };
                     });
        postProcessingQuad.BindMeshes(&screenQuad, 1);
        postProcessingQuad.UseShader(
            R"(#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
void main() {
    FragColor = vec4(1.0 - texture(screenTexture, TexCoords).rgb, 1.0);
})");
        gdevice.BindTexture(renderResult);
    }

    void TestPostProcessing::OnUpdate(float deltaTime) {
        Test::OnUpdate(deltaTime);
    }

    void TestPostProcessing::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);
        Maths::mat3D mat = Maths::mat3D::transform(modelTranslation, modelScale, modelRotation);

        scene.SetCamera(mat);

        Graphics::Render::EnableDepth();
        if (usePostProcessing) {
            fbo.Bind();
            Graphics::Render::Clear();
        }

        scene.ResetData();
        Graphics::Shader& shader = scene.GetShader();
        shader.Bind();
        shader.SetUniformMatrix4x4("u_projection", projection);
        shader.SetUniformMatrix4x4("u_view", mat);
        Graphics::Render::Draw(scene.GetRenderData());

        if (usePostProcessing) {
            Graphics::Render::DisableDepth();
            fbo.Unbind();

            // Graphics::Render::SetClearColor(1);
            Graphics::Render::ClearColorBit();

            postProcessingQuad.GetShader().Bind();
            postProcessingQuad.GetShader().SetUniformTex("screenTexture", renderResult);

            postProcessingQuad.ResetData();
            Graphics::Render::Draw(postProcessingQuad.GetRenderData());
        }
    }

    void TestPostProcessing::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);
        ImGui::DragFloat3("Translation" , modelTranslation.begin(), 0.01f);
        ImGui::DragFloat3("Scale"       , modelScale.begin(),       0.01f);
        ImGui::DragFloat3("Rotation"    , modelRotation.begin(),    0.01f);

        ImGui::Checkbox("Use Post Processing", &usePostProcessing);
    }

    void TestPostProcessing::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        scene.Destroy();
        Graphics::Render::EnableDepth();
    }
}
