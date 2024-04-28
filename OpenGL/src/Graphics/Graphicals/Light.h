#pragma once
#include "Vector.h"
#include "Color.h"
#include "stdu/macros.h"
#include "stdu/ref.h"

namespace Graphics {
    struct SunLight {
        Maths::fvec3 direction;
    };

    struct PointLight {
        Maths::fvec3 position;
        float constant, linear, quadratic;
    };

    struct FlashLight {
        Maths::fvec3 position;
        float yaw, pitch, innerCut, outerCut;
    };

    enum class LightType {
        SUNLIGHT, POINTLIGHT, FLASHLIGHT,
        NONE = -1
    };

    using LightTypesList = stdu::typelist            <SunLight,            PointLight,            FlashLight>;
    using LightTypeIndex = stdu::valuelist<LightType, LightType::SUNLIGHT, LightType::POINTLIGHT, LightType::FLASHLIGHT>;
    using LightTypeMap = stdu::typemap<LightTypesList, LightTypeIndex>;
    template <class L> concept LightCaster = stdu::is_in<L, LightTypesList>;

    class Light {
    public:
        LightType type;
        Maths::colorf color;
    private:
        union {
            SunLight sunLight;
            PointLight pointLight;
            FlashLight flashLight = { 0, 0.0f, 0.0f, 0.0f, 0.0f }; // largest size so default constructor
            stdu::empty lightAddressBegin;
        };
    public:
        template <class L> static constexpr LightType LightIndex = stdu::query_map<L, LightTypeMap>;

        Light() : type(LightType::NONE) {}

        static constexpr std::size_t LightTypeCount() { return LightTypesList::size; }
        static STDU_ENUM_TOSTR(LightType, ToStr, (SUNLIGHT, "Sun")(POINTLIGHT, "Point")(FLASHLIGHT, "Spot"), "null")
        static STDU_ENUM_TOSTR(LightType, Nameof, (SUNLIGHT, "SunLight")(POINTLIGHT, "PointLight")(FLASHLIGHT, "FlashLight"), "")

        template <LightCaster L> stdu::ref<L> AsUnchecked();
        template <LightCaster L> [[nodiscard]] stdu::ref<const L> AsUnchecked() const;

        template <LightCaster L> stdu::ref<L> As() { return LightIndex<L> == type ? AsUnchecked<L>() : nullptr; }
        template <LightCaster L> stdu::ref<const L> As() const { return LightIndex<L> == type ? AsUnchecked<L>() : nullptr; }

        template <LightCaster L> [[nodiscard]] bool Is() const { return type == LightIndex<L>; }
        [[nodiscard]] bool Is(const LightType t) const { return type == t; }

        [[nodiscard]] Maths::fvec3 Position() const;
        [[nodiscard]] Maths::fvec3 Direction() const;

        template <LightCaster L> void Set(const L& l) { type = LightIndex<L>; *As<L>() = l; }
        template <LightCaster L> Light& operator=(const L& l) { this->Set(l); return *this; }

        void ImGuiEdit(const char* title);
        template <LightCaster L> void ImGuiEditLight() = delete;
        void CopyState() const;
    };

    template <> void Light::ImGuiEditLight<SunLight>();
    template <> void Light::ImGuiEditLight<PointLight>();
    template <> void Light::ImGuiEditLight<FlashLight>();
}
