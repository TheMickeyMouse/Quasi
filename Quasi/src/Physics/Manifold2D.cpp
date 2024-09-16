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
        const Shape& base = sat.base, &target = sat.target;
        const PhysicsTransform& baseXf = sat.baseXf, &targetXf = sat.targetXf;
        const Math::fVector2& n = sat.seperatingAxis;

        const Math::fLine2D baseClips   = base  .BestEdgeFor(n,  baseXf),
                            targetClips = target.BestEdgeFor(-n, targetXf);

        const bool flip = std::abs(baseClips.forward().dot(n)) > std::abs(targetClips.forward().dot(n));
        const Math::fLine2D& ref = flip ? targetClips : baseClips, &inc = flip ? baseClips : targetClips;

        const Math::fVector2 refFwd = ref.forward().norm();

        Manifold manifold = Clip(inc.start, inc.end, refFwd, refFwd.dot(ref.start));
        if (manifold.contactCount < 2) return None();

        manifold = Clip(manifold.contactPoint[0], manifold.contactPoint[1], -refFwd, -refFwd.dot(ref.end));
        if (manifold.contactCount < 2) return None();

        Manifold result = None();
        result.seperatingNormal = n;
        Math::fVector2 refNorm = refFwd.perpend();
        if ((refNorm.dot(n) < 0.0f) ^ !flip) refNorm *= -1.0f;
        const float max = ref.start.dot(refNorm);

        if (const float d = refNorm.dot(manifold.contactPoint[0]) - max; d >= 0) {
            result.AddPoint(manifold.contactPoint[0], d);
        }
        if (const float d = refNorm.dot(manifold.contactPoint[1]) - max; d >= 0) {
            result.AddPoint(manifold.contactPoint[1], d);
        }
        // return the valid points
        return result;
    }

    Manifold Manifold::Clip(const Math::fVector2& v0, const Math::fVector2& v1, const Math::fVector2& normal, float threshold) {
        const float d0 = v0.dot(normal) - threshold, d1 = v1.dot(normal) - threshold;

        Manifold manifold {};

        if (d0 >= 0.0f) // correct sides
            manifold.AddPoint(v0);
        if (d1 >= 0.0f) // correct sides
            manifold.AddPoint(v1);

        if (d0 * d1 < 0.0f) { // different sides
            manifold.AddPoint(Math::fLine2D { v0, v1 }.lerp(d0 / (d0 - d1)));
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

    void Manifold::AddPoint(const Math::fVector2& point, float depth) {
        contactPoint[contactCount] = point;
        contactDepth[contactCount] = depth;
        ++contactCount;
    }
} // Quasi