#pragma once
#include "IShape2D.h"
#include "Vector.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;

    struct Manifold {
        fVector2 seperatingNormal;
        fVector2 contactPoint[2];
        float contactDepth[2];
        u32 contactCount = 0;

        static Manifold None();

        static Manifold From(const SeperatingAxisSolver& sat);

        static Manifold Clip(const fVector2& v0, const fVector2& v1,
                             const fVector2& normal, float threshold);

        void Invert();
        static Manifold Flip(Manifold&& m);
        void AddPoint(const fVector2& point, float depth = 0);
    };
} // Quasi
