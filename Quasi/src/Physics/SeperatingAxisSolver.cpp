#include "SeperatingAxisSolver.h"

#include "Shape2D.h"

namespace Quasi::Physics2D {
    SeperatingAxisSolver SeperatingAxisSolver::CheckOverlapFor(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        return { s1, xf1, s2, xf2, OVERLAP };
    }

    SeperatingAxisSolver SeperatingAxisSolver::CheckCollisionFor(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        return { s1, xf1, s2, xf2, COLLISION };
    }

    bool SeperatingAxisSolver::CheckAxisFor(Subject s) {
        SetCheckFor(s);
        return CurrentlyCheckedShape().AddSeperatingAxes(*this, CurrentlyCheckedTransform());
    }

    void SeperatingAxisSolver::SetCheckFor(Subject s) {
        currentChecked = s;
        axisIndex = 0;
    }

    bool SeperatingAxisSolver::CheckAxis(const Math::fVector2& axis) {
        ++axisIndex;
        if (!collides) return false;

        const Math::fVector2 pAxis = axis.norm();
        const Math::fRange bproj = currentChecked == BASE   ? base  ->ProjectOntoOwnAxis(axisIndex - 1, pAxis, baseXf)   : base  ->ProjectOntoAxis(pAxis, baseXf),
                           tproj = currentChecked == TARGET ? target->ProjectOntoOwnAxis(axisIndex - 1, pAxis, targetXf) : target->ProjectOntoAxis(pAxis, targetXf);
        if (!bproj.overlaps(tproj)) {
            collides = false;
            return false;
        }

        if (checkMode == OVERLAP) return false;

        const float depth = std::min(bproj.max - tproj.min, tproj.max - bproj.min);
        if (depth < overlap) {
            overlap = depth;
            seperatingAxis = pAxis;
            return true;
        }
        return false;
    }

    void SeperatingAxisSolver::Finish() {
        if (checkMode == OVERLAP) return;

        if ((targetXf * target->CenterOfMass() - baseXf * base->CenterOfMass()).dot(seperatingAxis) < 0.0f) {
            seperatingAxis = -seperatingAxis;
        }
    }

    float SeperatingAxisSolver::GetDepth() const {
        return overlap;
    }

    const Math::fVector2& SeperatingAxisSolver::GetSepAxis() const {
        return seperatingAxis;
    }
} // Quasi