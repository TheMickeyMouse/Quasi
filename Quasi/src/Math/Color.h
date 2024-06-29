#pragma once
#include "Vector.h"

namespace Quasi::Math {
    struct Color3;
    struct fColor3;
    struct Color;
    struct fColor;

    template <class T>
    concept ColorLike =
        std::is_same_v<T, Color3> || std::is_same_v<T, fColor3> ||
        std::is_same_v<T, Color>  || std::is_same_v<T, fColor>;

    template <bool HasAlpha, bool IsFloaty>
    using ColorFrom = std::conditional_t<HasAlpha,
        std::conditional_t<IsFloaty, fColor,  Color>,
        std::conditional_t<IsFloaty, fColor3, Color3>
    >;

    namespace details {
        template <class> struct color_traits;
        template <> struct color_traits<Color3>  { static constexpr bool has_alpha = false, is_floating = false; using scalar = byte; };
        template <> struct color_traits<fColor3> { static constexpr bool has_alpha = false, is_floating = true;  using scalar = f32;  };
        template <> struct color_traits<Color>   { static constexpr bool has_alpha = true,  is_floating = false; using scalar = byte; };
        template <> struct color_traits<fColor>  { static constexpr bool has_alpha = true,  is_floating = true;  using scalar = f32;  };

        template <class> struct color_base;
    }

#define NODISC [[nodiscard]]
    template <class TColor>
    struct details::color_base {
        static constexpr bool traits_has_alpha = color_traits<TColor>::has_alpha,
                              traits_is_floating = color_traits<TColor>::is_floating;
        using scalar = typename color_traits<TColor>::scalar;
    private:
        static constexpr scalar get_channel_max() {
            if constexpr (traits_is_floating) return 1.0f;
            else return (byte)255;
        }
    public:
        static constexpr scalar CHANNEL_MAX_VALUE = get_channel_max();
        static constexpr u32 dimension = 3 + traits_has_alpha;

        using without_alpha_t = ColorFrom<false, traits_is_floating>;
        using with_alpha_t    = ColorFrom<true,  traits_is_floating>;
        using alternate_data_form = ColorFrom<traits_has_alpha, !traits_is_floating>;

        static constexpr Str params = traits_has_alpha ? "rgba" : "rgb";

    private:
        NODISC const TColor& color() const { return *(const TColor*)this; }
        TColor& color() { return *(TColor*)this; }
    public:
        scalar* begin() { return (scalar*)this; }
        scalar* end()   { return begin() + dimension; }
        NODISC const scalar* begin()  const { return (const scalar*)this; }
        NODISC const scalar* cbegin() const { return (const scalar*)this; }
        NODISC const scalar* end()    const { return begin() + dimension; }
        NODISC const scalar* cend()   const { return cbegin() + dimension; }

        NODISC scalar alpha(scalar def = CHANNEL_MAX_VALUE) const;

        static TColor from_hex(Str hex);
        NODISC String hexcode() const;

        NODISC bool eq(const TColor& other) const;
        NODISC bool loose_eq(const TColor& other) const;
        NODISC bool neq(const TColor& other) const;
        NODISC bool operator==(const color_base& other) const;

        NODISC TColor neg() const;
        NODISC TColor lerp(const TColor& other, f32 t) const;
        NODISC TColor mul(const TColor& other) const;
        NODISC TColor screen(const TColor& other) const;
        NODISC TColor overlay(const TColor& other) const;
        NODISC without_alpha_t mul_alpha() const;

        NODISC f32 luminance() const;

        NODISC bVector3 as_rgb()   const;
        NODISC bVector4 as_rgba()  const;
        NODISC fVector3 as_rgbf()  const;
        NODISC fVector4 as_rgbaf() const;
        NODISC fVector3 as_hsl()   const;
        NODISC fVector4 as_hsla()  const;
        static TColor from_hsl(f32 hue, f32 saturation = 1, f32 lightness = 1) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], L: [0-1] */
        static TColor from_hsl(f32 hue, f32 saturation = 1, f32 lightness = 1, f32 alpha = 1) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], L: [0-1] */
        static TColor from_hsl(const fVector3& hsl) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], L: [0-1] */
        static TColor from_hsl(const fVector4& hsla) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], L: [0-1] */
        NODISC fVector3 as_hsv()  const;
        NODISC fVector4 as_hsva() const;
        static TColor from_hsv(f32 hue, f32 saturation = 1, f32 value = 1) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], val: [0-1] */
        static TColor from_hsv(f32 hue, f32 saturation = 1, f32 value = 1, f32 alpha = 1) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], val: [0-1] */
        static TColor from_hsv(const fVector3& hsv) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], val: [0-1] */
        static TColor from_hsv(const fVector4& hsva) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], val: [0-1] */

        NODISC without_alpha_t rgb() const;
        NODISC with_alpha_t with_alpha(scalar alpha = CHANNEL_MAX_VALUE) const;
        NODISC with_alpha_t rgb1() const;

        NODISC operator without_alpha_t() const requires traits_has_alpha;
        NODISC operator with_alpha_t() const requires (!traits_has_alpha);
        NODISC operator alternate_data_form() const;

        static TColor random(RandomGenerator& rg, f32 sat, f32 val);

