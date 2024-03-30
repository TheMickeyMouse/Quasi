// because gl.h cant be included before glew.h, weird ordering
#include <GL/glew.h>

#include "Render.h"
#include "GraphicsDevice.h"

#include <algorithm>

#include "IO.h"
#include "Mouse.h"
#include "Keyboard.h"

#include "GLDebug.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Texture.h"

namespace Graphics {
    GraphicsDevice::GraphicsDevice(GLFWwindow* window, Maths::ivec2 winSize) : 
        windowSize(winSize), mainWindow{ window }, ioDevice(*this),
        randDevice(std::make_unique<Maths::random_gen>()) {

        Instance = *this;
        Texture::Init();
    }

    void GraphicsDevice::Quit() {
        DeleteAllRenders(); // delete gl objects
        // UnbindAllTextures(); // delete textures
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
        for (const RenderHandle& h : dest.renders)
            h->device = &dest;

        dest.windowSize = from.windowSize;

        dest.mainWindow = from.mainWindow;
        from.mainWindow = nullptr;

        dest.useWireRender = from.useWireRender;

        dest.fontDevice = std::move(from.fontDevice);
        dest.ioDevice = std::move(from.ioDevice);
        dest.randDevice = std::move(from.randDevice);

        Instance = dest;
    }

    void GraphicsDevice::BeginRender() {
        if (IsClosed()) return;
        
        Render::Clear();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawWireframe(useWireRender);

        ioDevice.Update();
    }

    void GraphicsDevice::EndRender() {
        if (IsClosed()) return;
        
        ImGui::Render();
        
        glfwPollEvents();
            
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
        glfwSwapBuffers(mainWindow);
    }
    
    void GraphicsDevice::BindRender(RenderData& render) {
        render.device = this;
        render.deviceIndex = (uint)(renders.size() - 1);
    }

    void GraphicsDevice::DeleteRender(uint index) {
        renders[index]->device = nullptr;
        renders.erase(renders.begin() + (int)index);
    }

    void GraphicsDevice::DeleteAllRenders() {
        std::ranges::for_each(renders, [](const RenderHandle& r){ r->device = nullptr; });
        renders.clear();
    }

    RenderData& GraphicsDevice::GetRender(uint index) {
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
    }

    // void GraphicsDevice::BindTexture(Texture& texture, int slot) {
    //     if (slot == -1) {
    //         const auto free = std::ranges::find_if(textures, [](Texture* t) { return t == nullptr; });
    //         slot = free - textures.begin();
    //     }
    //     texture.Register(slot, this);
    //     textures[slot] = &texture;
    // }

    // void GraphicsDevice::UnbindTexture(int slot) {
    //     Texture* t = textures[slot];
    //     textures[slot] = nullptr;
    //     if (t)
    //         t->Remove();
    // }
    //
    // void GraphicsDevice::UnbindAllTextures() {
    //     for (Texture*& t : textures) {
    //         if (!t) continue;
    //         t->Destroy();
    //         t = nullptr;
    //     }
    // }

    void GraphicsDevice::ClearColor(const Maths::colorf& color) {
        Render::SetClearColor(color);
    }

    bool GraphicsDevice::WindowIsOpen() const {
        return mainWindow && !glfwWindowShouldClose(mainWindow);
    }

    void GraphicsDevice::SetWireframe(bool usewire) {
        useWireRender = usewire;
    }

    void GraphicsDevice::DrawWireframe(bool usewire) {
        Render::SetRenderWireframe(usewire);
    }

    void GraphicsDevice::DebugMenu() {
        static bool enabled = false;
        if (ImGui::Button(enabled ? "Hide Debug Menu" : "Show Debug Menu")) enabled = !enabled;

        if (!enabled) goto skipDebugs; // i know goto is not great but its more readable imo  // NOLINT(cppcoreguidelines-avoid-goto, hicpp-avoid-goto)
        
        ImGui::Text("Application Averages %.2fms/frame (%d FPS)", 1000.0 * ioDevice.Time.deltaTime, (int)ioDevice.Time.Framerate());
        if (ImGui::Button(useWireRender ? "Draw Fill" : "Draw Wireframe")) { useWireRender = !useWireRender; }

        if (ImGui::CollapsingHeader("Mouse Input")) {
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

            if (ImGui::TreeNode("Advanced")) {
                ImGui::Text("Pressed: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!ioDevice.Mouse.ButtonPressed(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }

                ImGui::TextDisabled("// The following below actually works, you just can't see it.");

                ImGui::Text("On Pressed: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!ioDevice.Mouse.ButtonOnPress(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }

                ImGui::Text("On Release: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!ioDevice.Mouse.ButtonOnRelease(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }
                
                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Keyboard Input")) {
            ImGui::Text("Keys Pressed Are:");

            for (const auto key : ioDevice.Keyboard.KeysPressed()) {
                ImGui::Text("   %s", IO::KeyboardType::KeyToStr(key));
            }
        }

        if (ImGui::CollapsingHeader("ImGui Demo")) {
            ImGui::ShowDemoWindow();
        }
        // ImGui::ShowDemoWindow();

        skipDebugs:
        if (ImGui::Button("Quit Application"))
            Quit();
    }

    GraphicsDevice GraphicsDevice::Initialize(Maths::ivec2 winSize) {
        InitGLLog();
        /* Initialize the library */
        if (!glfwInit()) {
            GLError("GLFW failed to initialize");
        }

        /* Create a windowed mode window and its OpenGL context */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        GLFWwindow* window = glfwCreateWindow(winSize.x, winSize.y, "Hello World", nullptr, nullptr);

        if (!window) {
            glfwTerminate();
            GLError("Failed to create window");
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);

        glfwSwapInterval(1);

        /* SETTING UP GLEW W/ GLEWINIT*/
        if (glewInit() != GLEW_OK) {
            GLError("GLEW failed to initialize");
        }

        GLInfo((const char*)glGetString(GL_VERSION));

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
        ImGui_ImplOpenGL3_Init((const char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
        
        return GraphicsDevice(window, winSize);
    }
}
