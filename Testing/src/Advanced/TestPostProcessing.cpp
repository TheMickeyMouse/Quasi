#include "TestPostProcessing.h"

#include "Mesh.h"
#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "Meshes/Cube.h"
#include "Meshes/Plane.h"
#include "Meshes/Quad.h"

namespace Test {
    void TestPostProcessing::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Graphics::VertexColor3D>(216, 324);
        postProcessingQuad = gdevice.CreateNewRender<Graphics::VertexTexture2D>(4, 2);

        cubes.Reserve(9);

        using namespace Math;
        constexpr float s = 0.3f;
        for (int i = 0; i < 8; ++i) {
            cubes.Push(Graphics::Meshes::Cube().Create(QGLCreateBlueprint$(Graphics::VertexColor3D, (
                in (Position),
                out (Position) = Position;,
                out (Color) = fColor::Better::Colors[i];
            )), Transform3D(0, s, {})));

            cubes[i].SetTransform(Transform3D(fv3::FromCorner({ (bool)(i & 1), (bool)(i & 2), (bool)(i & 4) }, 1)));
        }
        cubes.Push(
        Graphics::Meshes::Cube().Create(QGLCreateBlueprint$(Graphics::VertexColor3D, (
                in (Position),
                out (Position) = Position;,
                out (Color) = fColor::Better::Gray();
        )), Transform3D(0, s, {})));

        scene.UseShader(Graphics::Shader::StdColored);
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

        screenQuad = Graphics::Meshes::Quad().Create(QGLCreateBlueprint$(Graphics::VertexTexture2D, (
            in (Position),
            out (Position) = Position;,
            out (TextureCoordinate) = (Position + 1) * 0.5f;
        )));

        String vert = RES_STR("vertex.vert");
        const CStr vertloc = vert.IntoCStr();
        postProcessingQuad.UseShaderFromFile(vertloc, RES("none.frag"));

        shaderInv        = Graphics::Shader::FromFile(vertloc, RES("invert.frag"));
        shaderHsv        = Graphics::Shader::FromFile(vertloc, RES("hsv.frag"));
        shaderBlur       = Graphics::Shader::FromFile(vertloc, RES("simple_blur.frag"));
        shaderEdgeDetect = Graphics::Shader::FromFile(vertloc, RES("simple_ed.frag"));
        shaderOutline    = Graphics::Shader::FromFile(RES("outline.vert"), RES("outline.frag"));

        currShader = &postProcessingQuad->shader;
        renderResult.Activate(0);
    }

    void TestPostProcessing::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        transform.rot += turnSpeed * deltaTime;
    }

    void TestPostProcessing::OnRender(Graphics::GraphicsDevice& gdevice) {
        scene.SetCamera(transform.IntoMatrix());

        if (currShader != &shaderOutline) {
            Graphics::Render::EnableDepth();
            if (usePostProcessing) {
                fbo.Bind();
                Graphics::Render::Clear();
            }
        }

        scene.Draw(cubes);

        if (usePostProcessing) {
            if (currShader == &shaderOutline) {
                Graphics::Render::UseStencilTest(Graphics::CmpOperation::NOTEQUAL, 1); // pass if it hasnt been set (drawn to) yet
                Graphics::Render::DisableDepth();
                Graphics::Render::DisableStencilWrite();

                for (auto& cube : cubes) cube.modelTransform.scale = outlineSize;
                scene.Draw(cubes, UseShaderWithArgs(shaderOutline, {{ "outlineColor", Math::fColor { 1 } }}));
                for (auto& cube : cubes) cube.modelTransform.scale = 1;

                Graphics::Render::UseStencilTest(Graphics::CmpOperation::ALWAYS, 1);
                Graphics::Render::EnableStencilWrite(); // write to stencil
                Graphics::Render::EnableDepth();
                return;
            }

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
            postProcessingQuad.Draw(screenQuad, UseShader(*currShader, false));
        }
    }

    void TestPostProcessing::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditTransform("Transform", transform, 0.01);

        ImGui::EditRotation3D("Spin Speed", turnSpeed);

        ImGui::Separator();

        ImGui::Checkbox("Use Post Processing", &usePostProcessing);

        const Graphics::Shader* prev = currShader;
#define TAB_ITEM(N, P, C) if (ImGui::BeginTabItem(N)) { currShader = &(P); ImGui::EndTabItem(); C }
        if (ImGui::BeginTabBar("Post Processing Shader")) {
            TAB_ITEM("None", postProcessingQuad->shader, )
            TAB_ITEM("Color Invert", shaderInv, )
            TAB_ITEM("Color Hue", shaderHsv,
                ImGui::EditScalar("Hue Shift", hueShift, 0.01f, fRange { 0, 1 });
                ImGui::EditScalar("Saturation Multiplier", satMul, 0.01f, fRange { 0, 10 });
                ImGui::EditScalar("Value Shift", valShift, 0.01f, fRange { -1, 1 });)
            TAB_ITEM("Blur", shaderBlur, ImGui::EditVector("Blur Offset", effectOff, 0.1f); )
            TAB_ITEM("Edge Detection", shaderEdgeDetect, ImGui::EditVector("Detect Offset", effectOff, 0.1f); )
            TAB_ITEM("Outline (Stencil)", shaderOutline,
                ImGui::EditScalar("Outline Size", outlineSize, 0.01f, fRange { 1, 2 });
            )
            ImGui::EndTabBar();
        }
#undef TAB_ITEM

        if (prev != currShader) {
            if (currShader == &shaderOutline) {
                Graphics::Render::EnableStencil();
                Graphics::Render::UseStencilTest(Graphics::CmpOperation::NOTEQUAL, 1); // always pass stencil test
                Graphics::Render::UseStencilWriteOp(
                    Graphics::StencilOperation::KEEP,
                    Graphics::StencilOperation::KEEP,
                    Graphics::StencilOperation::REPLACE // set to 1 (ref from stencil func)
                );
                Graphics::Render::EnableStencilWrite(); // write to stencil
            } else {
                Graphics::Render::DisableStencil();
            }
        }
    }

    void TestPostProcessing::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
        Graphics::Render::EnableDepth();
        Graphics::Render::DisableStencil();
    }
}
