#pragma once
#include "Mesh.h"
#include "Test.h"

namespace Test {
    class TestCubeRender : public Test {
    private:
        // unsigned int faceOrder[6] = { 0, 1, 2, 3, 4, 5 };
        Graphics::RenderObject<VertexColor3D> render;
        Graphics::Mesh<VertexColor3D> cube;

        Maths::mat3D projection = Maths::mat3D::ortho_projection({ -5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f });
        Maths::fvec3 modelTranslation = 0;
        Maths::fvec3 modelScale       = 1;
        Maths::fvec3 modelRotation    = 0;
        float alpha = 1.0f;
    public:
        TestCubeRender() {}
        ~TestCubeRender() override {}

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
