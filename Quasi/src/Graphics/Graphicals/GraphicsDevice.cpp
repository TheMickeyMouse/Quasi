// because gl.h cant be included before glew.h, weird ordering
#include <glp.h>
#include <GLFW/glfw3.h>

#include "GraphicsDevice.h"

#include <algorithm>
#include <ranges>

#include "IO.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../GLs/Texture.h"
#include "GLs/GLDebug.h"

namespace Quasi::Graphics {
    class RenderData;

    GraphicsDevice::GraphicsDevice(GLFWwindow* window, Math::iVector2 winSize) :
        windowSize(winSize), mainWindow{ window }, ioDevice(*this) {
        Instance = *this;
        Texture::Init();
    }

    void GraphicsDevice::Quit() {
        DeleteAllRenders(); // delete gl objects
        glfwSetWindowShouldClose(mainWindow, true);
    }

    void GraphicsDevice::Terminate() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(mainWindow);
        glfwTerminate();

        mainWindow = nullptr; // mark as terminated
    }

    GraphicsDevice::~GraphicsDevice() {
        if (IsClosed()) return;
        Quit();
        Terminate();
    }

    void GraphicsDevice::Transfer(GraphicsDevice& dest, GraphicsDevice&& from) {
        dest.renders = std::move(from.renders);
        for (RenderHandle& h : dest.renders)
            h->device = dest;

        dest.windowSize = from.windowSize;

        dest.mainWindow = from.mainWindow;
        from.mainWindow = nullptr;

        dest.renderOptions = from.renderOptions;

        dest.fontDevice = std::move(from.fontDevice);
        dest.ioDevice = std::move(from.ioDevice);
        dest.randDevice = from.randDevice;

        Instance = dest;
    }

    void GraphicsDevice::Begin() {
        if (IsClosed()) return;

        frameBeginTime = Debug::Timer::Now();
        GLDebugContainer::GpuProcessDuration = Debug::Timer::Instant();

        Render::Clear();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderInMode(renderOptions.renderMode);

        ioDevice.Update();

        renderOptions.drawCalls = 0;
    }

    void GraphicsDevice::End() {
        if (IsClosed()) return;

        const auto end = Debug::Timer::Now();
        frameDurationTime = end - frameBeginTime;
        frameBeginTime = end;

        ImGui::Render();
        
        glfwPollEvents();
            
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
        glfwSwapBuffers(mainWindow);
    }
    
    void GraphicsDevice::BindRender(RenderData& render) {
        render.device = *this;
        render.deviceIndex = (u32)(renders.Length() - 1);
    }

    void GraphicsDevice::DeleteRender(u32 index) {
        renders[index]->device = nullptr;
        renders.Pop(index);
        for (u32 i = index; i < renders.Length(); ++i)
            renders[i]->deviceIndex = i;
    }

    void GraphicsDevice::DeleteAllRenders() {
        for (auto& r : renders)
            r->device = nullptr;
        renders.Clear();
    }

    RenderData& GraphicsDevice::GetRender(u32 index) {
        return *renders[index];
    }

    void GraphicsDevice::Render(RenderData& r, Shader& s, const ShaderArgs& args, bool setDefaultShaderArgs) {
        s.Bind();
        s.SetUniformArgs(args);
        if (setDefaultShaderArgs) {
            s.SetUniformMat4x4("u_projection", r.projection);
            s.SetUniformMat4x4("u_view", r.camera);
        }
        Render::Draw(r, s);
        ++renderOptions.drawCalls;
    }

    void GraphicsDevice::RenderInstanced(RenderData& r, int instances, Shader& s, const ShaderArgs& args, bool setDefaultShaderArgs) {
        s.Bind();
        s.SetUniformArgs(args);
        if (setDefaultShaderArgs) {
            s.SetUniformMat4x4("u_projection", r.projection);
            s.SetUniformMat4x4("u_view", r.camera);
        }
        Render::DrawInstanced(r, s, instances);
        ++renderOptions.drawCalls;
    }

    void GraphicsDevice::ClearColor(const Math::fColor& color) {
        Render::SetClearColor(color);
    }

    bool GraphicsDevice::WindowIsOpen() const {
        return mainWindow && !glfwWindowShouldClose(mainWindow);
    }

    void GraphicsDevice::SetDrawMode(const RenderMode mode) {
        renderOptions.renderMode = mode;
    }

    void GraphicsDevice::RenderInMode(const RenderMode mode) {
        Render::SetRenderMode(mode);
    }

    void GraphicsDevice::DebugMenu() {
        if (ImGui::Button(ShowDebugMenu ? "Hide Debug Menu" : "Show Debug Menu")) {
            ShowDebugMenu = !ShowDebugMenu;
        }

        if (ShowDebugMenu) ShowDebugWindow();

        if (ImGui::Button("Quit Application"))
            Quit();
    }

    void GraphicsDevice::ShowDebugWindow() {
        ImGui::Begin("Debug Menu", &ShowDebugMenu);
        const u32 totalUs = Debug::Timer::UnitConvert<Debug::Microsecond>(frameDurationTime),
                  gpuUs = Debug::Timer::UnitConvert<Debug::Microsecond>(GLDebugContainer::GpuProcessDuration),
                  cpuUs = totalUs - gpuUs;
        ImGui::Text("CPU   Time: %d.%03dms", cpuUs   / 1000, cpuUs   % 1000);
        ImGui::Text("GPU   Time: %d.%03dms", gpuUs   / 1000, gpuUs   % 1000);
        ImGui::Text("Application Averages %.2fms/frame (%.1f FPS)", 1000.0 * ioDevice.Time.DeltaTime(), ioDevice.Time.Framerate());
        ImGui::Text("       Theoretically %.2fms/frame (%.1f FPS)", (float)totalUs / 1000.0f, 1'000'000.0f / (float)totalUs);

        ImGui::BeginTabBar("Debug Items");

        if (ImGui::BeginTabItem("Basics")) {
            ImGui::Text("Draw as: "); ImGui::SameLine();
            ImGui::RadioButton("Fill",   (int*)&renderOptions.renderMode, (int)RenderMode::FILL);  ImGui::SameLine();
            ImGui::RadioButton("Lines",  (int*)&renderOptions.renderMode, (int)RenderMode::LINES); ImGui::SameLine();
            ImGui::RadioButton("Points", (int*)&renderOptions.renderMode, (int)RenderMode::POINTS);

            if (renderOptions.renderMode == RenderMode::POINTS) {
                const float ps = renderOptions.pointSize;
                ImGui::DragFloat("Point Size", &renderOptions.pointSize, 0.1f, 0);
                if (ps != renderOptions.pointSize) Render::SetPointSize(renderOptions.pointSize);
            }

            {
                const Math::fColor prevClearColor = renderOptions.clearColor;
                ImGui::ColorEdit4("Clear Color", renderOptions.clearColor.begin());
                if (renderOptions.clearColor != prevClearColor)
                    Render::SetClearColor(renderOptions.clearColor);
            }

            static bool showImGuiDebugMenu = false;
            ImGui::Checkbox("Show ImGui Debug Menu", &showImGuiDebugMenu);

            if (showImGuiDebugMenu) ImGui::ShowDemoWindow();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Inputs")) {
            ImGui::BulletText("Mouse");
            ImGui::Indent();
            {
                ImGui::Text("Mouse Position is at: (%f, %f),",
                    ioDevice.Mouse.GetMousePosPx().x,
                    ioDevice.Mouse.GetMousePosPx().y);
                ImGui::Text("   relative at: (%f, %f),",
                    ioDevice.Mouse.GetMousePos().x,
                    ioDevice.Mouse.GetMousePos().y);
                ImGui::Text("which is%s in the window", ioDevice.Mouse.IsInWindow() ? "" : " not");
                ImGui::NewLine();

                ImGui::Text("Left Mouse Pressed: %s",   ioDevice.Mouse.LeftPressed()   ? "True" : "False");
                ImGui::Text("Right Mouse Pressed: %s",  ioDevice.Mouse.RightPressed()  ? "True" : "False");
                ImGui::Text("Middle Mouse Pressed: %s", ioDevice.Mouse.MiddlePressed() ? "True" : "False");

                ImGui::Text("Mouse Scroll is: (%f, %f),",
                    ioDevice.Mouse.GetMouseScroll().x,
                    ioDevice.Mouse.GetMouseScroll().y);
                ImGui::Text("       delta is: (%f, %f)",
                    ioDevice.Mouse.GetMouseScrollDelta().x,
                    ioDevice.Mouse.GetMouseScrollDelta().y);

                ImGui::Text("Pressed: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!ioDevice.Mouse.ButtonPressed(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i).Data());
                }
            }
            ImGui::Unindent();

            ImGui::BulletText("Keyboard");
            ImGui::Indent();
            {
                ImGui::Text("Keys Pressed Are:");
                ioDevice.Keyboard.VisitKeysPressed([] (IO::Key k) {
                    ImGui::Text("   %s", IO::KeyboardType::KeyToStr(k).Data());
                });
            }
            ImGui::Unindent();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Data")) {
            u32 vCount = 0, tCount = 0;
            for (u32 i = 0; i < renders.Length(); ++i) {
                const RenderHandle& data = renders[i];
                vCount += data->vbo.dataOffset;
                tCount += data->ibo.dataOffset / 3;
                if (ImGui::TreeNode((const void*)(intptr_t)i, "Render #%d", i)) {
                    ImGui::Text("%d Vertices, %d Triangles", data->vbo.dataOffset, data->ibo.dataOffset / 3);
                    ImGui::Unindent();

                    ImGui::TreePop();
                }
            }
            ImGui::Text("Total: %d Vertices, %d Triangles", vCount, tCount);
            ImGui::Text("Draw Calls: %d", renderOptions.drawCalls);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();

        ImGui::End();
    }

    GraphicsDevice GraphicsDevice::Initialize(Math::iVector2 winSize) {
        InitGLDebugTools();
        /* Initialize the library */
        if (!glfwInit()) {
            GLLogger().QError$("GLFW failed to initialize");
        }

        /* Create a windowed mode window and its OpenGL context */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        GLFWwindow* window = glfwCreateWindow(winSize.x, winSize.y, "Hello World", nullptr, nullptr);

        if (!window) {
            glfwTerminate();
            GLLogger().QError$("Failed to create window");
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);

        glfwSwapInterval(1);

        /* SETTING UP GLEW W/ GLEWINIT*/
        if (GL::InitGLEW() != 0) {
            GLLogger().QError$("GLEW failed to initialize");
        }

        GLLogger().QInfo$("{}", (const char*)GL::GetString(GL::VERSION));

        Render::EnableBlend();
        Render::UseBlendFunc(BlendFactor::SRC_ALPHA, BlendFactor::INVERT_SRC_ALPHA);
        Render::UseBlendConstColor(1);

        Render::EnableDepth();
        Render::UseDepthFunc(CmpOperation::LEQUAL);

        // IMGUI INIT
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        const ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init((const char*)GL::GetString(GL::NUM_SHADING_LANGUAGE_VERSIONS));
        
        return GraphicsDevice(window, winSize);
    }
}
