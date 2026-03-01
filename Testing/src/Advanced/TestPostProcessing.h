#pragma once
#include "GLs/FrameBuffer.h"
#include "../../../Quasi/src/Graphics/Mesh.h"
#include "GLs/RenderBuffer.h"
#include "Test.h"
#include "GLs/Texture.h"

namespace Test {
    // all from https://learnopengl.com/Advanced-OpenGL/Framebuffers & https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    class TestPostProcessing : public Test {
    private:
        Graphics::RenderObject<Graphics::VertexNormal3D> scene;

        Graphics::FrameBuffer fbo;
        Graphics::Texture2D renderResult;
        Graphics::RenderBuffer depthStencilAttachment;

        Graphics::Mesh<Graphics::VertexNormal3D> cube;

        Math::Transform3D transform { { 0, 0, -5 } };
        Math::Rotor3D turnSpeed {};

        Math::fv3 locations[9];
        Math::fColor colors[9];

        Math::fv2 effectOff = 3;
        float hueShift = 0, satMul = 0, valShift = 0;
        float outlineSize = 1.1f;

        Graphics::Shader defaultShader, shaderInv, shaderHsv, shaderBlur, shaderEdgeDetect, *currShader;

        bool usePostProcessing = true;

        DEFINE_TEST_T(TestPostProcessing, ADVANCED)
    public:
        TestPostProcessing() = default;

        void OnInit(Graphics::GraphicsDevice& gdevice) override;
        void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) override;
        void OnRender(Graphics::GraphicsDevice& gdevice) override;
        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
