#pragma once
#include <vector>

#include "Body2D.h"

namespace Quasi::Physics2D {
    class World {
    public:
        Vec<Body> bodies;
        Vec<u32> bodyIndicesSorted;

        Math::fVector2 gravity;
    private:
        void ClearWithoutUpdate() { bodies.clear(); bodyIndicesSorted.clear(); }
        [[nodiscard]] const Body& BodyAt(u32 i) const { return bodies[i]; }
        Body& BodyAt(u32 i) { return bodies[i]; }
    public:
        World() = default;
        World(const Math::fVector2& gravity) : gravity(gravity) {}

        [[nodiscard]] usize BodyCount() const { return bodies.size(); }
        void Reserve(usize size) { bodies.reserve(size); }
        void ReserveExtra(usize size) { bodies.reserve(BodyCount() + size); }
        void Clear();

        BodyHandle CreateBody(const BodyCreateOptions& options, Shape shape);
        template <class S, class... Rs> BodyHandle CreateBody(const BodyCreateOptions& options, Rs&&... args) {
            return this->CreateBody(options, S(std::forward<Rs>(args)...));
        }

        void Update(float dt);
        void Update(float dt, int simUpdates);

        friend struct BodyHandle;
    };
} // Physics
