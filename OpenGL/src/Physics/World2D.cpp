#include "World2D.h"

#include <list>

#include "sorted_vector.h"

namespace Physics2D {
    void World::Clear() {
        for (const BodyHandle& body : bodies)
            body->world = nullptr;
        ClearWithoutUpdate();
    }

    Body* World::CreateBodyWithHeap(const BodyCreateOptions& options, std::unique_ptr<Shape>&& heapAllocShape) {
        const float area = heapAllocShape->ComputeArea();
        std::unique_ptr body = std::make_unique<Body>(
            options.position,
            area * options.density,
            options.type,
            this,
            std::move(heapAllocShape)
        );
        return bodies.emplace_back(std::move(body)).get();
    }

    Body* World::CreateBody(const BodyCreateOptions& options, const Shape& shape) {
        return CreateBodyWithHeap(options, std::unique_ptr<Shape>(shape.CloneHeap()));
    }

    void World::Update(float dt, int maxCollisionSteps) {
        dt /= (float)maxCollisionSteps;
        for (int t = 0; t < maxCollisionSteps; ++t) {
            for (const BodyHandle& b : bodies) {
                b->velocity *= std::pow(drag, dt);
                if (b->type == BodyType::DYNAMIC)
                    b->velocity += gravity * dt;
                b->Update(dt);
            }

            const auto cmpr = [](const Body& p) { return p.ComputeBoundingBox().min.x; };

            for (int i = 1; i < bodies.size(); ++i) {
                for (int j = i - 1; j >= 0; --j) {
                    // this is different than lt ('<') because nan always returns false
                    if (!(cmpr(*bodies[j]) >= cmpr(*bodies[j + 1]))) break;
                    std::swap(bodies[j], bodies[j + 1]);
                }
            }

            // std::ranges::sort(bodies, [&](const BodyHandle& a, const BodyHandle& b) { return cmpr(*a) < cmpr(*b); });

            // sweep impl
            // std::vector<std::tuple<Body*, Body*, Collision::Event>> collisionPairs;
            std::vector<usize> active;
            for (usize i = 0; i < bodies.size(); ++i) {
                Body* b = bodies[i].get();
                const float min = b->ComputeBoundingBox().min.x;
                for (auto actIt = active.begin(); actIt != active.end();) {
                    Body* c = bodies[*actIt].get();
                    if (c->ComputeBoundingBox().max.x > min) {
                        const Collision::Event event = b->CollidesWith(*c);
                        if (event && !event.Seperator().iszero()) {
                            // collisionPairs.emplace_back(b, c, event);
                            StaticResolve (*b, *c, event);
                            DynamicResolve(*b, *c, event);
                        }
                        ++actIt;
                    } else {
                        actIt = active.erase(actIt);
                    }
                }
                active.emplace_back(i);
            }

            // for (const auto& [base, target, event] : collisionPairs) {
            //     StaticResolve (*base, *target, event);
            //     DynamicResolve(*base, *target, event);
            // }

            // for (uint i = 0; i < BodyCount(); ++i) {
            //     Body& base = bodies[i];
            //     Maths::fvec2 prevPosition = base.position, prevVelocity = base.velocity;
            //     for (uint j = i + 1; j < BodyCount(); ++j) {
            //         Body& target = bodies[j];
            //         if (const Collision::Event event = base.CollidesWith(target)) {
            //             if (event.Seperator().iszero()) continue;
            //             StaticResolve (base, target, event);
            //             DynamicResolve(base, target, event);
            //             if (outCollisions) outCollisions->emplace_back(event);
            //         }
            //     }
            //     base.simTime -= (prevPosition - base.position).len() / prevVelocity.len();
            // }
        }
    }

    void World::Update(float dt, int simUpdates, int maxCollisionSteps) {
        for (int i = 0; i < simUpdates; ++i) {
            Update(dt / (float)simUpdates, maxCollisionSteps);
        }
    }
} // Physics