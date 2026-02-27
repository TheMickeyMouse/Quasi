#include "DemoFlappyBird.h"

#include "GLs/VertexBlueprint.h"
#include "Meshes/Circle.h"
#include "Meshes/Quad.h"
#include "Utils/Iter/MapIter.h"

namespace Test {
    void DemoFlappyBird::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Vertex>(2048, 2048);
        scene.UseShaderFromFile(RES("shader.vert"), RES("shader.frag"));
        scene.SetProjection(Math::Matrix3D::OrthoProjection({ { -320, -240, -1 }, { 320, 240, 1 } }));

        font = Graphics::Font::LoadFile(RES("consola.ttf"), 48);

        scene->shader.SetUniformTex("u_font", font.GetTexture(), 1);

        using namespace Math;
        const auto blueprint = QGLCreateBlueprint$(Vertex, (
            in (Position),
            out (Position) = Position;,
            out (Color)    = fColor::Better::Green();
        ));

        Graphics::Meshes::Quad().Merge(blueprint, Transform2D { { 0, +240 }, { 320, 20 } }, mBg.NewBatch());
        Graphics::Meshes::Quad().Merge(blueprint, Transform2D { { 0, -240 }, { 320, 20 } }, mBg.NewBatch());

        time = gdevice.GetIO().GetTime();
        nextSpawnTime = 0;
        Graphics::Render::SetClearColor(fColor::Better::Black());

        world.gravity = { 0, -600 };
        playerBody = world.CreateBody({ { -150, 120 } }, Physics2D::CircleShape { 30.0f });
        SetObstacle(world.CreateBody({ { 0, +240 }, 0, Physics2D::BodyType::STATIC }, Physics2D::RectShape { 320.0f, 20.0f }));
        SetObstacle(world.CreateBody({ { 0, -240 }, 0, Physics2D::BodyType::STATIC }, Physics2D::RectShape { 320.0f, 20.0f }));

        bloom.kneeOff = 0.6;
    }

    void DemoFlappyBird::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        world.Update(deltaTime, 3);

        if (isEnd) {
            if (!completedDeathAnim) {
                const Math::fv2 playerCenter = playerBody->position;
                world.DeleteBody(*playerBody);

                for (u32 i = 0; i < 10; ++i)
                    world.CreateBody({
                        playerCenter + Math::fv2::FromPolar(24, Math::Degrees(36 * (float)i))
                    }, Physics2D::CircleShape { 6.0f });
                completedDeathAnim = true;
            }
            return;
        }

        if (gdevice.GetIO()[IO::Key::SPACE].OnPress()) {
            playerBody->velocity.y = 250;
        }

        score = (int)(gdevice.GetIO().GetTime() - time);

        ManageSpikes(gdevice);
    }

    void DemoFlappyBird::OnRender(Graphics::GraphicsDevice& gdevice) {
        using namespace Graphics;
        bloom.SetToRenderTarget();
        Render::Clear();

        // mText = font.RenderText(Text::Format("{}", score), 80,
        //     TextAlign { {{ -20, 100 }, { 20, 140 }} }.SpaceOut(1, -16))
        //     .GeometryConvert([](const Font::Vertex& v) -> Vertex {
        //         return { v.Position, Math::fColor::Better::Gray(), v.TextureCoord, 1 };
        // });

        scene.BeginContext();
        scene.AddMesh(mBg);
        // scene.AddMesh(mText);
        Mesh<Vertex> spikeMesh;
        spikeMesh.vertices.ResizeWith(3, [] { return Vertex { 0, Math::fColor::Better::Lime(), 0, 0 }; });
        spikeMesh.indices.Push({ 0, 1, 2 });
        const Math::fColor playerColor = isEnd ? Math::fColor::Better::Red() : Math::fColor::Better::White();
        for (const auto& body : world.bodies) {
            if (const auto tri = body->shape.As<Physics2D::StaticPolygonShape>()) {
                spikeMesh.vertices[0].Position = tri->points[0] + body->position;
                spikeMesh.vertices[1].Position = tri->points[1] + body->position;
                spikeMesh.vertices[2].Position = tri->points[2] + body->position;
                scene.AddMesh(spikeMesh);
            } else if (const auto p = body->shape.As<Physics2D::CircleShape>()) {
                scene.AddMeshB(Meshes::Circle(p->radius),
                    [&] (const Vertex2D& v) {
                        return Vertex { v.Position + body->position, playerColor, 0, 0 };
                    });
            }
        }
        scene.EndContext();
        scene.DrawContext();

        bloom.ApplyEffect();
    }

    void DemoFlappyBird::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {}

    void DemoFlappyBird::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
        Graphics::Render::SetClearColor(0);
    }

    void DemoFlappyBird::DeathTrigger() {
        isEnd = true;
    }

    void DemoFlappyBird::SetObstacle(Physics2D::Body& obstacle) {
        obstacle.SetTrigger(FuncRefs::FromRaw(this,
        +[] (void* self, const Physics2D::Body&, const Physics2D::Body&, Physics2D::EventType) {
            Memory::UpcastPtr<DemoFlappyBird>(self)->DeathTrigger();
        }));
    }

    void DemoFlappyBird::ManageSpikes(Graphics::GraphicsDevice& gdevice) {
        using namespace Math;

        if (isEnd) return;
        nextSpawnTime += gdevice.GetIO().DeltaTime();
        if (nextSpawnTime > 2) {
            nextSpawnTime -= 2;
            const float midY = gdevice.GetRand().Get(-100.0f, 100.0f);
            const fTriangle2D top = { { -50,  220 }, { 50,  220 }, { gdevice.GetRand().Get(-15.0f, 15.0f), midY + 90 } };
            const fTriangle2D bot = { { -50, -220 }, { 50, -220 }, { gdevice.GetRand().Get(-15.0f, 15.0f), midY - 90 } };

            SetObstacle(world.CreatePolygon({ { 370.0f, 0.0f }, 0, Physics2D::BodyType::STATIC }, top.points));
            SetObstacle(world.CreatePolygon({ { 370.0f, 0.0f }, 0, Physics2D::BodyType::STATIC }, bot.points));
        }

        for (auto& spike : world.bodies) {
            if (spike->shape.Is<Physics2D::StaticPolygonShape>())
                spike->position.x -= 150 * gdevice.GetIO().DeltaTime();
        }
        world.bodies.Keep([] (const Box<Physics2D::Body>& bod) { return bod->position.x > -370.0f; });
    }
}
