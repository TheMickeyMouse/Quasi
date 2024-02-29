#include "Vector.h"

namespace Maths {
    namespace Color {
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
            float v = max, d = max - min;
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
                return v; // Achromatic case
            h *= 6;
            int i = (int)h;
            float f = h - (float)i;

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

        bool channel_eq(float a, float b) { // loose equality
            return std::abs(a - b) < (1 / 255.0f);
        }
        
        bool color_eq(const float* a, const float* b) {
            return channel_eq(a[0], b[0]) && channel_eq(a[1], b[1]) && channel_eq(a[2], b[2]);
        }

        char channel_hex(const char i) {
            return (i < 10) ? (char)('0' + i) : (char)('a' + i - 10);
        }
    }
    
#define CM(F) STDU_IF_ELSE(F, (255.0f), (1))
#define CM_N(F) STDU_IF_ELSE(F, (1), (255))
#define A(HAS_A) STDU_IF_ELSE(HAS_A, (a), (1))
#define S scalar
#define COLOR_IMPL(T, HAS_A, F) \
    std::string T::hexcode() const { \
        return std::format(STDU_IF_ELSE(HAS_A, ("#{}{}{}{}{}{}{}{}"), ("#{}{}{}{}{}{}")), \
            Color::channel_hex((char)(r * CM(F)) / 16), \
            Color::channel_hex((char)(r * CM(F)) % 16), \
            Color::channel_hex((char)(g * CM(F)) / 16), \
            Color::channel_hex((char)(g * CM(F)) % 16), \
            Color::channel_hex((char)(b * CM(F)) / 16), \
            Color::channel_hex((char)(b * CM(F)) % 16) \
            STDU_IF(HAS_A, , \
                Color::channel_hex((int)(a * CM(F)) / 16), \
                Color::channel_hex((int)(a * CM(F)) % 16)) \
        ); \
    } \
    \
    bool T::eq(const T& other) const { return r == other.r && g == other.g && b == other.b STDU_IF(HAS_A, && r == other.r); } \
    bool T::loose_eq(const T& other) const { \
        return STDU_IF_ELSE(F, \
            (Color::color_eq((const float*)this, (const float*)&other) \
             STDU_IF(HAS_A, && Color::channel_eq(a, other.a))), \
            (eq(other)) \
        ); \
    } \
    bool T::neq(const T& other) const { return !eq(other); } \
    bool T::operator==(const T& other) const { return eq(other); } \
    \
    T T::neg() const { return { (S)(CM_N(F) - r), (S)(CM_N(F) - g), (S)(CM_N(F) - b) STDU_IF(HAS_A, , (S)(CM_N(F) - a))}; } \
    T T::lerp(const T& other, float t) const { \
        float s = (1 - t); \
        return { \
            (S)(((float)r*s + (float)other.r*t) / CM_N(F)), (S)(((float)g*s + (float)other.g*t) / CM_N(F)), \
            (S)(((float)b*s + (float)other.b*t) / CM_N(F)) STDU_IF(HAS_A, , (S)(((float)a*s + (float)other.a*t) / CM_N(F))) \
        }; \
    } \
    T T::mul(const T& other) const { \
        return { \
            (S)(r * other.r / CM_N(F)), (S)(g * other.g / CM_N(F)), \
            (S)(b * other.b / CM_N(F)) STDU_IF(HAS_A, , (S)(a * other.a / CM_N(F))) \
        }; \
    } \
    T T::screen(const T& other) const { return neg().mul(other).neg(); } \
    T T::overlay(const T& other) const { \
        return { \
            Color::overlay_channel(r, other.r), Color::overlay_channel(g, other.g), \
            Color::overlay_channel(b, other.b) STDU_IF(HAS_A, , Color::overlay_channel(r, other.r)) \
        }; \
    } \
    STDU_IF(HAS_A, T::without_alpha_t T::mul_alpha() const { \
        return { (S)(r * a / CM_N(F)), (S)(g * a / CM_N(F)), (S)(b * a / CM_N(F)) }; \
    }) \
    \
    float T::luminance() const { return (0.2126f * (float)r + 0.7152f * (float)g + 0.0722f * (float)b) / CM_N(F); /*https://en.wikipedia.org/wiki/Relative_luminance*/ } \
    bvec3 T::as_rgb()   const { return { (uchar)(r * CM(F)), (uchar)(g * CM(F)), (uchar)(b * CM(F)) }; } \
    bvec4 T::as_rgba()  const { return as_rgb() STDU_IF(HAS_A, .with_w((uchar)(A(HAS_A) * CM(F)))); } \
    fvec3 T::as_rgbf()  const { return { (float)r / CM_N(F), (float)g / CM_N(F), (float)b / CM_N(F) }; } \
    fvec4 T::as_rgbaf() const { return as_rgbf() STDU_IF(HAS_A, .with_w((float)A(HAS_A) / CM_N(F))); } \
    fvec3 T::as_hsl()   const { return Color::rgb2hsl((float)r / CM_N(F), (float)g / CM_N(F), (float)b / CM_N(F)); } \
    fvec4 T::as_hsla()  const { return as_hsl() STDU_IF(HAS_A, .with_w((float)A(HAS_A) / CM_N(F))); } \
    T T::from_hsl(float hue, float saturation, float lightness STDU_IF(HAS_A, , float alpha)) { \
        return Color::hsl2rgb(hue / 360.0f, saturation, lightness) STDU_IF(HAS_A, .with_w(alpha)) .color(); \
    } \
    T T::from_hsl(STDU_IF_ELSE(HAS_A, (fvec4 hsla), (fvec3 hsl))) { \
        return STDU_IF_ELSE(HAS_A, (from_hsl(hsla.x, hsla.y, hsla.z, hsla.w)), (from_hsl(hsl.x, hsl.y, hsl.z))); \
    } \
    fvec3 T::as_hsv()  const { return Color::rgb2hsv((float)r / CM_N(F), (float)g / CM_N(F), (float)b / CM_N(F)); } \
    fvec4 T::as_hsva() const { return as_hsv() STDU_IF(HAS_A, .with_w((float)A(HAS_A) / CM_N(F))); } \
    T T::from_hsv(float hue, float saturation, float value STDU_IF(HAS_A, , float alpha)) { \
        return Color::hsv2rgb(hue / 360.0f, saturation, value) STDU_IF(HAS_A, .with_w(alpha)) .color(); \
    } \
    T T::from_hsv(STDU_IF_ELSE(HAS_A, (fvec4 hsva), (fvec3 hsv))) { \
        return STDU_IF_ELSE(HAS_A, (from_hsv(hsva.x, hsva.y, hsva.z, hsva.w)), (from_hsv(hsv.x, hsv.y, hsv.z))); \
    } \
    \
    STDU_IF(HAS_A, T::without_alpha_t T::rgb() const { return { r, g, b }; }) \
    T STDU_IF_NOT(HAS_A, ::with_alpha_t) T::with_alpha(scalar alpha) const { return { r, g, b, alpha }; } \
    T STDU_IF_NOT(HAS_A, ::with_alpha_t) T::rgb1() const { return { r, g, b, CM_N(F) }; } \
    \
    STDU_IF_ELSE(HAS_A, (T::operator T::without_alpha_t() const { return rgb(); }), \
                        (T::operator T::with_alpha_t()    const { return with_alpha(); })) \
    \
    STDU_IF_ELSE(HAS_A, (STDU_IF_ELSE(F, (T::operator color()   const { return { (uchar)(r * 255), (uchar)(g * 255), (uchar)(b * 255), (uchar)(a * 255) }; }), \
                                         (T::operator colorf()  const { return { (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f }; } ))), \
                        (STDU_IF_ELSE(F, (T::operator color3()  const { return { (uchar)(r * 255), (uchar)(g * 255), (uchar)(b * 255) }; }), \
                                         (T::operator color3f() const { return { (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f }; } )))) \
    T T::color_id(const int id) { \
        switch (id) { \
            case  0: return BETTER_BLACK(); \
            case  1: return BETTER_RED(); \
            case  2: return BETTER_GREEN(); \
            case  3: return BETTER_YELLOW(); \
            case  4: return BETTER_BLUE(); \
            case  5: return BETTER_PURPLE(); \
            case  6: return BETTER_CYAN(); \
            case  7: return BETTER_WHITE(); \
            default: return BLACK(); \
        } \
    } \

    COLOR_IMPL(color3f, 0, 1);
    COLOR_IMPL(color3,  0, 0);
    COLOR_IMPL(colorf,  1, 1);
    COLOR_IMPL(color,   1, 0);
#undef COLOR_IMPL
#undef A
#undef CM
#undef CM_N
#undef S
    
// #define COLOR_CONSTANT(T, C) \
//     const T T::BLACK 	  = { 0   / C, 0   / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::DARK_GRAY  = { 64  / C, 64  / C, 64  / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::GRAY 	  = { 128 / C, 128 / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIGHT_GRAY = { 192 / C, 192 / C, 192 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::WHITE 	  = { 255 / C, 255 / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T& T::SILVER = T::LIGHT_GRAY; \
//     \
//     const T T::RED     = { 255 / C, 0   / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::ORANGE  = { 255 / C, 128 / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::YELLOW  = { 255 / C, 255 / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIME    = { 128 / C, 255 / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::GREEN   = { 0   / C, 255 / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::SEAFOAM = { 0   / C, 255 / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::CYAN    = { 0   / C, 255 / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::AZURE   = { 0   / C, 128 / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BLUE    = { 0   / C, 0   / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::PURPLE  = { 128 / C, 0   / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::MAGENTA = { 255 / C, 0   / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::ROSE    = { 255 / C, 0   / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     \
//     const T T::LIGHT_RED     = { 255 / C, 128 / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIGHT_YELLOW  = { 255 / C, 255 / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIGHT_GREEN   = { 128 / C, 255 / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIGHT_CYAN    = { 128 / C, 255 / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIGHT_BLUE    = { 128 / C, 128 / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::LIGHT_MAGENTA = { 255 / C, 128 / C, 255 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T& T::SALMON     = T::LIGHT_RED;     \
//     const T& T::LEMON      = T::LIGHT_YELLOW;  \
//     const T& T::MINT       = T::LIGHT_GREEN;   \
//     const T& T::SKY        = T::LIGHT_CYAN;    \
//     const T& T::CORNFLOWER = T::LIGHT_BLUE;    \
//     const T& T::PINK       = T::LIGHT_MAGENTA; \
//     \
//     const T T::DARK_RED     = { 128 / C, 0   / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::DARK_YELLOW  = { 128 / C, 128 / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::DARK_GREEN   = { 0   / C, 128 / C, 0   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::DARK_CYAN    = { 0   / C, 128 / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::DARK_BLUE    = { 0   / C, 0   / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::DARK_MAGENTA = { 128 / C, 0   / C, 128 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T& T::MAROON  = DARK_RED;     \
//     const T& T::OLIVE   = DARK_YELLOW;  \
//     const T& T::AVOCADO = DARK_GREEN;   \
//     const T& T::TEAL    = DARK_CYAN;    \
//     const T& T::NAVY    = DARK_BLUE;    \
//     const T& T::VIOLET  = DARK_MAGENTA; \
//     \
//     const T T::BETTER_RED        = { 255 / C, 48  / C, 48  / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_ORANGE     = { 255 / C, 128 / C, 043 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_YELLOW     = { 255 / C, 192 / C, 020 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_LIME       = { 135 / C, 247 / C, 037 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_GREEN      = { 12  / C, 133 / C, 1   / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_AQUA       = { 40  / C, 206 / C, 247 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_CYAN       = { 23  / C, 160 / C, 179 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_BLUE       = { 16  / C, 65  / C, 227 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_PURPLE     = { 120 / C, 24  / C, 237 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_MAGENTA    = { 203 / C, 42  / C, 235 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_PINK       = { 250 / C, 87  / C, 198 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_BROWN      = { 69  / C, 32  / C, 13  / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_BLACK      = { 22  / C, 25  / C, 29  / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_DARK_GRAY  = { 76  / C, 75  / C, 93  / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_GRAY       = { 116 / C, 126 / C, 134 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_LIGHT_GRAY = { 175 / C, 186 / C, 193 / C }; /* NOLINT(bugprone-macro-parentheses) */ \
//     const T T::BETTER_WHITE      = { 232 / C, 247 / C, 249 / C }  /* NOLINT(bugprone-macro-parentheses) */ \
//
//     COLOR_CONSTANT(color, 1);
//     COLOR_CONSTANT(color3, 1);
//     COLOR_CONSTANT(colorf, 255.0f);
//     COLOR_CONSTANT(color3f, 255.0f);
//
//     const colorf colorf::CLEAR = { 0, 0, 0, 1   };
//     const color  color ::CLEAR = { 0, 0, 0, 255 };
// #undef COLOR_CONSTANT
}