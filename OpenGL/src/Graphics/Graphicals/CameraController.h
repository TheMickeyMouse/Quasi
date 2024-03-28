#pragma once
#include "Matrix.h"
#include "Rect.h"

namespace Graphics {
    class GraphicsDevice;

    class CameraController {
    public:
        Maths::fvec3 worldUp = Maths::fvec3::UP(), worldFront = Maths::fvec3::BACK();

        Maths::fvec3 position = { 0, 0, -2 };
        float yaw = 0.0f, pitch = 0.0f; // yaw is looking A <-> B rotation, pitch is ^^^ rotation
        Maths::dvec2 lastMouse;
        bool initialMouse = true;

        float speed = 1.5f, sensitivity = 0.12f, fov = 45.0f, viewFov = 45.0f;
        Maths::rangef fovRange = { 1.0f, 45.0f };
        float zoomRatio = 0.5f;

        bool enabled = false;
        float smoothZoom = -1.0f;

        inline static bool showControls = false;

        [[nodiscard]] OPENGL_API Maths::fvec3 Right() const;

        OPENGL_API void Update(GraphicsDevice& gd, float dt);
        OPENGL_API void Toggle(GraphicsDevice& gd);

        [[nodiscard]] OPENGL_API Maths::mat3D GetViewMat() const;
        [[nodiscard]] OPENGL_API Maths::mat3D GetProjMat() const;

        [[nodiscard]] bool UsesSmoothZoom() const { return !std::signbit(smoothZoom); }

        OPENGL_API void ImGuiEdit(const char* title = "Camera Controller");
        OPENGL_API void CopyState() const;

        OPENGL_API static void HintControls();
    };
}
