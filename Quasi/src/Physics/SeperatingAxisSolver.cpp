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

    OptRef<const Shape> SeperatingAxisSolver::CurrentlyCheckedShape() const {
        return currentChecked == BASE   ? OptRefs::Some(base)   :
               currentChecked == TARGET ? OptRefs::Some(target) :
               nullptr;
    }

    OptRef<const PhysicsTransform> SeperatingAxisSolver::CurrentlyCheckedTransform() const {
        return currentChecked == BASE   ? OptRefs::Some(baseXf)   :
               currentChecked == TARGET ? OptRefs::Some(targetXf) :
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

    bool SeperatingAxisSolver::CheckAxis(const fv2& axis) {
        ++axisIndex;
        if (!collides) return false;

        const fv2 worldAxis = IsChecking(BASE)   ? baseXf  ->TransformDir(axis) :
                                   IsChecking(TARGET) ? targetXf->TransformDir(axis) :
                                   axis;

        const fRange bproj = (IsChecking(BASE) ?
                             base->ProjectOntoOwnAxis(axisIndex - 1, axis) :
                             base->ProjectOntoAxis(baseXf->TransformInverseDir(worldAxis)))
                           + baseXf->position.Dot(worldAxis),
                     tproj = (IsChecking(TARGET) ?
                             target->ProjectOntoOwnAxis(axisIndex - 1, axis) :
                             target->ProjectOntoAxis(targetXf->TransformInverseDir(worldAxis)))
                           + targetXf->position.Dot(worldAxis);

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

    const fv2& SeperatingAxisSolver::GetSepAxis() const {
        return seperatingAxis;
    }
} // Quasi