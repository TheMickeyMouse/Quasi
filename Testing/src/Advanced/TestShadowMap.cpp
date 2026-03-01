#include "TestShadowMap.h"

#include <imgui.h>

#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "ModelLoading/OBJModelLoader.h"

namespace Test {
    void TestShadowMap::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Vertex>(3200, 1280);

        Graphics::OBJModelLoader mloader;
        mloader.LoadFile(RES("untitled.obj"));
        Graphics::OBJModel model = mloader.RetrieveModel();

        for (int i = 0; i < model.objects.Length(); ++i) {
            meshes.Push(
                model.objects[i].mesh.GeometryConvert(QGLCreateBlueprint$(Vertex, (
                    in (Position, Normal),
                    out (Position) = Position;,
                    out (Color) = Math::fColor::Better::Colors[i];,
                    out (Normal) = Normal;
                ))
            ));
        }

        depthShader = Graphics::Shader::FromFile(RES("depth.vert"), RES("depth.frag"));
        scene.UseShaderFromFile(RES("shadow.vert"), RES("shadow.frag"));

        depthMap = Graphics::FrameBuffer::New();
        depthTex = Graphics::Texture2D::New(
            nullptr, gdevice.GetWindowSize(), {
                .format = Graphics::TextureFormat::DEPTH,
                .internalformat = Graphics::TextureIFormat::DEPTH_16,
                .type = Graphics::TID::FLOAT
            }
        );
        depthTex.Bind();
        depthTex.SetBorderColor(Math::fColor::White());
        depthMap.Bind();
        depthMap.Attach(depthTex, 0, Graphics::AttachmentType::DEPTH);
        Graphics::Render::SetColorWrite(Graphics::BufferMode::NONE);
        depthMap.Complete();
        depthMap.Unbind();

        drawDepthBuffer = Graphics::Shader::New(String(QShaderQuad$(330)) + Text::ReadFile(RES("display.frag")).Unwrap());

        Graphics::Render::EnableCullFace();
        Graphics::Render::SetFrontFacing(Graphics::OrientationMode::CLOCKWISE);

        camera.position = { -9.684464, 10.050441, -1.4917407 };
        camera.yaw = -8.13158; camera.pitch = 0.6891256;
        camera.speed = 5;
        camera.sensitivity = 0.12;
        camera.fov = 90;
        camera.fovRange = { 1, 90 };
        camera.zoomRatio = 0.5;
        camera.smoothZoom = 120;
    }

    void TestShadowMap::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);
    }

    void TestShadowMap::OnRender(Graphics::GraphicsDevice& gdevice) {
        const Math::Matrix3D lightProj = Math::Matrix3D::PerspectiveFov(90.0_deg, gdevice.GetAspectRatio(), clipDistance.min, clipDistance.max),
                             lightView = Math::Matrix3D::LookAt(lightPosition, -lightPosition.Norm(), Math::fv3::Up());

        {
            Graphics::Render::SetCullFace(Graphics::FacingMode::FRONT);
            depthMap.Bind();
            Graphics::Render::ClearDepthBit();
            scene.SetProjection(lightProj);
            scene.SetCamera(lightView);

            scene.Draw(meshes, UseShader(depthShader));
            depthMap.Unbind();
        }
        Graphics::Render::SetCullFace(Graphics::FacingMode::BACK);
        Graphics::Render::ClearColorBit();
        Graphics::Render::ClearDepthBit();
        if (showDepthMap) {
            drawDepthBuffer.Bind();
            drawDepthBuffer.SetUniformTex("displayTex", depthTex, 1);
            drawDepthBuffer.SetUniformFloat("near", clipDistance.min);
            drawDepthBuffer.SetUniformFloat("far",  clipDistance.max);
            Graphics::Render::DrawScreenQuad(drawDepthBuffer);
        } else {
            scene.SetProjection(camera.GetProjMat());
            scene.SetCamera(camera.GetViewMat());

            scene.Draw(meshes, Graphics::UseArgs({
                { "lightSpaceMat", lightProj * lightView },
                { "depthMap", depthTex, 1 },
                { "lightPos", lightPosition },
                { "viewPos", camera.position },
                { "ambStrength", ambStrength },
                { "useSmoothShadows", useSmoothShadows }
            }));
        }
    }

    void TestShadowMap::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::Checkbox("Show Depth Mag [Debug]", &showDepthMap);
        ImGui::Checkbox("Use Smooth Shading", &useSmoothShadows);
        ImGui::EditScalar("Ambient Strength", ambStrength, 0.05f);
        ImGui::EditRange("Clip Distance", clipDistance);
        ImGui::EditVector("Light Position", lightPosition, 0.2f);
        ImGui::EditCameraController("Camera", camera);
        // ImGui::Image((void*)(intptr_t)depthTex.rendererID, { 400.0f, 300.0f }, { 0, 1 }, { 1, 0 });
    }

    void TestShadowMap::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
        Graphics::Render::DisableCullFace();
    }
} // Test