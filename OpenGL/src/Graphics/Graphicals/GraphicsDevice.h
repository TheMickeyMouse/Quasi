#pragma once

#include "Render.h"

#include <GLFW/glfw3.h>

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

        inline static stdu::ref<GraphicsDevice> Instance = nullptr;
    public:
        OPENGL_API explicit GraphicsDevice(GLFWwindow* window, Maths::ivec2 winSize);
        using stringr = const std::string&;

        OPENGL_API void Quit();
        OPENGL_API void Terminate();
        OPENGL_API ~GraphicsDevice();
        
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
