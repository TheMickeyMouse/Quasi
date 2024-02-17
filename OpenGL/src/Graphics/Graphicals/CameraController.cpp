#include "CameraController.h"

#include "imgui.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace Graphics {
    void CameraController::Update(const float dt) {


        if (lock) return;
        using namespace Maths;
        using namespace IO;
        const mat3D rotation = mat3D::rotate_y(rot.y);
        if (Keyboard.KeyPressed(Key::W))      trans += rotation * fvec3::FRONT() * -speed * dt;
        if (Keyboard.KeyPressed(Key::S))      trans += rotation * fvec3::BACK()  * -speed * dt;
        if (Keyboard.KeyPressed(Key::D))      trans += rotation * fvec3::LEFT()  *  speed * dt;
        if (Keyboard.KeyPressed(Key::A))      trans += rotation * fvec3::RIGHT() *  speed * dt;
        if (Keyboard.KeyPressed(Key::SPACE))  trans += fvec3::UP()    * -speed * dt;
        if (Keyboard.KeyPressed(Key::LSHIFT)) trans += fvec3::DOWN()  * -speed * dt;

        rot = Mouse.GetMousePosPx().perpend() * -sensitivity * dt;
    }

    void CameraController::Toggle() {
        lock ^= true;
        if (lock) {
            IO::Mouse.Lock();
        } else {
            IO::Mouse.Show();
        }
    }

    Maths::mat3D CameraController::GetViewMat() const {
        return Maths::mat3D::transform(trans, scale, rot).inv();
    }

    void CameraController::ImGuiEdit(const char* const title) {
        ImGui::Separator();
        ImGui::Text("%s", title);
        ImGui::SliderFloat3("Rot", rot.begin(),   -3.14f, 3.14f);
        ImGui::SliderFloat3("Sca", scale.begin(),  0,     3    );
        ImGui::SliderFloat3("Tra", trans.begin(), -10,    10   );
        ImGui::SliderFloat("Speed", &speed, 0, 1);
        ImGui::SliderFloat("Sensitivity", &sensitivity, 0, 1);
    }
}
