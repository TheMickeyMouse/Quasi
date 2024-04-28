#pragma once
#include <span>
#include "Vector.h"


namespace Maths {
    struct color3;
    struct color3f;
    struct color;
    struct colorf;

    template <class T>
    concept color_t =
        std::is_same_v<T, color3> || std::is_same_v<T, color3f> ||
        std::is_same_v<T, color>  || std::is_same_v<T, colorf>;

    template <bool HasAlpha, bool IsFloaty>
    using color_from = std::conditional_t<HasAlpha,
        std::conditional_t<IsFloaty, colorf,  color>,
        std::conditional_t<IsFloaty, color3f, color3>
    >;

    template <class> struct color_traits;
    template <> struct color_traits<color3>  { static constexpr bool has_alpha = false, is_floating = false; using scalar = byte;  };
    template <> struct color_traits<color3f> { static constexpr bool has_alpha = false, is_floating = true;  using scalar = float; };
    template <> struct color_traits<color>   { static constexpr bool has_alpha = true,  is_floating = false; using scalar = byte;  };
    template <> struct color_traits<colorf>  { static constexpr bool has_alpha = true,  is_floating = true;  using scalar = float; };

#define NODISC [[nodiscard]]
    template <class Color>
    struct color_base {
        static constexpr bool traits_has_alpha = color_traits<Color>::has_alpha,
                              traits_is_floating = color_traits<Color>::is_floating;
        using scalar = typename color_traits<Color>::scalar;
    private:
        static constexpr scalar get_channel_max() {
            if constexpr (traits_is_floating) return 1.0f;
            else return (byte)255;
        }
    public:
        static constexpr scalar CHANNEL_MAX_VALUE = get_channel_max();
        static constexpr uint dimension = 3 + traits_has_alpha;

        using without_alpha_t = color_from<false, traits_is_floating>;
        using with_alpha_t    = color_from<true,  traits_is_floating>;
        using alternate_data_form = color_from<traits_has_alpha, !traits_is_floating>;

        static constexpr const swizz::vector_swizzle_data<dimension>& comp_params() {
            if constexpr (traits_has_alpha) return "rgba";
            else return "rgb";
        }
        template <auto S>
        vecn<S.N, byte> swizzle() const {
            return swizz::swizzle_impl<byte, dimension, comp_params(), swizz::swizzle<dimension, comp_params()> { S }>(this->begin());
        }

    private:
        NODISC const Color& color() const { return *(const Color*)this; }
        Color& color() { return *(Color*)this; }
    public:
        scalar* begin() { return (scalar*)this; }
        scalar* end()   { return begin() + dimension; }
        NODISC const scalar* begin()  const { return (const scalar*)this; }
        NODISC const scalar* cbegin() const { return (const scalar*)this; }
        NODISC const scalar* end()    const { return begin() + dimension; }
        NODISC const scalar* cend()   const { return cbegin() + dimension; }

        NODISC scalar alpha(scalar def = CHANNEL_MAX_VALUE) const;

        static Color from_hex(std::string_view hex);
        NODISC std::string hexcode() const;

        NODISC bool eq(const Color& other) const;
        NODISC bool loose_eq(const Color& other) const;
        NODISC bool neq(const Color& other) const;
        NODISC bool operator==(const color_base& other) const;

        NODISC Color neg() const;
        NODISC Color lerp(const Color& other, float t) const;
        NODISC Color mul(const Color& other) const;
        NODISC Color screen(const Color& other) const;
        NODISC Color overlay(const Color& other) const;
        NODISC without_alpha_t mul_alpha() const;

        NODISC float luminance() const;

