#include "TestDrawInstances.h"

#include <imgui.h>

#include "GUI/ImGuiExt.h"
#include "Meshes/MeshBuilder.h"

namespace Test {
    void TestDrawInstances::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Vertex>();

        transforms.Resize(INSTANCE_NUM);
        colors.Resize(INSTANCE_NUM);

        using namespace Math;
        const fv4 black = fColor3::Better::Black().AsRGBAfVec();
        const Matrix4x4 colorTransformer = Matrix4x4::FromColumns({
            fColor3::Better::Red() .AsRGBAfVec() - black,
            fColor3::Better::Lime().AsRGBAfVec() - black,
            fColor3::Better::Blue().AsRGBAfVec() - black,
            black
        });
        for (u32 i = 0; i < INSTANCE_NUM; ++i) {
            const u32 x = i % 3, y = i / 3 % 3, z = i / 9;
            const fv3 pos = { (f32)x, (f32)y, (f32)z };
            transforms[i].pos = (pos - 1) * 3;
            transforms[i].scale = 0.75f;
            colors[i] = fColor3 { (fv3)(colorTransformer * (pos * pos / 4)) };
        }

        cube = Graphics::Meshes::Cube().Create().IntoMesh(
            [&] (const fv3& p, const fv3& n) { return Graphics::VertexNormal3D { p, n }; }
        );

        scene.UseShaderFromFile(RES("instanced.vert"), RES("instanced.frag"));

        camera.position = { -6.8653593, -7.7674685, -6.846223 };
        camera.yaw = -2.2986794; camera.pitch = -0.55294377;
        camera.speed = 5;
        camera.sensitivity = 0.12;
        camera.fov = 90;
        camera.fovRange = { 1, 90 };
        camera.zoomRatio = 0.5;
        camera.smoothZoom = 120;
    }

    void TestDrawInstances::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);
    }

    void TestDrawInstances::OnRender(Graphics::GraphicsDevice& gdevice) {
        scene.SetProjection(camera.GetProjMat());
        scene.SetCamera(camera.GetViewMat());

        Vec<Math::Matrix3D> modelMats;
        Vec<Math::Matrix3x3> normMats;
        modelMats.Reserve(INSTANCE_NUM);
        normMats.Reserve(INSTANCE_NUM);
        for (const auto& t : transforms) {
            modelMats.Push(t.IntoMatrix());
            normMats.Push(t.IntoMatrixN());
        }

        scene.DrawInstanced(Spans::Only(cube), INSTANCE_NUM, Graphics::UseArgs({
            { "models",         modelMats },
            { "normMat",        normMats },
            { "colors",         colors },
            { "lightDirection", Math::fv3::FromSpheric(1, lightYaw, lightPitch) },
            { "ambientStrength", ambStrength },
        }));
    }

    void TestDrawInstances::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditYawPitch("Light Rotation", *lightYaw, *lightPitch);
        ImGui::EditScalar("Ambient", ambStrength, 0.05f);
        if (ImGui::TreeNode("Cube Instances")) {
            for (u32 i = 0; i < INSTANCE_NUM; ++i) {
                if (!ImGui::TreeNode(std::format("Cube #{}", i + 1).c_str())) continue;
                ImGui::EditTransform("Transform", transforms[i], 0.2f);
                ImGui::EditColor("Color", colors[i]);

                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        if (ImGui::Button("Randomize Rotations")) RandomizeRotations(gdevice);

        ImGui::EditCameraController("Camera", camera);
    }

    void TestDrawInstances::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
    }

    void TestDrawInstances::RandomizeRotations(Graphics::GraphicsDevice& gdevice) {
        for (u32 i = 0; i < INSTANCE_NUM; ++i) {
            transforms[i].rot = Math::Rotor3D::Random(gdevice.GetRand());
        }
    }
} // Test