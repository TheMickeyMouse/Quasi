#include "Color.h"

#include "Random.h"
#include "Vector.h"

namespace Quasi::Math::details {
#pragma region Util Funcs
        template <class T> T convert_type(auto channel) {
            if constexpr (std::is_same_v<decltype(channel), T>) return channel;
            else if constexpr (std::is_same_v<decltype(channel), byte> && std::is_same_v<T, f32>) return (f32)channel / 255.0f;
            else if constexpr (std::is_same_v<decltype(channel), f32> && std::is_same_v<T, byte>) return (byte)(channel * 255.0f);
            else return nullptr;
        }
        template byte  convert_type<byte, byte>(byte);
        template byte  convert_type<byte, f32> (f32);
        template f32   convert_type<f32,  byte>(byte);
        template f32   convert_type<f32,  f32> (f32);

        template <class T> T channel_from_hexcode(char D0, char D1) {
            return convert_type<T>((byte)((D0 - (D0 < ':' ? '0' : 'W')) << 4 | (D1 - (D1 < ':' ? '0' : 'W'))));
        }
        template byte channel_from_hexcode<byte>(char, char);
        template f32  channel_from_hexcode<f32> (char, char);

        fVector3 rgb2hsl(f32 r, f32 g, f32 b) {
            // https://stackoverflow.com/a/9493060
            const f32 max = std::max({r, g, b}), min = std::min({r, g, b});
            f32 h = 0, l = (max + min) / 2;

            if (max == min) // NOLINT(clang-diagnostic-f32-equal) no need to worry abt this
                return { 0, 0, l }; // achromatic

            const f32 d = max - min;
            f32 s = d / (l > 0.5f ? (2 - max - min) : (max + min));
            if (max == r) h = (g - b) / d + (g < b ? 6.0f : 0.0f);  // NOLINT(clang-diagnostic-f32-equal)
            if (max == g) h = (b - r) / d + 2;                      // NOLINT(clang-diagnostic-f32-equal)
            if (max == b) h = (r - g) / d + 4;                      // NOLINT(clang-diagnostic-f32-equal)
            h /= 6;

            return { h, s, l };
        }

