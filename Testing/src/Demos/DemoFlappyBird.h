#pragma once
#include <queue>

#include "Test.h"
#include "Mesh.h"
#include "Effects/Bloom.h"
#include "Utils/Math/Geometry.h"
#include "Fonts/Font.h"
#include "Physics/World2D.h"

namespace Test {
    class DemoFlappyBird : public Test {
        struct Vertex {
            Math::fv2 Position;
            Math::fColor Color;
            Math::fv2 TextureCoord;
            int RenderType;

            QuasiDefineVertex$(Vertex, 2D, (Position, Position)(Color)(TextureCoord)(RenderType));
        };

        Graphics::RenderObject<Vertex> scene;
        Graphics::Bloom bloom {{ 1200, 900 }};
        Physics2D::World world;
        OptRef<Physics2D::Body> playerBody;

        Graphics::Font font;
        Graphics::Mesh<Vertex> mBg;
        u32 loopStart = 0;

        double time = 0, nextSpawnTime = 0;

        int score = 0;
        bool isEnd = false;
        bool completedDeathAnim = false;

        DEFINE_TEST_T(DemoFlappyBird, DEMO)
    public:
        DemoFlappyBird() = default;
        ~DemoFlappyBird() override = default;

        void OnInit(Graphics::GraphicsDevice& gdevice) override;
        void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) override;
        void OnRender(Graphics::GraphicsDevice& gdevice) override;
        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        void OnDestroy(Graphics::GraphicsDevice& gdevice) override;

        void DeathTrigger();
        void SetObstacle(Physics2D::Body& obstacle);
        void ManageSpikes(Graphics::GraphicsDevice& gdevice);
    };
}
