#include "TestDynamicVertexGeometry.h"

#include "imgui.h"

namespace Test {
    void TestDynamicVertexGeometry::OnInit(Graphics::GraphicsDevice& gdevice) {
        render = gdevice.CreateNewRender<VertexColor3D>(8, 4);

        render.UseShader(Graphics::Shader::StdColored);
        render.SetProjection(projection);

        using namespace Maths;
        VertexColor3D vertices[] = {
            { { -240.0f, -80.0f, 0.0f }, colorf::RED()   },
            { { -80.00f, -80.0f, 0.0f }, colorf::GREEN() },
            { { -80.00f, +80.0f, 0.0f }, colorf::BLUE()  },
            { { -240.0f, +80.0f, 0.0f }, colorf::GRAY()  },
            
            { { +80.00f, -80.0f, 0.0f }, colorf::MAGENTA() },
            { { +240.0f, -80.0f, 0.0f }, colorf::YELLOW()  },
            { { +240.0f, +80.0f, 0.0f }, colorf::CYAN()    },
            { { +80.00f, +80.0f, 0.0f }, colorf::WHITE()   },
        };

        Graphics::TriIndices indices[4] = {
            { 0, 1, 2 },  { 2, 3, 0 },
            { 4, 5, 6 },  { 6, 7, 4 }
        };

        mesh = Graphics::Mesh(
            std::vector(vertices, vertices + 8),
            std::vector(indices, indices + 4)
        );

        render.BindMeshes(mesh);
    }

    void TestDynamicVertexGeometry::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);

        render.ResetData();
        render.Render();
    }

    void TestDynamicVertexGeometry::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);
 
        VertexColor3D* vertices = mesh.GetVertices().data();
        ImGui::DragFloat3("Red    Vertex [0]", vertices[0].Position.begin());
        ImGui::DragFloat3("Green  Vertex [1]", vertices[1].Position.begin());
        ImGui::DragFloat3("Blue   Vertex [2]", vertices[2].Position.begin());
        ImGui::DragFloat3("Gray   Vertex [3]", vertices[3].Position.begin());

        ImGui::DragFloat3("Purple Vertex [4]", vertices[4].Position.begin());
        ImGui::DragFloat3("Yellow Vertex [5]", vertices[5].Position.begin());
        ImGui::DragFloat3("Cyan   Vertex [6]", vertices[6].Position.begin());
        ImGui::DragFloat3("White  Vertex [7]", vertices[7].Position.begin());
    }

    void TestDynamicVertexGeometry::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        render.Destroy();
    }
}
