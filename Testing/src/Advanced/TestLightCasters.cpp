#include "TestLightCasters.h"

#include "imgui.h"
#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "Meshes/MeshBuilder.h"
#include "ModelLoading/OBJModelLoader.h"

namespace Test {
    void TestLightCasters::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Vertex>();
        lightScene = gdevice.CreateNewRender<Graphics::Vertex3D>();

        Graphics::OBJModelLoader mloader;
        mloader.LoadFile(RES("lights.obj"));
        Graphics::OBJModel model = mloader.RetrieveModel();

        materials = std::move(model.materials);
        for (Graphics::OBJObject& obj : model.objects) {
            worldMesh->Add(std::move(obj.mesh).GeometryConvert(
                [&] (const Graphics::OBJVertex& v) { return Vertex { v.Position, v.Normal, obj.materialIndex }; }
            ));
        }

        lightBox = Graphics::Meshes::Cube().Create().IntoMesh(
            [] (const Math::fv3& p, const Math::fv3& _n) { return Graphics::Vertex3D { p }; }
        );

        scene.UseShaderFromFile(RES("shader.vert"), RES("shader.frag"));
        scene.SetProjection(Math::Matrix3D::PerspectiveFov(90.0_deg, gdevice.GetAspectRatio(), 0.01f, 100.0f));

        camera.position = { 8.746245, 16.436476, 7.217131 };
        camera.yaw = -5.5221653; camera.pitch = 1.1316143;
        camera.speed = 5;
        camera.sensitivity = 0.12;
        camera.fov = 90;
        camera.fovRange = { 1, 90 };
        camera.zoomRatio = 0.5;
        camera.smoothZoom = 120;

        lights.Reserve(MAX_LIGHTS);
        AddPointLight({
            .position = { -0, 7, 0 },
            .constant = 1.7f,
            .linear = 0,
            .quadratic = 0
        }, { 1, 1, 1 });

        AddPointLight({
            .position = { 12, 3, -14 },
            .constant = 1,
            .linear = 0,
            .quadratic = 0.001f,
         }, { 1, 0.3f, 0.3f });

        AddPointLight({
            .position = { -9, 3, 6 },
            .constant = 1.4f,
            .linear = 0,
            .quadratic = 0.001f,
         }, { 0.3f, 0.6f, 1 });

        AddPointLight({
            .position = { 12, 3, 16 },
            .constant = 1.4f,
            .linear = 0,
            .quadratic = 0.001f,
         }, { 0.4f, 1, 0.2f });

        lightScene.UseShaderFromFile(RES("light.glsl"));
    }

    void TestLightCasters::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);
    }

    void TestLightCasters::OnRender(Graphics::GraphicsDevice& gdevice) {
        bloom.SetToRenderTarget();
        Graphics::Render::Clear();

        lightScene.SetProjection(camera.GetProjMat());
        lightScene.SetCamera(camera.GetViewMat());

        Math::fv3 locations[8];
        Math::fColor colors[8];
        Math::fv4 infos[8];
        int tags[8];
        for (u32 i = 0; i < lights.Length(); i++) {
            locations[i] = lights[i].Position();
            colors[i] = lights[i].color;
            infos[i] = lights[i].Visit<Math::fv4>(
                [&] (const Graphics::SunLight& sun) { return 0; },
                [&] (const Graphics::PointLight& point) {
                    return Math::fv4 { point.constant, point.linear, point.quadratic };
                },
                [&] (const Graphics::FlashLight& flash) {
                    return Math::fv4 { *flash.yaw, *flash.pitch, *flash.innerCut, *flash.outerCut };
                }
            );
            tags[i] = lights[i].GetTag() + 1;
        }

        lightScene.DrawInstanced(Spans::Only(lightBox), lights.Length(), {
            .arguments = {
                { "lightPos", locations },
                { "colors",   colors }
            }
        });

        scene->shader.Bind();
        for (u32 i = 0; i < materials.Length(); ++i) {
            UniformMaterial(Text::Format("materials[{}]", i), materials[i]);
        }

        scene.SetProjection(camera.GetProjMat());
        scene.SetCamera(camera.GetViewMat());
        scene.Draw(Spans::Only(worldMesh), Graphics::UseArgs({
            { "ambientStrength",   ambientStrength },
            { "viewPosition",      camera.position },
            { "specularIntensity", specularStrength },
            { "lightLoc",          locations },
            { "lightColor",        colors },
            { "lightInfo",         infos },
            { "lightId",           tags }
        }));
        bloom.ApplyEffect();
    }

    void TestLightCasters::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditScalar("Ambient Strength", ambientStrength, 0.01f);
        ImGui::EditScalar("Specular Strength", specularStrength, 0.01f);

        ImGui::EditCameraController("Camera", camera);

        if (ImGui::TreeNode("Lights")) {
            if (ImGui::Button("+") && lights.Length() < MAX_LIGHTS) {
                AddPointLight({
                    .position = 0,
                    .constant = 1, .linear = 0, .quadratic = 0
                }, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("-") && lights) {
                lights.Pop();
            }

            for (u32 i = 0; i < lights.Length(); ++i) {
                ImGui::EditLight(Text::Format("Light {}", i + 1), lights[i]);
            }
            ImGui::TreePop();
        }

        ImGui::SliderFloat("Bloom Threshold", &bloom.threshold, 0.0f, 1.0f);
        ImGui::SliderFloat("Bloom Knee Off", &bloom.kneeOff, 0.0f, 1.0f);
        ImGui::SliderFloat("Bloom Intensity", &bloom.intensity, 0.0f, 1.0f);
    }

    void TestLightCasters::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
        lightScene.Destroy();
    }

    void TestLightCasters::UniformMaterial(const String& name, const Graphics::MTLMaterial& material) {
        Graphics::Shader& shader = scene->shader;
        shader.SetUniformArgs({
            { name + ".ambient",   material.Ka },
            { name + ".diffuse",   material.Kd },
            { name + ".specular",  material.Ks },
            { name + ".shininess", material.Ns },
        });
    }

    void TestLightCasters::UniformLight(const String& name, const Graphics::Light& light) {
        Graphics::Shader& shader = scene->shader;
        Math::fv3 top, bottom;

        shader.SetUniformArgs({
            { name + ".lightId", (int)light.GetTag() + 1 },
            { name + ".d1", top },
            { name + ".d2", bottom },
            { name + ".d3", light.Is<Graphics::FlashLight>() ? *light.As<Graphics::FlashLight>()->outerCut : 0 },
            { name + ".color", light.color },
        });
    }

    void TestLightCasters::AddPointLight(const Graphics::PointLight& point, const Math::fColor& color) {
        auto& L = lights.Push({ Graphics::PointLight { point } });
        L.color = color;
        L.Position() = point.position;
    }
}
