#include "CameraController2D.h"

#include "GraphicsDevice.h"

namespace Quasi::Graphics {
    void CameraController2D::Update(GraphicsDevice& gd, float dt) {
        using namespace Math;
        using namespace IO;

        const auto& Keyboard = gd.GetIO().Keyboard;
        const auto& Mouse = gd.GetIO().Mouse;

        if (useSmoothZoom) {
            displayScale = std::lerp(scale, displayScale, std::exp2f(-80.0f * dt));
        } else {
            displayScale = scale;
        }

        if (!enabled) { return; }
        if (Keyboard.KeyPressed(W)) position.y += scale * speed * dt;
        if (Keyboard.KeyPressed(S)) position.y -= scale * speed * dt;
        if (Keyboard.KeyPressed(D)) position.x += scale * speed * dt;
        if (Keyboard.KeyPressed(A)) position.x -= scale * speed * dt;

        if (Keyboard.KeyOnPress(CAPS_LOCK))   scale = 3;
        if (Keyboard.KeyOnRelease(CAPS_LOCK)) scale = 1;

        if (Keyboard.KeyOnPress(LCONTROL)) speed *= 2;
        if (Keyboard.KeyOnRelease(LCONTROL)) speed /= 2;

        scale += (float)Mouse.GetMouseScrollDelta().y * scale * 0.06f;
        if (Mouse.MiddlePressed())
            position -= (fv2)Mouse.FlipMouseY(Mouse.GetMousePosDeltaPx()) * 2.0f / scale;
    }

    void CameraController2D::Toggle() {
        enabled ^= true;
    }

    Math::fRect2D CameraController2D::GetViewport() const {
        const Math::fv2 winsize = (Math::fv2)GraphicsDevice::GetDeviceInstance().GetWindowSize();
        return Math::fRect2D::FromCenter(position, winsize / displayScale);
    }

    Math::Transform2D CameraController2D::GetViewTransform() const {
        return { position, 1.0f / displayScale, {} };
    }

    Math::Transform3D CameraController2D::GetViewTransform3D() const {
        const float invScale = 1.0f / displayScale;
        return { position.AddZ(0), { invScale, invScale, 1 }, {} };
    }

    Math::Matrix2D CameraController2D::GetViewMatrix() const {
        return Math::Matrix2D::Transform(-position, displayScale, {});
    }

    Math::Matrix3D CameraController2D::GetViewMatrix3D() const {
        return Math::Matrix3D::Transform(-position.AddZ(0), { displayScale, displayScale, 1 }, {});
    }
}
