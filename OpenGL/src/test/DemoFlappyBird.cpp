#include "DemoFlappyBird.h"

#include "imgui.h"
#include "Keyboard.h"
#include "Quad.h"
#include "Random.h"
#include "..\IO\Time.h"
#include "Tri.h"

namespace Test {
    void DemoFlappyBird::OnInit(Graphics::GraphicsDevice& gdevice) {
        Test::OnInit(gdevice);

        render = gdevice.CreateNewRender<Vertex>(128, 128);
        render.UseShader(
            "#shader vertex\n"
            "#version 330 core\n"
            "layout(location = 0) in vec4 position;\n"
            "layout(location = 1) in vec4 color;\n"
            "layout(location = 2) in vec2 texCoord;\n"
            "layout(location = 3) in float rtype;\n"
            "out vec4 v_color;\n"
            "out vec2 v_TexCoord;\n"
            "flat out int v_rtype;\n"
            "uniform mat4 u_projection;\n"
            "uniform mat4 u_view;\n"
            "void main() {\n"
            "   gl_Position = u_projection * u_view * position;\n"
            "   v_rtype = int(rtype);\n"
            "   v_TexCoord = texCoord;\n"
            "   v_color = color;\n"
            "}\n"
            "#shader fragment\n"
            "#version 330 core\n"
            "layout(location = 0) out vec4 color;\n"
            "in vec4 v_color;\n"
            "in vec2 v_TexCoord;\n"
            "flat in int v_rtype;\n"
            "uniform sampler2D u_font;\n"
            "void main() {\n"
            "   color = v_color;\n"
            "   if (v_rtype == 1) {\n"
            "       float alpha = texture(u_font, v_TexCoord).r;\n"
            "       alpha = smoothstep(0.492, 0.508, alpha);\n"
            "       color.a = alpha;\n"
            "   }\n"
            "}\n"
        );
        render.SetProjection(Maths::mat3D::ortho_projection({ -320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f }));

        font = Graphics::Font::LoadFile(GL_WIN_FONTS "consolab.ttf");
        font.SetSize(48);
        font.RenderBitmap();
        gdevice.BindTexture(font.GetTexture());

        mPlayer = Graphics::MeshUtils::CircleMesh(30.0f, 32)
                 .Convert<Vertex>([](const Maths::fvec2 pos) -> Vertex {
                     return { pos, 1, 0, 0 };
                 });
        using Graphics::Primitives::Quad;
        mBg = Quad { { 0, 240, 0 }, { 320, 0, 0 }, { 0, 20, 0 } }
             .IntoMesh<Vertex>([](Maths::fvec3 v) -> Maths::fvec2 { return v.xy(); }, &Vertex::Position)
             .Add(
                 Quad { { 0, -240, 0 }, { 320, 0, 0 }, { 0, 20, 0 } }
                .IntoMesh<Vertex>([](Maths::fvec3 v) -> Maths::fvec2 { return v.xy(); }, &Vertex::Position))
             .ApplyMaterial(&Vertex::Color, Maths::colorf::BETTER_GREEN());

        render.BindMeshes({ &mBg, &mPlayer });

        time = IO::Time.currentTime;
        nextSpawnTime = 0;
        Graphics::Render::SetClearColor(Maths::colorf::BETTER_BLACK());
    }

    void DemoFlappyBird::OnUpdate(float deltaTime) {
        Test::OnUpdate(deltaTime);

        if (!isEnd) {
            if (IO::Keyboard.KeyOnPress(IO::Key::SPACE)) {
                velocityY = 250;
            }

            yPos += velocityY * IO::Time.DeltaTimef();
            velocityY -= 400 * IO::Time.DeltaTimef();
            score = (int)(IO::Time.currentTime - time);
        }

        ManageSpikes();
        CheckPlayerCollisions();
    }

