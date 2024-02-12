// because gl.h cant be included before glew.h, weird ordering
#include "Render.h"
#include "GraphicsDevice.h"

#include <algorithm>

#include "IO.h"
#include "Mouse.h"
#include "Keyboard.h"

#include <numeric>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Texture.h"
#include "IO/TimeType.h"

namespace Graphics {
    GraphicsDevice::GraphicsDevice(GLFWwindow* window, Maths::ivec2 winSize) : 
        windowSize(winSize), mainWindow{ window } {
        Instance = *this;
        IO::Init(*this);

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

    void GraphicsDevice::BeginRender() {
        if (IsClosed()) return;
        
        Render::Clear();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawWireframe(useWireRender);

        IO::Update();
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
        render.deviceIndex = renders.size() - 1;
    }

    void GraphicsDevice::DeleteRender(uint index) {
        renders[index]->device = nullptr;
        renders.erase(renders.begin() + (int)index);
    }

    void GraphicsDevice::DeleteAllRenders() {
        std::ranges::for_each(renders, [](RenderHandle& r){ r->device = nullptr; });
        renders.clear();
    }

    RenderData& GraphicsDevice::GetRender(uint index) {
        return *renders[index];
    }

    void GraphicsDevice::Render(RenderData& r) {
        r.EnableShader();
        Render::Draw(r);
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
        
        ImGui::Text("Application Averages %fms/frame (~%f FPS)", 1000.0 * IO::Time.deltaTime, IO::Time.Framerate());
        if (ImGui::Button(useWireRender ? "Draw Fill" : "Draw Wireframe")) { useWireRender = !useWireRender; }

        if (ImGui::CollapsingHeader("Mouse Input")) {
            ImGui::Text("Mouse Position is at: (%f, %f),",
                IO::Mouse.GetMousePosPx().x,
                IO::Mouse.GetMousePosPx().y);
            ImGui::Text("   relative at: (%f, %f),",
                IO::Mouse.GetMousePos().x,
                IO::Mouse.GetMousePos().y);
            ImGui::Text("which is%s in the window", IO::Mouse.IsInWindow() ? "" : " not");
            ImGui::NewLine();
            
            ImGui::Text("Left Mouse Pressed: %s",   IO::Mouse.LeftPressed()   ? "True" : "False");
            ImGui::Text("Right Mouse Pressed: %s",  IO::Mouse.RightPressed()  ? "True" : "False");
            ImGui::Text("Middle Mouse Pressed: %s", IO::Mouse.MiddlePressed() ? "True" : "False");

            if (ImGui::TreeNode("Advanced")) {
                ImGui::Text("Pressed: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!IO::Mouse.ButtonPressed(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }

                ImGui::TextDisabled("// The following below actually works, you just can't see it.");

                ImGui::Text("On Pressed: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!IO::Mouse.ButtonOnPress(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }

                ImGui::Text("On Release: ");
                for (int i = 0; i < IO::MouseType::LAST_MOUSE; ++i) {
                    if (!IO::Mouse.ButtonOnRelease(i)) continue;
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }
                
                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Keyboard Input")) {
            ImGui::Text("Keys Pressed Are:");

            for (const auto key : IO::Keyboard.KeysPressed()) {
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
        /* Initialize the library */
        if (!glfwInit())
            ASSERT(false);

        /* Create a windowed mode window and its OpenGL context */
        GLFWwindow* window = glfwCreateWindow(winSize.x, winSize.y, "Hello World", nullptr, nullptr);
        
        if (!window) {
            glfwTerminate();
            ASSERT(false);
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);

        glfwSwapInterval(1);

        /* SETTING UP GLEW W/ GLEWINIT*/
        if (glewInit() != GLEW_OK) {
            LOG("ERR: ERROR SETTING UP glewInit()");
        }

        LOG(glGetString(GL_VERSION));

        GLCALL(glEnable(GL_BLEND));
        GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GLCALL(glBlendColor(1.0f, 1.0f, 1.0f, 1.0f)); // prevents texture artifacts
        GLCALL(glEnable(GL_DEPTH_TEST));
        glDepthFunc(GL_LEQUAL);

        // IMGUI INIT
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init((const char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
        
        return GraphicsDevice(window, winSize);
    }
}
