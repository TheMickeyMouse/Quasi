#pragma once
#include <vector>

#include "Body2D.h"

namespace Quasi::Physics2D {
    class World {
    public:
        Vec<Box<Body>> bodies;
        fVector2 gravity;
    public:
        World() = default;
        World(const fVector2& gravity) : gravity(gravity) {}
    public:
        usize BodyCount() const { return bodies.Length(); }
        void Reserve(usize size);
        void Clear();

        Body& CreateBody(const BodyCreateOptions& options, Shape shape);
        template <class S, class... Rs> Body& CreateBody(const BodyCreateOptions& options, Rs&&... args) {
            return this->CreateBody(options, S(std::forward<Rs>(args)...));
        }
        void DeleteBody(usize i);
        void DeleteBody(Ref<Body> body);

        void Update(float dt);
        void Update(float dt, int simUpdates);

        OptRef<Body> BodyAt(usize i);
        OptRef<const Body> BodyAt(usize i) const;

        friend struct BodyHandle;
    };
} // Physics
