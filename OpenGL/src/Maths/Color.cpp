#include "Color.h"
#include "Vector.h"

namespace Maths {
    namespace color_utils {
        template <class T> T convert_type(auto channel) {
            if constexpr (std::is_same_v<decltype(channel), T>) return channel;
            else if constexpr (std::is_same_v<decltype(channel), byte> && std::is_same_v<T, float>) return (float)channel / 255.0f;
            else if constexpr (std::is_same_v<decltype(channel), float> && std::is_same_v<T, byte>) return (byte)(channel * 255.0f);
            else return nullptr;
        }
        template OPENGL_API byte  convert_type<byte,  byte>(byte);
        template OPENGL_API byte  convert_type<byte,  float>(float);
        template OPENGL_API float convert_type<float, byte>(byte);
        template OPENGL_API float convert_type<float, float>(float);

        template <class T> T channel_from_hexcode(char D0, char D1) {
            return convert_type<T>((byte)((D0 - (D0 < ':' ? '0' : 'W')) << 4 | (D1 - (D1 < ':' ? '0' : 'W'))));
        }
        template OPENGL_API byte  channel_from_hexcode<byte> (char, char);
        template OPENGL_API float channel_from_hexcode<float>(char, char);

        fvec3 rgb2hsl(float r, float g, float b) {
            // https://stackoverflow.com/a/9493060
            const float max = std::max({r, g, b}), min = std::min({r, g, b});
            float h = 0, l = (max + min) / 2;

            if (max == min) // NOLINT(clang-diagnostic-float-equal) no need to worry abt this
                return { 0, 0, l }; // achromatic

            const float d = max - min;
            float s = d / (l > 0.5f ? (2 - max - min) : (max + min));
            if (max == r) h = (g - b) / d + (g < b ? 6.0f : 0.0f);  // NOLINT(clang-diagnostic-float-equal)
            if (max == g) h = (b - r) / d + 2;                      // NOLINT(clang-diagnostic-float-equal)
            if (max == b) h = (r - g) / d + 4;                      // NOLINT(clang-diagnostic-float-equal)
            h /= 6;

            return { h, s, l };
        }

        float hue2rgb(float p, float q, float t) {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0f/6) return p + (q - p) * 6 * t;
            if (t < 1.0f/2) return q;
            if (t < 2.0f/3) return p + (q - p) * (2.0f/3 - t) * 6;
            return p;
        }
    
        fvec3 hsl2rgb(float h, float s, float l) {
            float r, g, b;
            if (s == 0) {  // NOLINT(clang-diagnostic-float-equal)
                r = g = b = l; // achromatic
            } else {
                const float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
                const float p = 2 * l - q;
                r = hue2rgb(p, q, h + 1.0f/3);
                g = hue2rgb(p, q, h);
                b = hue2rgb(p, q, h - 1.0f/3);
            }
            return { r, g, b };
        }

        fvec3 rgb2hsv(float r, float g, float b) {
            // https://stackoverflow.com/a/6930407
            const float max = std::max({r, g, b}), min = std::min({r, g, b});
            const float v = max, d = max - min;
            if (v == 0) // NOLINT(clang-diagnostic-float-equal)
                return { NAN, 0, v }; // acromatic
            float s = d / v;

            if (d == 0)  // NOLINT(clang-diagnostic-float-equal)
                return { 0, 0, v }; // acromatic

            float h = 0;
            if (max == r) h = g - b;      // NOLINT(clang-diagnostic-float-equal)
            if (max == g) h = 2 + b - r;  // NOLINT(clang-diagnostic-float-equal)
            if (max == b) h = 4 + r - g;  // NOLINT(clang-diagnostic-float-equal)
            h /= d * 6;
            h += (float)(h < 0);
            return { h, s, v };
        }

