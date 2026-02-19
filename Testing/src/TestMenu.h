#pragma once

#include "Test.h"

namespace Test {
    class TestManager;

    class TestMenu : public Test {
    public:
        struct TestMenuItem {
            Str name, description;
            FuncPtr<Test*> testConstruct;
        };

        struct TestSection {
            TestType::E type;
            zRange span;
            Str description;
        };

    private:
        Ref<TestManager> manager;
        Vec<TestMenuItem> menuItems;
        Vec<TestSection> testTypeSegments;
        TestType::E currentType = (TestType::E)0;
    public:
        TestMenu(Ref<TestManager> manager);

        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;

        template <class T>
        static Test* TestFactory() { return new T(); }

        template <typename T>
        void RegisterTest(Str name) {
            // LOG("Registered " << name << " Test");
            menuItems.Push({ name, "", TestFactory<T> });
        }

        void DeclareTestType(TestType::E type);
        void FinishDeclare();
        void AddSectionDescription(Str desc);
        void AddDescription(Str desc);

        void OnInit(Graphics::GraphicsDevice& gdevice) override {}
        void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) override {}
        void OnRender(Graphics::GraphicsDevice& gdevice) override {}
        void OnDestroy(Graphics::GraphicsDevice& gdevice) override {}
    };
}
