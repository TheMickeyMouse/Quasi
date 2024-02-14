#include "TestPostProcessing.h"

#include "Constants.h"
#include "imgui.h"
#include "Mesh.h"
#include "Quad.h"

#define POST_VERTEX_SHADER \
( \
    layout (location = 0) in vec2 aPos; \
    layout (location = 1) in vec2 aTexCoords; \
    \
    out vec2 TexCoords; \
    \
    void main() { \
        TexCoords = aTexCoords; \
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); \
    } \
)

namespace Test {
    void TestPostProcessing::OnInit(Graphics::GraphicsDevice& gdevice) {
        Test::OnInit(gdevice);

        scene = gdevice.CreateNewRender<VertexColor3D>(72, 108);
        postProcessingQuad = gdevice.CreateNewRender<VertexTexture2D>(4, 2);

        cubes.reserve(9);

        constexpr float s = 0.5f;
        cubes.push_back(Graphics::MeshUtils::CubeMesh(0, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_GRAY()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1,  1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_WHITE()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1,  1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_YELLOW()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1, -1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_PURPLE()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({  1, -1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_RED()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1,  1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_CYAN()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1,  1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_LIME()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1, -1,  1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_BLUE()));
        cubes.push_back(Graphics::MeshUtils::CubeMesh({ -1, -1, -1 }, s, s, s)
            .ApplyMaterial(&VertexColor3D::Color, Maths::colorf::BETTER_BLACK()));

        scene.BindMeshes(cubes);
        scene.UseShader(Graphics::Shader::StdColored);
        scene.SetProjection(projection);

        const auto [winX, winY] = gdevice.GetWindowSize();
        fbo = Graphics::FrameBuffer {{}};
        fbo.Bind();

        renderResult = Graphics::Texture {
            nullptr, (uint)winX, (uint)winY,
            true, Graphics::TextureFormat::RGB, (Graphics::TextureInternalFormat)Graphics::TextureFormat::RGB
        };

        fbo.Attach(renderResult);

        depthStencilAttachment = Graphics::RenderBuffer {
            Graphics::TextureInternalFormat::DEPTH_24_STENCIL_8, gdevice.GetWindowSize()
        };

        fbo.Bind();
        fbo.Attach(depthStencilAttachment, Graphics::AttachmentType::DEPTH_STENCIL);
        fbo.Complete();
        fbo.Unbind();

        screenQuad = Graphics::Primitives::Quad { 0, Maths::fvec3::RIGHT(), Maths::fvec3::UP() }
                    .IntoMesh<VertexTexture2D>(
                        [](Maths::fvec3 v) -> Maths::fvec2 { return v.xy(); }, &VertexTexture2D::Position)
                    .Convert<VertexTexture2D>([](const VertexTexture2D& v) {
                        return VertexTexture2D { v.Position, (v.Position + 1) * 0.5f };
                     });
        postProcessingQuad.BindMeshes(screenQuad);

        postProcessingQuad.UseShader(
            GLSL_SHADER(
                330,
                POST_VERTEX_SHADER,
                (
                    layout (location = 0) out vec4 FragColor;

                    in vec2 TexCoords;

                    uniform sampler2D screenTexture;

                    void main() {
                        FragColor = texture(screenTexture, TexCoords);
                    }
                )
            )
        );
        // inv shader
        {
            shaderInv = Graphics::Shader { GLSL_SHADER(
                330,
                POST_VERTEX_SHADER,
                (
                    layout (location = 0) out vec4 FragColor;

                    in vec2 TexCoords;

                    uniform sampler2D screenTexture;

                    void main() {
                        FragColor = vec4(1.0 - texture(screenTexture, TexCoords).rgb, 1.0);
                    }
                )
            )};
        }
        // hsl shader
        {
            shaderHsv = Graphics::Shader { GLSL_SHADER(
                330,
                POST_VERTEX_SHADER,
                (
                    layout (location = 0) out vec4 FragColor;

                    in vec2 TexCoords;

                    uniform sampler2D screenTexture;
                    uniform float dh, ds, dv;

                    vec3 rgb2hsv(vec3 c) {
                        vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
                        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
                        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

                        float d = q.x - min(q.w, q.y);
                        float e = 1.0e-10;
                        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
                    }

                    vec3 hsv2rgb(vec3 c) {
                        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
                        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
                    }

                    void main() {
                        vec3 hsv = rgb2hsv(texture(screenTexture, TexCoords).rgb);
                        hsv = vec3(fract(hsv.x + dh), clamp(hsv.y * ds, 0, 1), clamp(hsv.z + dv, 0, 1));
                        FragColor = vec4(hsv2rgb(hsv), 1.0);
                    }
                )
            )};
        }
        // blur shader
        {
            shaderBlur = Graphics::Shader { GLSL_SHADER(
                330,
                POST_VERTEX_SHADER,
                (
                    layout (location = 0) out vec4 FragColor;

                    in vec2 TexCoords;

                    uniform sampler2D screenTexture;
                    uniform vec2 blurOff;

                    void main() {
                        vec2 off = blurOff / vec2(textureSize(screenTexture, 0));
                        vec3 total = vec3(0.0);
                        for (int i = -4; i < 5; ++i) {
                            float multiplier = float(1 << ((i & 1) + 2 * int(i == 0))) / 16.0;
                            vec2 offset = off * vec2(float((i + 4) % 3) - 1.0, float((i + 4) / 3) - 1.0);
                            total += multiplier * texture(screenTexture, TexCoords + offset).rgb;
                        }
                        FragColor = vec4(total, 1.0);
                    }
                )
            )};
        }
        // edge detect shader
        {
            shaderEdgeDetect = Graphics::Shader { GLSL_SHADER(
                330,
                POST_VERTEX_SHADER,
                (
                    layout (location = 0) out vec4 FragColor;

                    in vec2 TexCoords;

                    uniform sampler2D screenTexture;
                    uniform vec2 detectOff;

                    void main() {
                        vec2 off = detectOff / vec2(textureSize(screenTexture, 0));
                        vec3 total = vec3(0.0);
                        for (int i = 0; i < 9; ++i) {
                            vec2 offset = off * vec2(float(i % 3) - 1.0, float(i / 3) - 1.0);
                            total += (i == 4 ? -8.0 : 1.0) * texture(screenTexture, TexCoords + offset).rgb;
                        }
                        FragColor = vec4(total, 1.0);
                    }
                )
            )};
        }

        currShader = &postProcessingQuad.GetShader();
        renderResult.Activate(0);
    }

    void TestPostProcessing::OnUpdate(float deltaTime) {
        Test::OnUpdate(deltaTime);
        modelRotation += turnSpeed * deltaTime;
        modelRotation = modelRotation.apply([](const float f) -> float { return std::fmod(f, Maths::TAU); });
    }

    void TestPostProcessing::OnRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnRender(gdevice);
        Maths::mat3D mat = Maths::mat3D::transform(modelTranslation, modelScale, modelRotation);

        scene.SetCamera(mat);

        Graphics::Render::EnableDepth();
        if (usePostProcessing) {
            fbo.Bind();
            Graphics::Render::Clear();
        }

        scene.ResetData();
        Graphics::Shader& shader = scene.GetShader();
        shader.Bind();
        shader.SetUniformMatrix4x4("u_projection", projection);
        shader.SetUniformMatrix4x4("u_view", mat);
        Graphics::Render::Draw(scene.GetRenderData());

        if (usePostProcessing) {
            Graphics::Render::DisableDepth();
            fbo.Unbind();

            // Graphics::Render::SetClearColor(1);
            Graphics::Render::ClearColorBit();

            currShader->Bind();
            currShader->SetUniformTex("screenTexture", renderResult);
            if (currShader == &shaderBlur) {
                currShader->SetUniform2F("blurOff", effectOff.begin());
            } else if (currShader == &shaderEdgeDetect) {
                currShader->SetUniform2F("detectOff", effectOff.begin());
            } else if (currShader == &shaderHsv) {
                currShader->SetUniform1F("dh", hueShift);
                currShader->SetUniform1F("ds", satMul);
                currShader->SetUniform1F("dv", valShift);
            }

            postProcessingQuad.ResetData();
            Graphics::Render::Draw(postProcessingQuad.GetRenderData(), *currShader);
        }
    }

    void TestPostProcessing::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        Test::OnImGuiRender(gdevice);
        ImGui::DragFloat3("Translation" , modelTranslation.begin(), 0.01f);
        ImGui::DragFloat3("Scale"       , modelScale.begin(),       0.01f);
        ImGui::DragFloat3("Rotation"    , modelRotation.begin(),    0.01f);

        ImGui::DragFloat3("Spin Speed", turnSpeed.begin(), 0.01f, -10, 10);

        ImGui::Separator();

        ImGui::Checkbox("Use Post Processing", &usePostProcessing);

#define TAB_ITEM(X, N, P, C) if (ImGui::BeginTabItem(N)) { currShader = &(P); ImGui::EndTabItem(); C }
        if (ImGui::BeginTabBar("Post Processing Shader")) {
            TAB_ITEM(NONE, "None", postProcessingQuad.GetShader(), )
            TAB_ITEM(COLOR_INVERT, "Color Invert", shaderInv, )
            TAB_ITEM(COLOR_HSL, "Color Hue", shaderHsv,
                ImGui::DragFloat("Hue Shift", &hueShift, 0.01f, 0, 1);
                ImGui::DragFloat("Saturation Multiplier", &satMul, 0.01f, 0);
                ImGui::DragFloat("Value Shift", &valShift, 0.01f, -1, 1);)
            TAB_ITEM(BLUR, "Blur", shaderBlur, ImGui::DragFloat2("Blur Offset", effectOff.begin(), 0.1f); )
            TAB_ITEM(EDGE_DETECT, "Edge Detection", shaderEdgeDetect, ImGui::DragFloat2("Detect Offset", effectOff.begin(), 0.1f); )
            ImGui::EndTabBar();
        }
#undef TAB_ITEM
    }

    void TestPostProcessing::OnDestroy(Graphics::GraphicsDevice& gdevice) {
        Test::OnDestroy(gdevice);
        scene.Destroy();
        Graphics::Render::EnableDepth();
    }
}

#undef POST_VERTEX_SHADER