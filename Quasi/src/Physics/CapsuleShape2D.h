#pragma once
#include "IShape2D.h"
#include "Math/Rect.h"
#include "Math/Vector.h"

namespace Quasi::Physics2D {
    class CapsuleShape : public IShape {
    public:
        fVector2 forward;
        float length = 1.0f, invLength = 1.0f, invLenSq;
        float radius = 0.0f;

        CapsuleShape(const fVector2& fwd, float r)
            : forward(fwd), length(fwd.len()), invLength(1.0f / length), invLenSq(invLength * invLength), radius(r) {}

        float ComputeArea() const;
        fRect2D ComputeBoundingBox() const;
        float Inertia() const;

        fVector2 NearestPointTo(const fVector2& point) const;
        fVector2 FurthestAlong(const fVector2& normal) const;
        fRange ProjectOntoAxis(const fVector2& axis) const;
        fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;

        void SetForward(const fVector2& f);
    };
} // Quasi
