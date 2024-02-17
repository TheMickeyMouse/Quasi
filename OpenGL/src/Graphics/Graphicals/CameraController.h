#pragma once
#include "Matrix.h"
#include "Vector.h"

namespace Graphics {
    class GraphicsDevice;
}

namespace Graphics {
    class CameraController {
    public:
        Maths::fvec3 trans = { 0, 0, -2 }, scale = 1, rot;
        float speed = 1.5f, sensitivity = 0.002f;

        bool enabled = true;

        OPENGL_API void Update(GraphicsDevice& gd, float dt);
        OPENGL_API void Toggle(GraphicsDevice& gd);

        OPENGL_API [[nodiscard]] Maths::mat3D GetViewMat() const;

        OPENGL_API void ImGuiEdit(const char* title = "Camera Controller");
    };
}
