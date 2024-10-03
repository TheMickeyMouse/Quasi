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

        const auto cmpr = [](const Body& p) { return p.boundingBox.min.x; };

        for (int i = 1; i < bodies.size(); ++i) {
            for (int j = i - 1; j >= 0; --j) {
                // this is different than lt ('<') because nan always returns false
                const float curr = cmpr(bodies[bodyIndicesSorted[j]]), next = cmpr(bodies[bodyIndicesSorted[j + 1]]);
                if (std::isnan(next) || (!std::isnan(curr) && curr < next)) break;
                std::swap(bodyIndicesSorted[j], bodyIndicesSorted[j + 1]);
            }
        }

        // std::ranges::sort(bodyIndicesSorted, [&](u32 i, u32 j) { return cmpr(bodies[i]) < cmpr(bodies[j]); });

        // sweep impl
        // std::vector<std::tuple<Body*, Body*, Collision::Event>> collisionPairs;
        Vec<u32> active;
        for (u32 i : bodyIndicesSorted) {
            Body& b = BodyAt(i);
            if (!b.enabled) continue;
            const float min = b.boundingBox.min.x;
            for (u32 j = 0; j < active.size();) {
                Body& c = BodyAt(active[j]);
                if (c.boundingBox.max.x > min) {
                    const bool bDyn = b.IsDynamic(), cDyn = c.IsDynamic();
                    if ((bDyn || cDyn) && c.boundingBox.yrange().overlaps(b.boundingBox.yrange())) {
                        const Manifold manifold = b.CollideWith(c);
                        if (manifold.contactCount && std::max(manifold.contactDepth[0], manifold.contactDepth[1]) > Math::EPSILON) {
                            StaticResolve(b, c, manifold);
                            DynamicResolve(b, c, manifold);
                            if (bDyn) b.UpdateTransformShape();
                            if (cDyn) c.UpdateTransformShape();
                        }
                    }
                    ++j;
                } else {
                    std::swap(active[j], active.back());
                    active.pop_back();
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