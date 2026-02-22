#include "DemoGlass.h"
#include "imgui.h"
#include "GUI/ImGuiExt.h"
#include "Utils/Algorithm.h"

namespace Test {
    Math::fv2 DemoGlass::Refract(const Math::fv2& I, const Math::fv2& N, float eta) {
        const float cos = N.Dot(I);
        const float k = 1.0 - eta * eta * (1.0 - cos * cos);
        if (k < 0.0)
            return { 0 };
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

        shader = Graphics::Shader::FromFile(RES("glass.glsl"));
    }

    void DemoGlass::OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) {
        auto& io = gdevice.GetIO();
        if (io.LeftMouse().Pressed()) {
            x = io.GetMousePos().x * 16.0 / 9.0;
        }
        if (io.LeftMouse().OnPress()) {
            Math::fv2 mPos = io.GetMousePos().As<float>() / 100.0f;
            mPos.y = 9 - mPos.y;

            Span(wave).RotateRight(1);
            wave[0] = 0.0f;
            Span(waveOrigin).RotateRight(1);
            waveOrigin[0] = mPos;
        }

        glowSize += dG * deltaTime;
        dG       += 256.0f * ((io.LeftMouse().Pressed() ? 0.7f : 0.3f) - glowSize) * deltaTime;
        dG       *= (1 - 12.0f * deltaTime);

        for (float& w : wave) {
            w += deltaTime;
        }
    }

    void DemoGlass::OnRender(Graphics::GraphicsDevice& gdevice) {
#if 1
        canvas.BeginFrame();
        canvas.DrawTexture(plain, 0, { 32.0 / 9.0, 2 }, true);
        canvas.EndFrame();

        Math::fv2 mouse = gdevice.GetIO().GetMousePos().As<float>() / 100.0f;
        mouse.y = 9 - mouse.y;
        shader.Bind();
        shader.SetUniformFloat("radius", radius);
        shader.SetUniformFloat("height", height);
        shader.SetUniformFv2("rectCenter", (max + min) / 2);
        shader.SetUniformFv2("rectSize",   (max - min) / 2);
        shader.SetUniformFv3("lightSource", mouse.AddZ(radius + glowSize));
        shader.SetUniformFloatArr("wave", wave);
        shader.SetUniformFv2Arr("waveOrigin", waveOrigin);
        shader.SetUniformTex("glassFloor", glass, 0);
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
        ImGui::DragFloat("glow",   &glowSize, 0.01f, 0.1f, 0.5f);
        ImGui::EditVector("min", min, 0.01f);
        ImGui::EditVector("max", max, 0.01f);
    }

    void DemoGlass::OnDestroy(Graphics::GraphicsDevice& gdevice) {

    }
}
