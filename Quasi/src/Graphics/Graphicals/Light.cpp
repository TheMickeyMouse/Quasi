#include "Light.h"

namespace Quasi::Graphics {
    Math::fVector3 Light::Position() const {
        return Visit<Math::fVector3>([]<class L> (const L& light) {
            if constexpr (std::is_convertible_v<L, SunLight>) return -light.direction;
            else return light.position;
        });
    }

    Math::fVector3 Light::Direction() const {
        return Visit<Math::fVector3>(
            [](const SunLight&   sun)   { return sun.direction; },
            [](const PointLight& point) { return -point.position; },
            [](const FlashLight& flash) { return Math::fVector3::from_spheric(1, flash.yaw, flash.pitch); }
        );
    }
}
