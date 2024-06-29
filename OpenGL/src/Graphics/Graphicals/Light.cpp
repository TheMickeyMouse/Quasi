#include "Light.h"

#include <cmath>

#include "Constants.h"
#include "imgui.h"

namespace Quasi::Graphics {
    Math::fVector3 Light::Position() const {
        return Visit([]<class L> (const L& light) {
            if constexpr (std::is_convertible_v<L, SunLight>) return -light.direction;
            else return light.position;
        });
    }

    Math::fVector3 Light::Direction() const {
        return Visit(
            [](const SunLight&   sun)   { return sun.direction; },
            [](const PointLight& point) { return -point.position; },
            [](const FlashLight& flash) { return Math::fVector3::from_spheric(1, flash.yaw, flash.pitch); }
        );
    }

    void Light::ImGuiEdit(const char* const title) {
        ImGui::Separator();
        if (!ImGui::TreeNode(title)) return;

        ImGui::BeginTabBar("Light Edit");

        using namespace Math;
        if (ImGui::TabItemButton("Sun") && !Is<SunLight>())
            Set(SunLight {
                .direction = Direction()
            });
        if (ImGui::TabItemButton("Point") && !Is<PointLight>())
            Set(PointLight {
                .position = Position(),
                .constant = 1.0f,
                .linear = 0.0f,
                .quadratic = 0.001f
            });
        if (ImGui::TabItemButton("Spot") && !Is<FlashLight>()) {
            const auto [_, yaw, pitch] = Direction().spheric();
            Set(FlashLight {
                .position = Position(),
                .yaw = yaw, .pitch = pitch,
                .innerCut = 12.5_deg, .outerCut = 20.0_deg
            });
        }

        ImGui::ColorEdit3("Color", color.begin());
        ImGui::DragFloat("Intensity", &color.a, 0.01f);

        Visit([] (auto& light) { ImGuiEditVisitor(light); });

        ImGui::EndTabBar();
        if (ImGui::Button("Copy State to Clipboard")) CopyState();
        ImGui::TreePop();
    }

    void Light::CopyState() const {
        ImGui::SetClipboardText(Visit(
            [&](const SunLight& sun) {
                return std::format(
                    "light = Graphics::SunLight {{\n"
                    "   .direction = {{ {}, {}, {} }},\n"
                    "}};\n"
                    "light.color = {{ {}, {}, {} }};\n",
                    sun.direction.x, sun.direction.y, sun.direction.z,
                    color.r, color.g, color.b);
            },
            [&](const PointLight& point) {
                return std::format(
                    "light = Graphics::PointLight {{\n"
                    "   .position = {{ {}, {}, {} }},\n"
                    "   .constant = {},\n"
                    "   .linear = {},\n"
                    "   .quadratic = {},\n"
                    "}};\n"
                    "light.color = {{ {}, {}, {} }};\n",
                    point.position.x, point.position.y, point.position.z,
                    point.constant, point.linear, point.quadratic,
                    color.r, color.g, color.b);
            },
            [&](const FlashLight& flash) {
                return std::format(
                        "light = Graphics::FlashLight {{\n"
                        "   .position = {{ {}, {}, {} }},\n"
                        "   .yaw = {}, .pitch = {},\n"
                        "   .innerCut = {}, .outerCut = {}\n"
                        "}};\n"
                        "light.color = {{ {}, {}, {} }};\n",
                        flash.position.x, flash.position.y, flash.position.z,
                        flash.yaw, flash.pitch,
                        flash.innerCut, flash.outerCut,
                        color.r, color.g, color.b);
            }
        ).c_str());
    }

    template <> void Light::ImGuiEditVisitor<SunLight>(SunLight& light) {
        auto [len, yaw, pitch] = light.direction.spheric();
        yaw   = std::isnan(yaw)   ? 0 : yaw;
        pitch = std::isnan(pitch) ? 0 : pitch;
        ImGui::SliderFloat("Yaw",    &yaw,   -Math::PI,      Math::PI);
        ImGui::SliderFloat("Pitch",  &pitch, -Math::HALF_PI, Math::HALF_PI);
        ImGui::SliderFloat("Length", &len, 0, 10);
        light.direction = Math::fVector3::from_spheric(len, yaw, pitch);
    }

    template <> void Light::ImGuiEditVisitor<PointLight>(PointLight& light) {
        ImGui::DragFloat3("Position", light.position.begin());
        if (ImGui::TreeNode("Light Attenuation")) {
            ImGui::DragFloat("Constant",  &light.constant,  0.01f);
            ImGui::DragFloat("Linear",    &light.linear,    0.01f);
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.01f);

            ImGui::TreePop();
        }
    }

    template <> void Light::ImGuiEditVisitor<FlashLight>(FlashLight& light) {
        ImGui::DragFloat3("Position",  light.position.begin());

        light.yaw   = std::isnan(light.yaw)   ? 0 : light.yaw;
        light.pitch = std::isnan(light.pitch) ? 0 : light.pitch;
        ImGui::SliderFloat("Yaw",   &light.yaw,   -Math::PI,      Math::PI);
        ImGui::SliderFloat("Pitch", &light.pitch, -Math::HALF_PI, Math::HALF_PI);

        ImGui::SliderFloat("Inner Cutoff", &light.innerCut, 0, Math::PI);
        ImGui::SliderFloat("Outer Cutoff", &light.outerCut, 0, Math::PI);
    }
}
