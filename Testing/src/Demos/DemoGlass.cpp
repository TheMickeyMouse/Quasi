#include "DemoGlass.h"
#include "imgui.h"
#include "GUI/ImGuiExt.h"

namespace Test {
    Math::fv2 DemoGlass::Refract(const Math::fv2& I, const Math::fv2& N, float eta) {
        const float cos = N.Dot(I);
        const float k = 1.0 - eta * eta * (1.0 - cos * cos);
        if (k < 0.0)
            return { 0 };       // or genDType(0.0)
        else
            return eta * I - (eta * cos + std::sqrt(k)) * N;
    }

    void DemoGlass::OnInit(Graphics::GraphicsDevice& gdevice) {
        canvas = { gdevice };
        canvas.SetViewport(Math::fRect2D::FromCenter(0, { 32.0 / 9.0, 2 }));
        Graphics::Render::SetClearColor(0.95f);

        plain = Graphics::Texture2D::LoadPNG(RES("plain.png"), { .border = Graphics::TextureBorder::CLAMP_TO_BORDER });
        plain.Bind();
        plain.SetBorderColor(0);
        glass = Graphics::Texture2D::LoadPNG(RES("glass.png"), { .border = Graphics::TextureBorder::CLAMP_TO_BORDER });
        glass.Bind();
        glass.SetBorderColor(0);

        shader = Graphics::Shader::New(
            "// #shader vertex\n"
            "#version 330\n"
            "out vec2 pos;"
            "void main() {"
            "   gl_Position = vec4(vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3))[gl_VertexID], 0, 1);"
            "   pos = (gl_Position.xy + 1) * vec2(8, 4.5);"
            "}\n"
            "// #shader fragment\n"
            "#version 330 \n"
            "in vec2 pos; \n"
            "layout (location = 0) out vec4 glColor; \n"
            "uniform float radius; \n"
            "uniform float height; \n"
            "uniform vec2 cornerMin, cornerMax; \n"
            "uniform vec3 lightSource; \n"
            "uniform sampler2D plainFloor; \n"
            "uniform sampler2D glassFloor; \n"
            "void main() {\n"
            "   vec2 rPos = pos - (cornerMin + cornerMax) / 2;"
            "   vec2 d = abs(rPos) - (cornerMax - cornerMin) / 2;"
            "   vec2 p = max(d, 0.0) * sign(rPos);"
            "   float lensq = dot(p, p);\n"
            "   if (lensq >= radius * radius) {\n"
            "       glColor = texture(plainFloor, pos / vec2(16, 9));\n"
            "       return;\n"
            "   }\n"
            "   vec3 normal  = vec3(p / radius, sqrt(1 - lensq / (radius * radius)));\n"
            "   vec3 hit     = normal * radius;\n"
            "   vec3 rDir    = refract(vec3(0, 0, -1), normal, 0.667);\n"
            "   vec3 hit2    = hit - 2 * radius * rDir * dot(rDir, normal);\n"
            "   vec3 n2      = -hit2 / radius;\n"
            "   vec3 xDir    = refract(rDir, n2, 1.5);\n"
            "   vec2 surface = hit2.xy - xDir.xy * ((height + hit2.z) / xDir.z);\n"
            "   vec3 result  = texture(glassFloor, (surface + pos - p) / vec2(16, 9)).rgb;\n"
            "   result *= 1 + max(0, 0.1 + dot(normalize(lightSource - hit - vec3(pos - p, 0.0)), normal));\n"
            "   glColor = vec4(result, 1.0);\n"
            "}"
        );
    }

    void DemoGlass::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        if (gdevice.GetIO().Mouse.LeftPressed()) {
            x = gdevice.GetIO().Mouse.GetMousePos().x * 16.0 / 9.0;
        }
    }

    void DemoGlass::OnRender(Graphics::GraphicsDevice& gdevice) {
#if 1
        Math::fv2 mouse = gdevice.GetIO().Mouse.GetMousePosPx().As<float>() / 100.0f;
        mouse.y = 9 - mouse.y;
        shader.Bind();
        shader.SetUniformFloat("radius", radius);
        shader.SetUniformFloat("height", height);
        shader.SetUniformFv2("cornerMin", min);
        shader.SetUniformFv2("cornerMax", max);
        shader.SetUniformFv3("lightSource", mouse.AddZ(radius + 0.1f));
        shader.SetUniformTex("plainFloor", plain, 0);
        shader.SetUniformTex("glassFloor", glass, 1);
        Graphics::Render::DrawScreenQuad(shader);
#else
        canvas.BeginFrame();

        canvas.NoFill();
        canvas.Stroke(0);
        canvas.StrokeWeight(0.007f);
        canvas.DrawCircle(0, radius);
        canvas.DrawPoint(0);
        canvas.DrawLine({ -1.8, -height }, { 1.8, -height });

        using namespace Math;

        canvas.Stroke({ 1.0f, 0, 0 });
        const float lensq = x * x;
        if (lensq > radius * radius) {
            canvas.DrawLine({ x, 1 }, { x, -height });
            canvas.EndFrame();
            return;
        }
        fv2 normal = { x / radius, std::sqrt(1 - lensq / (radius * radius)) };
        fv2 hit    = normal * radius;
        canvas.DrawLine({ x, 1 }, hit);

        fv2 rDir   = Refract({ 0, -1 }, normal, 0.667f);
        fv2 hit2   = hit - 2 * radius * rDir * rDir.Dot(normal);

        canvas.DrawLine(hit, hit2);

        fv2 n2     = -hit2 / radius;
        fv2 xDir   = Refract(rDir, n2, 1.5);
        fv2 surface = hit2 - xDir * ((height + hit2.y) / xDir.y);

        canvas.DrawLine(hit2, surface);

        canvas.Stroke({ 0, 0, 1 });
        canvas.DrawLine(hit + normal * 0.1f, hit  - normal * 0.05f);
        canvas.DrawLine(hit2 + n2 * 0.1f,    hit2 - n2 * 0.05f);

        canvas.Stroke({ 0, 0.8f, 0 });
        canvas.DrawLine(hit, hit + rDir * 0.07f);

        canvas.Stroke({ 1, 0, 0 });
        canvas.StrokeWeight(0.02f);
        canvas.DrawPoint(surface);

        canvas.EndFrame();
#endif
    }

    void DemoGlass::OnImGuiRender(Graphics::GraphicsDevice& gdevice) {
        ImGui::DragFloat("radius", &radius, 0.01f, 0, 1.0f);
        ImGui::DragFloat("height", &height, 0.01f, radius, 10.0f);
        ImGui::EditVector("min", min, 0.01f);
        ImGui::EditVector("max", max, 0.01f);
    }

    void DemoGlass::OnDestroy(Graphics::GraphicsDevice& gdevice) {

    }
}