        fvec3 hsv2rgb(float h, float s, float v) {
            if (s == 0)  // NOLINT(clang-diagnostic-float-equal)
                return { v }; // Achromatic case
            h *= 6;
            const int i = (int)h;
            const float f = h - (float)i;

            float p = v * (1 - s);
            float q = v * (1 - f * s);
            float t = v * (1 - (1 - f) * s);

            switch (i) {
                case 0: return { v, t, p };
                case 1: return { q, v, p };
                case 2: return { p, v, t };
                case 3: return { p, q, v };
                case 4: return { t, p, v };
                case 5: return { v, p, q };
                default: return 0;
            }
        }

        float overlay_channel(float a, float b) {
            return a < 0.5f ? 2 * a * b : 1 - 2 * (1 - a) * (1 - b);
        }

        uchar overlay_channel(uchar a, uchar b) {
            return a < 128 ? a * b * 255 / 2 : 255 - 255 * (1 - a) * (1 - b) / 2;
        }

        bool channel_loose_eq(float a, float b) { // loose equality
            return std::abs(a - b) < (1 / 255.0f);
        }
        
        bool color_loose_eq(const float* a, const float* b) {
            return channel_loose_eq(a[0], b[0]) && channel_loose_eq(a[1], b[1]) && channel_loose_eq(a[2], b[2]);
        }

        bool color4_loose_eq(const float* a, const float* b) {
            return channel_loose_eq(a[0], b[0]) && channel_loose_eq(a[1], b[1])
                && channel_loose_eq(a[2], b[2]) && channel_loose_eq(a[3], b[3]);
        }

        char channel_hex(const char i) {
            return (i < 10) ? (char)('0' + i) : (char)('a' + i - 10);
        }

        char hexdigit_1(auto channel) {
            return channel_hex(convert_type<byte>(channel) % 16);
        }

