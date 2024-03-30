#include "CameraController.h"

#include "Constants.h"
#include "GraphicsDevice.h"

#include "imgui.h"

namespace Graphics {
    Maths::fvec3 CameraController::Right() const { return worldFront.cross(worldUp); }

    void CameraController::Update(GraphicsDevice& gd, const float dt) {
        using namespace Maths;
        using namespace IO;
        const auto& Keyboard = gd.GetIO().Keyboard;
        auto& Mouse = gd.GetIO().Mouse;
        if(Keyboard.KeyOnPress(Key::ESCAPE)) Toggle(gd);

        if (UsesSmoothZoom()) {
            viewFov = std::lerp(viewFov, fov, std::exp2f(-smoothZoom * dt));
        } else {
            viewFov = fov;
        }

        if (!enabled) { return; }
        const fvec3 localFront = Right() * std::cos(yaw) + worldFront * std::sin(yaw);
        const fvec3 localRight = localFront.cross(worldUp);
        if (Keyboard.KeyPressed(Key::W))      position += localFront * speed * dt;
        if (Keyboard.KeyPressed(Key::S))      position -= localFront * speed * dt;
        if (Keyboard.KeyPressed(Key::D))      position += localRight * speed * dt;
        if (Keyboard.KeyPressed(Key::A))      position -= localRight * speed * dt;
        if (Keyboard.KeyPressed(Key::SPACE))  position += fvec3::UP()   * speed * dt;
        if (Keyboard.KeyPressed(Key::LSHIFT)) position += fvec3::DOWN() * speed * dt;

        if (Keyboard.KeyOnPress(Key::CAPS_LOCK)) fov = fovRange.max * zoomRatio;
        if (Keyboard.KeyOnRelease(Key::CAPS_LOCK)) fov = fovRange.max;

        if (Keyboard.KeyOnPress(Key::LCONTROL)) speed *= 2;
        if (Keyboard.KeyOnRelease(Key::LCONTROL)) speed /= 2;

        if (Keyboard.KeyPressed(Key::CAPS_LOCK)) {
            fov -= (float)Mouse.GetMouseScrollDelta().y;
            fov = fovRange.clamp(fov);
        }

        const dvec2 mouse = Mouse.GetMousePosPx();

        if (initialMouse) {
            lastMouse = mouse;
            initialMouse = false;
        }

        yaw   += (float)(mouse.x - lastMouse.x) * -(sensitivity * dt);
        pitch += (float)(mouse.y - lastMouse.y) * -(sensitivity * dt);
        pitch = std::clamp(pitch, -HALF_PI * 0.95f, +HALF_PI * 0.95f);
        lastMouse = mouse;
    }

    void CameraController::Toggle(GraphicsDevice& gd) {
        enabled ^= true;
        if (enabled) {
            gd.GetIO().Mouse.Lock();
        } else {
            gd.GetIO().Mouse.Show();
            initialMouse = true;
        }
    }

    Maths::mat3D CameraController::GetViewMat() const {
        const Maths::fvec3 front =
            Right() * (std::cos(yaw) * std::cos(pitch)) + // like x
            worldFront * (std::sin(yaw) * std::cos(pitch)) +
            worldUp * std::sin(pitch);

        return Maths::mat3D::look_at(position, position + front, worldUp);
    }

    Maths::mat3D CameraController::GetProjMat() const {
        const float aspect = 1.0f / GraphicsDevice::GetDeviceInstance().GetWindowSize().slope();
        return Maths::mat3D::perspective_fov(viewFov, aspect, 0.01f, 100.0f);
    }

    void CameraController::ImGuiEdit(const char* const title) {
        ImGui::Separator();
        ImGui::TextDisabled(enabled ? " (press Esc to disable!)" : " (press Esc to enable!)");

        if (!ImGui::TreeNode(title)) return;

        ImGui::SliderFloat("Light Yaw",   &yaw,   -Maths::PI,             Maths::PI);
        ImGui::SliderFloat("Light Pitch", &pitch, -Maths::HALF_PI * 0.9f, Maths::HALF_PI * 0.9f);
        ImGui::DragFloat3("Position", position.begin(), 0.5f);
        ImGui::DragFloat("Speed", &speed, 0.5f);
        ImGui::SliderFloat("Sensitivity", &sensitivity, 0, 1);

        if (ImGui::TreeNode("FOV & Zoom")) {
            ImGui::SliderFloat("FOV", &fov, fovRange.min, fovRange.max);
            ImGui::DragFloatRange2("FOV Range", &fovRange.min.value(), &fovRange.max.value(), 0.5f, 0.0f, 90.0f);
            ImGui::DragFloat("Zoom Lerp", &smoothZoom);
            ImGui::DragFloat("Zoom Factor", &zoomRatio);

            bool sz = UsesSmoothZoom();
            ImGui::Checkbox("Use Smooth Zoom", &sz);
            if (sz != UsesSmoothZoom()) smoothZoom *= -1;

            ImGui::TreePop();
        }

        if (ImGui::Button("Copy State to Clipboard")) {
            CopyState();
        }
        ImGui::SameLine();
        if (ImGui::Button(showControls ? "Hide Controls" : "Show Controls")) {
            showControls ^= true;
        }
        if (showControls) HintControls();

        ImGui::TreePop();
    }

    void CameraController::CopyState() const {
        ImGui::SetClipboardText(
            std::format(
                "camera.position = {{ {}, {}, {} }};\n"
                "camera.yaw = {}; camera.pitch = {};\n"
                "camera.speed = {};\n"
                "camera.sensitivity = {};\n"
                "camera.fov = {};\n"
                "camera.fovRange = {{ {}, {} }};\n"
                "camera.zoomRatio = {};\n"
                "camera.smoothZoom = {};\n",
                position.x, position.y, position.z,
                yaw, pitch,
                speed,
                sensitivity,
                fov,
                fovRange.min.value(), fovRange.max.value(),
                zoomRatio,
                smoothZoom)
            .c_str());
    }

    void CameraController::HintControls() {
        ImGui::Begin("Camera Controls", &showControls);

        ImGui::Text("[Esc] - Toggle Camera View");
        ImGui::Text("Movement"); ImGui::SameLine(); ImGui::Separator();
        ImGui::Text(
            "   [W]    + [Space]/[Shift]/[Ctrl]\n"
            "[A][S][D]    (up)   (down)  (speed up)\n"
        );
        ImGui::Text("Zoom"); ImGui::SameLine(); ImGui::Separator();
        ImGui::Text(
            "[Mouse] - Look around\n"
            "[Caps Lock] + [Scroll Wheel] - Zoom"
        );

        ImGui::End();
    }
}
