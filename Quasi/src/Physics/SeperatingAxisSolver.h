#pragma once
#include "PhysicsTransform2D.h"
#include "Ref.h"

namespace Quasi::Physics2D {
    class Shape;

    class SeperatingAxisSolver {
        Ref<const Shape> base, target;
        PhysicsTransform baseXf, targetXf;
    public:
        enum class Subject { BASE = 0, TARGET = 1 };
        enum class CheckMode { OVERLAP, COLLISION };
        using enum Subject;
        using enum CheckMode;
    private:
        CheckMode checkMode = COLLISION;
        Subject currentChecked = BASE;
        bool collides = true;

        Math::fVector2 seperatingAxis;
        float overlap = INFINITY;
        u32 axisIndex = 0;


        SeperatingAxisSolver(const Shape& s1, const PhysicsTransform& xf1,
                             const Shape& s2, const PhysicsTransform& xf2, CheckMode mode) :
            base(s1), target(s2), baseXf(xf1), targetXf(xf2), checkMode(mode) {}
    public:
        static SeperatingAxisSolver CheckOverlapFor(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);
        static SeperatingAxisSolver CheckCollisionFor(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);

        [[nodiscard]] const Shape& CurrentlyCheckedShape() const { return currentChecked == BASE ? base : target; }
        [[nodiscard]] const PhysicsTransform& CurrentlyCheckedTransform() const { return currentChecked == BASE ? baseXf : targetXf; }

        void SetCheckFor(Subject s);
        bool CheckAxisFor(Subject s);
        bool CheckAxis(const Math::fVector2& axis);
        [[nodiscard]] bool Collides() const { return collides; }

        void Finish();

        [[nodiscard]] float GetDepth() const;
        [[nodiscard]] const Math::fVector2& GetSepAxis() const;

        friend struct Manifold;
    };
} // Quasi
