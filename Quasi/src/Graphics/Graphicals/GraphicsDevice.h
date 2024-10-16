#pragma once

#include "Render.h"

#include "IO.h"
#include "Math/Random.h"
#include "Fonts/FontDevice.h"

namespace Quasi::Graphics {
    class GraphicsDevice {
        static constexpr u32 MAX_VERTEX_COUNT = 1024;
        static constexpr u32 MAX_INDEX_COUNT = 1024;

        using RenderHandle = UniqueRef<RenderData>;
    private:
        Vec<RenderHandle> renders;

        Math::iVector2 windowSize;
        GLFWwindow* mainWindow;

        struct GLRenderOptions {
            RenderMode renderMode = RenderMode::FILL;
            float pointSize = 10.0f;
            Math::fColor clearColor;
            u32 drawCalls;
        } renderOptions;

        FontDevice fontDevice = {};
        IO::IO ioDevice {};
        Math::RandomGenerator randDevice {};

        Debug::DateTime frameBeginTime;
        Debug::TimeDuration frameDurationTime;

        inline static Ref<GraphicsDevice> Instance = nullptr;
        inline static bool ShowDebugMenu = false;
    public:
        explicit GraphicsDevice(GLFWwindow* window, Math::iVector2 winSize);

        void Quit();
        void Terminate();
        ~GraphicsDevice();

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;
        static void Transfer(GraphicsDevice& dest, GraphicsDevice&& from);
        GraphicsDevice(GraphicsDevice&& gd) noexcept { Transfer(*this, std::move(gd)); }
        GraphicsDevice& operator=(GraphicsDevice&& gd) noexcept { Transfer(*this, std::move(gd)); return *this; }

        void Begin();
        void End();

        template <class T> RenderObject<T> CreateNewRender(u32 vsize = MAX_VERTEX_COUNT, u32 isize = MAX_INDEX_COUNT);
        void BindRender(RenderData& render);
        void DeleteRender(u32 index);
        void DeleteAllRenders();
        RenderData& GetRender(u32 index);

        void Render(RenderData& r, Shader& s, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
        void Render(RenderData& r, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { Render(r, r.shader, args, setDefaultShaderArgs); }
        void Render(u32 index, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { Render(GetRender(index), args, setDefaultShaderArgs); }
        void RenderInstanced(RenderData& r, int instances, Shader& s, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
        void RenderInstanced(RenderData& r, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) {
            RenderInstanced(r, instances, r.shader, args, setDefaultShaderArgs);
        }
        void RenderInstanced(u32 index, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) {
            RenderInstanced(GetRender(index), instances, args, setDefaultShaderArgs);
        }

        void ClearColor(const Math::fColor& color);

        [[nodiscard]] bool IsClosed() const { return !mainWindow; }
        [[nodiscard]] bool WindowIsOpen() const;
    
        [[nodiscard]] Math::iVector2 GetWindowSize() const { return windowSize; }
        [[nodiscard]] float GetAspectRatio() const { return (float)windowSize.x / (float)windowSize.y; }
        GLFWwindow* GetWindow() { return mainWindow; }
        [[nodiscard]] const GLFWwindow* GetWindow() const { return mainWindow; }

        FontDevice& GetFontDevice() { return fontDevice; }
        [[nodiscard]] const FontDevice& GetFontDevice() const { return fontDevice; }

        void SetDrawMode(RenderMode mode);
        static void RenderInMode(RenderMode mode);

        void DebugMenu();
    private:
        void ShowDebugWindow();
    public:

        static GraphicsDevice& GetDeviceInstance() { return *Instance; }
        static GLFWwindow* GetMainWindow() { return Instance->mainWindow; }

        IO::IO& GetIO() { return ioDevice; }
        [[nodiscard]] const IO::IO& GetIO() const { return ioDevice; }
        Math::RandomGenerator& GetRand() { return randDevice; }
        [[nodiscard]] const Math::RandomGenerator& GetRand() const { return randDevice; }

        static GraphicsDevice Initialize(Math::iVector2 winSize = { 640, 480 });

        friend class Texture;
    };

    template <class T>
    RenderObject<T> GraphicsDevice::CreateNewRender(u32 vsize, u32 isize) {
        renders.push_back(NewUnique<RenderData>());
        RenderData::New<T>(vsize, isize, *renders.back());
        BindRender(*renders.back());
        return *renders.back();
    }
}
