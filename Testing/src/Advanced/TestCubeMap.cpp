#include "TestCubeMap.h"

#include <imgui.h>

#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "Meshes/MeshBuilder.h"

namespace Test {
    void TestCubeMap::OnInit(Graphics::GraphicsDevice& gdevice) {
        using Math::fv2;

        scene = gdevice.CreateNewRender<Vertex>();
        skybox = Graphics::Meshes::Cube().Create().IntoMesh(
            [] (const Math::fv3& p, const Math::fv3& n) {
                return Vertex { p, p, n };
            }
        );

        cubemap = Graphics::TextureCubemap::LoadCubemapPNG(
            { RES("right.jpg"), RES("left.jpg"),
              RES("top.jpg"),   RES("bottom.jpg"),
              RES("front.jpg"), RES("back.jpg") });

        boxTex = Graphics::Texture2D::LoadPNG(RES("box.png"));

        u32 i = -1;
        box = Graphics::Meshes::Cube().Create().IntoMesh(
            [&] (const Math::fv3& p, const Math::fv3& n) {
                i++;
                return Vertex { p, (0.5f + fv2::FromCorner({ (bool)(i & 1), (bool)(i & 2) }) * 0.5f).AddZ(0), n };
            }
        );

        cubemapShader = Graphics::Shader::FromFile(RES("cubemap.vert"), RES("cubemap.frag"));
        boxShader     = Graphics::Shader::FromFile(RES("box.vert"),     RES("box.frag"));
        reflectShader = Graphics::Shader::FromFile(RES("reflect.vert"), RES("reflect.frag"));
        refractShader = Graphics::Shader::FromFile(RES("refract.vert"), RES("refract.frag"));

        scene.SetProjection(Math::Matrix3D::PerspectiveFov(90.0_deg, gdevice.GetAspectRatio(), 0.01f, 100.0f));

        camera.position = { 0.5531298, 0.82467157, -1.2348987 };
        camera.yaw = 2.2044506f; camera.pitch = 0.20220234f;
        camera.speed = 1.5;
        camera.sensitivity = 0.12f;
        camera.fov = 90;
        camera.fovRange = { 1, 90 };
        camera.zoomRatio = 0.5;
        camera.smoothZoom = 120;
    }

    void TestCubeMap::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);
    }

    void TestCubeMap::OnRender(Graphics::GraphicsDevice& gdevice) {
        scene.SetProjection(camera.GetProjMat());
        scene.SetCamera(camera.GetViewMat());

        scene.Draw(Spans::Only(skybox), UseShaderWithArgs(cubemapShader, {{ "cubemap", cubemap, 1 }}));

        switch (shaderID) {
            case DIFFUSE_SHADER_ID:
                scene.Draw(Spans::Only(box), UseShaderWithArgs(boxShader, {
                    { "boxTex", boxTex, 2 },
                    { "lightPosition", Math::fv3::FromSpheric(10.0f, lightYaw, lightPitch) },
                    { "ambientStrength", ambStrength }
                }));
            break;
            case REFLECTION_SHADER_ID:
                scene.Draw(Spans::Only(box), UseShaderWithArgs(reflectShader, {
                    { "skybox", cubemap, 1 },
                    { "viewPosition", camera.position },
                    { "ambStrength", ambStrength }
                }));
            break;
            case REFRACTION_SHADER_ID:
                scene.Draw(Spans::Only(box), UseShaderWithArgs(refractShader, {
                    { "skybox", cubemap, 1 },
                    { "viewPosition", camera.position },
                    { "ambStrength", ambStrength },
                    { "invRefractIndex", 1.0f / refractiveIndex }
                }));
            break;
            default:;
        }
    }

    void TestCubeMap::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditCameraController("Camera", camera);

        ImGui::EditYawPitch("Light Rotation", *lightYaw, *lightPitch);
        ImGui::EditScalar("Ambient Strength", ambStrength, 0.05f);

        ImGui::BeginTabBar("Shader Type");

        if (ImGui::TabItemButton("Diffuse"))    shaderID = DIFFUSE_SHADER_ID;
        if (ImGui::TabItemButton("Reflection")) shaderID = REFLECTION_SHADER_ID;
        if (ImGui::TabItemButton("Refraction")) shaderID = REFRACTION_SHADER_ID;

        switch (shaderID) {
            case REFRACTION_SHADER_ID:
                ImGui::EditScalar("Refractive Index", refractiveIndex, 0.05f);
            case DIFFUSE_SHADER_ID:
            case REFLECTION_SHADER_ID:
                ImGui::EditScalar("Ambient Strength", ambStrength, 0.05f);
            default:;
        }

        ImGui::EndTabBar();
    }

    void TestCubeMap::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
    }
}
