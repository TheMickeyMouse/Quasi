#include "CameraController.h"

#include "GraphicsDevice.h"

namespace Quasi::Graphics {
    Math::fVector3 CameraController::Right() const { return worldFront.cross(worldUp); }

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
        const fVector3 localRight = -(Right() * std::cos(yaw) + worldFront * std::sin(yaw));
        const fVector3 localFront = -localRight.cross(worldUp);
        if (Keyboard.KeyPressed(W))      position += localFront * speed * dt;
        if (Keyboard.KeyPressed(S))      position -= localFront * speed * dt;
        if (Keyboard.KeyPressed(D))      position += localRight * speed * dt;
        if (Keyboard.KeyPressed(A))      position -= localRight * speed * dt;
        if (Keyboard.KeyPressed(SPACE))  position += fVector3::UP()   * speed * dt;
        if (Keyboard.KeyPressed(LSHIFT)) position += fVector3::DOWN() * speed * dt;

        if (Keyboard.KeyOnPress(CAPS_LOCK)) fov = fovRange.max * zoomRatio;
        if (Keyboard.KeyOnRelease(CAPS_LOCK)) fov = fovRange.max;

        if (Keyboard.KeyOnPress(LCONTROL)) speed *= 2;
        if (Keyboard.KeyOnRelease(LCONTROL)) speed /= 2;

        if (Keyboard.KeyPressed(CAPS_LOCK)) {
            fov -= (float)Mouse.GetMouseScrollDelta().y;
            fov = fovRange.clamp(fov);
        }

        const dVector2 mouse = Mouse.GetMousePosPx();

        if (initialMouse) {
            lastMouse = mouse;
            initialMouse = false;
        }

        yaw   += (float)(mouse.x - lastMouse.x) * -(sensitivity * dt);
        pitch += (float)(mouse.y - lastMouse.y) *  (sensitivity * dt);
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

    Math::Matrix3D CameraController::GetViewMat() const {
        return GetViewTransform().TransformMatrix().inv();
    }

    Math::Transform3D CameraController::GetViewTransform() const {
        // const Math::fVector3 front =
        //     Right()    * (std::cos(yaw) * std::cos(pitch)) + // like x
        //     worldFront * (std::sin(yaw) * std::cos(pitch)) +
        //     worldUp    * std::sin(pitch);
        // return { -position, 1, Math::Quaternion::look_at(front, worldFront).inv() };
        return { position, 1, Math::Quaternion::rotate_axis(Right(), pitch).then(Math::Quaternion::rotate_axis(worldUp, yaw)) };
    }

    Math::Matrix3D CameraController::GetProjMat() const {
        const float aspect = 1.0f / GraphicsDevice::GetDeviceInstance().GetWindowSize().slope();
        return Math::Matrix3D::perspective_fov(viewFov, aspect, 0.01f, 100.0f);
    }
}
