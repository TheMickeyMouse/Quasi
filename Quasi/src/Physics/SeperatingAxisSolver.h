#pragma once
#include "PhysicsTransform2D.h"
#include "Ref.h"
#include "Vector.h"

namespace Quasi::Physics2D {
    class Shape;
    
    class SeperatingAxisSolver {
        Ref<const Shape> base, target;
        Ref<const PhysicsTransform> baseXf, targetXf;
    public:
        enum class Subject   { BASE = 0, TARGET = 1, NEITHER };
        enum class CheckMode { OVERLAP, COLLISION };
        using enum Subject;
        using enum CheckMode;
    private:
        CheckMode checkMode = COLLISION;
        Subject currentChecked = BASE;
        bool collides = true;

        fVector2 seperatingAxis;
        float overlap = INFINITY;
        u32 axisIndex = 0;


        SeperatingAxisSolver(const Shape& s1, const PhysicsTransform& xf1,
                             const Shape& s2, const PhysicsTransform& xf2, CheckMode mode) :
            base(s1), target(s2), baseXf(xf1), targetXf(xf2), checkMode(mode) {}
    public:
        static SeperatingAxisSolver CheckOverlapFor  (const Shape& s1, const PhysicsTransform& xf1,
                                                      const Shape& s2, const PhysicsTransform& xf2);
        static SeperatingAxisSolver CheckCollisionFor(const Shape& s1, const PhysicsTransform& xf1,
                                                      const Shape& s2, const PhysicsTransform& xf2);

        [[nodiscard]] Ref<const Shape>            CurrentlyCheckedShape() const;
        [[nodiscard]] Ref<const PhysicsTransform> CurrentlyCheckedTransform() const;

        void SetCheckFor(Subject s);
        bool CheckAxisFor(Subject s);
        bool CheckAxis(const fVector2& axis);
        [[nodiscard]] bool IsChecking(Subject subject) const;
        [[nodiscard]] bool Collides() const { return collides; }

        [[nodiscard]] float GetDepth() const;
        [[nodiscard]] const fVector2& GetSepAxis() const;

        friend struct Manifold;
    };
} // Quasi
