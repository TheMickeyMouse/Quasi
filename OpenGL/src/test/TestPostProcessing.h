#pragma once
#include "FrameBuffer.h"
#include "Mesh.h"
#include "RenderBuffer.h"
#include "Test.h"

namespace Test {
    // all from https://learnopengl.com/Advanced-OpenGL/Framebuffers & https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    class TestPostProcessing : public Test {
    private:
        Graphics::RenderObject<VertexColor3D> scene;
        Graphics::RenderObject<VertexTexture2D> postProcessingQuad;

        Graphics::FrameBuffer fbo;
        Graphics::Texture renderResult;
        Graphics::RenderBuffer depthStencilAttachment;

        std::vector<Graphics::Mesh<VertexColor3D>> cubes;
        Graphics::Mesh<VertexTexture2D> screenQuad;

        Maths::mat3D projection = Maths::mat3D::perspective_fov(90.0f, 4.0f / 3.0f, 0.01f, 100.0f);
        Maths::fvec3 modelTranslation = 0;
        Maths::fvec3 modelScale       = 1;
        Maths::fvec3 modelRotation    = 0;

        Maths::fvec2 effectOff = 3;
        float hueShift = 0, satMul = 0, valShift = 0;

        Maths::fvec3 turnSpeed = 1;

        Graphics::Shader shaderInv, shaderHsv, shaderBlur, shaderEdgeDetect, *currShader;

        bool usePostProcessing = true;
    public:
        TestPostProcessing() = default;
        ~TestPostProcessing() override = default;

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnUpdate(float deltaTime) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