        NODISC bvec3 as_rgb()   const;
        NODISC bvec4 as_rgba()  const;
        NODISC fvec3 as_rgbf()  const;
        NODISC fvec4 as_rgbaf() const;
        NODISC fvec3 as_hsl()   const;
        NODISC fvec4 as_hsla()  const;
        static Color from_hsl(float hue, float saturation = 1, float lightness = 1) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], L: [0-1] */
        static Color from_hsl(float hue, float saturation = 1, float lightness = 1, float alpha = 1) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], L: [0-1] */
        static Color from_hsl(const fvec3& hsl) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], L: [0-1] */
        static Color from_hsl(const fvec4& hsla) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], L: [0-1] */
        NODISC fvec3 as_hsv()  const;
        NODISC fvec4 as_hsva() const;
        static Color from_hsv(float hue, float saturation = 1, float value = 1) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], val: [0-1] */
        static Color from_hsv(float hue, float saturation = 1, float value = 1, float alpha = 1) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], val: [0-1] */
        static Color from_hsv(const fvec3& hsv) requires (!traits_has_alpha); /* hue: [0-360], sat: [0-1], val: [0-1] */
        static Color from_hsv(const fvec4& hsva) requires traits_has_alpha; /* hue: [0-360], sat: [0-1], val: [0-1] */

        NODISC without_alpha_t rgb() const;
        NODISC with_alpha_t with_alpha(scalar alpha = CHANNEL_MAX_VALUE) const;
        NODISC with_alpha_t rgb1() const;

        NODISC operator without_alpha_t() const requires traits_has_alpha;
        NODISC operator with_alpha_t() const requires (!traits_has_alpha);
        NODISC operator alternate_data_form() const;

        static Color random(random_gen& rg, float sat, float val);