#pragma region Color Constants
        static TColor color_id(int id);

        static TColor BLACK()      { return from_hex("000000"); } /* solid black: rgb(000, 000, 000) or #000000 */
        static TColor DARK_GRAY()  { return from_hex("404040"); } /*   25% white: rgb(064, 064, 064) or #404040 */
        static TColor GRAY()       { return from_hex("808080"); } /*   50% white: rgb(128, 128, 128) or #808080 */
        static TColor LIGHT_GRAY() { return from_hex("c0c0c0"); } /*   75% white: rgb(192, 192, 192) or #c0c0c0 */
        static TColor WHITE()      { return from_hex("ffffff"); } /* solid white: rgb(255, 255, 255) or #ffffff */
        static TColor SILVER() { return LIGHT_GRAY(); }  /* same as light_gray, rgb(192, 192, 192) or #c0c0c0 */

        static TColor RED()     { return from_hex("ff0000"); } /* red    (100%   red):              rgb(255, 000, 000) or #ff0000 */
        static TColor ORANGE()  { return from_hex("ff8000"); } /* orange (100%   red +  50% green): rgb(255, 128, 000) or #ff8000 */
        static TColor YELLOW()  { return from_hex("ffff00"); } /* yellow (100%   red + 100% green): rgb(255, 255, 000) or #ffff00 */
        static TColor LIME()    { return from_hex("80ff00"); } /* lime   (100% green +  50%   red): rgb(128, 255, 000) or #80ff00 */
        static TColor GREEN()   { return from_hex("00ff00"); } /* green  (100% green):              rgb(000, 255, 000) or #00ff00 */
        static TColor SEAFOAM() { return from_hex("00ff80"); } /* seafoam(100% green +  50%  blue): rgb(000, 255, 128) or #00ff80 */
        static TColor CYAN()    { return from_hex("00ffff"); } /* cyan   (100% green + 100%  blue): rgb(000, 255, 255) or #00ffff */
        static TColor AZURE()   { return from_hex("0080ff"); } /* azure  (100%  blue +  50% green): rgb(000, 128, 255) or #0080ff */
        static TColor BLUE()    { return from_hex("0000ff"); } /* blue   (100%  blue):              rgb(000, 000, 255) or #0000ff */
        static TColor PURPLE()  { return from_hex("8000ff"); } /* purple (100%  blue +  50%   red): rgb(128, 000, 255) or #8000ff */
        static TColor MAGENTA() { return from_hex("ff00ff"); } /* magenta(100%  blue + 100%   red): rgb(255, 000, 255) or #ff00ff */
        static TColor ROSE()    { return from_hex("ff0080"); } /* rose   (100%   red +  50%  blue): rgb(255, 000, 128) or #ff0080 */

        static TColor LIGHT_RED()     { return from_hex("ff8080"); } /* light_red    (100%   red +  50% green +  50%  blue): rgb(255, 128, 128) or #ff8080 */
        static TColor LIGHT_YELLOW()  { return from_hex("ffff80"); } /* light_yellow (100%   red + 100% green +  50%  blue): rgb(255, 255, 128) or #ffff80 */
        static TColor LIGHT_GREEN()   { return from_hex("80ff80"); } /* light_green  ( 50%   red + 100% green +  50%  blue): rgb(128, 255, 128) or #80ff80 */
        static TColor LIGHT_CYAN()    { return from_hex("80ffff"); } /* light_cyan   ( 50%   red + 100% green + 100%  blue): rgb(128, 255, 255) or #80ffff */
        static TColor LIGHT_BLUE()    { return from_hex("8080ff"); } /* light_blue   ( 50%   red +  50% green + 100%  blue): rgb(128, 128, 255) or #8080ff */
        static TColor LIGHT_MAGENTA() { return from_hex("ff80ff"); } /* light_magenta(100%   red +  50% green + 100%  blue): rgb(255, 128, 255) or #ff80ff */
        static TColor SALMON()     { return LIGHT_RED();     } /* same as light_red,     rgb(255, 128, 128) or #ff8080 */
        static TColor LEMON()      { return LIGHT_YELLOW();  } /* same as light_yellow,  rgb(255, 255, 128) or #ffff80 */
        static TColor MINT()       { return LIGHT_GREEN();   } /* same as light_green,   rgb(128, 255, 128) or #80ff80 */
        static TColor SKY()        { return LIGHT_CYAN();    } /* same as light_cyan,    rgb(128, 255, 255) or #80ffff */
        static TColor CORNFLOWER() { return LIGHT_BLUE();    } /* same as light_blue,    rgb(128, 128, 255) or #8080ff */
        static TColor PINK()       { return LIGHT_MAGENTA(); } /* same as light_magenta, rgb(255, 128, 255) or #ff80ff */

        static TColor DARK_RED()     { return from_hex("800000"); } /* dark_red   ( 50%   red):              rgb(128, 000, 000) or #800000 */
        static TColor DARK_YELLOW()  { return from_hex("808000"); } /* dark_yellow( 50%   red +  50% green): rgb(128, 128, 000) or #808000 */
        static TColor DARK_GREEN()   { return from_hex("008000"); } /* dark_green ( 50% green):              rgb(000, 128, 000) or #008000 */
        static TColor DARK_CYAN()    { return from_hex("008080"); } /* dark_cyan  ( 50% green +  50%  blue): rgb(000, 128, 128) or #008080 */
        static TColor DARK_BLUE()    { return from_hex("000080"); } /* dark_blue  ( 50%  blue):              rgb(000, 000, 128) or #000080 */
        static TColor DARK_MAGENTA() { return from_hex("800080"); } /* dark_purple( 50%  blue +  50%   red): rgb(128, 000, 128) or #800080 */
        static TColor MAROON()  { return DARK_RED();     } /* same as dark_red,    rgb(128, 000, 000) or #800000 */
        static TColor OLIVE()   { return DARK_YELLOW();  } /* same as dark_yellow, rgb(128, 128, 000) or #808000 */
        static TColor AVOCADO() { return DARK_GREEN();   } /* same as dark_green,  rgb(000, 128, 000) or #008000 */
        static TColor TEAL()    { return DARK_CYAN();    } /* same as dark_cyan,   rgb(000, 128, 128) or #008080 */
        static TColor NAVY()    { return DARK_BLUE();    } /* same as dark_blue,   rgb(000, 000, 128) or #000080 */
        static TColor VIOLET()  { return DARK_MAGENTA(); } /* same as dark_purple, rgb(128, 000, 128) or #800080 */

        static TColor BETTER_RED()        { return from_hex("ff3030"); } /* more appealing red:     rgb(255, 048, 048) or #ff3030 */
        static TColor BETTER_ORANGE()     { return from_hex("ff802b"); } /* more appealing orange:  rgb(255, 128, 043) or #ff802b */
        static TColor BETTER_YELLOW()     { return from_hex("ffc014"); } /* more appealing yellow:  rgb(255, 192, 020) or #ffc014 */
        static TColor BETTER_LIME()       { return from_hex("87f725"); } /* more appealing lime:    rgb(135, 247, 037) or #87f725 */
        static TColor BETTER_GREEN()      { return from_hex("0c8501"); } /* more appealing green:   rgb(012, 133, 001) or #0c8501 */
        static TColor BETTER_AQUA()       { return from_hex("28cef7"); } /* more appealing aqua:    rgb(040, 206, 247) or #28cef7 */
        static TColor BETTER_CYAN()       { return from_hex("17a0b3"); } /* more appealing cyan:    rgb(023, 160, 179) or #17a0b3 */
        static TColor BETTER_BLUE()       { return from_hex("1041e3"); } /* more appealing blue:    rgb(016, 065, 227) or #1041e3 */
        static TColor BETTER_PURPLE()     { return from_hex("7818ed"); } /* more appealing purple:  rgb(120, 024, 237) or #7818ed */
        static TColor BETTER_MAGENTA()    { return from_hex("cb2aeb"); } /* more appealing magenta: rgb(203, 042, 235) or #cb2aeb */
        static TColor BETTER_PINK()       { return from_hex("fa57c6"); } /* more appealing pink:    rgb(250, 087, 198) or #fa57c6 */
        static TColor BETTER_BROWN()      { return from_hex("45200d"); } /* more appealing brown:   rgb(069, 032, 013) or #45200d */
        static TColor BETTER_BLACK()      { return from_hex("16191d"); } /* more appealing black:   rgb(022, 025, 029) or #16191d */
        static TColor BETTER_DARK_GRAY()  { return from_hex("4c4b5d"); } /* more appealing gray:    rgb(076, 075, 093) or #4c4b5d */
        static TColor BETTER_GRAY()       { return from_hex("747e86"); } /* more appealing gray:    rgb(116, 126, 134) or #747e86 */
        static TColor BETTER_LIGHT_GRAY() { return from_hex("afbac1"); } /* more appealing gray:    rgb(175, 186, 193) or #afbac1 */
        static TColor BETTER_WHITE()      { return from_hex("e8f7f9"); } /* more appealing white:   rgb(232, 247, 249) or #e8f7f9 */

        static TColor CLEAR() requires traits_has_alpha { return from_hex("00000000"); } /* complete transparency. RGBA is (0, 0, 0, 0). */
