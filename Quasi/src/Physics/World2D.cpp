#include "World2D.h"

#include <algorithm>

namespace Quasi::Physics2D {
    void World::Clear() {
        for (auto& body : bodies)
            body.world = nullptr;
        ClearWithoutUpdate();
    }

    BodyHandle World::CreateBody(const BodyCreateOptions& options, Shape shape) {
        const float area = shape.ComputeArea();
        bodies.emplace_back(
            options.position,
            area * options.density,
            options.restitution,
            options.type,
            *this,
            std::move(shape)
        );
        bodyIndicesSorted.emplace_back(bodies.size() - 1);
        return BodyHandle::At(*this, bodies.size() - 1);
    }

    void World::Update(float dt) {
        for (auto& b : bodies) {
            if (!b.enabled) continue;

            if (b.type == BodyType::DYNAMIC)
                b.velocity += gravity * dt;
            b.Update(dt);
        }

        const auto cmpr = [](const Body& p) { return p.ComputeBoundingBox().min.x; };

        // for (int i = 1; i < bodies.size(); ++i) {
        //     for (int j = i - 1; j >= 0; --j) {
        //         // this is different than lt ('<') because nan always returns false
        //         const float curr = cmpr(bodies[j]), next = cmpr(bodies[j + 1]);
        //         if (std::isnan(next) || (!std::isnan(curr) && curr < next)) break;
        //         std::swap(bodies[j], bodies[j + 1]);
        //     }
        // }

        std::ranges::sort(bodyIndicesSorted, [&](u32 i, u32 j) { return cmpr(bodies[i]) < cmpr(bodies[j]); });

        // sweep impl
        // std::vector<std::tuple<Body*, Body*, Collision::Event>> collisionPairs;
        Vec<u32> active;
        for (u32 i = 0; i < bodyIndicesSorted.size(); ++i) {
            Body& b = BodyAt(bodyIndicesSorted[i]);
            if (!b.enabled) continue;
            const float min = b.ComputeBoundingBox().min.x;
            for (auto actIt = active.begin(); actIt != active.end();) {
                Body& c = BodyAt(bodyIndicesSorted[*actIt]);
                if (c.ComputeBoundingBox().max.x > min) {
#if 0
                    const Collision::Event event = b->CollideWith(c);
                    if (event && event.Valid()) {
                        // collisionPairs.emplace_back(b, c, event);
                        StaticResolve (b, c, event);
                        DynamicResolve(b, c, event);
                    }
#endif
                    const Manifold manifold = b.CollideWith(c);
                    if (manifold.contactCount) {
                        StaticResolve(b, c, manifold);
                        DynamicResolve(b, c, manifold);
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