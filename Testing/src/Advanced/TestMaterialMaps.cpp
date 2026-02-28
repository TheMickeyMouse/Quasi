#include "TestMaterialMaps.h"

#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "ModelLoading/OBJModelLoader.h"
#include "Meshes/Cube.h"

namespace Test {
    void TestMaterialMaps::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Graphics::VertexTextureNormal3D>();
        lightScene = gdevice.CreateNewRender<Graphics::VertexColor3D>(8, 12);

        Graphics::OBJModelLoader mloader;
        mloader.LoadFile(RES("boxes.obj"));

        meshes = mloader.GetModel().RetrieveMeshes();

        diffuseMap = Graphics::Texture2D::LoadPNG(RES("diffuse.png"));
        specularMap = Graphics::Texture2D::LoadPNG(RES("specular.png"));
        diffuseMap.Activate(0);
        specularMap.Activate(1);

        scene.UseShaderFromFile(RES("shader.vert"), RES("shader.frag"));

        scene->shader.Bind();
        scene->shader.SetUniformTex("diffuseMap", diffuseMap, 1);
        scene->shader.SetUniformTex("specularMap", specularMap, 2);
        scene->shader.Unbind();

        using namespace Math;
        Graphics::Meshes::Cube().Merge(QGLCreateBlueprint$(Graphics::VertexColor3D, (
            in (Position),
            out (Position) = Position;,
            out (Color)    = fColor::Better::Aqua();,
        )), lightSource.NewBatch());
        lightScene.UseShader(Graphics::Shader::StdColored);

        camera.position = { 6.3579054, 11.043502, 0.9266781 };
        camera.yaw = -11.125194; camera.pitch = 0.94481784;
        camera.speed = 1.5;
        camera.sensitivity = 0.12;
        camera.fov = 45;
        camera.fovRange = { 1, 45 };
        camera.zoomRatio = 0.5;
        camera.smoothZoom = 120;
    }

    void TestMaterialMaps::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);
    }

    void TestMaterialMaps::OnRender(Graphics::GraphicsDevice& gdevice) {
        lightScene.SetProjection(camera.GetProjMat());
        lightScene.SetCamera(camera.GetViewMat());

        for (auto& v : lightSource.vertices)
            v.Color = (Math::fColor)lightColor;
        lightSource.SetTransform(Math::Transform3D(lightPos));
        lightScene.Draw(lightSource);

        scene.SetProjection(camera.GetProjMat());
        scene.SetCamera(camera.GetViewMat());

        scene.Draw(meshes, Graphics::UseArgs({
            { "lightPosition",     lightPos },
            { "lightColor",        lightColor },
            { "ambientStrength",   ambientStrength },
            { "viewPosition",      camera.position },
            { "specularIntensity", specularStrength },
        }));
    }

    void TestMaterialMaps::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::EditVector("Light Position", lightPos);
        ImGui::EditColor("Light Color", lightColor);
        ImGui::EditScalar("Ambient Strength", ambientStrength, 0.01f);
        ImGui::EditScalar("Specular Strength", specularStrength, 0.01f);

        ImGui::EditCameraController("Camera", camera);
    }

    void TestMaterialMaps::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
        lightScene.Destroy();
    }
}