    void DemoFlappyBird::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);

        render.GetShader().Bind();
        render.GetShader().SetUniformTex("u_font", font.GetTexture());

        using namespace Graphics;
        mPlayer.SetTransform(Maths::mat3D::translate_mat({ -150, yPos, 0 }));
        mText.Replace(font.RenderText(std::to_string(score), 80,
            TextAlign { { -20, 20, 100, 140 } }.SpaceOut(1, -16))
            .Convert<Vertex>([](Font::Vertex v) -> Vertex {
                return { v.Position, v.Color, v.TextureCoord, 1 };
            }));

        render.ClearData();
        render.AddBoundMeshes();
        for (const Spike& spike : spikes) render.AddNewMeshes(&spike.mesh, 1);
        render.AddNewMeshes(&mText, 1);
        render.Render();
    }

    void DemoFlappyBird::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);

        ImGui::Text("Spike Count: %d", spikes.size());
    }

    void DemoFlappyBird::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        render.Destroy();
        Graphics::Render::SetClearColor(0);
    }

    void DemoFlappyBird::ManageSpikes() {
        if (isEnd) return;
        nextSpawnTime += IO::Time.deltaTime;
        if (nextSpawnTime > 2) {
            nextSpawnTime -= 2;
            const float midY = Maths::rand.getf(-100.0f, 100.0f);
            Maths::Geometry::ftriangle2d top = { { -50,  220 }, { 50,  220 }, { Maths::rand.getf(-15.0f, 15.0f), midY + 90 } };
            Maths::Geometry::ftriangle2d bot = { { -50, -220 }, { 50, -220 }, { Maths::rand.getf(-15.0f, 15.0f), midY - 90 } };

            spikes.emplace_back(
                Graphics::Primitives::Tri::FromGeometry(top)
               .IntoMesh<Vertex>(
                   [](const Maths::fvec3& v) -> Maths::fvec2 { return v.xy(); },
                   &Vertex::Position)
               .ApplyMaterial(&Vertex::Color, Maths::colorf::BETTER_LIME()),
                top, 370.0f);
            spikes.emplace_back(
                Graphics::Primitives::Tri::FromGeometry(bot)
               .IntoMesh<Vertex>(
                   [](const Maths::fvec3& v) -> Maths::fvec2 { return v.xy(); },
                   &Vertex::Position)
               .ApplyMaterial(&Vertex::Color, Maths::colorf::BETTER_AQUA()),
                bot, 370.0f);
        }

        for (Spike& spike : spikes) {
            spike.xOff -= 150 * IO::Time.DeltaTimef();
            spike.mesh.SetTransform(Maths::mat3D::translate_mat({ spike.xOff, 0, 0 }));
        }

        if (!spikes.empty() && spikes.front().xOff <= -370.0f) {
            spikes.pop_front();
            spikes.pop_front();
        }
    }

    void DemoFlappyBird::CheckPlayerCollisions() {
        if (yPos <= -190.0f || 190.0f <= yPos) goto die; // NOLINT(cppcoreguidelines-avoid-goto, hicpp-avoid-goto)
        for (const Spike& spike : spikes) {
            if (-120.0f < spike.collider.p2.x + spike.xOff) break;
            for (int i = 1; i < 3; ++i) {
                const Maths::Geometry::fline2d line = spike.collider.line(i) + Maths::fvec2::unit_x(spike.xOff);
                const Maths::fvec2 center           = Maths::fvec2 { -150, yPos },
                                   toCenter         = center - line.start,
                                   dist             = line.as_vec(),
                                   proj             = toCenter.project(dist);
                const float factor                  = std::abs(dist.x) > std::abs(dist.y) ? proj.x / dist.x : proj.y / dist.y;
                if (center.distsq(factor <= 0 ? line.start : (factor >= 1 ? line.end : proj + line.start)) <= 30.0f * 30.0f)
                    goto die;
            }
        }
        return;

        die:
        isEnd = true;
        mPlayer.ApplyMaterial(&Vertex::Color, Maths::colorf::BETTER_RED());
    }
}
