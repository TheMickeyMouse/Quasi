#pragma once
#include <vector>

#include "Body2D.h"
#include "core.h"

namespace Physics2D {
    using BodyHandle = std::unique_ptr<Body>;

    class World {
        std::vector<BodyHandle> bodies;
        Maths::fvec2 gravity;
        float drag = 0.0f;
    private:
        OPENGL_API Body* CreateBodyWithHeap(const BodyCreateOptions& options, std::unique_ptr<Shape>&& heapAllocShape);
        void ClearWithoutUpdate() { bodies.clear(); }
    public:
        World() = default;
        World(const Maths::fvec2& gravity, float drag) : gravity(gravity), drag(drag) {}

        [[nodiscard]] const std::vector<BodyHandle>& Bodies() const { return bodies; }
        std::vector<BodyHandle>& Bodies() { return bodies; }

        [[nodiscard]] usize BodyCount() const { return bodies.size(); }
        void Reserve(usize size) { bodies.reserve(size); }
        void ReserveExtra(usize size) { bodies.reserve(BodyCount() + size); }
        OPENGL_API void Clear();

        OPENGL_API Body* CreateBody(const BodyCreateOptions& options, const Shape& shape);
        template <class S, class... Rs> Body* CreateBody(const BodyCreateOptions& options, Rs&&... args) {
            return this->CreateBodyWithHeap(options, std::make_unique<S>(std::forward<Rs>(args)...));
        }

        OPENGL_API void Update(float dt, int maxCollisionSteps);
        OPENGL_API void Update(float dt, int simUpdates, int maxCollisionSteps);
    };

    inline float Drag(float drag, float fps) {
        return std::pow(drag, fps);
    }
} // Physics
