#include "TestCircleCollision2D.h"

#include <imgui.h>

#include "GLs/VertexBlueprint.h"
#include "GUI/ImGuiExt.h"
#include "Meshes/Circle.h"

#include "Utils/Iter/MapIter.h"

namespace Test {
    void TestCircleCollision2D::OnInit(Graphics::GraphicsDevice& gdevice) {
        scene = gdevice.CreateNewRender<Vertex>();

        circleMesh = Graphics::Meshes::Circle().Create(QGLCreateBlueprint$(Vertex, (
            in (Position),
            out (Position) = Position;
        )));

        scene.UseShaderFromFile(RES("circle.vert"), RES("circle.frag"));
        lineShader = Graphics::Shader::FromFile(RES("line.vert"), RES("line.frag"), RES("line.geom"));

        world = { { 0, -40.0f } };
        ResetBalls(gdevice);

        auto meshp = totalLineMesh.NewBatch();
        for (int i = 0; i < 10; ++i)
            meshp.PushV({ 0.0f });
        meshp.PushI(0, 1, 1);
        meshp.PushI(2, 3, 3);
        meshp.PushI(4, 5, 5);
        meshp.PushI(6, 7, 7);
        meshp.PushI(8, 9, 9);

        camera.position = { 40.0f, 30.0f };
        camera.speed = 7.0f;
        camera.scale = 15.0f;
    }

    void TestCircleCollision2D::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        camera.Update(gdevice, deltaTime);

        const Math::fRect2D viewport = camera.GetViewport();
        const auto& io = gdevice.GetIO();
        const Math::fv2 screen = (Math::fv2)gdevice.GetWindowSize();
        const Math::fv2 mousePos = io.GetMousePos().MapCoords(Math::fRect2D { { 0, screen.y }, { screen.x, 0 } }, viewport);
        const auto& left = io.LeftMouse(), &right = io.RightMouse();
        if (left.OnPress() && !selected) {
            selected = FindBallAt(mousePos);
            if (selected)
                selectOffset = mousePos - selected->position;
        }

        if (left.Pressed() && selected) {
            const Math::fv2 newPos = mousePos - selectOffset;
            selected->position = newPos;
            selected->velocity = 0;
        }

        if (left.OnRelease()) selected = nullptr;

        if (right.OnPress() && !selected) {
            selected = FindBallAt(mousePos);
            selected = selected && selected->IsDynamic() ? selected : nullptr;
        }

        if (right.Pressed() && selected) {
            totalLineMesh.vertices[8].Position = selected->position;
            totalLineMesh.vertices[9].Position = mousePos;
        }

        if (right.OnRelease() && selected && selected->IsDynamic()) {
            const bool scale = io[IO::Key::LCONTROL].Pressed();
            selected->velocity -= (scale ? 10.0f : 1.0f) * (mousePos - selected->position);
            totalLineMesh.vertices[8].Position = 0;
            totalLineMesh.vertices[9].Position = 0;
            selected = nullptr;
        }

        {
            // [[maybe_unused]] Timer t { "PhysicsUpdate", Debug::Logger::InternalLog };
            world.Update(deltaTime, 32);
        }

        for (int i = 0; i < 4; ++i) {
            auto r = edge[i]->shape.As<Physics2D::RectShape>();
            totalLineMesh.vertices[2 * i + 0].Position = r->Corner(i == 0, i == 2) + edge[i]->position;
            totalLineMesh.vertices[2 * i + 1].Position = r->Corner(i != 1, i != 3) + edge[i]->position;
        }

