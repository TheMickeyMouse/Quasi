#pragma once

#include "Test.h"

namespace Test {
    class TestDynamicQuadGeometry : public Test {
    private:
        static constexpr int MAX_QUAD = 8;
        static constexpr int MAX_VERTEX = MAX_QUAD * 4;
        static constexpr int MAX_INDEX = MAX_QUAD * 6;
        
        bool isMin = false;
        bool isMax = false;

        Graphics::RenderObject<VertexColor3D> render;
        std::vector<Graphics::Mesh<VertexColor3D>> quads;

        Maths::mat3D projection = Maths::mat3D::ortho_projection({ -320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f });
    public:
        TestDynamicQuadGeometry() {}
        ~TestDynamicQuadGeometry() override {}

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;

        OPENGL_API Graphics::Mesh<VertexColor3D> NewQuad();
        
        static Maths::colorf COLORS[8];
    };
}
