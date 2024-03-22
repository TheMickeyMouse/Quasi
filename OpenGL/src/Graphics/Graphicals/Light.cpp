#include "Light.h"

#include "Constants.h"
#include "imgui.h"

namespace Graphics {
    template <LightCaster L>
    OPENGL_API stdu::ref<L> Light::AsUnchecked() {
        return *(L*)&lightAddressBegin;  // NOLINT(clang-diagnostic-cast-qual)
        // not very pretty but hey, its all i got ok?
    }

    template <LightCaster L>
    OPENGL_API stdu::ref<const L> Light::AsUnchecked() const {
        return *(const L*)&lightAddressBegin;
    }

    Maths::fvec3 Light::Position() const {
        switch (type) {
            case LightType::SUNLIGHT:
                return -sunLight.direction;
            case LightType::POINTLIGHT:
                return pointLight.position;
            case LightType::FLASHLIGHT:
                return flashLight.position;
            case LightType::NONE:
                return NAN;
        }
        return NAN;
    }

    Maths::fvec3 Light::Direction() const {
        switch (type) {
            case LightType::SUNLIGHT:
                return sunLight.direction;
            case LightType::POINTLIGHT:
                return -pointLight.position;
            case LightType::FLASHLIGHT:
                return Maths::fvec3::from_polar(1, flashLight.yaw, flashLight.pitch);
            case LightType::NONE:
                return 0;
        }
        return NAN;
    }

    void Light::ImGuiEdit(const char* const title) {
        ImGui::Separator();
        if (!ImGui::TreeNode(title)) return;

        ImGui::BeginTabBar("Light Edit");

        for (uint i = 0; i < LightTypeCount(); ++i) {
            if (ImGui::TabItemButton(ToStr((LightType)i))) {
                type = (LightType)i;
            }
        }

        ImGui::ColorEdit3("Color", color.begin());
        ImGui::DragFloat("Intensity", &color.a, 0.01f);
        switch (type) {
            case LightType::SUNLIGHT:
                ImGuiEditLight<SunLight>();
            break;
            case LightType::POINTLIGHT:
                ImGuiEditLight<PointLight>();
            break;
            case LightType::FLASHLIGHT:
                ImGuiEditLight<FlashLight>();
            break;
            case LightType::NONE:
                break;
        }

        ImGui::EndTabBar();
        if (ImGui::Button("Copy State to Clipboard")) CopyState();
        ImGui::TreePop();
    }

    void Light::CopyState() const {
        switch (type) {
            case LightType::SUNLIGHT: {
                ImGui::SetClipboardText(
                    std::format(
                        "light = Graphics::SunLight {{\n"
                        "   .direction = {{ {}, {}, {} }},\n"
                        "}};\n"
                        "light.color = {{ {}, {}, {} }};\n",
                        sunLight.direction.x, sunLight.direction.y, sunLight.direction.z,
                        color.r, color.g, color.b)
                    .c_str()
                );
                break;
            }
            case LightType::POINTLIGHT: {
                ImGui::SetClipboardText(
                    std::format(
                        "light = Graphics::PointLight {{\n"
                        "   .position = {{ {}, {}, {} }},\n"
                        "   .constant = {},\n"
                        "   .linear = {},\n"
                        "   .quadratic = {},\n"
                        "}};\n"
                        "light.color = {{ {}, {}, {} }};\n",
                        pointLight.position.x, pointLight.position.y, pointLight.position.z,
                        pointLight.constant, pointLight.linear, pointLight.quadratic,
                        color.r, color.g, color.b)
                    .c_str()
                );
                break;
            }
            case LightType::FLASHLIGHT: {
                ImGui::SetClipboardText(
                    std::format(
                        "light = Graphics::FlashLight {{\n"
                        "   .position = {{ {}, {}, {} }},\n"
                        "   .yaw = {}, .pitch = {},\n"
                        "   .innerCut = {}, .outerCut = {}\n"
                        "}};\n"
                        "light.color = {{ {}, {}, {} }};\n",
                        flashLight.position.x, flashLight.position.y, flashLight.position.z,
                        flashLight.yaw, flashLight.pitch,
                        flashLight.innerCut, flashLight.outerCut,
                        color.r, color.g, color.b)
                    .c_str()
                );
                break;
            }
            case LightType::NONE:
                break;
        }
    }

    template <> void Light::ImGuiEditLight<SunLight>() {
        if (!Is<SunLight>()) Set(SunLight { .direction = Direction() });

        auto [len, yaw, pitch] = sunLight.direction.spheric();
        yaw = std::isnan(yaw) ? 0 : yaw;
        pitch = std::isnan(pitch) ? 0 : pitch;
        ImGui::SliderFloat("Yaw", &yaw, -Maths::PI, Maths::PI);
        ImGui::SliderFloat("Pitch", &pitch, -Maths::HALF_PI, Maths::HALF_PI);
        ImGui::SliderFloat("Length", &len, 0, 10);
        sunLight.direction = Maths::fvec3::from_polar(len, yaw, pitch);
    }

    template <> void Light::ImGuiEditLight<PointLight>() {
        if (!Is<PointLight>())
            Set(PointLight {
                .position = Position(),
                .constant = 1.0f,
                .linear = 0.0f,
                .quadratic = 0.001f
            });

        ImGui::DragFloat3("Position", flashLight.position.begin());
        if (ImGui::TreeNode("Light Attenuation")) {
            ImGui::DragFloat("Constant",  &pointLight.constant, 0.01f);
            ImGui::DragFloat("Linear",    &pointLight.linear, 0.01f);
            ImGui::DragFloat("Quadratic", &pointLight.quadratic, 0.01f);

            ImGui::TreePop();
        }
    }

    template <> void Light::ImGuiEditLight<FlashLight>() {
        using Maths::operator ""_deg;
        if (!Is<FlashLight>()) {
            auto [_, yaw, pitch] = Direction().spheric();
            Set(FlashLight {
                .position = Position(),
                .yaw = yaw, .pitch = pitch,
                .innerCut = 12.5_deg, .outerCut = 20.0_deg
            });
        }

        ImGui::DragFloat3("Position",  flashLight.position.begin());

        flashLight.yaw = std::isnan(flashLight.yaw) ? 0 : flashLight.yaw;
        flashLight.pitch = std::isnan(flashLight.pitch) ? 0 : flashLight.pitch;
        ImGui::SliderFloat("Yaw", &flashLight.yaw, -Maths::PI, Maths::PI);
        ImGui::SliderFloat("Pitch", &flashLight.pitch, -Maths::HALF_PI, Maths::HALF_PI);

        ImGui::SliderFloat("Inner Cutoff", &flashLight.innerCut, 0, Maths::PI);
        ImGui::SliderFloat("Outer Cutoff", &flashLight.outerCut, 0, Maths::PI);
    }

    template OPENGL_API stdu::ref<SunLight> Light::AsUnchecked<SunLight>();
    template OPENGL_API stdu::ref<PointLight> Light::AsUnchecked<PointLight>();
    template OPENGL_API stdu::ref<FlashLight> Light::AsUnchecked<FlashLight>();
    template OPENGL_API stdu::ref<const SunLight> Light::AsUnchecked<SunLight>() const;
    template OPENGL_API stdu::ref<const PointLight> Light::AsUnchecked<PointLight>() const;
    template OPENGL_API stdu::ref<const FlashLight> Light::AsUnchecked<FlashLight>() const;
}