        lineShader.Bind();
    }

    void TestCircleCollision2D::OnRender(Graphics::GraphicsDevice& gdevice) {
        scene->shader.Bind();
        Array<Math::fv2, TOTAL_BALL_COUNT> offsets;
        Array<float, TOTAL_BALL_COUNT> scales;
        Array<Math::fColor, TOTAL_BALL_COUNT> colors;

        usize i = 0;
        int selectedIndex = -1;
        for (auto& body : world.bodies) {
            if (body->shape.Is<Physics2D::RectShape>()) continue;
            if (selectedIndex == -1 && selected && selected.RefEquals(body.AsRef())) selectedIndex = (int)i;
            offsets[i] = body->position;
            scales[i] = body->shape.As<Physics2D::CircleShape>()->radius;
            colors[i] = Math::fColor::FromHSV(
                Math::Clamp(fRange { 0, 80.0f }.MapTo(offsets[i].x, { 0, 1 }), 0.0f, 1.0f),
                body->IsStatic() ? 0.2f : 0.8f,
                body->IsStatic() ? 0.5f : 0.8f);
            ++i;
        }

        scene.SetProjection(Math::Matrix3D::OrthoProjection(camera.GetViewport().AddZ({ -1, 1 })));
        scene.DrawInstanced(circleMesh, TOTAL_BALL_COUNT, Graphics::UseArgs({
            { "u_projection", scene->projection },
            { "selected",     selectedIndex },
            { "offsets",      offsets },
            { "scales",       scales },
            { "colors",       colors }
        }, false));

        scene.Draw(totalLineMesh, UseShaderWithArgs(lineShader, {
            { "u_projection", scene->projection }
        }, false));
    }

    void TestCircleCollision2D::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        if (ImGui::Button("Reset Balls")) ResetBalls(gdevice);

        ImGui::BulletText("Controls");
        ImGui::Text("Left Click = Drag");
        ImGui::Text("Right Click = Add Velocity");
        ImGui::Text(" + Control for increased Force");
        ImGui::Text("Middle Click = Move");

        ImGui::EditVector("Gravity", world.gravity);
    }

    void TestCircleCollision2D::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        scene.Destroy();
    }

    void TestCircleCollision2D::AddRandomBall(Graphics::GraphicsDevice& gdevice) {
        auto& rand = gdevice.GetRand();

        const Math::fv2 position = Math::fv2::Random(rand, { 3.0f, { 77.0f, 57.0f } });
        const float radius = rand.Get(1.0f, 3.0f);
        world.CreateBody({ .position = position, .density = 5.0f }, Physics2D::CircleShape { radius });
    }

    void TestCircleCollision2D::ResetBalls(Graphics::GraphicsDevice& gdevice) {
        world.Clear();

        for (u32 i = 0; i < DYNAMIC_BALL_COUNT; ++i)
            AddRandomBall(gdevice);

        using namespace Physics2D;
        auto& rand = gdevice.GetRand();

        const fRect2D viewport = { 0, { 80.0f, 60.0f } };
        for (u32 i = 0; i < STATIC_BALL_COUNT; ++i) {
            world.CreateBody({
                .position = fv2::Random(rand, viewport),
                .type = BodyType::STATIC,
                .density = 0.0f },
            CircleShape { rand.Get(4.0f, 6.0f) });
        }

        edge[0] = world.CreateBody({ .position = {                0, +viewport.Height() * 0.5f }, .type = BodyType::STATIC }, RectShape { 1.0f, viewport.Height() * 0.5f - 1.0f });;
        edge[1] = world.CreateBody({ .position = { viewport.Width(), +viewport.Height() * 0.5f }, .type = BodyType::STATIC }, RectShape { 1.0f, viewport.Height() * 0.5f - 1.0f });
        edge[2] = world.CreateBody({ .position = { viewport.Width() * 0.5f, +                0 }, .type = BodyType::STATIC }, RectShape { viewport.Width() * 0.5f - 1.0f,  1.0f });
        edge[3] = world.CreateBody({ .position = { viewport.Width() * 0.5f, +viewport.Height() }, .type = BodyType::STATIC }, RectShape { viewport.Width() * 0.5f - 1.0f,  1.0f });
    }

    OptRef<Physics2D::Body> TestCircleCollision2D::FindBallAt(const Math::fv2& mousePos) {
        const Physics2D::Shape mouse = Physics2D::CircleShape { 0.0f };
        for (auto& circ : world.bodies) {
            if (circ->OverlapsWith(mouse, Math::Pose2D(mousePos))) {
                return *circ;
            }
        }
        return nullptr;
    }
} // Test