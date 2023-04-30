#include "Game.h"

#include "IO/IO.h"
#include "imgui.h"

namespace Game
{
    Game::Game() : GraphicsDevice(Graphics::GraphicsDevice::Initialize()) {
        GraphicsDevice->UseShader("res/shaders/basic_tex.glsl");
        // https://en.wikipedia.org/wiki/Isometric_projection#Overview for calculations
        // constexpr float ISO_X_ROT = -0.78539816339744830961566084581988f; // simply 45° ( 2π / 8 ), inverted bc camera
        // constexpr float ISO_Y_ROT = -0.61547970867038734106746458912399f; /* since sinθ = 1 (top corner y),
        //                                                                   and cosθ = √2 (length of corner to corner x),
        //                                                                 sinθ / cosθ = tanθ = 1 / √2. θ = atan1/√2.
        //                                                                  */
        // constexpr float ISO_Z_ROT = -1.57079632679489661923132169163980f; // rotate 90°. ( 2π / 4 )

        /*
         * line (-1, -1, -1) ~ (1, 1, 1) will pass through midpoint
         * (0, 0, 1), (0, 1, 0), (1, 0, 0) makes equil-triangle, side length √2
         * dist from mid to point = r, thus r cos 30° = √2 / 2. (triangle 1/6 of original)
         * solve for r = √6 / 3
         * from mid to three axis makes circle, radius = √6 / 3
         * three axis in front of origin after transform, √(r² + h²) = 1, r = √6 / 3
         * solve for h = √3 / 3,
         * then each axis after tr = (r cos θ, r sin θ, h) where θ is mulitple of 120.
        */

        // Maths::Matrix3D mat = { 1, 2, 3, 4, 2, 3, 4, 1, 3, 4, 1, 2, 4, 1, 2, 3 };
        constexpr float PI = 3.14159265358979323846f;
        rot = { -35.26f / 180 * PI, 45.0f / 180 * PI, 0 };
        trans = { 5, 2, 3.5f };

        texAtlas = std::make_unique<Graphics::TextureAtlas>("res/textures/simple_tm.png", 6, 2);
        texAtlas->Bind(0);
        GraphicsDevice->SetUniform1Int("tex", 0);
        GraphicsDevice->SetUniform2Int("texSize", texAtlas->GetAtlasSize());
        
        world.Load("res/levels/level_test.json");
        world.Render(*GraphicsDevice);

        // cube = Graphics::MeshUtils::CubeMesh( {}, 1, 1, 1 );
        // cube.Bind(*GraphicsDevice);
        // LOG(cube.IsBound());
        GraphicsDevice->RegisterElements();
    }
    
    void Game::Update()
    {
        TickRising();
        // Time.Update();
    }

    void Game::Render()
    {
        GraphicsDevice->BeginRender();
        
        GraphicsDevice->ClearRegistered();
        GraphicsDevice->RegisterElements();
        GraphicsDevice->EnableShader();
        GraphicsDevice->RenderRegistered();

        Maths::Matrix3D mat = Maths::Matrix3D::Transform(trans, scale, rot);
        GraphicsDevice->SetCamera(mat.Inverse());
        ImGui::SliderFloat3("Rot", (float*)&rot, -3.14f, 3.14f);
        ImGui::SliderFloat3("Sca", (float*)&scale, 0, 3);
        ImGui::SliderFloat3("Tra", (float*)&trans, -10, 10);
        
        GraphicsDevice->DebugMenu();
        
        GraphicsDevice->EndRender();
    }

    void Game::TickRising() {
        IO::Update();
    }

    Game::~Game()
    {
        // cube.Unbind();
        // cube.Delete();
    }
}
