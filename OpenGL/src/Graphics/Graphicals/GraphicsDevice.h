#pragma once

#include "Render.h"

#include <GLFW/glfw3.h>

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
        bool useWireRender = false;
        
        FontDevice fontDevice = {};
        IO::IO ioDevice {};
        std::unique_ptr<Maths::random_gen> randDevice {};

        inline static stdu::ref<GraphicsDevice> Instance = nullptr;
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
        OPENGL_API void Render(RenderData& r);
        void Render(uint index) { Render(GetRender(index)); }

        OPENGL_API void ClearColor(const Maths::colorf& color);

        [[nodiscard]] bool IsClosed() const { return !mainWindow; }
        OPENGL_API [[nodiscard]] bool WindowIsOpen() const;
    
        [[nodiscard]] Maths::ivec2 GetWindowSize() const { return windowSize; }
        GLFWwindow* GetWindow() { return mainWindow; }
        [[nodiscard]] const GLFWwindow* GetWindow() const { return mainWindow; }

        FontDevice& GetFontDevice() { return fontDevice; }
        [[nodiscard]] const FontDevice& GetFontDevice() const { return fontDevice; }

        OPENGL_API void SetWireframe(bool usewire);
        OPENGL_API static void DrawWireframe(bool usewire);

        OPENGL_API void DebugMenu();

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
