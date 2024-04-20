#include "World2D.h"

namespace Physics2D {
    void World::Clear() {
        for (Body& body : bodies)
            body.world = nullptr;
        ClearWithoutUpdate();
    }

    Body* World::CreateBodyWithHeap(const BodyCreateOptions& options, std::unique_ptr<Shape>&& heapAllocShape) {
        const float area = heapAllocShape->ComputeArea();
        Body body {
            options.position,
            area * options.density,
            options.type,
            this,
            std::move(heapAllocShape),
        };
        bodies.emplace_back(std::move(body));
        return &bodies.back();
    }

    Body* World::CreateBody(const BodyCreateOptions& options, const Shape& shape) {
        return CreateBodyWithHeap(options, std::unique_ptr<Shape>(shape.CloneHeap()));
    }

    void World::Update(float dt, int maxCollisionSteps, std::vector<Collision::Pair>* outCollisions /* may be null */) {
        for (Body& b : bodies) b.simTime = dt;


        for (int t = 0; t < maxCollisionSteps; ++t) {
            for (Body& b : bodies) {
                if (b.simTime <= 0 || std::isnan(b.simTime)) continue;

                b.velocity *= (1.0f - drag * b.simTime / (float)maxCollisionSteps);
                b.Update(b.simTime);
            }

            for (uint i = 0; i < BodyCount(); ++i) {
                Body& base = bodies[i];
                Maths::fvec2 prevPosition = base.position, prevVelocity = base.velocity;
                for (uint j = i + 1; j < BodyCount(); ++j) {
                    Body& target = bodies[j];
                    if (const Collision::Event event = base.CollidesWith(target)) {
                        if (event.Seperator().iszero()) continue;
                        StaticResolve (base, target, event);
                        DynamicResolve(base, target, event);
                        if (outCollisions) outCollisions->emplace_back(base, target);
                    }
                }
                base.simTime -= (prevPosition - base.position).len() / prevVelocity.len();
            }
        }
    }

    void World::Update(float dt, int simUpdates, int maxCollisionSteps, std::vector<Collision::Pair>* outCollisions) {
        for (int i = 0; i < simUpdates; ++i) {
            Update(dt / (float)simUpdates, maxCollisionSteps, outCollisions);
        }
    }
} // Physics