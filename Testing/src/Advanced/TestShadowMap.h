#pragma once
#include "Test.h"
#include "GLs/FrameBuffer.h"
#include "GLs/Texture.h"
#include "Graphics/CameraController3D.h"
#include "Graphics/Mesh.h"

namespace Test {
    class TestShadowMap : public Test {
        using Vertex = Graphics::VertexColorNormal3D;
        Graphics::RenderObject<Vertex> scene;
        Vec<Graphics::Mesh<Vertex>> meshes;
        Graphics::FrameBuffer depthMap;
        Graphics::Texture2D depthTex;
        Graphics::Shader depthShader, drawDepthBuffer;

        Math::fv3 lightPosition { -8.2f, 11.2f, 6.6f };
        float ambStrength = 0.2f;
        fRange clipDistance = { 1.0f, 100.0f };
        bool showDepthMap = false, useSmoothShadows = true;

        Graphics::CameraController3D camera;

        DEFINE_TEST_T(TestShadowMap, ADVANCED);
    public:
        void OnInit(Graphics::GraphicsDevice& gdevice) override;
        void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) override;
        void OnRender(Graphics::GraphicsDevice& gdevice) override;
        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
} // Test
