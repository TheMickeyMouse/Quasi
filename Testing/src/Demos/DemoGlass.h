#pragma once
#include "Test.h"
#include "GLs/Texture.h"
#include "GUI/Canvas.h"

namespace Test {
    class DemoGlass : public Test {
        Graphics::Texture2D plain, glass;
        Graphics::Shader shader;
        Graphics::Canvas canvas;

        Math::fv2 min = { 4, 3 }, max = { 12, 6 };
        float radius = 0.3f, height = 0.7f;
        float glowSize = 0.3f, dG = 0.0f, d2G = 0.0f;
        float wave[6];
        Math::fv2 waveOrigin[6];

        float x = 0.1f;

        DEFINE_TEST_T(DemoGlass, DEMO)
    public:
        DemoGlass() = default;
        ~DemoGlass() override = default;

        Math::fv2 Refract(const Math::fv2& I, const Math::fv2& N, float eta);

        void OnInit(Graphics::GraphicsDevice& gdevice) override;
        void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) override;
        void OnRender(Graphics::GraphicsDevice& gdevice) override;
        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