        char hexdigit_2(auto channel) {
            return channel_hex(convert_type<byte>(channel) / 16);
        }
    }

    template <class Color>
    typename color_base<Color>::scalar color_base<Color>::alpha(scalar def) const {
        if constexpr (traits_has_alpha) return color().a;
        else return def;
    }

    template <class Color> Color color_base<Color>::from_hex(std::string_view hex) {
        if constexpr (traits_has_alpha)
            return hex.size() > 6 ? Color {
                color_utils::channel_from_hexcode<scalar>(hex[0], hex[1]),
                color_utils::channel_from_hexcode<scalar>(hex[2], hex[3]),
                color_utils::channel_from_hexcode<scalar>(hex[4], hex[5]),
                color_utils::channel_from_hexcode<scalar>(hex[6], hex[7])
            } : Color {
                color_utils::channel_from_hexcode<scalar>(hex[0], hex[1]),
                color_utils::channel_from_hexcode<scalar>(hex[2], hex[3]),
                color_utils::channel_from_hexcode<scalar>(hex[4], hex[5]),
            };
        else return Color {
            color_utils::channel_from_hexcode<scalar>(hex[0], hex[1]),
            color_utils::channel_from_hexcode<scalar>(hex[2], hex[3]),
            color_utils::channel_from_hexcode<scalar>(hex[4], hex[5]),
        };
    }

    template <class Color> std::string color_base<Color>::hexcode() const {
        if constexpr (traits_has_alpha) return std::format("{}{}{}{}{}{}{}{}",
            color_utils::hexdigit_1(color().r),
            color_utils::hexdigit_1(color().r),
            color_utils::hexdigit_1(color().g),
            color_utils::hexdigit_1(color().g),
            color_utils::hexdigit_1(color().b),
            color_utils::hexdigit_1(color().b),
            color_utils::hexdigit_1(color().a),
            color_utils::hexdigit_1(color().a)
        ); else return std::format("{}{}{}{}{}{}",
            color_utils::hexdigit_1(color().r),
            color_utils::hexdigit_1(color().r),
            color_utils::hexdigit_1(color().g),
            color_utils::hexdigit_1(color().g),
            color_utils::hexdigit_1(color().b),
            color_utils::hexdigit_1(color().b)
        );
    }

    template <class Color> bool color_base<Color>::eq(const Color& other) const {
        if constexpr (traits_has_alpha)
            return color().r == other.r && color().g == other.g && color().b == other.b && color().a == other.a;
        else
            return color().r == other.r && color().g == other.g && color().b == other.b;
    }

    template <class Color> bool color_base<Color>::operator==(const Color& other) const { return eq(other); }
    template <class Color> bool color_base<Color>::neq(const Color& other) const { return !eq(other); }

    template <class Color> bool color_base<Color>::loose_eq(const Color& other) const {
        if constexpr (traits_is_floating)
            if constexpr (traits_has_alpha) return color_utils::color4_loose_eq(begin(), other.begin());
            else return color_utils::color_loose_eq(begin(), other.begin());
        else return eq(other);
    }

    template <class Color> Color color_base<Color>::neg() const {
        return color_utils::make_color<Color>(
            CHANNEL_MAX_VALUE - color().r,
            CHANNEL_MAX_VALUE - color().g,
            CHANNEL_MAX_VALUE - color().b,
            CHANNEL_MAX_VALUE - alpha()
        );
    }

    template <class Color> Color color_base<Color>::lerp(const Color& other, float t) const {
        const float s = 1 - t;
        using namespace color_utils;
        return make_color<Color>(
            convert_type<float>(color().r) * s + convert_type<float>(other.r) * t,
            convert_type<float>(color().g) * s + convert_type<float>(other.g) * t,
            convert_type<float>(color().b) * s + convert_type<float>(other.b) * t,
            convert_type<float>(alpha())   * s + convert_type<float>(other.alpha()) * t
        );
    }

    template <class Color> Color color_base<Color>::mul(const Color& other) const {
        return color_utils::make_color<Color>(
            color().r * other.r / CHANNEL_MAX_VALUE,
            color().g * other.g / CHANNEL_MAX_VALUE,
            color().b * other.b / CHANNEL_MAX_VALUE,
            alpha() * other.alpha() / CHANNEL_MAX_VALUE
        );
    }

    template <class Color> Color color_base<Color>::screen(const Color& other) const { return neg().mul(other).neg(); }
    template <class Color> Color color_base<Color>::overlay(const Color& other) const {
        using namespace color_utils;
        return make_color<Color>(
            overlay_channel(color().r, other.r), overlay_channel(color().g, other.g),
            overlay_channel(color().b, other.b), overlay_channel(alpha(), other.alpha())
        );
    }

    template <class Color> typename color_base<Color>::without_alpha_t color_base<Color>::mul_alpha() const {
        return { color().r * alpha() / CHANNEL_MAX_VALUE, color().g * alpha() / CHANNEL_MAX_VALUE, color().b * alpha() / CHANNEL_MAX_VALUE, };
    }

    template <class Color> float color_base<Color>::luminance() const {
        // https://en.wikipedia.org/wiki/Relative_luminance
        return (0.2126f * color_utils::convert_type<float>(color().r) + 0.7152f * color_utils::convert_type<float>(color().g) + 0.0722f * color_utils::convert_type<float>(color().b));
    }

    template <class Color> bvec3 color_base<Color>::as_rgb()   const { return { color_utils::convert_type<byte>(color().r), color_utils::convert_type<byte>(color().g), color_utils::convert_type<byte>(color().b) }; }
    template <class Color> bvec4 color_base<Color>::as_rgba()  const { return as_rgb().with_w(color_utils::convert_type<byte>(alpha())); }
    template <class Color> fvec3 color_base<Color>::as_rgbf()  const { return { color_utils::convert_type<float>(color().r), color_utils::convert_type<float>(color().g), color_utils::convert_type<float>(color().b) }; }
    template <class Color> fvec4 color_base<Color>::as_rgbaf() const { return as_rgbf().with_w(color_utils::convert_type<float>(alpha())); }

    template <class Color> fvec3 color_base<Color>::as_hsl()  const { return color_utils::rgb2hsl(color_utils::convert_type<float>(color().r), color_utils::convert_type<float>(color().g), color_utils::convert_type<float>(color().b)); }
    template <class Color> fvec4 color_base<Color>::as_hsla() const { return as_hsl().with_w(color_utils::convert_type<float>(alpha())); }

    template <class Color> Color color_base<Color>::from_hsl(float hue, float saturation, float lightness) requires (!traits_has_alpha) {
        return color_utils::hsl2rgb(hue / 360.0f, saturation, lightness).to_color3();
    }
    template <class Color> Color color_base<Color>::from_hsl(float hue, float saturation, float lightness, float alpha) requires traits_has_alpha {
        return color_utils::hsl2rgb(hue / 360.0f, saturation, lightness).with_w(alpha).to_color();
    }

    template <class Color> Color color_base<Color>::from_hsl(const fvec3& hsl) requires (!traits_has_alpha) { return from_hsl(hsl.x, hsl.y, hsl.z); }
    template <class Color> Color color_base<Color>::from_hsl(const fvec4& hsla) requires traits_has_alpha { return from_hsl(hsla.x, hsla.y, hsla.z, hsla.w); }

    template <class Color> fvec3 color_base<Color>::as_hsv()  const { return color_utils::rgb2hsv(color_utils::convert_type<float>(color().r), color_utils::convert_type<float>(color().g), color_utils::convert_type<float>(color().b)); }
    template <class Color> fvec4 color_base<Color>::as_hsva() const { return as_hsv().with_w(color_utils::convert_type<float>(alpha())); }

    template <class Color> Color color_base<Color>::from_hsv(float hue, float saturation, float value) requires (!traits_has_alpha) {
        return color_utils::hsv2rgb(hue / 360.0f, saturation, value).to_color3();
    }
    template <class Color> Color color_base<Color>::from_hsv(float hue, float saturation, float value, float alpha) requires traits_has_alpha {
        return color_utils::hsv2rgb(hue / 360.0f, saturation, value).with_w(alpha).to_color();
    }

    template <class Color> Color color_base<Color>::from_hsv(const fvec3& hsv) requires (!traits_has_alpha) { return from_hsv(hsv.x, hsv.y, hsv.z); }
    template <class Color> Color color_base<Color>::from_hsv(const fvec4& hsva) requires traits_has_alpha { return from_hsv(hsva.x, hsva.y, hsva.z, hsva.w); }
    template <class Color> typename color_base<Color>::without_alpha_t color_base<Color>::rgb() const { return { color().r, color().g, color().b }; }
    template <class Color> typename color_base<Color>::with_alpha_t color_base<Color>::with_alpha(scalar alpha) const { return { color().r, color().g, color().b, alpha }; }
    template <class Color> typename color_base<Color>::with_alpha_t color_base<Color>::rgb1() const { return { color().r, color().g, color().b, CHANNEL_MAX_VALUE }; }

    template <class Color> color_base<Color>::operator without_alpha_t() const requires traits_has_alpha { return rgb(); }
    template <class Color> color_base<Color>::operator with_alpha_t() const requires (!traits_has_alpha) { return rgb1(); }
    template <class Color> color_base<Color>::operator alternate_data_form() const {
        using alt = typename color_traits<alternate_data_form>::scalar;
        return color_utils::make_color<alternate_data_form>(
            color_utils::convert_type<alt>(color().r),
            color_utils::convert_type<alt>(color().g),
            color_utils::convert_type<alt>(color().b),
            color_utils::convert_type<alt>(alpha())
        );
    }

    template <class Color> Color color_base<Color>::color_id(int id) {
        switch (id) {
            case 0: return BETTER_BLACK();
            case 1: return BETTER_RED();
            case 2: return BETTER_GREEN();
            case 3: return BETTER_YELLOW();
            case 4: return BETTER_BLUE();
            case 5: return BETTER_PURPLE();
            case 6: return BETTER_CYAN();
            case 7: return BETTER_WHITE();
            default: return BLACK();
        }
    }

    template struct color_base<color3>;
    template struct color_base<color3f>;
    template struct color_base<color>;
    template struct color_base<colorf>;
}