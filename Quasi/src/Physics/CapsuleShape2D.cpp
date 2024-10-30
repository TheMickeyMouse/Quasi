#include "CapsuleShape2D.h"

#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    float CapsuleShape::ComputeArea() const {
        return radius * (PI * radius + length * 4);
    }

    float CapsuleShape::Inertia() const {
        // $$ m \frac {1} {4L + \pi r} (\frac {\pi r^3}{2} + \frac {L^3 + 4r^2L} {3}) $$
        const float rSquared = radius * radius;
        return (HALF_PI * rSquared * radius +
                length * (length * length + 4 * rSquared) / 3)
              / (PI * radius + 4 * length);
    }

    void CapsuleShape::SetForward(const fVector2& f) {
        forward = f;
        length = f.len();
        invLength = 1 / length;
    }

    fRect2D CapsuleShape::ComputeBoundingBox() const {
        return fRect2D { -forward, forward }.corrected().extrude(radius);
    }

    fVector2 CapsuleShape::NearestPointTo(const fVector2& point) const {
        const float t = std::clamp(fwdOverLsq.dot(point), -1.0f, 1.0f);
        const fVector2 closest = forward * t;
        return closest + (point - closest).norm(radius);
    }

    fVector2 CapsuleShape::FurthestAlong(const fVector2& normal) const {
        return (0 > forward.dot(normal) ? -forward : forward) + normal * radius;
    }

    fRange CapsuleShape::ProjectOntoAxis(const fVector2& axis) const {
        const float f = std::abs(forward.dot(axis));
        return fRange { -f, f }.extrude(radius);
    }

    fRange CapsuleShape::ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const {
        return { -radius, +radius };
    }

    bool CapsuleShape::AddSeperatingAxes(SeperatingAxisSolver& sat) const {
        return sat.CheckAxis(forward.perpend() * invLength);
    }
} // Quasi