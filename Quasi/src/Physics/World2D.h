#pragma once
#include <vector>

#include "ArenaAllocator.h"
#include "Body2D.h"

namespace Quasi::Physics2D {
    class World {
        ArenaAllocator allocator { 8 * 1024 };
    public:
        Vec<Ref<Body>> bodies;
        Math::fVector2 gravity;
    private:
        Ref<Body> CreateBodyWithHeap(const BodyCreateOptions& options, Ref<Shape> heapAllocShape);
        void ClearWithoutUpdate() { bodies.clear(); allocator.Clear(); }
    public:
        World() = default;
        World(const Math::fVector2& gravity) : gravity(gravity) {}

        [[nodiscard]] usize BodyCount() const { return bodies.size(); }
        void Reserve(usize size) { bodies.reserve(size); }
        void ReserveExtra(usize size) { bodies.reserve(BodyCount() + size); }
        void Clear();

        Ref<Body> CreateBody(const BodyCreateOptions& options, const Shape& shape);
        template <class S, class... Rs> Ref<Body> CreateBody(const BodyCreateOptions& options, Rs&&... args) {
            return this->CreateBodyWithHeap(options, *allocator.Create<S>(std::forward<Rs>(args)...));
        }

        void Update(float dt);
        void Update(float dt, int simUpdates);
    };
} // Physics
