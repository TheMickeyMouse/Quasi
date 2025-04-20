#include "ImGuiExt.h"

#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include "Utils/Array.h"

#include "Complex.h"
#include "Math/Quaternion.h"
#include "Graphicals/CameraController.h"
#include "Graphicals/Light.h"
#include "Math/Transform2D.h"
#include "Math/Transform3D.h"
#include "Utils/Text/Formatting.h"

struct InputTextCallback_UserData;

namespace ImGui {
    static const Quasi::Math::Color COLOR_X_AXIS = Quasi::Math::Color::BETTER_RED();
    static const Quasi::Math::Color COLOR_Y_AXIS = Quasi::Math::Color::BETTER_GREEN();
    static const Quasi::Math::Color COLOR_Z_AXIS = Quasi::Math::Color::BETTER_BLUE();
    static const Quasi::Math::Color COLOR_W_AXIS = Quasi::Math::Color::BETTER_YELLOW();
    static const Quasi::Math::Color COLOR_ALPHA  = Quasi::Math::Color::BETTER_GRAY();
    static const Quasi::Math::Color COLOR_X_ANGLE = Quasi::Math::Color::BETTER_ORANGE();
    static const Quasi::Math::Color COLOR_Y_ANGLE = Quasi::Math::Color::BETTER_CYAN();
    static const Quasi::Math::Color COLOR_Z_ANGLE = Quasi::Math::Color::BETTER_PURPLE();

    float GetUsableWidth() {
        return GetWindowWidth() - 2 * GetStyle().WindowPadding.x;
    }

    float GetRemWidth(float width) {
        return std::isnan(width) ? GetUsableWidth() : width;
    }

    float GetLastItemWidth() {
        return GetItemRectSize().x;
    }

    float GetSpacingWidth() {
        return GetStyle().ItemSpacing.x;
    }

    float GetItemDefaultWidth(float totalWidth) {
        totalWidth = GetRemWidth(totalWidth);
        return totalWidth * 0.80f;
    }

    float GetItemRemainingWidth(float totalWidth) {
        totalWidth = GetRemWidth(totalWidth);
        return totalWidth * 0.20f - GetCurrentWindow()->DC.Indent.x;
    }

#define Q Quasi::
#define Q_IMGUI_EDITOR(NAME, ...) void NAME(Q Str title, __VA_ARGS__, float width)

    void DisplaySimpleIcon(const char* text, const Q Math::Color& bgColor) {
        const ImVec4 c = { (float)bgColor.r / 255.0f, (float)bgColor.g / 255.0f, (float)bgColor.b / 255.0f, 1 };
        PushStyleColor(ImGuiCol_Button,        c);
        PushStyleColor(ImGuiCol_ButtonActive,  c);
        PushStyleColor(ImGuiCol_ButtonHovered, c);
        PushID("icon");
        Button(text);
        PopID();
        PopStyleColor(3);
    }

    void DisplayTextCropped(Q Str text, float width) {
        const char* begin = text.Data(), *end = begin + text.Length();
        const ImVec2 textSize = CalcTextSize(begin, end),
                     cursor = GetCursorScreenPos();
        const float clipX = cursor.x + width;
        const ImRect bb { cursor, { cursor.x + std::min(textSize.x, width), cursor.y + textSize.y } };

        ItemSize({ std::min(textSize.x, width), textSize.y } , 0.0f);
        if (!ItemAdd(bb, 0))
            return;

        RenderTextEllipsis(GetWindowDrawList(), cursor, { clipX, cursor.y + textSize.y }, clipX, clipX, begin, end, &textSize);
    }