        f32 hue2rgb(f32 p, f32 q, f32 t) {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0f/6) return p + (q - p) * 6 * t;
            if (t < 1.0f/2) return q;
            if (t < 2.0f/3) return p + (q - p) * (2.0f/3 - t) * 6;
            return p;
        }
    
        fVector3 hsl2rgb(f32 h, f32 s, f32 l) {
            f32 r, g, b;
            if (s == 0) {  // NOLINT(clang-diagnostic-f32-equal)
                r = g = b = l; // achromatic
            } else {
                const f32 q = l < 0.5 ? l * (1 + s) : l + s - l * s;
                const f32 p = 2 * l - q;
                r = hue2rgb(p, q, h + 1.0f/3);
                g = hue2rgb(p, q, h);
                b = hue2rgb(p, q, h - 1.0f/3);
            }
            return { r, g, b };
        }

        fVector3 rgb2hsv(f32 r, f32 g, f32 b) {
            // https://stackoverflow.com/a/6930407
            const f32 max = std::max({r, g, b}), min = std::min({r, g, b});
            const f32 v = max, d = max - min;
            if (v == 0) // NOLINT(clang-diagnostic-f32-equal)
                return { NAN, 0, v }; // acromatic
            f32 s = d / v;

            if (d == 0)  // NOLINT(clang-diagnostic-f32-equal)
                return { 0, 0, v }; // acromatic

            f32 h = 0;
            if (max == r) h = g - b;      // NOLINT(clang-diagnostic-f32-equal)
            if (max == g) h = 2 + b - r;  // NOLINT(clang-diagnostic-f32-equal)
            if (max == b) h = 4 + r - g;  // NOLINT(clang-diagnostic-f32-equal)
            h /= d * 6;
            h += (f32)(h < 0);
            return { h, s, v };
        }

        fVector3 hsv2rgb(f32 h, f32 s, f32 v) {
            if (s == 0)  // NOLINT(clang-diagnostic-f32-equal)
                return { v }; // Achromatic case
            h *= 6;
            const int i = (int)h;
            const f32 f = h - (f32)i;

            f32 p = v * (1 - s);
            f32 q = v * (1 - f * s);
            f32 t = v * (1 - (1 - f) * s);

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

        f32 overlay_channel(f32 a, f32 b) {
            return a < 0.5f ? 2 * a * b : 1 - 2 * (1 - a) * (1 - b);
        }

        byte overlay_channel(byte a, byte b) {
            return a < 128 ? a * b * 255 / 2 : 255 - 255 * (1 - a) * (1 - b) / 2;
        }

        bool channel_loose_eq(f32 a, f32 b) { // loose equality
            return std::abs(a - b) < (1 / 255.0f);
        }
        
        bool color_loose_eq(const f32* a, const f32* b) {
            return channel_loose_eq(a[0], b[0]) && channel_loose_eq(a[1], b[1]) && channel_loose_eq(a[2], b[2]);
        }

        bool color4_loose_eq(const f32* a, const f32* b) {
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
#pragma endregion

#pragma region Color Impl

        template <class TColor>
        typename color_base<TColor>::scalar color_base<TColor>::alpha(scalar def) const {
            if constexpr (traits_has_alpha) return color().a;
            else return def;
        }

        template <class TColor> TColor color_base<TColor>::from_hex(Str hex) {
            if constexpr (traits_has_alpha)
                return hex.size() > 6 ? TColor {
                    details::channel_from_hexcode<scalar>(hex[0], hex[1]),
                    details::channel_from_hexcode<scalar>(hex[2], hex[3]),
                    details::channel_from_hexcode<scalar>(hex[4], hex[5]),
                    details::channel_from_hexcode<scalar>(hex[6], hex[7])
                } : TColor {
                    details::channel_from_hexcode<scalar>(hex[0], hex[1]),
                    details::channel_from_hexcode<scalar>(hex[2], hex[3]),
                    details::channel_from_hexcode<scalar>(hex[4], hex[5]),
                };
            else return TColor {
                details::channel_from_hexcode<scalar>(hex[0], hex[1]),
                details::channel_from_hexcode<scalar>(hex[2], hex[3]),
                details::channel_from_hexcode<scalar>(hex[4], hex[5]),
            };
        }

        template <class TColor> String color_base<TColor>::hexcode() const {
            if constexpr (traits_has_alpha) return std::format("{}{}{}{}{}{}{}{}",
                details::hexdigit_2(color().r),
                details::hexdigit_1(color().r),
                details::hexdigit_2(color().g),
                details::hexdigit_1(color().g),
                details::hexdigit_2(color().b),
                details::hexdigit_1(color().b),
                details::hexdigit_2(color().a),
                details::hexdigit_1(color().a)
            ); else return std::format("{}{}{}{}{}{}",
                details::hexdigit_2(color().r),
                details::hexdigit_1(color().r),
                details::hexdigit_2(color().g),
                details::hexdigit_1(color().g),
                details::hexdigit_2(color().b),
                details::hexdigit_1(color().b)
            );
        }

        template <class TColor> bool color_base<TColor>::eq(const TColor& other) const {
            if constexpr (traits_has_alpha)
                return color().r == other.r && color().g == other.g && color().b == other.b && color().a == other.a;
            else
                return color().r == other.r && color().g == other.g && color().b == other.b;
        }
        template <class TColor> bool color_base<TColor>::operator==(const color_base& other) const { return eq(other.color()); }

        template <class TColor> bool color_base<TColor>::neq(const TColor& other) const { return !eq(other); }

        template <class TColor> bool color_base<TColor>::loose_eq(const TColor& other) const {
            if constexpr (traits_is_floating)
                if constexpr (traits_has_alpha) return details::color4_loose_eq(begin(), other.begin());
                else return details::color_loose_eq(begin(), other.begin());
            else return eq(other);
        }

        template <class TColor> TColor color_base<TColor>::neg() const {
            return details::make_color<TColor>(
                CHANNEL_MAX_VALUE - color().r,
                CHANNEL_MAX_VALUE - color().g,
                CHANNEL_MAX_VALUE - color().b,
                CHANNEL_MAX_VALUE - alpha()
            );
        }

        template <class TColor> TColor color_base<TColor>::lerp(const TColor& other, f32 t) const {
            const f32 s = 1 - t;
            return make_color<TColor>(
                convert_type<f32>(color().r) * s + convert_type<f32>(other.r) * t,
                convert_type<f32>(color().g) * s + convert_type<f32>(other.g) * t,
                convert_type<f32>(color().b) * s + convert_type<f32>(other.b) * t,
                convert_type<f32>(alpha())   * s + convert_type<f32>(other.alpha()) * t
            );
        }

        template <class TColor> TColor color_base<TColor>::mul(const TColor& other) const {
            return details::make_color<TColor>(
                color().r * other.r / CHANNEL_MAX_VALUE,
                color().g * other.g / CHANNEL_MAX_VALUE,
                color().b * other.b / CHANNEL_MAX_VALUE,
                alpha() * other.alpha() / CHANNEL_MAX_VALUE
            );
        }

        template <class TColor> TColor color_base<TColor>::screen(const TColor& other) const { return neg().mul(other).neg(); }
        template <class TColor> TColor color_base<TColor>::overlay(const TColor& other) const {
            return make_color<TColor>(
                overlay_channel(color().r, other.r), overlay_channel(color().g, other.g),
                overlay_channel(color().b, other.b), overlay_channel(alpha(), other.alpha())
            );
        }

        template <class TColor> typename color_base<TColor>::without_alpha_t color_base<TColor>::mul_alpha() const {
            return { color().r * alpha() / CHANNEL_MAX_VALUE, color().g * alpha() / CHANNEL_MAX_VALUE, color().b * alpha() / CHANNEL_MAX_VALUE, };
        }

        template <class TColor> f32 color_base<TColor>::luminance() const {
            // https://en.wikipedia.org/wiki/Relative_luminance
            return (0.2126f * details::convert_type<f32>(color().r) + 0.7152f * details::convert_type<f32>(color().g) + 0.0722f * details::convert_type<f32>(color().b));
        }

        template <class TColor> bVector3 color_base<TColor>::as_rgb()   const { return { details::convert_type<byte>(color().r), details::convert_type<byte>(color().g), details::convert_type<byte>(color().b) }; }
        template <class TColor> bVector4 color_base<TColor>::as_rgba()  const { return as_rgb().with_w(details::convert_type<byte>(alpha())); }
        template <class TColor> fVector3 color_base<TColor>::as_rgbf()  const { return { details::convert_type<f32>(color().r), details::convert_type<f32>(color().g), details::convert_type<f32>(color().b) }; }
        template <class TColor> fVector4 color_base<TColor>::as_rgbaf() const { return as_rgbf().with_w(details::convert_type<f32>(alpha())); }

        template <class TColor> fVector3 color_base<TColor>::as_hsl()  const { return details::rgb2hsl(details::convert_type<f32>(color().r), details::convert_type<f32>(color().g), details::convert_type<f32>(color().b)); }
        template <class TColor> fVector4 color_base<TColor>::as_hsla() const { return as_hsl().with_w(details::convert_type<f32>(alpha())); }

        template <class TColor> TColor color_base<TColor>::from_hsl(f32 hue, f32 saturation, f32 lightness) requires (!traits_has_alpha) {
            return hsl2rgb(hue / 360.0f, saturation, lightness).to_color3();
        }
        template <class TColor> TColor color_base<TColor>::from_hsl(f32 hue, f32 saturation, f32 lightness, f32 alpha) requires traits_has_alpha {
            return hsl2rgb(hue / 360.0f, saturation, lightness).with_w(alpha).to_color();
        }

        template <class TColor> TColor color_base<TColor>::from_hsl(const fVector3& hsl) requires (!traits_has_alpha) { return from_hsl(hsl.x, hsl.y, hsl.z); }
        template <class TColor> TColor color_base<TColor>::from_hsl(const fVector4& hsla) requires traits_has_alpha { return from_hsl(hsla.x, hsla.y, hsla.z, hsla.w); }

        template <class TColor> fVector3 color_base<TColor>::as_hsv()  const { return details::rgb2hsv(details::convert_type<f32>(color().r), details::convert_type<f32>(color().g), details::convert_type<f32>(color().b)); }
        template <class TColor> fVector4 color_base<TColor>::as_hsva() const { return as_hsv().with_w(details::convert_type<f32>(alpha())); }

        template <class TColor> TColor color_base<TColor>::from_hsv(f32 hue, f32 saturation, f32 value) requires (!traits_has_alpha) {
            return hsv2rgb(hue / 360.0f, saturation, value).to_color3();
        }
        template <class TColor> TColor color_base<TColor>::from_hsv(f32 hue, f32 saturation, f32 value, f32 alpha) requires traits_has_alpha {
            return hsv2rgb(hue / 360.0f, saturation, value).with_w(alpha).to_color();
        }

        template <class TColor> TColor color_base<TColor>::from_hsv(const fVector3& hsv) requires (!traits_has_alpha) { return from_hsv(hsv.x, hsv.y, hsv.z); }
        template <class TColor> TColor color_base<TColor>::from_hsv(const fVector4& hsva) requires traits_has_alpha { return from_hsv(hsva.x, hsva.y, hsva.z, hsva.w); }
        template <class TColor> typename color_base<TColor>::without_alpha_t color_base<TColor>::rgb() const { return { color().r, color().g, color().b }; }
        template <class TColor> typename color_base<TColor>::with_alpha_t color_base<TColor>::with_alpha(scalar alpha) const { return { color().r, color().g, color().b, alpha }; }
        template <class TColor> typename color_base<TColor>::with_alpha_t color_base<TColor>::rgb1() const { return { color().r, color().g, color().b, CHANNEL_MAX_VALUE }; }

        template <class TColor> color_base<TColor>::operator without_alpha_t() const requires traits_has_alpha { return rgb(); }
        template <class TColor> color_base<TColor>::operator with_alpha_t() const requires (!traits_has_alpha) { return rgb1(); }
        template <class TColor> color_base<TColor>::operator alternate_data_form() const {
            using alt = typename color_traits<alternate_data_form>::scalar;
            return details::make_color<alternate_data_form>(
                details::convert_type<alt>(color().r),
                details::convert_type<alt>(color().g),
                details::convert_type<alt>(color().b),
                details::convert_type<alt>(alpha())
            );
        }

        template<class TColor>
        TColor color_base<TColor>::random(RandomGenerator& rg, f32 sat, f32 val) {
            return from_hsv(rg.Get(0.0f, 360.0f), sat, val);
        }

        template <class TColor> TColor color_base<TColor>::color_id(int id) {
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

        template struct color_base<Color3>;
        template struct color_base<fColor3>;
        template struct color_base<Color>;
        template struct color_base<fColor>;

#pragma endregion
}