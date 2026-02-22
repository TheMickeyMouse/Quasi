#include <imgui.h>

#include "Utils/Debug/Logger.h"

#include "TestManager.h"

// #include "Advanced/TestFontRender.h"
// #include "Advanced/TestPostProcessing.h"
// // #include "Advanced/TestAdvancedLighting.h"
// #include "Advanced/TestMaterialMaps.h"
// #include "Advanced/TestLightCasters.h"
// #include "Advanced/TestCubeMap.h"
// #include "Advanced/TestDrawInstances.h"
// #include "Advanced/TestGeometryShader.h"
// #include "Advanced/TestShadowMap.h"

// #include "Demos/DemoFlappyBird.h"
#include "Demos/DemoUIPlayground.h"
#include "Demos/DemoGlass.h"

// #include "Physics/TestCircleCollision2D.h"
// #include "Physics/TestPhysicsPlayground2D.h"

namespace Test {
    class DemoGlass;

    TestManager::TestManager() : gdevice(Graphics::GraphicsDevice::Initialize({ 1600, 900 })) {}

    void TestManager::OnInit() {
        menu = Box<TestMenu>::Build(*this);
        currentTest = *menu;
        {
            // menu->DeclareTestType(TestType::ADVANCED);
            // menu->AddSectionDescription("Advanced OpenGL Tests focused on more realistic rendering. \nIncluding: Text Rendering, Post Processing Effects and Lighting");
            //
            // // menu->RegisterTest<TestFontRender>("Font Rendering");
            // // menu->AddDescription("Draws text (not 100% finished yet: markdown).");
            //
            // menu->RegisterTest<TestPostProcessing>("Post Processing");
            // menu->AddDescription("Draws a simple scene with post processing.");
            //
            // menu->RegisterTest<TestAdvancedLighting>("Advanced Lighting");
            // menu->AddDescription("Draws a simple scene with lighting.");
            //
            // menu->RegisterTest<TestMaterialMaps>("Material Map");
            // menu->AddDescription("Draws materials with texture maps.");
            //
            // menu->RegisterTest<TestLightCasters>("Light Casters");
            // menu->AddDescription("Draws a scene with many types of light casting.");
            //
            // menu->RegisterTest<TestCubeMap>("Cube Map");
            // menu->AddDescription("Draws a skybox using a cubemap texture.");
            //
            // menu->RegisterTest<TestGeometryShader>("Geometry Shader");
            // menu->AddDescription("Draws spheres using geometry shaders.");
            //
            // menu->RegisterTest<TestDrawInstances>("Draw Instanced");
            // menu->AddDescription("Draws cubes using instancing.");
            //
            // menu->RegisterTest<TestShadowMap>("Shadow Map");
            // menu->AddDescription("Draws a scene with simple shadows.");
            // // =========================================================================
            //
            // menu->DeclareTestType(TestType::SIM_PHYSICS);
            // menu->AddSectionDescription("Physics Simulations designed to test Physics implementations.");
            //
            // menu->RegisterTest<TestCircleCollision2D>("2D Circle Collisions");
            // menu->AddDescription("Checks for Collisions between circles.");
            //
            // menu->RegisterTest<TestPhysicsPlayground2D>("2D Physics Playground");
            // menu->AddDescription("A physics sandbox in 2D");
            //
            // // =========================================================================
            //
            menu->DeclareTestType(TestType::DEMO);
            menu->AddSectionDescription("Simple OpenGL 'Games' created to explore the limitations of the engine. \nJust to see what's possible without over-development.");
            //
            // menu->RegisterTest<DemoFlappyBird>("Flappy Bird");
            // menu->AddDescription("A dumb-down version of Flappy Bird.");

            menu->RegisterTest<DemoUIPlayground>("UI Playground");
            menu->AddDescription("A Simple UI Renderer.");
            menu->RegisterTest<DemoGlass>("Glass");
            menu->AddDescription("glass lol.");

            // =========================================================================

            menu->DeclareTestType(TestType::OTHER);
            menu->AddSectionDescription("Other Tests that didn't fit in. \nThere's nothing here yet!");

            menu->FinishDeclare();
        }

        {
            const CStr fontFile = Q_USER_FONTS "JetBrainsMono-Regular.ttf";
            if (!Text::ExistsFile(fontFile)) {
                Debug::QWarn$("couldn't load font {}, some characters may not load properly", fontFile);
                return;
            }

            const ImGuiIO& io = ImGui::GetIO();

            ImVector<ImWchar> glyphRange;
            ImFontGlyphRangesBuilder builder;
            builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
            builder.AddText((const char*)u8"°αβγνξΔδεζπΠρΘΛθιητυκλμΨΣςσΦφΩψω"); // greek letters
            builder.BuildRanges(&glyphRange);

            io.Fonts->AddFontFromFileTTF(fontFile.Data(), 21, nullptr, glyphRange.Data);
            io.Fonts->Build();
        }
    }

    void TestManager::OnUpdate(float deltaTime) {
        deltaTime = std::isnan(deltaTime) ? gdevice.GetIO().DeltaTime() : deltaTime;
        currentTest->OnUpdate(gdevice, deltaTime);
    }

    void TestManager::OnRender() {
        currentTest->OnRender(gdevice);
    }

    void TestManager::OnImGuiRender() {
        ImGui::Begin("Test");
        if (currentTest.RefEquals(*testInstance) && ImGui::Button("<< Back")) {
            currentTest->OnDestroy(gdevice);
            testInstance.Close();
            currentTest = *menu;
            currentTest->OnInit(gdevice);
        }

        currentTest->OnImGuiRender(gdevice);

        ImGui::Separator();
        gdevice.DebugMenu();

        ImGui::End();
    }

    void TestManager::OnDestroy() {}

    void TestManager::OnRun() {
        gdevice.Begin();
        OnUpdate();
        OnRender();
        OnImGuiRender();
        gdevice.End();
    }
}