    void EditString(Q Str label, Q String& string) {
        auto* const callback = +[] (ImGuiInputTextCallbackData* data) -> int {
            Q String* s = (Q String*)data->UserData;
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                s->Resize(data->BufTextLen);
                data->Buf = (char*)s->Data();
            }
            return 0;
        };
        InputTextMultiline(label.Data(), string.Data(), string.Capacity(), ImVec2(0, 0), ImGuiInputTextFlags_CallbackResize, callback, &string);
    }

    template <class T> ImGuiDataType ImGuiDataEnum() {
        if constexpr (Q SameAs<T, Q f32>) return ImGuiDataType_Float;
        if constexpr (Q SameAs<T, Q f64>) return ImGuiDataType_Double;
        if constexpr (Q SameAs<T, Q u8 >) return ImGuiDataType_U8;
        if constexpr (Q SameAs<T, Q i8 >) return ImGuiDataType_S8;
        if constexpr (Q SameAs<T, Q u16>) return ImGuiDataType_U16;
        if constexpr (Q SameAs<T, Q i16>) return ImGuiDataType_S16;
        if constexpr (Q SameAs<T, Q u32>) return ImGuiDataType_U32;
        if constexpr (Q SameAs<T, Q i32>) return ImGuiDataType_S32;
        if constexpr (Q SameAs<T, Q u64>) return ImGuiDataType_U64;
        if constexpr (Q SameAs<T, Q i64>) return ImGuiDataType_S64;
        return ImGuiDataType_COUNT;
    }

    template <class T> requires std::is_arithmetic_v<T>
    Q_IMGUI_EDITOR(EditScalar, T& value, float speed, Q NoInfer<Q Option<Q Math::Range<T>>> range) {
        width = GetRemWidth(width);

        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());

        SetNextItemWidth(GetItemDefaultWidth(width));
        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());

        PushID(title.Data());
        DragScalar("", ImGuiDataEnum<T>(), &value, speed,
            range ? &range->min.x : nullptr, range ? &range->max.x : nullptr);
        PopID();
    }

    template <Q u32 N, class T>
    Q_IMGUI_EDITOR(EditVector, Q Math::VectorN<N, T>& vector, float speed, Q NoInfer<const Q Option<Q Math::RectN<N, T>>&> range) {
        width = GetRemWidth(width);

        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());
        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());

        const float fullWidth = GetItemDefaultWidth(width) - (float)(N - 1) * GetSpacingWidth(),
                    unitWidth = fullWidth / (float)N;

        for (Q u32 i = 0; i < N; ++i) {

            PushID(title.Data());
            EditScalarWithIcon(
                &"X:\0Y:\0Z:\0W:"[i * 3],
                Q Array { COLOR_X_AXIS, COLOR_Y_AXIS, COLOR_Z_AXIS, COLOR_W_AXIS }[i], // amazing hack
                vector[i],
                speed,
                range ? Q Options::Some(Q Math::Range<T> { range->min[i], range->max[i] }) : nullptr,
                {},
                unitWidth
            );
            if (i != N - 1) SameLine();
            PopID();
        }
    }

    template <class T>
    Q_IMGUI_EDITOR(EditRange, Q Math::Range<T>& range, float speed, Q NoInfer<const Q Option<Q Math::Range<T>>&> constraint) {
        width = GetRemWidth(width);

        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());

        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());
        const float fullWidth = GetItemDefaultWidth(width) - GetSpacingWidth();
        PushID(title.Data());
        EditScalarWithIcon("Min", COLOR_ALPHA, range.min.x, speed, constraint, {}, fullWidth * 0.5f);
        SameLine();
        EditScalarWithIcon("Max", COLOR_ALPHA, range.max.x, speed, constraint, {}, fullWidth * 0.5f);
        PopID();
    }

    template <Q u32 N, class T>
    Q_IMGUI_EDITOR(EditRect, Q Math::RectN<N, T>& range, float speed, Q NoInfer<const Q Option<Q Math::RectN<N, T>>&> constraint) {
        width = GetRemWidth(width);

        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());

        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());
        const float rWidth = GetItemDefaultWidth(width);
        PushID(title.Data());
        EditVector("Min", range.min, speed, constraint, rWidth);
        Indent(GetItemRemainingWidth(width));
        EditVector("Max", range.max, speed, constraint, rWidth);
        PopID();
        Unindent(GetItemRemainingWidth(width));
    }

    Q_IMGUI_EDITOR(EditComplexRotation, Q Math::fComplex& complex) {
        width = GetRemWidth(width);

        float rotation = complex.angle() * Q Math::RAD2DEG;
        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());

        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());

        const float w = GetItemDefaultWidth(width);
        PushID(title.Data());
        EditScalarWithIcon((const char*)u8"θ:", COLOR_Z_ANGLE, rotation, 1, Q Math::fRange { -180, 180 }, "%f°", w);
        PopID();

        complex = Q Math::fComplex::rotate(rotation * Quasi::Math::DEG2RAD);
    }

    Q_IMGUI_EDITOR(EditQuatRotation, Q Math::Quaternion& quaternion) {
        width = GetRemWidth(width);

        auto [xrot, yrot, zrot] = quaternion.xyzrot() * Q Math::RAD2DEG;
        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());

        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());

        const float w = (GetItemDefaultWidth(width) - 2 * GetSpacingWidth()) / 3;
        PushID(title.Data());
        EditScalarWithIcon((const char*)u8"θ:", COLOR_X_ANGLE, xrot, 1, Q Math::fRange {  -90,  90 }, "%f°", w); SameLine();
        EditScalarWithIcon((const char*)u8"Ψ:", COLOR_Y_ANGLE, yrot, 1, Q Math::fRange { -180, 180 }, "%f°", w); SameLine();
        EditScalarWithIcon((const char*)u8"φ:", COLOR_Z_ANGLE, zrot, 1, Q Math::fRange { -180, 180 }, "%f°", w);
        PopID();

        quaternion = Q Math::Quaternion::rotate_xyz(
            { xrot * Q Math::DEG2RAD,
              yrot * Q Math::DEG2RAD,
              zrot * Q Math::DEG2RAD }
        );
    }

    Q_IMGUI_EDITOR(EditRotation, float& yaw, float& pitch) {
        const float iw = (GetItemDefaultWidth(width) - GetSpacingWidth()) / 2;
        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());
        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());
        EditScalarWithIcon("Yaw",   COLOR_X_ANGLE, yaw,   0.01f, Q Math::fRange { -Q Math::PI,             Q Math::PI }, {}, iw);
        SameLine();
        EditScalarWithIcon("Pitch", COLOR_Y_ANGLE, pitch, 0.01f, Q Math::fRange { -Q Math::HALF_PI * 0.9f, Q Math::HALF_PI * 0.9f }, {}, iw);
    }

    template <Q Math::ColorLike C>
    Q_IMGUI_EDITOR(EditColor, C& color) {
        width = GetRemWidth(width);
        DisplayTextCropped(title, GetItemRemainingWidth(width) - GetSpacingWidth());

        SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());
        const float fullWidth = GetItemDefaultWidth(width) - (float)C::dimension * GetSpacingWidth() - GetFrameHeight(),
                    unitWidth = fullWidth / (float)C::dimension;

        for (Q u32 i = 0; i < C::dimension; ++i) {

            Q Math::Color c = Q Array<Q Math::Color, 4> { COLOR_X_AXIS, COLOR_Y_AXIS, COLOR_Z_AXIS, COLOR_ALPHA }[i];

            const Q Math::Range<typename C::scalar> range = { 0, (typename C::scalar)(C::traits_is_floating ? 1 : 255) };
            PushID(title.Data());
            EditScalarWithIcon(&"R:\0G:\0B:\0A:"[i * 3], c, color[i],
                C::traits_is_floating ? 0.005f : 1.0f, range, {}, unitWidth);
            SameLine();
            PopID();
        }

        PushID(title.Data(), "button");
        static constexpr Q u32 FLAGS = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
        if constexpr (C::traits_is_floating) {
            if constexpr (C::dimension == 3) {
                ColorEdit3("", color.begin(), FLAGS);
            } else {
                ColorEdit4("", color.begin(), FLAGS);
            }
        } else {
            Q Math::ColorFrom<C::traits_has_alpha, true> floatColor = color;
            if constexpr (C::dimension == 3) {
                ColorEdit3("", floatColor.begin(), FLAGS);
            } else {
                ColorEdit4("", floatColor.begin(), FLAGS);
            }
            color = floatColor;
        }
        PopID();
    }

    Q_IMGUI_EDITOR(EditTransform, Q Math::Transform2D& transform, float speed) {
        if (!TreeNode(title.Data())) return;
        EditVector         ("Position", transform.position, speed);
        EditVector         ("Scale",    transform.scale, speed);
        EditComplexRotation("Rotation", transform.rotation);
        TreePop();
    }

    Q_IMGUI_EDITOR(EditTransform, Q Math::Transform3D& transform, float speed) {
        if (!TreeNode(title.Data())) return;
        EditVector      ("Position", transform.position, speed);
        EditVector      ("Scale",    transform.scale, speed);
        EditQuatRotation("Rotation", transform.rotation);
        TreePop();
    }

    Q_IMGUI_EDITOR(EditCameraController, Q Graphics::CameraController& camera) {
        width = GetRemWidth(width);

        static bool showControls = false;

        if (!TreeNode(title.Data())) return;

        TextDisabled(camera.enabled ? " (press Esc to disable!)" : " (press Esc to enable!)");

        EditVector("Position", camera.position, 0.5f);
        EditRotation("Rotation", camera.yaw, camera.pitch);

        if (TreeNode("Controls")) {
            EditScalar("Speed", camera.speed, 0.5f);
            EditScalar("Sensitivity", camera.sensitivity, 0.01, Q Math::fRange { 0, 1 });
            EditScalar("FOV", camera.fov, 0.5, camera.fovRange);
            EditRange("FOV Range", camera.fovRange, 0.5f, Q Math::fRange { 0.0f, 90.0f });

            {
                const float iw = (GetItemDefaultWidth(width) - GetSpacingWidth()) / 2;
                DisplayTextCropped("Zoom", GetItemRemainingWidth(width) - GetSpacingWidth());
                SameLine(0, GetItemRemainingWidth(width) - GetLastItemWidth());
                EditScalarWithIcon("Lerp",   COLOR_ALPHA, camera.smoothZoom, 0.01f, Q Math::fRange { 0, 1 }, {}, iw);
                SameLine();
                EditScalarWithIcon("Factor", COLOR_ALPHA, camera.zoomRatio,  0.01f, Q Math::fRange { 0, 1 }, {}, iw);

                bool sz = camera.UsesSmoothZoom();
                Checkbox("Use Smooth Zoom", &sz);
                if (sz != camera.UsesSmoothZoom()) camera.smoothZoom *= -1;
            }

            TreePop();
        }

        if (Button("Copy State to Clipboard")) {
            SetClipboardText(Quasi::Text::Format(
                "camera.position = fVector3 {:($),};\n"
                "camera.yaw = {}; camera.pitch = {};\n"
                "camera.speed = {};\n"
                "camera.sensitivity = {};\n"
                "camera.fov = {};\n"
                "camera.fovRange = {{ {}, {} }};\n"
                "camera.zoomRatio = {};\n"
                "camera.smoothZoom = {};\n\0",
                camera.position,
                camera.yaw, camera.pitch,
                camera.speed,
                camera.sensitivity,
                camera.fov,
                camera.fovRange.min.value(), camera.fovRange.max.value(),
                camera.zoomRatio,
                camera.smoothZoom)
            .Data());
        }
        SameLine();
        if (Button(showControls ? "Hide Controls" : "Show Controls")) {
            showControls ^= true;
        }
        if (showControls) {
            Begin("Camera Controls", &showControls);

            Text("[Esc] - Toggle Camera View");
            Text("Movement"); SameLine(); Separator();
            Text(
                "   [W]    + [Space]/[Shift]/[Ctrl]\n"
                "[A][S][D]    (up)   (down)  (speed up)\n"
            );
            Text("Zoom"); SameLine(); Separator();
            Text(
                "[Mouse] - Look around\n"
                "[Caps Lock] + [Scroll Wheel] - Zoom"
            );

            End();
        }

        TreePop();
    }

    Q_IMGUI_EDITOR(EditLight, Q Graphics::Light& light) {
        Separator();
        if (!TreeNode(title.Data())) return;

        BeginTabBar("Light Edit");

        using namespace Q Math;
        using namespace Q Graphics;
        if (TabItemButton("Sun") && !light.Is<SunLight>())
            light.Set(SunLight {
                .direction = light.Direction()
            });
        if (TabItemButton("Point") && !light.Is<PointLight>())
            light.Set(PointLight {
                .position = light.Position(),
                .constant = 1.0f,
                .linear = 0.0f,
                .quadratic = 0.001f
            });
        if (TabItemButton("Spot") && !light.Is<FlashLight>()) {
            const auto [_, yaw, pitch] = light.Direction().spheric();
            light.Set(FlashLight {
                .position = light.Position(),
                .yaw = yaw, .pitch = pitch,
                .innerCut = 12.5_degf, .outerCut = 20.0_degf
            });
        }

        EditColor("Color + Intensity", light.color);
        light.VisitMut(
            [](SunLight& sun) {
                auto [len, yaw, pitch] = sun.direction.spheric();
                yaw   = std::isnan(yaw)   ? 0 : yaw;
                pitch = std::isnan(pitch) ? 0 : pitch;
                EditScalar("Yaw",    yaw,   0.01, fRange { -PI,      PI });
                EditScalar("Pitch",  pitch, 0.01, fRange { -HALF_PI, HALF_PI });
                EditScalar("Length", len, 0.1, fRange { 0, 10 });
                sun.direction = fVector3::from_spheric(len, yaw, pitch);
            },
            [](PointLight& point) {
                EditVector("Position", point.position);
                if (TreeNode("Light Attenuation")) {
                    EditScalar("Constant",  point.constant,  0.01f);
                    EditScalar("Linear",    point.linear,    0.01f);
                    EditScalar("Quadratic", point.quadratic, 0.01f);
                    TreePop();
                }
            },
            [](FlashLight& flash) {
                EditVector("Position", flash.position);

                flash.yaw   = std::isnan(flash.yaw)   ? 0 : flash.yaw;
                flash.pitch = std::isnan(flash.pitch) ? 0 : flash.pitch;
                EditScalar("Yaw",   flash.yaw,   0.01, fRange { -PI,      PI });
                EditScalar("Pitch", flash.pitch, 0.01, fRange { -HALF_PI, HALF_PI });

                EditScalar("Inner Cutoff", flash.innerCut, 0.01, fRange { 0, PI });
                EditScalar("Outer Cutoff", flash.outerCut, 0.01, fRange { 0, PI });
            }
        );

        EndTabBar();
        if (Button("Copy State to Clipboard")) {
            SetClipboardText(light.Visit<Quasi::String>(
                [&](const SunLight& sun) {
                    return Quasi::Text::Format(
                        "light = Graphics::SunLight {{\n"
                        "   .direction = fVector3 {:($),},\n"
                        "}};\n"
                        "light.color = fColor {:($),};\n\0",
                        sun.direction,
                        light.color);
                },
                [&](const PointLight& point) {
                    return Quasi::Text::Format(
                        "light = Graphics::PointLight {{\n"
                        "   .position = fVector3 {:($),},\n"
                        "   .constant = {},\n"
                        "   .linear = {},\n"
                        "   .quadratic = {},\n"
                        "}};\n"
                        "light.color = fColor {:($),};\n\0",
                        point.position,
                        point.constant, point.linear, point.quadratic,
                        light.color);
                },
                [&](const FlashLight& flash) {
                    return Quasi::Text::Format(
                        "light = Graphics::FlashLight {{\n"
                        "   .position = fVector3 {:($),},\n"
                        "   .yaw = {}, .pitch = {},\n"
                        "   .innerCut = {}, .outerCut = {}\n"
                        "}};\n"
                        "light.color = fColor {:($),};\n\0",
                        flash.position,
                        flash.yaw, flash.pitch,
                        flash.innerCut, flash.outerCut,
                        light.color);
                }
            ).Data());
        }
        TreePop();
    }

