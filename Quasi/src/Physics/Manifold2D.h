#pragma once
#include "Shape2D.h"
#include "Vector.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;

    struct Manifold {
        Math::fVector2 seperatingNormal;
        Math::fVector2 contactPoint[2];
        float contactDepth[2];
        u32 contactCount = 0;

        static Manifold None();

        static Manifold From(const SeperatingAxisSolver& sat);

        static Manifold Clip(const Math::fVector2& v0, const Math::fVector2& v1,
                             const Math::fVector2& normal, float threshold);

        void Invert();
        static Manifold Flip(Manifold&& m);
        void AddPoint(const Math::fVector2& point, float depth = 0);
    };
} // Quasi
