#include "TestCubeRender.h"
#include "Quad.h"

#include "imgui.h"

namespace Test {
    void TestCubeRender::OnInit(Graphics::GraphicsDevice& gdevice) {
        render = gdevice.CreateNewRender<VertexColor3D>(4 * 6, 12);

        using Graphics::Primitives::Quad;
        using namespace Maths;

        cube = Graphics::Mesh<VertexColor3D>::Combine<6>(std::array {
                Quad({ +1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 })
                    .IntoMesh<VertexColor3D>()
                    .ApplyMaterial(&VertexColor3D::Color, colorf::RED()),
                Quad({ -1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 })
                    .IntoMesh<VertexColor3D>()
                    .ApplyMaterial(&VertexColor3D::Color, colorf::CYAN()),
                Quad({ 0, +1, 0 }, { 1, 0, 0 }, { 0, 0, 1 })
                    .IntoMesh<VertexColor3D>()
                    .ApplyMaterial(&VertexColor3D::Color, colorf::GREEN()),
                Quad({ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 })
                    .IntoMesh<VertexColor3D>()
                    .ApplyMaterial(&VertexColor3D::Color, colorf::MAGENTA()),
                Quad({ 0, 0, +1 }, { 1, 0, 0 }, { 0, 1, 0 })
                    .IntoMesh<VertexColor3D>()
                    .ApplyMaterial(&VertexColor3D::Color, colorf::BLUE()),
                Quad({ 0, 0, -1 }, { 1, 0, 0 }, { 0, 1, 0 })
                    .IntoMesh<VertexColor3D>()
                    .ApplyMaterial(&VertexColor3D::Color, colorf::YELLOW()),
                }
            );

        render.BindMeshes(cube);

        render.UseShader(
            GLSL_SHADER(
                330,
                (
                    layout(location = 0) in vec4 position;
                    layout(location = 1) in vec4 color;

                    out vec4 v_color;
                    out float v_alpha;

                    uniform mat4 u_projection;
                    uniform mat4 u_view;
                    uniform float u_alpha;

                    void main() {
                        gl_Position = u_projection * u_view * position;
                        v_color = color;
                        v_alpha = u_alpha;
                    }
                ),
                (
                    layout(location = 0) out vec4 color;

                    in vec4 v_color;
                    in float v_alpha;

                    void main() {
                        color = v_color;
                        color.a = v_alpha;
                    }
                )
            ));
        render.SetProjection(projection);
    }

    void TestCubeRender::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);

        Maths::mat3D mat = Maths::mat3D::transform(modelTranslation, modelScale, modelRotation);

        render.SetCamera(mat);
        render.GetShader().Bind();
        render.GetShader().SetUniform1F("u_alpha", alpha);

        render.ResetData();
        render.Render();
    }

    void TestCubeRender::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);

        ImGui::DragFloat3("Translation" , modelTranslation.begin(), 0.01f);
        ImGui::DragFloat3("Scale"       , modelScale.begin(),       0.01f);
        ImGui::DragFloat3("Rotation"    , modelRotation.begin(),    0.01f);
        ImGui::DragFloat ("Transparency", &alpha,                   0.01f);
    }

    void TestCubeRender::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        render.Destroy();
    }
}
