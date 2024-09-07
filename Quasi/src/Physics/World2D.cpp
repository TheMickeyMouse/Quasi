#include "World2D.h"

#include <algorithm>

namespace Quasi::Physics2D {
    void World::Clear() {
        for (auto body : bodies)
            body->world = nullptr;
        ClearWithoutUpdate();
    }

    Ref<Body> World::CreateBodyWithHeap(const BodyCreateOptions& options, Ref<Shape> heapAllocShape) {
        const float area = heapAllocShape->ComputeArea();
        RefImpl body = *allocator.Create<Body>(
            options.position,
            area * options.density,
            options.type,
            *this,
            heapAllocShape
        );
        return bodies.emplace_back(body);
    }

    Ref<Body> World::CreateBody(const BodyCreateOptions& options, const Shape& shape) {
        return CreateBodyWithHeap(options, shape.CloneOn(allocator));
    }

    void World::Update(float dt) {
        for (auto b : bodies) {
            if (!b->enabled) continue;

            if (b->type == BodyType::DYNAMIC)
                b->velocity += gravity * dt;
            b->Update(dt);
        }

        const auto cmpr = [](const Body& p) { return p.ComputeBoundingBox().min.x; };

        for (int i = 1; i < bodies.size(); ++i) {
            for (int j = i - 1; j >= 0; --j) {
                // this is different than lt ('<') because nan always returns false
                const float curr = cmpr(bodies[j]), next = cmpr(bodies[j + 1]);
                if (std::isnan(next) || (!std::isnan(curr) && curr < next)) break;
                std::swap(bodies[j], bodies[j + 1]);
            }
        }

        // std::ranges::sort(bodies, [&](const auto a, const auto b) { return cmpr(*a) < cmpr(*b); });

        // sweep impl
        // std::vector<std::tuple<Body*, Body*, Collision::Event>> collisionPairs;
        Vec<usize> active;
        for (usize i = 0; i < bodies.size(); ++i) {
            Ref<Body> b = bodies[i];
            if (!b->enabled) continue;
            const float min = b->ComputeBoundingBox().min.x;
            for (auto actIt = active.begin(); actIt != active.end();) {
                Ref<Body> c = bodies[*actIt];
                if (c->ComputeBoundingBox().max.x > min) {
#if 0
                    const Collision::Event event = b->CollideWith(c);
                    if (event && event.Valid()) {
                        // collisionPairs.emplace_back(b, c, event);
                        StaticResolve (b, c, event);
                        DynamicResolve(b, c, event);
                    }
#endif
                    const Manifold manifold = b->CollideWith(c);
                    if (manifold.contactCount) {
                        Collision::StaticResolve(b, c, manifold);
                        Collision::DynamicResolve(b, c, manifold);
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
        //     Math::fVector2 prevPosition = base.position, prevVelocity = base.velocity;
        //     for (uint j = i + 1; j < BodyCount(); ++j) {
        //         Body& target = bodies[j];
        //         if (const Collision::Event event = base.CollideWith(target)) {
        //             if (event.Seperator().iszero()) continue;
        //             StaticResolve (base, target, event);
        //             DynamicResolve(base, target, event);
        //             if (outCollisions) outCollisions->emplace_back(event);
        //         }
        //     }
        //     base.simTime -= (prevPosition - base.position).len() / prevVelocity.len();
        // }
    }

    void World::Update(float dt, int simUpdates) {
        for (int i = 0; i < simUpdates; ++i) {
            Update(dt / (float)simUpdates);
        }
    }
} // Physics