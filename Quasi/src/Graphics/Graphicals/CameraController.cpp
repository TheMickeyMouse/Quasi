#include "CameraController.h"

#include "GraphicsDevice.h"

namespace Quasi::Graphics {
    Math::fv3 CameraController::Right() const { return worldFront.Cross(worldUp); }

    void CameraController::Update(GraphicsDevice& gd, const float dt) {
        using namespace Math;
        using namespace IO;
        const auto& Keyboard = gd.GetIO().Keyboard;
        auto& Mouse = gd.GetIO().Mouse;
        if(Keyboard.KeyOnPress(ESCAPE)) Toggle(gd);

        if (UsesSmoothZoom()) {
            viewFov = std::lerp(viewFov, fov, std::exp2f(-smoothZoom * dt));
        } else {
            viewFov = fov;
        }

        if (!enabled) { return; }
        const fv3 localRight = -(Right() * std::cos(yaw) + worldFront * std::sin(yaw));
        const fv3 localFront = -localRight.Cross(worldUp);
        if (Keyboard.KeyPressed(W))      position += localFront * speed * dt;
        if (Keyboard.KeyPressed(S))      position -= localFront * speed * dt;
        if (Keyboard.KeyPressed(D))      position += localRight * speed * dt;
        if (Keyboard.KeyPressed(A))      position -= localRight * speed * dt;
        if (Keyboard.KeyPressed(SPACE))  position += fv3::Up()   * speed * dt;
        if (Keyboard.KeyPressed(LSHIFT)) position += fv3::Down() * speed * dt;

        if (Keyboard.KeyOnPress(CAPS_LOCK)) fov = fovRange.max * zoomRatio;
        if (Keyboard.KeyOnRelease(CAPS_LOCK)) fov = fovRange.max;

        if (Keyboard.KeyOnPress(LCONTROL)) speed *= 2;
        if (Keyboard.KeyOnRelease(LCONTROL)) speed /= 2;

        if (Keyboard.KeyPressed(CAPS_LOCK)) {
            fov -= (float)Mouse.GetMouseScrollDelta().y;
            fov = fovRange.Clamp(fov);
        }

        const dv2 mouse = Mouse.GetMousePosPx();

        if (initialMouse) {
            lastMouse = mouse;
            initialMouse = false;
        }

        yaw   += (float)(mouse.x - lastMouse.x) * -(sensitivity * dt);
        pitch += (float)(mouse.y - lastMouse.y) *  (sensitivity * dt);
        pitch = std::clamp(pitch, HALF_PI * -0.95f, HALF_PI * 0.95f);
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

    Math::Matrix3D CameraController::GetViewMat() const {
        return GetViewTransform().TransformMatrix().InvTransRot();
    }

    Math::Transform3D CameraController::GetViewTransform() const {
        // const Math::fv3 front =
        //     Right()    * (std::cos(yaw) * std::cos(pitch)) + // like x
        //     worldFront * (std::sin(yaw) * std::cos(pitch)) +
        //     worldUp    * std::sin(pitch);
        // return { -position, 1, Math::Quaternion::look_at(front, worldFront).inv() };
        return { position, 1,
                 Math::Rotation3D::RotateAxis(worldUp, Math::Radians(yaw)) +
                 Math::Rotation3D::RotateAxis(Right(), Math::Radians(pitch)) };
    }

    Math::Matrix3D CameraController::GetProjMat() const {
        const float aspect = GraphicsDevice::GetDeviceInstance().GetWindowSize().AspectRatio();
        return Math::Matrix3D::PerspectiveFov(Math::Radians::FromDegrees(viewFov), aspect, 0.01f, 100.0f);
    }
}
