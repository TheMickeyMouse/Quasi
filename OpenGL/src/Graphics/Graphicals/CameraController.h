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
        float speed = 0.75f, sensitivity = 0.3f;

        bool lock = true;

        void Update(GraphicsDevice& gd, float dt);
        void Toggle(GraphicsDevice& gd);

        [[nodiscard]] Maths::mat3D GetViewMat() const;

        void ImGuiEdit(const char* title = "Camera Controller");
    };
}
