#pragma once
#include "Test.h"
#include "RenderData.h"
#include "Mesh.h"

namespace Test {
    class TestBatchedColored : public Test {
    private:
        Graphics::RenderObject<VertexColor3D> render;
        Graphics::Mesh<VertexColor3D> mesh;

        Maths::mat3D projection = Maths::mat3D::ortho_projection({ -320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f });
        Maths::fvec3 modelTranslation = 0;
        Maths::fvec3 modelScale = 1;
        Maths::fvec3 modelRotation = 0;
    public:
        TestBatchedColored() {}
        ~TestBatchedColored() override {}

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
