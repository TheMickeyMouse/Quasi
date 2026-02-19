#pragma once

#include "Graphics/GraphicsDevice.h"

#define DEFINE_TEST_T(T, S) \
    using _Test_t = T; \
    inline static const TestType::E CATEGORY = TestType::S; \
    inline static const String RES_DIR = Text::Format("{}res\\{}\\" #T "\\", PROJECT_DIRECTORY, TestType::Name(CATEGORY)); \

#define RES_STR(RES_NAME) (RES_DIR + RES_NAME)
#define RES(RES_NAME) RES_STR(RES_NAME).IntoCStr()

namespace Test {
    using namespace Quasi;

    constexpr Str PROJECT_DIRECTORY = Str::Slice(__FILE__, sizeof(__FILE__) - sizeof("src\\Test.h"));

    struct TestType {
        enum E {
            NONE = 0,
            BASIC,
            ADVANCED,
            SIM_PHYSICS,
            DEMO,
            OTHER,
            NUM_TYPES
        };
        static const char* Name(E type) {
            switch (type) {
                case BASIC:       return "Basic";
                case ADVANCED:    return "Advanced";
                case SIM_PHYSICS: return "Physics";
                case DEMO:        return "Demos";
                case OTHER:       return "Other";
                default:          return "Unknown";
            }
        }
    };

    class Test {
    public:
        virtual ~Test() = default;
        virtual void OnInit(Graphics::GraphicsDevice& gdevice) = 0;
        virtual void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) = 0;
        virtual void OnRender(Graphics::GraphicsDevice& gdevice) = 0;
        virtual void OnImGuiRender(Graphics::GraphicsDevice& gdevice) = 0;
        virtual void OnDestroy(Graphics::GraphicsDevice& gdevice) = 0;
    };
}
