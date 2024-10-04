#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class TransformedCapsuleShape;

    class CapsuleShape : public IShape {
    public:
        Math::fVector2 forward;
        float radius = 0.0f;

        CapsuleShape(const Math::fVector2& fwd, float r) : forward(fwd), radius(r) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return forward * 0.5f; }

        using TransformedVariant = TransformedCapsuleShape;
        void TransformTo(const PhysicsTransform& xf, Out<TransformedVariant*> out) const;
        [[nodiscard]] TransformedVariant Transform(const PhysicsTransform& xf) const;
    };

    class TransformedCapsuleShape : public ITransformedShape {
    public:
        Math::fVector2 start, forward, fwdOverLensq;
        float radius = 0.0f, invLength = 1.0f;

        TransformedCapsuleShape() = default;
        TransformedCapsuleShape(const Math::fVector2& s, const Math::fVector2& fwd, float r)
            : start(s), forward(fwd), fwdOverLensq(fwd / fwd.lensq()), radius(r), invLength(1.0f / fwd.len()) {}
        TransformedCapsuleShape(const CapsuleShape& c, const PhysicsTransform& xf = {}) { c.TransformTo(xf, this); }

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return start + forward * 0.5f; }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };
} // Quasi
