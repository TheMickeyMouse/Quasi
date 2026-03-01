#include "TestPostProcessing.h"

#include "Mesh.h"
#include "GUI/ImGuiExt.h"
#include "Meshes/MeshBuilder.h"

namespace Test {
    void TestPostProcessing::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Graphics::VertexNormal3D>(216, 324);

        using namespace Math;
        cube = Graphics::Meshes::Cube().Create().IntoMesh(
            [&] (const fv3& p, const fv3& n) { return Graphics::VertexNormal3D { p, n }; }
        );

        for (int i = 0; i < 8; ++i) {
            locations[i] = fv3::FromCorner({ (bool)(i & 1), (bool)(i & 2), (bool)(i & 4) });
            colors[i] = fColor::Better::Colors[i];
        }
        colors[8] = fColor::Better::Gray();

        scene.UseShaderFromFile(RES("colored.glsl"));
        scene.SetProjection(Matrix3D::PerspectiveFov(90.0_deg, gdevice.GetAspectRatio(), 0.01f, 100.0f));

        fbo = Graphics::FrameBuffer::New();
        fbo.Bind();

        renderResult = Graphics::Texture2D::New(
            nullptr, gdevice.GetWindowSize(),
            { .format = Graphics::TextureFormat::RGB, .internalformat = Graphics::TextureIFormat::RGB_8 }
        );

        fbo.Attach(renderResult);

        depthStencilAttachment = Graphics::RenderBuffer::New(
            Graphics::TextureIFormat::DEPTH_24_STENCIL_8, gdevice.GetWindowSize()
        );

        fbo.Bind();
        fbo.Attach(depthStencilAttachment, Graphics::AttachmentType::DEPTH_STENCIL);
        fbo.Complete();
        fbo.Unbind();

        String vert = RES_STR("quad.vert");
        const CStr vertloc = vert.IntoCStr();
        defaultShader = Graphics::Shader::FromFile(vertloc, RES("none.frag"));

        shaderInv        = Graphics::Shader::FromFile(vertloc, RES("invert.frag"));
        shaderHsv        = Graphics::Shader::FromFile(vertloc, RES("hsv.frag"));
        shaderBlur       = Graphics::Shader::FromFile(vertloc, RES("simple_blur.frag"));
        shaderEdgeDetect = Graphics::Shader::FromFile(vertloc, RES("simple_ed.frag"));

        currShader = &defaultShader;
        renderResult.Activate(0);
    }

    void TestPostProcessing::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        transform.rot += turnSpeed * deltaTime;
    }

    void TestPostProcessing::OnRender(Graphics::GraphicsDevice& gdevice) {
        scene.SetCamera(transform.IntoMatrix());

        if (usePostProcessing) {
            fbo.Bind();
            Graphics::Render::Clear();
        }

        scene.DrawInstanced(Spans::Only(cube), 9, {
            .arguments = {
                { "locations", locations },
                { "colors", colors },
                { "scale", 0.3f }
            } });

        if (usePostProcessing) {
            Graphics::Render::DisableDepth();
            fbo.Unbind();

            // Graphics::Render::SetClearColor(1);
            Graphics::Render::ClearColorBit();

            currShader->Bind();
            currShader->SetUniformTex("screenTexture", renderResult, 1);
            if (currShader == &shaderBlur) {
                currShader->SetUniformFv2("blurOff", effectOff);
            } else if (currShader == &shaderEdgeDetect) {
                currShader->SetUniformFv2("detectOff", effectOff);
            } else if (currShader == &shaderHsv) {
                currShader->SetUniformFloat("dh", hueShift);
                currShader->SetUniformFloat("ds", satMul);
                currShader->SetUniformFloat("dv", valShift);
            }

            // Graphics::Render::Draw(postProcessingQuad.GetRenderData(), *currShader);
            Graphics::Render::DrawScreenQuad(*currShader);
            Graphics::Render::EnableDepth();
        }
    }

    void TestPostProcessing::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditTransform("Transform", transform, 0.01);

        ImGui::EditRotation3D("Spin Speed", turnSpeed);

        ImGui::Separator();

        ImGui::Checkbox("Use Post Processing", &usePostProcessing);

#define TAB_ITEM(N, P, C) if (ImGui::BeginTabItem(N)) { currShader = &(P); ImGui::EndTabItem(); C }
        if (ImGui::BeginTabBar("Post Processing Shader")) {
            TAB_ITEM("None", defaultShader, )
            TAB_ITEM("Color Invert", shaderInv, )
            TAB_ITEM("Color Hue", shaderHsv,
                ImGui::EditScalar("Hue Shift", hueShift, 0.01f, fRange { 0, 1 });
                ImGui::EditScalar("Saturation Multiplier", satMul, 0.01f, fRange { 0, 10 });
                ImGui::EditScalar("Value Shift", valShift, 0.01f, fRange { -1, 1 });)
            TAB_ITEM("Blur", shaderBlur, ImGui::EditVector("Blur Offset", effectOff, 0.1f); )
            TAB_ITEM("Edge Detection", shaderEdgeDetect, ImGui::EditVector("Detect Offset", effectOff, 0.1f); )
            ImGui::EndTabBar();
        }
#undef TAB_ITEM
    }

    void TestPostProcessing::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
        Graphics::Render::EnableDepth();
        Graphics::Render::DisableStencil();
    }
}
