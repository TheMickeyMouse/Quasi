#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class CapsuleShape : public IShape {
    public:
        fVector2 forward, fwdOverLsq;
        float length = 1.0f, invLength = 1.0f;
        float radius = 0.0f;

        CapsuleShape(const fVector2& fwd, float r)
            : forward(fwd), fwdOverLsq(fwd / fwd.lensq()), length(fwd.len()), invLength(1.0f / length), radius(r) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] fRect2D ComputeBoundingBox() const;
        [[nodiscard]] fVector2 CenterOfMass() const { return 0; }
        [[nodiscard]] float Inertia() const;

        [[nodiscard]] fVector2 NearestPointTo(const fVector2& point) const;
        [[nodiscard]] fVector2 FurthestAlong(const fVector2& normal) const;
        [[nodiscard]] fRange ProjectOntoAxis(const fVector2& axis) const;
        [[nodiscard]] fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;

        void SetForward(const fVector2& f);
    };
} // Quasi