#pragma region Instantiations
    template ImGuiDataType ImGuiDataEnum<float> ();
    template ImGuiDataType ImGuiDataEnum<double>();
    template ImGuiDataType ImGuiDataEnum<Q u8 > ();
    template ImGuiDataType ImGuiDataEnum<Q i8 > ();
    template ImGuiDataType ImGuiDataEnum<Q u16> ();
    template ImGuiDataType ImGuiDataEnum<Q i16> ();
    template ImGuiDataType ImGuiDataEnum<Q u32> ();
    template ImGuiDataType ImGuiDataEnum<Q i32> ();
    template ImGuiDataType ImGuiDataEnum<Q u64> ();
    template ImGuiDataType ImGuiDataEnum<Q i64> ();

    template void EditScalar<float> (Q Str, float&,  float, Q NoInfer<Q Option<Q Math::Range<float>>>,  float);
    template void EditScalar<double>(Q Str, double&, float, Q NoInfer<Q Option<Q Math::Range<double>>>, float);
    template void EditScalar<Q u8 > (Q Str, Q u8 &,  float, Q NoInfer<Q Option<Q Math::Range<Q u8 >>>,  float);
    template void EditScalar<Q i8 > (Q Str, Q i8 &,  float, Q NoInfer<Q Option<Q Math::Range<Q i8 >>>,  float);
    template void EditScalar<Q u16> (Q Str, Q u16&,  float, Q NoInfer<Q Option<Q Math::Range<Q u16>>>,  float);
    template void EditScalar<Q i16> (Q Str, Q i16&,  float, Q NoInfer<Q Option<Q Math::Range<Q i16>>>,  float);
    template void EditScalar<Q u32> (Q Str, Q u32&,  float, Q NoInfer<Q Option<Q Math::Range<Q u32>>>,  float);
    template void EditScalar<Q i32> (Q Str, Q i32&,  float, Q NoInfer<Q Option<Q Math::Range<Q i32>>>,  float);
    template void EditScalar<Q u64> (Q Str, Q u64&,  float, Q NoInfer<Q Option<Q Math::Range<Q u64>>>,  float);
    template void EditScalar<Q i64> (Q Str, Q i64&,  float, Q NoInfer<Q Option<Q Math::Range<Q i64>>>,  float);

    template void EditVector<2, float>  (Q Str, Q Math::VectorN<2, float>&,   float, Q NoInfer<const Q Option<Q Math::RectN<2, float>>&>,  float);
    template void EditVector<3, float>  (Q Str, Q Math::VectorN<3, float>&,   float, Q NoInfer<const Q Option<Q Math::RectN<3, float>>&>,  float);
    template void EditVector<4, float>  (Q Str, Q Math::VectorN<4, float>&,   float, Q NoInfer<const Q Option<Q Math::RectN<4, float>>&>,  float);
    template void EditVector<2, double> (Q Str, Q Math::VectorN<2, double>&,  float, Q NoInfer<const Q Option<Q Math::RectN<2, double>>&>, float);
    template void EditVector<3, double> (Q Str, Q Math::VectorN<3, double>&,  float, Q NoInfer<const Q Option<Q Math::RectN<3, double>>&>, float);
    template void EditVector<4, double> (Q Str, Q Math::VectorN<4, double>&,  float, Q NoInfer<const Q Option<Q Math::RectN<4, double>>&>, float);
    template void EditVector<2, Q i32>  (Q Str, Q Math::VectorN<2, Q i32>&,   float, Q NoInfer<const Q Option<Q Math::RectN<2, Q i32>>&>,  float);
    template void EditVector<3, Q i32>  (Q Str, Q Math::VectorN<3, Q i32>&,   float, Q NoInfer<const Q Option<Q Math::RectN<3, Q i32>>&>,  float);
    template void EditVector<4, Q i32>  (Q Str, Q Math::VectorN<4, Q i32>&,   float, Q NoInfer<const Q Option<Q Math::RectN<4, Q i32>>&>,  float);
    template void EditVector<2, Q u32>  (Q Str, Q Math::VectorN<2, Q u32>&,   float, Q NoInfer<const Q Option<Q Math::RectN<2, Q u32>>&>,  float);
    template void EditVector<3, Q u32>  (Q Str, Q Math::VectorN<3, Q u32>&,   float, Q NoInfer<const Q Option<Q Math::RectN<3, Q u32>>&>,  float);
    template void EditVector<4, Q u32>  (Q Str, Q Math::VectorN<4, Q u32>&,   float, Q NoInfer<const Q Option<Q Math::RectN<4, Q u32>>&>,  float);
    template void EditVector<2, Q byte> (Q Str, Q Math::VectorN<2, Q byte>&,  float, Q NoInfer<const Q Option<Q Math::RectN<2, Q byte>>&>, float);
    template void EditVector<3, Q byte> (Q Str, Q Math::VectorN<3, Q byte>&,  float, Q NoInfer<const Q Option<Q Math::RectN<3, Q byte>>&>, float);
    template void EditVector<4, Q byte> (Q Str, Q Math::VectorN<4, Q byte>&,  float, Q NoInfer<const Q Option<Q Math::RectN<4, Q byte>>&>, float);

    template void EditRange<float>  (Q Str, Q Math::Range<float>&,  float, Q NoInfer<const Q Option<Q Math::Range<float>>&>,  float);
    template void EditRange<double> (Q Str, Q Math::Range<double>&, float, Q NoInfer<const Q Option<Q Math::Range<double>>&>, float);
    template void EditRange<Q i32>  (Q Str, Q Math::Range<Q i32>&,  float, Q NoInfer<const Q Option<Q Math::Range<Q i32>>&>,  float);
    template void EditRange<Q u32>  (Q Str, Q Math::Range<Q u32>&,  float, Q NoInfer<const Q Option<Q Math::Range<Q u32>>&>,  float);
    template void EditRange<Q byte> (Q Str, Q Math::Range<Q byte>&, float, Q NoInfer<const Q Option<Q Math::Range<Q byte>>&>, float);

    template void EditRect<2, float>  (Q Str, Q Math::RectN<2, float>&,  float, Q NoInfer<const Q Option<Q Math::RectN<2, float>>&>,  float);
    template void EditRect<3, float>  (Q Str, Q Math::RectN<3, float>&,  float, Q NoInfer<const Q Option<Q Math::RectN<3, float>>&>,  float);
    template void EditRect<4, float>  (Q Str, Q Math::RectN<4, float>&,  float, Q NoInfer<const Q Option<Q Math::RectN<4, float>>&>,  float);
    template void EditRect<2, double> (Q Str, Q Math::RectN<2, double>&, float, Q NoInfer<const Q Option<Q Math::RectN<2, double>>&>, float);
    template void EditRect<3, double> (Q Str, Q Math::RectN<3, double>&, float, Q NoInfer<const Q Option<Q Math::RectN<3, double>>&>, float);
    template void EditRect<4, double> (Q Str, Q Math::RectN<4, double>&, float, Q NoInfer<const Q Option<Q Math::RectN<4, double>>&>, float);
    template void EditRect<2, Q i32>  (Q Str, Q Math::RectN<2, Q i32>&,  float, Q NoInfer<const Q Option<Q Math::RectN<2, Q i32>>&>,  float);
    template void EditRect<3, Q i32>  (Q Str, Q Math::RectN<3, Q i32>&,  float, Q NoInfer<const Q Option<Q Math::RectN<3, Q i32>>&>,  float);
    template void EditRect<4, Q i32>  (Q Str, Q Math::RectN<4, Q i32>&,  float, Q NoInfer<const Q Option<Q Math::RectN<4, Q i32>>&>,  float);
    template void EditRect<2, Q u32>  (Q Str, Q Math::RectN<2, Q u32>&,  float, Q NoInfer<const Q Option<Q Math::RectN<2, Q u32>>&>,  float);
    template void EditRect<3, Q u32>  (Q Str, Q Math::RectN<3, Q u32>&,  float, Q NoInfer<const Q Option<Q Math::RectN<3, Q u32>>&>,  float);
    template void EditRect<4, Q u32>  (Q Str, Q Math::RectN<4, Q u32>&,  float, Q NoInfer<const Q Option<Q Math::RectN<4, Q u32>>&>,  float);
    template void EditRect<2, Q byte> (Q Str, Q Math::RectN<2, Q byte>&, float, Q NoInfer<const Q Option<Q Math::RectN<2, Q byte>>&>, float);
    template void EditRect<3, Q byte> (Q Str, Q Math::RectN<3, Q byte>&, float, Q NoInfer<const Q Option<Q Math::RectN<3, Q byte>>&>, float);
    template void EditRect<4, Q byte> (Q Str, Q Math::RectN<4, Q byte>&, float, Q NoInfer<const Q Option<Q Math::RectN<4, Q byte>>&>, float);

    template void EditColor<Q Math::Color  >(Q Str, Q Math::Color&,   float);
    template void EditColor<Q Math::Color3 >(Q Str, Q Math::Color3&,  float);
    template void EditColor<Q Math::fColor >(Q Str, Q Math::fColor&,  float);
    template void EditColor<Q Math::fColor3>(Q Str, Q Math::fColor3&, float);

    #pragma endregion
} // ImGui