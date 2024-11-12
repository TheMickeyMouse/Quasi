#include "World2D.h"

#include <algorithm>

#include "Memory.h"

namespace Quasi::Physics2D {
    World::~World() {
        Span<Body> bodiesTemp = Memory::ReleaseData(std::move(bodies));
        for (u32 i = 0; i < bodiesTemp.size(); ++i) {
            if (!BodyIsValid(i)) continue;
            Memory::DestructAt(&bodiesTemp[i]);
        }
        Memory::FreeNoDestruct(bodiesTemp.data());
    }

    World::World(World&& w) noexcept {
        bodies            = std::move(w.bodies);
        bodySparseEnabled = std::move(w.bodySparseEnabled);
        bodyIndicesSorted = std::move(w.bodyIndicesSorted);
        bodyCount         = w.bodyCount;
        gravity           = w.gravity;
    }

    World& World::operator=(World&& w) noexcept {
        bodies            = std::move(w.bodies);
        bodySparseEnabled = std::move(w.bodySparseEnabled);
        bodyIndicesSorted = std::move(w.bodyIndicesSorted);
        bodyCount         = w.bodyCount;
        gravity           = w.gravity;
        return *this;
    }

    World World::Clone() const {
        World w;
        w.bodies            = bodies;
        w.bodySparseEnabled = bodySparseEnabled;
        w.bodyIndicesSorted = bodyIndicesSorted;
        w.bodyCount         = bodyCount;
        w.gravity           = gravity;
        return w;
    }

    void World::SortBodyIndices() {
        for (int i = 1; i < bodyIndicesSorted.size(); ++i) {
            for (int j = i - 1; j >= 0; --j) {
                // filters out (~0) to the back
                const u32 iCurr = bodyIndicesSorted[j], iNext = bodyIndicesSorted[j + 1];
                if (iNext == ~0 || (iCurr != ~0 &&
                    bodies[iCurr].boundingBox.min.x < bodies[iNext].boundingBox.min.x))
                    break;
                std::swap(bodyIndicesSorted[j], bodyIndicesSorted[j + 1]);
                /* if (iNext != ~0) */ bodies[iNext].sortedIndex = j;
                if (iCurr != ~0) bodies[iCurr].sortedIndex = j + 1;
            }
        }

        while (bodyIndicesSorted.back() == ~0)
            bodyIndicesSorted.pop_back();
    }

    void World::Reserve(usize size) {
        bodies.reserve(size);
        bodySparseEnabled.reserve((size + BITS_IN_USIZE - 1) / BITS_IN_USIZE);
        bodyIndicesSorted.reserve(size);
    }

    void World::Clear() {
        bodies.clear();
        bodyCount = 0;
        bodySparseEnabled.clear();
        bodyIndicesSorted.clear();
    }

    u32 World::FindVacantIndex() const {
        if (bodySparseEnabled.empty()) return 0;
        for (u32 i = 0; i < bodySparseEnabled.size(); ++i) {
            if (bodySparseEnabled[i] == ~(usize)0) continue;
            return i * BITS_IN_USIZE + std::countr_one(bodySparseEnabled[i]);
        }
        return bodySparseEnabled.size() * BITS_IN_USIZE;
    }

    BodyHandle World::CreateBody(const BodyCreateOptions& options, Shape shape) {
        const float area = shape.ComputeArea();
        const u32 i = FindVacantIndex();
        const bool isStatic = options.type == BodyType::STATIC;
        Body b {
            options.position,
            fComplex::rotate(RAD2DEG * options.rotAngle),
            isStatic ? 0 : area * options.density,
            options.type,
            *this,
            std::move(shape)
        };
        if (i >= bodySparseEnabled.size() * BITS_IN_USIZE) {
            bodySparseEnabled.push_back(1);
            bodies.emplace_back(std::move(b));
            bodies.back().sortedIndex = bodyIndicesSorted.size();
            bodyIndicesSorted.push_back(bodies.size() - 1);
        } else {
            bodySparseEnabled[i / BITS_IN_USIZE] |= (usize)1 << (i % BITS_IN_USIZE);
            if (i >= bodies.size()) {
                bodies.emplace_back(std::move(b));
            } else
                Memory::ConstructAt(&bodies[i], std::move(b));
            bodies[i].sortedIndex = bodyIndicesSorted.size();
            bodyIndicesSorted.push_back(i);
        }
        ++bodyCount;
        return BodyHandle::At(*this, i);
    }

    void World::DeleteBody(usize i) {
        if (BodyIsValid(i)) {
            bodySparseEnabled[i / BITS_IN_USIZE] &= ~((usize)1 << i % BITS_IN_USIZE);
            bodyIndicesSorted[bodies[i].sortedIndex] = ~0;
            Memory::DestructAt(&bodies[i]);
            --bodyCount;
        }
    }

    void World::Update(float dt) {
        for (u32 i = 0; i < bodies.size(); ++i) {
            if (!BodyIsValid(i)) continue;
            Body& b = bodies[i];
            if (!b.enabled) continue;

            if (b.type == BodyType::DYNAMIC)
                b.velocity += gravity * dt;
            b.Update(dt);
        }

        SortBodyIndices();
        // std::ranges::sort(bodyIndicesSorted, [&](u32 i, u32 j) { return cmpr(bodies[i]) < cmpr(bodies[j]); });

        // sweep impl
        // std::vector<std::tuple<Body*, Body*, Collision::Event>> collisionPairs;
        Vec<u32> active;
        for (u32 i : bodyIndicesSorted) {
            Body& b = BodyDirectAt(i);
            if (!b.enabled) continue;
            const float min = b.boundingBox.min.x;
            for (u32 j = 0; j < active.size();) {
                Body& c = BodyDirectAt(active[j]);
                if (c.boundingBox.max.x > min) {
                    const bool bDyn = b.IsDynamic(), cDyn = c.IsDynamic();
                    if ((bDyn || cDyn) && c.boundingBox.yrange().overlaps(b.boundingBox.yrange())) {
                        const Manifold manifold = b.CollideWith(c);
                        if (manifold.contactCount && std::max(manifold.contactDepth[0], manifold.contactDepth[1]) > EPSILON) {
                            StaticResolve(b, c, manifold);
                            DynamicResolve(b, c, manifold);
                            if (bDyn) b.TryUpdateTransforms();
                            if (cDyn) c.TryUpdateTransforms();
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
        //     fVector2 prevPosition = base.position, prevVelocity = base.velocity;
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

    OptRef<Body> World::BodyAt(usize i) {
        return QGetterMut$(BodyAt, i);
    }

    OptRef<const Body> World::BodyAt(usize i) const {
        return BodyIsValid(i) ? Refer(bodies[i]) : OptRef<const Body>::None();
    }

    bool World::BodyIsValid(usize i) const {
        return bodySparseEnabled[i / BITS_IN_USIZE] & ((usize)1 << i % BITS_IN_USIZE);
    }
} // Physics