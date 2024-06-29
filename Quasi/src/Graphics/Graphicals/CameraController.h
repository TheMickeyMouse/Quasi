#pragma once
#include <cmath>

#include "Matrix.h"
#include "Rect.h"

namespace Quasi::Graphics {
    class GraphicsDevice;

    class CameraController {
    public:
        Math::fVector3 worldUp = Math::fVector3::UP(), worldFront = Math::fVector3::BACK();

        Math::fVector3 position = { 0, 0, -2 };
        float yaw = 0.0f, pitch = 0.0f; // yaw is looking A <-> B rotation, pitch is ^^^ rotation
        Math::dVector2 lastMouse;
        bool initialMouse = true;

        float speed = 1.5f, sensitivity = 0.12f, fov = 45.0f, viewFov = 45.0f;
        Math::fRange fovRange = { 1.0f, 45.0f };
        float zoomRatio = 0.5f;

        bool enabled = false;
        float smoothZoom = -1.0f;

        inline static bool showControls = false;

        [[nodiscard]] Math::fVector3 Right() const;

        void Update(GraphicsDevice& gd, float dt);
        void Toggle(GraphicsDevice& gd);

        [[nodiscard]] Math::Matrix3D GetViewMat() const;
        [[nodiscard]] Math::Matrix3D GetProjMat() const;

        [[nodiscard]] bool UsesSmoothZoom() const { return !std::signbit(smoothZoom); }

        void ImGuiEdit(Str title = "Camera Controller");
        void CopyState() const;

        static void HintControls();
    };
}
