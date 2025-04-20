#pragma once
#include <vector>

#include "Body2D.h"

namespace Quasi::Physics2D {
    class World {
    public:
        Vec<Body> bodies;
        Vec<usize> bodySweptIndices;
        fVector2 gravity;
    public:
        World() = default;
        World(const fVector2& gravity) : gravity(gravity) {}
    public:
        usize BodyCount() const { return bodies.Length(); }
        void Reserve(usize size);
        void Clear();

        BodyHandle CreateBody(const BodyCreateOptions& options, Shape shape);
        template <class S, class... Rs> BodyHandle CreateBody(const BodyCreateOptions& options, Rs&&... args) {
            return this->CreateBody(options, S(std::forward<Rs>(args)...));
        }
        void DeleteBody(usize i);

        void Update(float dt);
        void Update(float dt, int simUpdates);

        OptRef<Body> BodyAt(usize i);
        OptRef<const Body> BodyAt(usize i) const;

        friend struct BodyHandle;
    };
} // Physics
