#include "World2D.h"

#include <algorithm>

namespace Quasi::Physics2D {
    World::~World() {
        Span<Body> bodiesTemp = bodies.LeakSpan();
        for (u32 i = 0; i < bodiesTemp.Length(); ++i) {
            if (!BodyIsValid(i)) continue;
            Memory::DestructAt(&bodiesTemp[i]);
        }
        Memory::FreeNoDestruct(bodiesTemp.Data());
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
        w.bodies            = bodies.Clone();
        w.bodySparseEnabled = bodySparseEnabled.Clone();
        w.bodyIndicesSorted = bodyIndicesSorted.Clone();
        w.bodyCount         = bodyCount;
        w.gravity           = gravity;
        return w;
    }

    void World::SortBodyIndices() {
        for (int i = 1; i < bodyIndicesSorted.Length(); ++i) {
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

        while (bodyIndicesSorted.Last() == ~0)
            bodyIndicesSorted.Pop();
    }

    void World::Reserve(usize size) {
        bodies.Reserve(size);
        bodySparseEnabled.Reserve((size + BITS_IN_USIZE - 1) / BITS_IN_USIZE);
        bodyIndicesSorted.Reserve(size);
    }

    void World::Clear() {
        bodies.Clear();
        bodyCount = 0;
        bodySparseEnabled.Clear();
        bodyIndicesSorted.Clear();
    }

    u32 World::FindVacantIndex() const {
        if (!bodySparseEnabled) return 0;
        for (u32 i = 0; i < bodySparseEnabled.Length(); ++i) {
            if (bodySparseEnabled[i] == usizes::MAX) continue;
            return i * BITS_IN_USIZE + std::countr_one(bodySparseEnabled[i]);
        }
        return bodySparseEnabled.Length() * BITS_IN_USIZE;
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
        if (i >= bodySparseEnabled.Length() * BITS_IN_USIZE) {
            bodySparseEnabled.Push(1);
            bodies.Push(std::move(b));
            bodies.Last().sortedIndex = bodyIndicesSorted.Length();
            bodyIndicesSorted.Push(bodies.Length() - 1);
        } else {
            bodySparseEnabled[i / BITS_IN_USIZE] |= (usize)1 << (i % BITS_IN_USIZE);
            if (i >= bodies.Length()) {
                bodies.Push(std::move(b));
            } else
                Memory::ConstructAt(&bodies[i], std::move(b));
            bodies[i].sortedIndex = bodyIndicesSorted.Length();
            bodyIndicesSorted.Push(i);
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
        for (u32 i = 0; i < bodies.Length(); ++i) {
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
            for (u32 j = 0; j < active.Length();) {
                Body& c = BodyDirectAt(active[j]);
                if (c.boundingBox.max.x > min) {
                    const bool bDyn = b.IsDynamic(), cDyn = c.IsDynamic();
                    if ((bDyn || cDyn) && c.boundingBox.yrange().overlaps(b.boundingBox.yrange())) {
                        const Manifold manifold = b.CollideWith(c);
                        if (manifold.contactCount && std::max(manifold.contactDepth[0], manifold.contactDepth[1]) > f32s::EPSILON) {
                            StaticResolve(b, c, manifold);
                            DynamicResolve(b, c, manifold);
                            if (bDyn) b.TryUpdateTransforms();
                            if (cDyn) c.TryUpdateTransforms();
                        }
                    }
                    ++j;
                } else {
                    active.PopUnordered(j);
                }
            }
            active.Push(i);
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
        return BodyIsValid(i) ? OptRefs::SomeRef(bodies[i]) : nullptr;
    }

    bool World::BodyIsValid(usize i) const {
        return bodySparseEnabled[i / BITS_IN_USIZE] & ((usize)1 << i % BITS_IN_USIZE);
    }
} // Physics