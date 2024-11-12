#pragma once
#include <vector>

#include "Body2D.h"

namespace Quasi::Physics2D {
    class World {
    public:
        Vec<Body> bodies;
        Vec<usize> bodySparseEnabled;
        Vec<u32> bodyIndicesSorted;
        u32 bodyCount = 0;
        static constexpr u32 BITS_IN_USIZE = 8 * sizeof(usize);

        fVector2 gravity;
    public:
        World() = default;
        World(const fVector2& gravity) : gravity(gravity) {}
        ~World();
        World(const World& w) = delete;
        World& operator=(const World& w) = delete;
        World(World&& w) noexcept;
        World& operator=(World&& w) noexcept;

        [[nodiscard]] World Clone() const;

    private:
        [[nodiscard]] const Body& BodyDirectAt(u32 i) const { return bodies[i]; }
        Body& BodyDirectAt(u32 i) { return bodies[i]; }
        void SortBodyIndices();
    public:
        [[nodiscard]] usize BodyCount() const { return bodyCount; }
        void Reserve(usize size);
        void Clear();
        [[nodiscard]] u32 FindVacantIndex() const;

        BodyHandle CreateBody(const BodyCreateOptions& options, Shape shape);
        template <class S, class... Rs> BodyHandle CreateBody(const BodyCreateOptions& options, Rs&&... args) {
            return this->CreateBody(options, S(std::forward<Rs>(args)...));
        }
        void DeleteBody(usize i);

        void Update(float dt);
        void Update(float dt, int simUpdates);

        OptRef<Body> BodyAt(usize i);
        [[nodiscard]] OptRef<const Body> BodyAt(usize i) const;
        [[nodiscard]] bool BodyIsValid(usize i) const;

        friend struct BodyHandle;
    };
} // Physics
