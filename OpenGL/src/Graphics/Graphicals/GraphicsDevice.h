#pragma once

#include "Render.h"

#include "IO.h"
#include "Random.h"
#include "Graphics/Utils/Fonts/FontDevice.h"
#include "stdu/ref.h"

namespace Graphics {
    class GraphicsDevice {
        static constexpr unsigned int MAX_VERTEX_COUNT = 1000;
        static constexpr unsigned int MAX_INDEX_COUNT = 1000;

        using RenderHandle = std::unique_ptr<RenderData>;
    private:
        std::vector<RenderHandle> renders;

        Maths::ivec2 windowSize;
        GLFWwindow* mainWindow;

        struct GLRenderOptions {
            RenderMode renderMode = RenderMode::FILL;
            float pointSize = 10.0f;
            Maths::colorf clearColor;
            int drawCalls;
        } renderOptions;

        FontDevice fontDevice = {};
        IO::IO ioDevice {};
        std::unique_ptr<Maths::random_gen> randDevice {};

        inline static stdu::ref<GraphicsDevice> Instance = nullptr;
        inline static bool ShowDebugMenu = false;
    public:
        OPENGL_API explicit GraphicsDevice(GLFWwindow* window, Maths::ivec2 winSize);
        using stringr = const std::string&;

        OPENGL_API void Quit();
        OPENGL_API void Terminate();
        OPENGL_API ~GraphicsDevice();

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;
        OPENGL_API static void Transfer(GraphicsDevice& dest, GraphicsDevice&& from);
        GraphicsDevice(GraphicsDevice&& gd) noexcept { Transfer(*this, std::move(gd)); }
        GraphicsDevice& operator=(GraphicsDevice&& gd) noexcept { Transfer(*this, std::move(gd)); return *this; }

        OPENGL_API void BeginRender();
        OPENGL_API void EndRender();

        template <class T> RenderObject<T> CreateNewRender(uint vsize = MAX_VERTEX_COUNT, uint isize = MAX_INDEX_COUNT);
        OPENGL_API void BindRender(RenderData& render);
        OPENGL_API void DeleteRender(uint index);
        OPENGL_API void DeleteAllRenders();
        OPENGL_API RenderData& GetRender(uint index);

        OPENGL_API void Render(RenderData& r, Shader& s, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
        void Render(RenderData& r, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { Render(r, r.Shader(), args, setDefaultShaderArgs); }
        void Render(uint index, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { Render(GetRender(index), args, setDefaultShaderArgs); }
        OPENGL_API void RenderInstanced(RenderData& r, int instances, Shader& s, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
        void RenderInstanced(RenderData& r, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) {
            RenderInstanced(r, instances, r.Shader(), args, setDefaultShaderArgs);
        }
        void RenderInstanced(uint index, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) {
            RenderInstanced(GetRender(index), instances, args, setDefaultShaderArgs);
        }

        OPENGL_API void ClearColor(const Maths::colorf& color);

        [[nodiscard]] bool IsClosed() const { return !mainWindow; }
        [[nodiscard]] OPENGL_API bool WindowIsOpen() const;
    
        [[nodiscard]] Maths::ivec2 GetWindowSize() const { return windowSize; }
        [[nodiscard]] float GetAspectRatio() const { return (float)windowSize.x / (float)windowSize.y; }
        GLFWwindow* GetWindow() { return mainWindow; }
        [[nodiscard]] const GLFWwindow* GetWindow() const { return mainWindow; }

        FontDevice& GetFontDevice() { return fontDevice; }
        [[nodiscard]] const FontDevice& GetFontDevice() const { return fontDevice; }

        OPENGL_API void SetDrawMode(RenderMode mode);
        OPENGL_API static void RenderInMode(RenderMode mode);

        OPENGL_API void DebugMenu();
    private:
        OPENGL_API void ShowDebugWindow();
    public:

        static GraphicsDevice& GetDeviceInstance() { return *Instance; }
        static GLFWwindow* GetMainWindow() { return Instance->mainWindow; }

        IO::IO& GetIO() { return ioDevice; }
        [[nodiscard]] const IO::IO& GetIO() const { return ioDevice; }
        Maths::random_gen& GetRand() { return *randDevice; }
        [[nodiscard]] const Maths::random_gen& GetRand() const { return *randDevice; }

        OPENGL_API static GraphicsDevice Initialize(Maths::ivec2 winSize = { 640, 480 });

        friend class Texture;
    };

    template <class T>
    RenderObject<T> GraphicsDevice::CreateNewRender(uint vsize, uint isize) {
        renders.push_back(RenderData::CreateHeap<T>(vsize, isize));
        BindRender(*renders.back());
        return *renders.back();
    }
}
