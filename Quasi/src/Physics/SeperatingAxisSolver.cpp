#include "SeperatingAxisSolver.h"

#include "Shape2D.h"

namespace Quasi::Physics2D {
    SeperatingAxisSolver SeperatingAxisSolver::CheckOverlapFor  (const TransformedShape& s1, const TransformedShape& s2) {
        return { s1, s2, OVERLAP };
    }

    SeperatingAxisSolver SeperatingAxisSolver::CheckCollisionFor(const TransformedShape& s1, const TransformedShape& s2) {
        return { s1, s2, COLLISION };
    }

    Ref<const TransformedShape> SeperatingAxisSolver::CurrentlyCheckedShape() const {
        return currentChecked == BASE   ? Refer(base)   :
               currentChecked == TARGET ? Refer(target) :
               nullptr;
    }

    bool SeperatingAxisSolver::CheckAxisFor(Subject s) {
        SetCheckFor(s);
        return CurrentlyCheckedShape()->AddSeperatingAxes(*this);
    }

    void SeperatingAxisSolver::SetCheckFor(Subject s) {
        currentChecked = s;
        axisIndex = 0;
    }

    bool SeperatingAxisSolver::CheckAxis(const Math::fVector2& axis) {
        ++axisIndex;
        if (!collides) return false;

        const Math::fRange bproj = currentChecked == BASE   ? base  ->ProjectOntoOwnAxis(axisIndex - 1, axis) : base  ->ProjectOntoAxis(axis),
                           tproj = currentChecked == TARGET ? target->ProjectOntoOwnAxis(axisIndex - 1, axis) : target->ProjectOntoAxis(axis);

        const float d1 = bproj.max - tproj.min, d2 = tproj.max - bproj.min,
                    depth = std::min(d1, d2);

        if (depth <= 0) {
            collides = false;
            return false;
        }
        if (checkMode == OVERLAP) return false;

        if (depth < overlap) {
            overlap = depth;
            seperatingAxis = d1 > d2 ? -axis : axis;
            return true;
        }
        return false;
    }

    float SeperatingAxisSolver::GetDepth() const {
        return overlap;
    }

    const Math::fVector2& SeperatingAxisSolver::GetSepAxis() const {
        return seperatingAxis;
    }
} // Quasi