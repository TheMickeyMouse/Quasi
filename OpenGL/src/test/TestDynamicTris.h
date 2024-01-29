#pragma once
#include "Test.h"
#include "Mesh.h"

namespace Test {
    class TestDynamicTris : public Test {
    private:
        static constexpr int MAX_TRIS = 8;

        bool isMax = false;
        bool isMin = false;
        
        Graphics::RenderObject<VertexColor3D> render;
        std::vector<Graphics::Mesh<VertexColor3D>> tris;
        
        Maths::mat3D projection = Maths::mat3D::ortho_projection({ -320.f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f });
        Maths::fvec2 modelTranslation = 0;
        Maths::fvec2 modelScale = 1;
        float modelRotation = 0.0f;

        Maths::mat3D ModelMatrix();
    public:
        TestDynamicTris() {}
        ~TestDynamicTris() override {}

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;

        OPENGL_API Graphics::Mesh<VertexColor3D> NewTri();

        static Maths::colorf COLORS[8];
    };
}
