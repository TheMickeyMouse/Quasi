#pragma once

#include "imgui.h"
#include "Test.h"
#include "TestClearColor.h"
#include "TestTexturedSquare.h"
#include "TestBatchedColored.h"
#include "TestBatchedTextured.h"
#include "TestDynamicVertexGeometry.h"
#include "TestDynamicQuadGeometry.h"
#include "TestDynamicTris.h"
#include "TestCubeRender.h"
#include "TestFontRender.h"

#include "DemoFlappyBird.h"
#include "TestPostProcessing.h"

namespace Test {
    class TestManager : public Test {
    private:
        Test* currentTest = nullptr;
        std::unique_ptr<TestMenu> menu = std::make_unique<TestMenu>(currentTest);
    public:
        TestManager();
        ~TestManager() override;
        
        void OnUpdate(float deltaTime = -1) override;
        void OnRender(Graphics::GraphicsDevice& gdevice) override;
        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
    };

    inline TestManager::TestManager() {
        currentTest = menu.get();
        {
            menu->RegisterTest<TestClearColor>           ("Clear Color Test        ");
            menu->RegisterTest<TestTexturedSquare>       ("Texture Square Test     ");
            menu->RegisterTest<TestBatchedColored>       ("Colored Batching Test   ");
            menu->RegisterTest<TestBatchedTextured>      ("Texture Batching Test   ");
            menu->RegisterTest<TestDynamicVertexGeometry>("Dyn Vertex Geometry Test");
            menu->RegisterTest<TestDynamicQuadGeometry>  ("Dyn Quad Geometry Test  ");
            menu->RegisterTest<TestDynamicTris>          ("Dyn Tris Test           ");
            menu->RegisterTest<TestCubeRender>           ("Cube 3D Rendering Test  ");
            menu->RegisterTest<TestFontRender>           ("Font Rendering Test     ");
            menu->RegisterTest<TestPostProcessing>       ("Post Processing Test    ");
            menu->RegisterTest<DemoFlappyBird>           ("Flappy Bird Demo        ");
        }
    }

    inline TestManager::~TestManager() {
        if (currentTest != menu.get())
            delete currentTest;
    }

    inline void TestManager::OnUpdate(float deltaTime) {
        Test::OnUpdate(deltaTime);
        // TODO: make it when dt = -1, auto calc dt
        if (currentTest)
            currentTest->OnUpdate(deltaTime);
    }
    
    inline void TestManager::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);
        if (currentTest)
            currentTest->OnRender(gdevice);
    }

    inline void TestManager::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);
        if (!currentTest) return;
        
        ImGui::Begin("Test");
        if (currentTest != menu.get() && ImGui::Button("<- Back")) {
            currentTest->OnDestroy(gdevice);
            delete currentTest;
            currentTest = menu.get();
            currentTest->OnInit(gdevice);
        }
            
        currentTest->OnImGuiRender(gdevice);
            
        ImGui::Separator();
        gdevice.DebugMenu();

        ImGui::End();
    }
}
