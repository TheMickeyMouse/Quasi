#include "TestAdvancedLighting.h"

#include "GUI/ImGuiExt.h"
#include "ModelLoading/OBJModelLoader.h"

namespace Test {
    void TestAdvancedLighting::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Vertex>(1024, 512);

        Graphics::OBJModelLoader mloader;
        mloader.LoadFile(RES("untitled.obj"));
        Graphics::OBJModel model = mloader.RetrieveModel();

        materials = std::move(model.materials);
        meshes.Reserve(model.objects.Length());
        for (Graphics::OBJObject& obj : model.objects) {
            meshes.Push(std::move(obj.mesh).GeometryConvert([&] (const Graphics::OBJVertex& v) {
                    return Vertex { v.Position, v.Normal, obj.materialIndex };
                }
            ));
        }

        scene.UseShaderFromFile(RES("shader.vert"), RES("shader.frag"));
        scene.SetProjection(Math::Matrix3D::PerspectiveFov(90.0_deg, gdevice.GetAspectRatio(), 0.01f, 100.0f));

        camera.position = { 4.7875953, 9.015127, 0.9847422 };
        camera.yaw = -11.068636; camera.pitch = 1.0935568;
        camera.speed = 3;
        camera.sensitivity = 0.12;
        camera.fov = 90;
        camera.fovRange = { 1, 90 };
        camera.zoomRatio = 0.5;
        camera.smoothZoom = 120;

        camera.Toggle(gdevice);
    }

    void TestAdvancedLighting::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);

        // lightPos = Math::fv2::from_polar(5, (float)gdevice.GetIO().Time.currentTime).with_z(0);
        // light.SetTransform(Math::Matrix3D::translate_mat(lightPos));
    }

    void TestAdvancedLighting::OnRender(Graphics::GraphicsDevice& gdevice) {
        scene->shader.Bind();

        for (u32 i = 0; i < materials.Length(); ++i) {
            UniformMaterial(i, materials[i]);
        }

        scene.SetProjection(camera.GetProjMat());
        scene.SetCamera(camera.GetViewMat());
        scene.Draw(meshes, Graphics::UseArgs({
            { "lightPosition",     lightPos },
            { "lightColor",        lightColor },
            { "ambientStrength",   ambientStrength },
            { "viewPosition",      camera.position },
            { "specularIntensity", specularStrength }
        }));
    }

    void TestAdvancedLighting::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditVector("Light Position", lightPos);
        ImGui::EditColor ("Light Color", lightColor);
        ImGui::EditScalar("Ambient Strength", ambientStrength, 0.01f);
        ImGui::EditScalar("Specular Strength", specularStrength, 0.01f);

        ImGui::EditCameraController("Camera", camera);
    }

    void TestAdvancedLighting::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
    }

    void TestAdvancedLighting::UniformMaterial(u32 index, const Graphics::MTLMaterial& material) {
        char strbuf[] = "materials[_].ambient\0\0";
        strbuf[10] = (char)('0' + index);
        char* const afterPeriod = strbuf + Q_STRLIT_LEN("materials[_].");

        scene->shader.SetUniformColor(strbuf, material.Ka);
        Memory::WriteU64Big("diffuse\0"_u64, afterPeriod);
        scene->shader.SetUniformColor(strbuf, material.Kd);
        Memory::WriteU64Big("specular"_u64, afterPeriod);
        scene->shader.SetUniformColor(strbuf, material.Ks);
        Memory::WriteU64Big("shinines"_u64, afterPeriod);
        afterPeriod[8] = 's';
        scene->shader.SetUniformFloat(strbuf, material.Ns);
    }
}
