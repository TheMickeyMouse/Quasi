#include "SeperatingAxisSolver.h"

#include "Shape2D.h"

namespace Quasi::Physics2D {
    SeperatingAxisSolver SeperatingAxisSolver::CheckOverlapFor(
        const Shape& s1, const PhysicsTransform& xf1,
        const Shape& s2, const PhysicsTransform& xf2) {
        return { s1, xf1, s2, xf2, OVERLAP };
    }

    SeperatingAxisSolver SeperatingAxisSolver::CheckCollisionFor(
        const Shape& s1, const PhysicsTransform& xf1,
        const Shape& s2, const PhysicsTransform& xf2) {
        return { s1, xf1, s2, xf2, COLLISION };
    }

    Ref<const Shape> SeperatingAxisSolver::CurrentlyCheckedShape() const {
        return currentChecked == BASE   ? Refer(base)   :
               currentChecked == TARGET ? Refer(target) :
               nullptr;
    }

    Ref<const PhysicsTransform> SeperatingAxisSolver::CurrentlyCheckedTransform() const {
        return currentChecked == BASE   ? Refer(baseXf)   :
               currentChecked == TARGET ? Refer(targetXf) :
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

    bool SeperatingAxisSolver::CheckAxis(const fVector2& axis) {
        ++axisIndex;
        if (!collides) return false;

        const fVector2 worldAxis = IsChecking(BASE)   ? baseXf  ->TransformOffset(axis) :
                                   IsChecking(TARGET) ? targetXf->TransformOffset(axis) :
                                   axis;

        const fRange bproj = (IsChecking(BASE) ?
                             base->ProjectOntoOwnAxis(axisIndex - 1, axis) :
                             base->ProjectOntoAxis(baseXf->TransformInverseOffset(worldAxis)))
                           + baseXf->position.dot(worldAxis),
                     tproj = (IsChecking(TARGET) ?
                             target->ProjectOntoOwnAxis(axisIndex - 1, axis) :
                             target->ProjectOntoAxis(targetXf->TransformInverseOffset(worldAxis)))
                           + targetXf->position.dot(worldAxis);

        const float d1 = bproj.max - tproj.min, d2 = tproj.max - bproj.min,
                    depth = std::min(d1, d2);

        if (depth <= 0) {
            collides = false;
            return false;
        }
        if (checkMode == OVERLAP) return false;

        if (depth < overlap) {
            overlap = depth;
            seperatingAxis = d1 > d2 ? -worldAxis : worldAxis;
            return true;
        }
        return false;
    }

    bool SeperatingAxisSolver::IsChecking(Subject subject) const {
        return currentChecked == subject;
    }

    float SeperatingAxisSolver::GetDepth() const {
        return overlap;
    }

    const fVector2& SeperatingAxisSolver::GetSepAxis() const {
        return seperatingAxis;
    }
} // Quasi