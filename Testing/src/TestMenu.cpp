#include "imgui.h"
#include "TestMenu.h"

#include "TestManager.h"

namespace Test {
    TestMenu::TestMenu(Ref<TestManager> manager) : manager(manager) {
        testTypeSegments.Resize(TestType::NUM_TYPES, { TestType::OTHER, {}, "" });
    }

    void TestMenu::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        if (!ImGui::BeginTabBar("Testing Projects")) return;
        for (const auto& [type, span, desc] : testTypeSegments) {
            if (ImGui::BeginTabItem(TestType::Name(type))) {
                ImGui::Text("%s", desc.Data());
                for (usize j = span.min; j < span.max; ++j) {
                    const TestMenuItem& testItem = menuItems[j];
                    if (ImGui::Button(testItem.name.Data())) {
                        // LOG("clicked " << testItem.name);
                        manager->testInstance.Replace(testItem.testConstruct());
                        manager->currentTest = *manager->testInstance;
                        manager->currentTest->OnInit(gdevice);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", testItem.description.Data());
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    void TestMenu::DeclareTestType(TestType::E type) {
        if (currentType)
            testTypeSegments[currentType].span.max = menuItems.Length();
        TestSection& section = testTypeSegments[type];
        section.type = type;
        section.span.min = menuItems.Length();
        currentType = type;
    }

    void TestMenu::FinishDeclare() {
        testTypeSegments[currentType].span.max = menuItems.Length();
    }

    void TestMenu::AddSectionDescription(const Str desc) {
        testTypeSegments[currentType].description = desc;
    }

    void TestMenu::AddDescription(const Str desc) {
        menuItems.Last().description = desc;
    }
}