#pragma region Color Constants
        static Color color_id(int id);

        static Color BLACK()      { return from_hex("000000"); } /* solid black: rgb(000, 000, 000) or #000000 */
        static Color DARK_GRAY()  { return from_hex("404040"); } /*   25% white: rgb(064, 064, 064) or #404040 */
        static Color GRAY()       { return from_hex("808080"); } /*   50% white: rgb(128, 128, 128) or #808080 */
        static Color LIGHT_GRAY() { return from_hex("c0c0c0"); } /*   75% white: rgb(192, 192, 192) or #c0c0c0 */
        static Color WHITE()      { return from_hex("ffffff"); } /* solid white: rgb(255, 255, 255) or #ffffff */
        static Color SILVER() { return LIGHT_GRAY(); }  /* same as light_gray, rgb(192, 192, 192) or #c0c0c0 */

        static Color RED()     { return from_hex("ff0000"); } /* red    (100%   red):              rgb(255, 000, 000) or #ff0000 */
        static Color ORANGE()  { return from_hex("ff8000"); } /* orange (100%   red +  50% green): rgb(255, 128, 000) or #ff8000 */
        static Color YELLOW()  { return from_hex("ffff00"); } /* yellow (100%   red + 100% green): rgb(255, 255, 000) or #ffff00 */
        static Color LIME()    { return from_hex("80ff00"); } /* lime   (100% green +  50%   red): rgb(128, 255, 000) or #80ff00 */
        static Color GREEN()   { return from_hex("00ff00"); } /* green  (100% green):              rgb(000, 255, 000) or #00ff00 */
        static Color SEAFOAM() { return from_hex("00ff80"); } /* seafoam(100% green +  50%  blue): rgb(000, 255, 128) or #00ff80 */
        static Color CYAN()    { return from_hex("00ffff"); } /* cyan   (100% green + 100%  blue): rgb(000, 255, 255) or #00ffff */
        static Color AZURE()   { return from_hex("0080ff"); } /* azure  (100%  blue +  50% green): rgb(000, 128, 255) or #0080ff */
        static Color BLUE()    { return from_hex("0000ff"); } /* blue   (100%  blue):              rgb(000, 000, 255) or #0000ff */
        static Color PURPLE()  { return from_hex("8000ff"); } /* purple (100%  blue +  50%   red): rgb(128, 000, 255) or #8000ff */
        static Color MAGENTA() { return from_hex("ff00ff"); } /* magenta(100%  blue + 100%   red): rgb(255, 000, 255) or #ff00ff */
        static Color ROSE()    { return from_hex("ff0080"); } /* rose   (100%   red +  50%  blue): rgb(255, 000, 128) or #ff0080 */

        static Color LIGHT_RED()     { return from_hex("ff8080"); } /* light_red    (100%   red +  50% green +  50%  blue): rgb(255, 128, 128) or #ff8080 */
        static Color LIGHT_YELLOW()  { return from_hex("ffff80"); } /* light_yellow (100%   red + 100% green +  50%  blue): rgb(255, 255, 128) or #ffff80 */
        static Color LIGHT_GREEN()   { return from_hex("80ff80"); } /* light_green  ( 50%   red + 100% green +  50%  blue): rgb(128, 255, 128) or #80ff80 */
        static Color LIGHT_CYAN()    { return from_hex("80ffff"); } /* light_cyan   ( 50%   red + 100% green + 100%  blue): rgb(128, 255, 255) or #80ffff */
        static Color LIGHT_BLUE()    { return from_hex("8080ff"); } /* light_blue   ( 50%   red +  50% green + 100%  blue): rgb(128, 128, 255) or #8080ff */
        static Color LIGHT_MAGENTA() { return from_hex("ff80ff"); } /* light_magenta(100%   red +  50% green + 100%  blue): rgb(255, 128, 255) or #ff80ff */
        static Color SALMON()     { return LIGHT_RED();     } /* same as light_red,     rgb(255, 128, 128) or #ff8080 */
        static Color LEMON()      { return LIGHT_YELLOW();  } /* same as light_yellow,  rgb(255, 255, 128) or #ffff80 */
        static Color MINT()       { return LIGHT_GREEN();   } /* same as light_green,   rgb(128, 255, 128) or #80ff80 */
        static Color SKY()        { return LIGHT_CYAN();    } /* same as light_cyan,    rgb(128, 255, 255) or #80ffff */
        static Color CORNFLOWER() { return LIGHT_BLUE();    } /* same as light_blue,    rgb(128, 128, 255) or #8080ff */
        static Color PINK()       { return LIGHT_MAGENTA(); } /* same as light_magenta, rgb(255, 128, 255) or #ff80ff */

        static Color DARK_RED()     { return from_hex("800000"); } /* dark_red   ( 50%   red):              rgb(128, 000, 000) or #800000 */
        static Color DARK_YELLOW()  { return from_hex("808000"); } /* dark_yellow( 50%   red +  50% green): rgb(128, 128, 000) or #808000 */
        static Color DARK_GREEN()   { return from_hex("008000"); } /* dark_green ( 50% green):              rgb(000, 128, 000) or #008000 */
        static Color DARK_CYAN()    { return from_hex("008080"); } /* dark_cyan  ( 50% green +  50%  blue): rgb(000, 128, 128) or #008080 */
        static Color DARK_BLUE()    { return from_hex("000080"); } /* dark_blue  ( 50%  blue):              rgb(000, 000, 128) or #000080 */
        static Color DARK_MAGENTA() { return from_hex("800080"); } /* dark_purple( 50%  blue +  50%   red): rgb(128, 000, 128) or #800080 */
        static Color MAROON()  { return DARK_RED();     } /* same as dark_red,    rgb(128, 000, 000) or #800000 */
        static Color OLIVE()   { return DARK_YELLOW();  } /* same as dark_yellow, rgb(128, 128, 000) or #808000 */
        static Color AVOCADO() { return DARK_GREEN();   } /* same as dark_green,  rgb(000, 128, 000) or #008000 */
        static Color TEAL()    { return DARK_CYAN();    } /* same as dark_cyan,   rgb(000, 128, 128) or #008080 */
        static Color NAVY()    { return DARK_BLUE();    } /* same as dark_blue,   rgb(000, 000, 128) or #000080 */
        static Color VIOLET()  { return DARK_MAGENTA(); } /* same as dark_purple, rgb(128, 000, 128) or #800080 */

        static Color BETTER_RED()        { return from_hex("ff3030"); } /* more appealing red:     rgb(255, 048, 048) or #ff3030 */
        static Color BETTER_ORANGE()     { return from_hex("ff802b"); } /* more appealing orange:  rgb(255, 128, 043) or #ff802b */
        static Color BETTER_YELLOW()     { return from_hex("ffc014"); } /* more appealing yellow:  rgb(255, 192, 020) or #ffc014 */
        static Color BETTER_LIME()       { return from_hex("87f725"); } /* more appealing lime:    rgb(135, 247, 037) or #87f725 */
        static Color BETTER_GREEN()      { return from_hex("0c8501"); } /* more appealing green:   rgb(012, 133, 001) or #0c8501 */
        static Color BETTER_AQUA()       { return from_hex("28cef7"); } /* more appealing aqua:    rgb(040, 206, 247) or #28cef7 */
        static Color BETTER_CYAN()       { return from_hex("17a0b3"); } /* more appealing cyan:    rgb(023, 160, 179) or #17a0b3 */
        static Color BETTER_BLUE()       { return from_hex("1041e3"); } /* more appealing blue:    rgb(016, 065, 227) or #1041e3 */
        static Color BETTER_PURPLE()     { return from_hex("7818ed"); } /* more appealing purple:  rgb(120, 024, 237) or #7818ed */
        static Color BETTER_MAGENTA()    { return from_hex("cb2aeb"); } /* more appealing magenta: rgb(203, 042, 235) or #cb2aeb */
        static Color BETTER_PINK()       { return from_hex("fa57c6"); } /* more appealing pink:    rgb(250, 087, 198) or #fa57c6 */
        static Color BETTER_BROWN()      { return from_hex("45200d"); } /* more appealing brown:   rgb(069, 032, 013) or #45200d */
        static Color BETTER_BLACK()      { return from_hex("16191d"); } /* more appealing black:   rgb(022, 025, 029) or #16191d */
        static Color BETTER_DARK_GRAY()  { return from_hex("4c4b5d"); } /* more appealing gray:    rgb(076, 075, 093) or #4c4b5d */
        static Color BETTER_GRAY()       { return from_hex("747e86"); } /* more appealing gray:    rgb(116, 126, 134) or #747e86 */
        static Color BETTER_LIGHT_GRAY() { return from_hex("afbac1"); } /* more appealing gray:    rgb(175, 186, 193) or #afbac1 */
        static Color BETTER_WHITE()      { return from_hex("e8f7f9"); } /* more appealing white:   rgb(232, 247, 249) or #e8f7f9 */

        static Color CLEAR() requires traits_has_alpha { return from_hex("00000000"); } /* complete transparency. RGBA is (0, 0, 0, 0). */