#pragma endregion // Color Constants
    };

    struct Color3 : details::color_base<Color3> {
        byte r, g, b;

        Color3(byte s = 0) : r(s), g(s), b(s) {}
        Color3(byte r, byte g, byte b) : r(r), g(g), b(b) {}
        Color3(std::integral auto r, std::integral auto g,
               std::integral auto b) : Color3((byte)r, (byte)g, (byte)b) {}
    };

    struct fColor3 : details::color_base<fColor3> {
        f32 r, g, b;

        fColor3(f32 s = 0) : r(s), g(s), b(s) {}
        fColor3(f32 r, f32 g, f32 b) : r(r), g(g), b(b) {}
        fColor3(std::floating_point auto r, std::floating_point auto g,
                std::floating_point auto b) : fColor3((f32)r, (f32)g, (f32)b) {}
    };

    struct Color : details::color_base<Color> {
        byte r, g, b, a;

        Color(byte s = 0, byte a = 255) : r(s), g(s), b(s), a(a) {}
        Color(byte r, byte g, byte b, byte a = 255) : r(r), g(g), b(b), a(a) {}
        Color(std::integral auto r, std::integral auto g,
              std::integral auto b, std::integral auto a = 255) : Color((byte)r, (byte)g, (byte)b, (byte)a) {}
    };

    struct fColor : details::color_base<fColor> {
        f32 r, g, b, a;

        fColor(f32 s = 0, f32 a = 1) : r(s), g(s), b(s), a(a) {}
        fColor(f32 r, f32 g, f32 b, f32 a = 1) : r(r), g(g), b(b), a(a) {}
        fColor(std::floating_point auto r, std::floating_point auto g,
               std::floating_point auto b, std::floating_point auto a = 1) : fColor((f32)r, (f32)g, (f32)b, (f32)a) {}
    };

    namespace details {
        template <class T> T convert_type(auto channel);
        template <class T> T channel_from_hexcode(char D0, char D1);

        template <class T, class U> T make_color(U r, U g, U b, U a) {
            using S = typename color_traits<T>::scalar;
            using I = std::conditional_t<std::is_integral_v<U>, byte, f32>;
            if constexpr (color_traits<T>::has_alpha)
                return T {
                    details::convert_type<S>((I)r),
                    details::convert_type<S>((I)g),
                    details::convert_type<S>((I)b),
                    details::convert_type<S>((I)a)
                };
            else
                return T {
                    details::convert_type<S>((I)r),
                    details::convert_type<S>((I)g),
                    details::convert_type<S>((I)b)
                };
        }
    }

#undef NODISC

    template <class T> auto VectorN<3, T>::to_color(T alpha) const -> std::conditional_t<traits_float, fColor, Color> { return { x, y, z, alpha }; }
    template <class T> auto VectorN<3, T>::to_color3() const -> std::conditional_t<traits_float, fColor3, Color3> { return { x, y, z }; }
    template <class T> auto VectorN<4, T>::to_color() const -> std::conditional_t<traits_float, fColor, Color> { return { x, y, z, w }; }
}
