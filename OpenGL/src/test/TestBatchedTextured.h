#pragma once
#include "Test.h"
#include "Mesh.h"
#include "Texture.h"

namespace Test {
    class TestBatchedTextured : public Test {
    private:
        Graphics::RenderObject<VertexColorTexture3D> render;
        Graphics::Mesh<VertexColorTexture3D> mesh;
        Graphics::Texture textures[2];

        Maths::mat3D projection = Maths::mat3D::ortho_projection({ -320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f });
        Maths::fvec3 modelTranslation = 0;
        Maths::fvec3 modelScale = 1;
        Maths::fvec3 modelRotation = 0;
    public:
        TestBatchedTextured() {}
        ~TestBatchedTextured() override {}

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
