#include "TestBatchedColored.h"
#include "VertexElement.h"
#include "TriIndices.h"

#include "imgui.h"

namespace Test {
    void TestBatchedColored::OnInit(Graphics::GraphicsDevice& gdevice) {
        render = gdevice.CreateNewRender<VertexColor3D>(8, 4);
        VertexColor3D vertices[4 * 2] = {
            { { -240.0f, -80.0f, 0 }, { 0.1255f, 0.2000f, 0.8588f } },
            { { -80.00f, -80.0f, 0 }, { 0.3647f, 0.2000f, 0.9000f } },
            { { -80.00f, +80.0f, 0 }, { 0.1255f, 0.2000f, 0.8588f } },
            { { -240.0f, +80.0f, 0 }, { 0.1608f, 0.4000f, 0.8059f } },

            { { +240.0f, -80.0f, 0 }, { 0.9020f, 0.5451f, 0.0431f } },
            { { +80.00f, -80.0f, 0 }, { 0.9490f, 0.7451f, 0.0745f } },
            { { +80.00f, +80.0f, 0 }, { 0.9686f, 0.9137f, 0.1176f } },
            { { +240.0f, +80.0f, 0 }, { 0.9490f, 0.7451f, 0.0745f } },
        };

        Graphics::TriIndices indices[4] = {
            { 0, 1, 2 },  { 2, 3, 0 },
            { 4, 5, 6 },  { 6, 7, 4 }
        };
        mesh = Graphics::Mesh {
            std::vector(vertices, vertices + 8),
            std::vector(indices,  indices  + 4)
        };

        render.BindMeshes(mesh);

        render.UseShader(Graphics::Shader::StdColored);
        render.SetProjection(projection);
    }

    void TestBatchedColored::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);
        //shader.SetUniform4F("u_Color", color);
        Maths::mat3D mat = Maths::mat3D::transform(modelTranslation,
                                                   modelScale,
                                                   modelRotation);
        render.SetCamera(mat);

        //LOG(mat);
        render.ResetData();
        render.Render();
    }

    void TestBatchedColored::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);
        
        ImGui::DragFloat3("Translation", modelTranslation.begin());
        ImGui::DragFloat3("Scale",       modelScale.begin(), 0.1f);
        ImGui::DragFloat3("Rotation",    modelRotation.begin(), 0.03f);
    }

    void TestBatchedColored::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        render.Destroy();
    }
}