#pragma endregion // Color Constants
    };

    struct color3 : color_base<color3> {
        byte r, g, b;

        color3(byte s = 0) : r(s), g(s), b(s) {}
        color3(byte r, byte g, byte b) : r(r), g(g), b(b) {}
        color3(std::integral auto r, std::integral auto g,
               std::integral auto b) : color3((byte)r, (byte)g, (byte)b) {}
    };

    struct color3f : color_base<color3f> {
        float r, g, b;

        color3f(float s = 0) : r(s), g(s), b(s) {}
        color3f(float r, float g, float b) : r(r), g(g), b(b) {}
        color3f(std::floating_point auto r, std::floating_point auto g,
                std::floating_point auto b) : color3f((float)r, (float)g, (float)b) {}
    };

    struct color : color_base<color> {
        byte r, g, b, a;

        color(byte s = 0, byte a = 255) : r(s), g(s), b(s), a(a) {}
        color(byte r, byte g, byte b, byte a = 255) : r(r), g(g), b(b), a(a) {}
        color(std::integral auto r, std::integral auto g,
              std::integral auto b, std::integral auto a = 255) : color((byte)r, (byte)g, (byte)b, (byte)a) {}
    };

    struct colorf : color_base<colorf> {
        float r, g, b, a;

        colorf(float s = 0, float a = 1) : r(s), g(s), b(s), a(a) {}
        colorf(float r, float g, float b, float a = 1) : r(r), g(g), b(b), a(a) {}
        colorf(std::floating_point auto r, std::floating_point auto g,
               std::floating_point auto b, std::floating_point auto a = 1) : colorf((float)r, (float)g, (float)b, (float)a) {}
    };

    namespace color_utils {
        template <class T> T convert_type(auto channel);
        template <class T> T channel_from_hexcode(char D0, char D1);

        template <class T, class U> T make_color(U r, U g, U b, U a) {
            using S = typename color_traits<T>::scalar;
            using I = std::conditional_t<std::is_integral_v<U>, byte, float>;
            if constexpr (color_traits<T>::has_alpha)
                return T {
                    color_utils::convert_type<S>((I)r),
                    color_utils::convert_type<S>((I)g),
                    color_utils::convert_type<S>((I)b),
                    color_utils::convert_type<S>((I)a)
                };
            else
                return T {
                    color_utils::convert_type<S>((I)r),
                    color_utils::convert_type<S>((I)g),
                    color_utils::convert_type<S>((I)b)
                };
        }
    }

#undef NODISC

    template <class T> auto vecn<3, T>::to_color(T alpha) const -> std::conditional_t<traits_float, colorf,  color> { return { x, y, z, alpha }; }
    template <class T> auto vecn<3, T>::to_color3() const -> std::conditional_t<traits_float, color3f, color3> { return { x, y, z }; }
    template <class T> auto vecn<4, T>::to_color() const -> std::conditional_t<traits_float, colorf, color> { return { x, y, z, w }; }
}
