// because gl.h cant be included before glew.h, weird ordering
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

        dest.renderOptions = from.renderOptions;

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

        RenderInMode(renderOptions.renderMode);

        ioDevice.Update();

        renderOptions.drawCalls = 0;
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
        for (uint i = index; i < renders.size(); ++i)
            renders[i]->deviceIndex = i;
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

    void GraphicsDevice::ClearColor(const Maths::colorf& color) {
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

        ImGui::BeginTabBar("Debug Items");

        if (ImGui::BeginTabItem("Basics")) {
            ImGui::Text("Application Averages %.2fms/frame (%d FPS)", 1000.0 * ioDevice.Time.deltaTime, (int)std::ceil(ioDevice.Time.Framerate()));
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
                const Maths::colorf prevClearColor = renderOptions.clearColor;
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
                    ImGui::Text("   %s", IO::MouseType::MouseButtonToStr(i));
                }
            }
            ImGui::Unindent();

            ImGui::BulletText("Keyboard");
            ImGui::Indent();
            {
                ImGui::Text("Keys Pressed Are:");
                for (const auto key : ioDevice.Keyboard.KeysPressed()) {
                    ImGui::Text("   %s", IO::KeyboardType::KeyToStr(key));
                }
            }
            ImGui::Unindent();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Data")) {
            uint vCount = 0, tCount = 0;
            for (uint i = 0; i < renders.size(); ++i) {
                const RenderHandle& data = renders[i];
                const VertexDebugTypeIndex vType = data->GetType();
                vCount += data->vbo.dataOffset;
                tCount += data->ibo.dataOffset / 3;
                if (ImGui::TreeNode((const void*)(intptr_t)i, "Render #%d", i)) {
                    ImGui::Text("%d Vertices, %d Triangles", data->vbo.dataOffset, data->ibo.dataOffset / 3);
                    ImGui::Text("Vertex Type: %.*s (size %d)", vType->name.size(), vType->name.data(), data->vbo.vertSize);

                    ImGui::Text("Vertex Properties:");
                    ImGui::Indent();
                    using namespace std::string_view_literals;
                    uint j = 0;
                    for (const auto pName : std::views::split(vType->propNames, "\0"sv)) {
                        const VertexBufferComponent& comp = vType->bufferLayout.GetComponents()[j];
                        const char* nameofCompType = GLTypeName(comp.type);
                        ImGui::Text("%s:%s%c%s%c%s",
                            pName.data(),
                            comp.count == 1 ? ""  : " Vector",
                            comp.count == 1 ? ' ' : '0' + comp.count,
                            comp.count == 1 ? ""  : " of ",
                            std::toupper(nameofCompType[0]),
                            nameofCompType + 1);
                        ++j;
                    }
                    ImGui::Unindent();

                    for (uint k = 0; k < data->meshes.size(); ++k) {
                        GenericMesh& mesh = data->meshes[k];
                        if (ImGui::TreeNode((const void*)(intptr_t)k, "Mesh #%d", k)) {
                            ImGui::Text("%d Vertices, %d Triangles", mesh.vSizeBytes() / vType->size, mesh.iSize());

                            ImGui::TreePop();
                        }
                    }

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
