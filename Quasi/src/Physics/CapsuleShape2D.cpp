#include "CapsuleShape2D.h"

#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    float CapsuleShape::ComputeArea() const {
        return radius * (Math::PI * radius + forward.len() * 2);
    }

    Math::fRect2D CapsuleShape::ComputeBoundingBox() const {
        const bool negX = forward.x < 0, negY = forward.y < 0;
        switch (2 * negX + negY) {
            case 0:  return { -radius, forward + radius };
            case 1:  return { { -radius, forward.y - radius }, { forward.x + radius, +radius } };
            case 2:  return { { forward.x - radius, -radius }, { +radius, forward.y + radius } };
            case 3:  return { forward - radius, +radius };
            default: return {};
        }
    }

    void CapsuleShape::TransformTo(const PhysicsTransform& xf, Out<TransformedVariant*> out) const {
        out->start = xf.position;
        out->forward = xf.rotation.rotate(forward);
        out->fwdOverLensq = out->forward / forward.lensq();
        out->radius = radius;
        out->invLength = 1.0f / forward.len();
    }

    CapsuleShape::TransformedVariant CapsuleShape::Transform(const PhysicsTransform& xf) const {
        return { *this, xf };
    }

    float TransformedCapsuleShape::ComputeArea() const {
        return radius * (Math::PI * radius + forward.len() * 2);
    }

    Math::fRect2D TransformedCapsuleShape::ComputeBoundingBox() const {
        const bool negX = forward.x < 0, negY = forward.y < 0;
        switch (2 * negX + negY) {
            case 0:  return { start - radius, start + forward + radius };
            case 1:  return { { start.x - radius, start.y + forward.y - radius }, { start.x + forward.x + radius, start.y + radius } };
            case 2:  return { { start.x + forward.x - radius, start.y - radius }, { start.x + radius, start.y + forward.y + radius } };
            case 3:  return { start + forward - radius, start + radius };
            default: return {};
        }
    }

    Math::fVector2 TransformedCapsuleShape::NearestPointTo(const Math::fVector2& point) const {
        const float t = std::clamp(fwdOverLensq.dot(point - start), 0.0f, 1.0f);
        const Math::fVector2 closest = start + (forward * t);
        return closest + (point - closest).norm(radius);
    }

    Math::fVector2 TransformedCapsuleShape::FurthestAlong(const Math::fVector2& normal) const {
        const Math::fVector2& furthest = (0 > forward.dot(normal) ? start : start + forward);
        return furthest + normal * radius;
    }

    Math::fRange TransformedCapsuleShape::ProjectOntoAxis(const Math::fVector2& axis) const {
        return Math::fRange { 0, forward.dot(axis) }.corrected().extrude(radius) + start.dot(axis);
    }

    Math::fRange TransformedCapsuleShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const {
        const float center = start.dot(axis);
        return { center - radius, center + radius };
    }

    bool TransformedCapsuleShape::AddSeperatingAxes(SeperatingAxisSolver& sat) const {
        return sat.CheckAxis(forward.perpend() * invLength);
    }
} // Quasi