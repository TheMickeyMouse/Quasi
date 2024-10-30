#include "Manifold2D.h"

#include "SeperatingAxisSolver.h"
#include "IShape2D.h"
#include "Shape2D.h"

namespace Quasi::Physics2D {
    Manifold Manifold::None() {
        return {
            .contactCount = 0
        };
    }

    Manifold Manifold::From(const SeperatingAxisSolver& sat) {
        const Shape&            base = sat.base, & target = sat.target;
        const PhysicsTransform& bXf  = sat.baseXf, tXf    = sat.targetXf;
        const fVector2& n = sat.seperatingAxis;

        const fLine2D baseClips   = bXf.TransformLine(base  .BestEdgeFor(bXf.TransformInverseOffset(n))),
                      targetClips = tXf.TransformLine(target.BestEdgeFor(tXf.TransformInverseOffset(-n)));

        const bool flip = std::abs(baseClips.forward().dot(n)) > std::abs(targetClips.forward().dot(n));
        const fLine2D& ref = flip ? targetClips : baseClips, &inc = flip ? baseClips : targetClips;

        const fVector2 refFwd = ref.forward().norm();

        Manifold manifold = Clip(inc.start, inc.end, refFwd, refFwd.dot(ref.start));

        manifold = Clip(manifold.contactPoint[0], manifold.contactPoint[1], -refFwd, -refFwd.dot(ref.end));

        Manifold result = None();
        result.seperatingNormal = n;
        const fVector2 refNorm = flip ? n : -n;
        const float max = ref.start.dot(refNorm);

        if (const float d = refNorm.dot(manifold.contactPoint[0]) - max; d > 0) {
            result.AddPoint(manifold.contactPoint[0], d);
        }
        if (const float d = refNorm.dot(manifold.contactPoint[1]) - max; d > 0) {
            result.AddPoint(manifold.contactPoint[1], d);
        }
        // return the valid points
        return result;
    }

    Manifold Manifold::Clip(const fVector2& v0, const fVector2& v1, const fVector2& normal, float threshold) {
        const float d0 = v0.dot(normal) - threshold, d1 = v1.dot(normal) - threshold;

        Manifold manifold {};

        if (d0 >= 0.0f) // correct sides
            manifold.AddPoint(v0);
        if (d1 >= 0.0f) // correct sides
            manifold.AddPoint(v1);

        if (d0 * d1 < 0.0f) { // different sides
            manifold.AddPoint(fLine2D { v0, v1 }.lerp(d0 / (d0 - d1)));
        }

        return manifold;
    }

    void Manifold::Invert() {
        seperatingNormal = -seperatingNormal;
    }

    Manifold Manifold::Flip(Manifold&& m) {
        m.Invert();
        return m;
    }

    void Manifold::AddPoint(const fVector2& point, float depth) {
        contactPoint[contactCount] = point;
        contactDepth[contactCount] = depth;
        ++contactCount;
    }
} // Quasi