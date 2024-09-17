#pragma once
#include "PhysicsTransform2D.h"
#include "Ref.h"

namespace Quasi::Physics2D {
    class TransformedShape;

    class SeperatingAxisSolver {
        Ref<const TransformedShape> base, target;
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


        SeperatingAxisSolver(const TransformedShape& s1,
                             const TransformedShape& s2, CheckMode mode) :
            base(s1), target(s2), checkMode(mode) {}
    public:
        static SeperatingAxisSolver CheckOverlapFor  (const TransformedShape& s1, const TransformedShape& s2);
        static SeperatingAxisSolver CheckCollisionFor(const TransformedShape& s1, const TransformedShape& s2);

        [[nodiscard]] const TransformedShape& CurrentlyCheckedShape() const { return currentChecked == BASE ? base : target; }

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
