#pragma once
#include "Vector.h"
#include "Utils/Match.h"

namespace Quasi::Math {
    namespace Colors { enum ColorID : u32; }

    template <class T, bool HasAlpha>
    struct IColor {
        T r = 0, g = 0, b = 0;
        [[no_unique_address]]
        IfElse<HasAlpha, T, Empty> a = 0;

        using Elm = T;
        enum { Dim = 3 + HasAlpha };
        static constexpr bool NoAlpha = !HasAlpha;
    public:
        IColor() = default;
        IColor(T s, T a = 1) requires HasAlpha : r(s), g(s), b(s), a(a) {}
        IColor(T s)          requires NoAlpha  : r(s), g(s), b(s) {}
        IColor(T r, T g, T b, T a = 1) requires HasAlpha : r(r), g(g), b(b), a(a) {}
        IColor(T r, T g, T b, T   = 1) requires NoAlpha : r(r), g(g), b(b) {}
        IColor(const Vec4<T>& rgba)     requires HasAlpha : r(rgba.x), g(rgba.y), b(rgba.z), a(rgba.w) {}
        IColor(const Vec3<T>& rgb, T a) requires HasAlpha : r(rgb.x),  g(rgb.y),  b(rgb.z),  a(a) {}
        IColor(const Vec3<T>& rgb)                        : r(rgb.x),  g(rgb.y),  b(rgb.z) {}

        static usize CompFromName(char c) { return Vector<T, Dim>::CompFromName(c); }
        T*       Data()          { return &r; }
        const T* Data() const    { return &r; }
        T*       DataEnd()       { return &r + Dim; }
        const T* DataEnd() const { return &r + Dim; }
        Span<const T> AsSpan() const { return Spans::Slice(Data(), 3 + Dim); }
        Span<T>       AsSpan()       { return Spans::Slice(Data(), 3 + Dim); }

        T&       At(usize i)       { return Data()[i]; }
        const T& At(usize i) const { return Data()[i]; }
        T&       operator[](usize i)       { return Data()[i]; }
        const T& operator[](usize i) const { return Data()[i]; }

        template <class U> U ChannelAs(usize i) const {
            if constexpr (Floating<T>) {
                if constexpr (Floating<U>) return At(i); else return (U)f32s::FastToIntUnsigned(At(i) * 255.0f);
            } else {
                if constexpr (Floating<U>) return At(i) / 255.0f; else return (U)At(i);
            }
        }
        template <class U> U RedAs()   const { return ChannelAs<U>(0); }
        template <class U> U GreenAs() const { return ChannelAs<U>(1); }
        template <class U> U BlueAs()  const { return ChannelAs<U>(2); }
        template <class U> U AlphaAs() const requires HasAlpha { return ChannelAs<U>(3); }
        template <class U = T> U AlphaOrFull() const {
            if constexpr (HasAlpha) return AlphaAs<U>();
            else if constexpr (Floating<U>) return 1.0f;
            else return (U)255;
        }

        template <usize M>
        Vector<T, M> Swizzle(const char (&swizzle)[M + 1]) const {
            Vector<T, M> swizzled;
            for (usize i = 0; i < M; ++i) {
                const usize comp = CompFromName(swizzle[i]);
                swizzled[i] = comp == -1 ? 0 : At(comp);
            }
            return swizzled;
        }
        template <usize M> Vector<T, M> operator[](const char (&swizzle)[M + 1]) { return Swizzle(swizzle); }

        usize Dimensions() const { return Dim; }

        static IColor FromSpan(Span<const T> elms) {
            IColor c;
            Memory::RangeCopy(c.Data(), elms.Data(), std::min<usize>(elms.Length(), Dim));
            return c;
        }
        static IColor Greyscale(T scale) {
            IColor v;
            Memory::RangeSet(v.Data(), scale, Dim);
            return v;
        }
        static IColor Black() { return {}; }
        static IColor White() { return Greyscale(1); }

        IColor<T, true> AddAlpha(T alpha) const {
            return { r, g, b, alpha };
        }
        IColor<T, false> RemAlpha() const requires NoAlpha {
            return { r, g, b };
        }

        Vector<T, Dim> AsVector() const {
            if constexpr (HasAlpha) return { r, g, b, a };
            else return { r, g, b };
        }
        template <class C> C As() const {
            using U = decltype(C {}.r);
            return { RedAs<U>(), GreenAs<U>(), BlueAs<U>(), AlphaOrFull<U>() };
        }
        template <class C> explicit operator C() const { return As<C>(); }

        bv3 AsRGBVec()   const;
        bv4 AsRGBAVec()  const;
        fv3 AsRGBfVec()  const;
        fv4 AsRGBAfVec() const;
        Tuple<f32, f32, f32>      GetHSL()  const;
        Tuple<f32, f32, f32, f32> GetHSLA() const;
        static IColor FromHSL (f32 hue, f32 saturation = 1, f32 lightness = 1); /* hue: [0-1], sat: [0-1], L: [0-1] */
        static IColor FromHSLA(f32 hue, f32 saturation = 1, f32 lightness = 1, f32 alpha = 1); /* hue: [0-1], sat: [0-1], L: [0-1] */
        Tuple<f32, f32, f32>      GetHSV()  const;
        Tuple<f32, f32, f32, f32> GetHSVA() const;
        static IColor FromHSV (f32 hue, f32 saturation = 1, f32 value = 1); /* hue: [0-1], sat: [0-1], val: [0-1] */
        static IColor FromHSVA(f32 hue, f32 saturation = 1, f32 value = 1, f32 alpha = 1); /* hue: [0-1], sat: [0-1], val: [0-1] */

        Comparison Cmp(const IColor& other) const { return AsSpan().Cmp(other.AsSpan()); }

        IColor Add(const IColor& other) const;
        IColor Sub(const IColor& other) const;
        IColor Mul(T scale)   const;
        IColor Mul(f32 scale) const requires Integer<T>;
        IColor Div(T scale)   const;
        IColor Div(f32 scale) const requires Integer<T>;
        IColor AddScalar(T off) const;
        IColor SubScalar(T off) const;

        IColor& AddAssign(const IColor& other);
        IColor& SubAssign(const IColor& other);
        IColor& MulAssign(T scale);
        IColor& MulAssign(f32 scale) requires Integer<T>;
        IColor& DivAssign(T scale);
        IColor& DivAssign(f32 scale) requires Integer<T>;
        IColor& AddScalarAssign(T off);
        IColor& SubScalarAssign(T off);

        IColor operator+() const { return *this; }

        IColor operator+(const IColor& other) const { return Add(other); }
        IColor operator+(T off)              const { return AddScalar(off); }
        IColor operator-(const IColor& other) const { return Sub(other); }
        IColor operator-(T off)              const { return SubScalar(off); }
        IColor operator*(const IColor& other) const { return Product(other); }
        IColor operator*(T scale)            const { return Mul(scale); }
        IColor operator*(f32 scale) const requires Integer<T> { return Mul(scale); }
        IColor operator/(T scale)            const { return Div(scale); }
        IColor operator/(f32 scale) const requires Integer<T> { return Div(scale); }

        IColor& operator+=(const IColor& other) { return AddAssign(other); }
        IColor& operator+=(T off)               { return AddScalarAssign(off); }
        IColor& operator-=(const IColor& other) { return SubAssign(other); }
        IColor& operator-=(T off)               { return SubScalarAssign(off); }
        IColor& operator*=(const IColor& other) { return *this = Product(other); }
        IColor& operator*=(T scale)             { return MulAssign(scale); }
        IColor& operator*=(f32 scale) requires Integer<T> { return MulAssign(scale); }
        IColor& operator/=(T scale)             { return DivAssign(scale); }
        IColor& operator/=(f32 scale) requires Integer<T> { return DivAssign(scale); }

        IColor Map(T other, const FnArgs<T, T> auto& mapping) const {
            if constexpr (HasAlpha)
                return { mapping(r, other), mapping(g, other), mapping(b, other), mapping(a, other) };
            else return { mapping(r, other), mapping(g, other), mapping(b, other) };
        }
        IColor BinaryMap(const IColor& other, const FnArgs<T, T> auto& mapping) const {
            if constexpr (HasAlpha)
                return { mapping(r, other.r), mapping(g, other.g), mapping(b, other.b), mapping(a, other.a) };
            else return { mapping(r, other.r), mapping(g, other.g), mapping(b, other.b) };
        }
        IColor MapRGB(T other, const FnArgs<T, T> auto& mapping, T alpha) const {
            return { mapping(r, other), mapping(g, other), mapping(b, other), alpha };
        }
        IColor BinaryMapRGB(const IColor& other, const FnArgs<T, T> auto& mapping, T alpha) const {
            return { mapping(r, other.r), mapping(g, other.g), mapping(b, other.b), alpha };
        }

        IColor Inverted() const;
        IColor Lerp(const IColor& other, f32 t) const;
        IColor Lerp(const IColor& other, int p, int q) const;
        IColor Blend(const IColor& top) requires HasAlpha;
        IColor LogBlend(const IColor& other, f32 t) const;
        IColor LogBlend(const IColor& other, int p, int q) const;
        IColor AdjustShade(int brightOff) const requires Integer<T>;
        IColor AdjustShade(f32 brightOff) const requires Floating<T>;
        IColor Product(const IColor& other) const;
        IColor Screen (const IColor& other) const;
        IColor Overlay(const IColor& other) const requires HasAlpha;
        IColor<T, false> MulAlpha() const;
        f32 Luminance() const;

        static IColor FromBytes(u8 r, u8 g, u8 b, u8 a = 255);
        static IColor FromIntCode(u32 rgb, u8 a = 255);
        static IColor FromIntWithAlpha(u32 rgba);
        static IColor FromIntLittleCode(u32 bgr, u8 a = 255);
        static IColor FromIntLittleWithAlpha(u32 abgr);
        static IColor FromFloats(f32 r, f32 g, f32 b, f32 a = 1.0f);
        static Option<IColor> FromHex(Str hex);
        u32    GetIntCode() const; // returns in format 0xRRGGBBAA
        String GetHexCode() const;

        bool operator==(const IColor&) const = default;
        bool LooseEq(const IColor& other) const;

        static IColor FromColorID(Colors::ColorID colorid);

        static IColor Random(RandomGenerator& rand);
        static IColor RandomPure(RandomGenerator& rand);
    };

    using fColor  = IColor<f32, true>;
    using uColor  = IColor<u8, true>;
    using fColor3 = IColor<f32, false>;
    using uColor3 = IColor<u8, false>;

    fColor  operator ""_fColor(usize x);
    uColor  operator ""_uColor(usize x);
    fColor3 operator ""_fColor3(usize x);
    uColor3 operator ""_uColor3(usize x);
    fColor  operator ""_fColor(const char* colorId, usize len);
    uColor  operator ""_uColor(const char* colorId, usize len);
    fColor3 operator ""_fColor3(const char* colorId, usize len);
    uColor3 operator ""_uColor3(const char* colorId, usize len);

    namespace Colors {
        enum ColorID : u32 {
            BLACK,      //  solid black: rgb(000, 000, 000) or #000000
            DARK_GRAY,  //    25% white: rgb(064, 064, 064) or #404040
            GRAY,       //    50% white: rgb(128, 128, 128) or #808080
            LIGHT_GRAY, //    75% white: rgb(192, 192, 192) or #c0c0c0
            WHITE,      //  solid white: rgb(255, 255, 255) or #ffffff

            RED,        // red    (100%   red):              rgb(255, 000, 000) or #ff0000
            ORANGE,     // orange (100%   red +  50% green): rgb(255, 128, 000) or #ff8000
            YELLOW,     // yellow (100%   red + 100% green): rgb(255, 255, 000) or #ffff00
            LIME,       // lime   (100% green +  50%   red): rgb(128, 255, 000) or #80ff00
            GREEN,      // green  (100% green):              rgb(000, 255, 000) or #00ff00
            SEAFOAM,    // seafoam(100% green +  50%  blue): rgb(000, 255, 128) or #00ff80
            CYAN,       // cyan   (100% green + 100%  blue): rgb(000, 255, 255) or #00ffff
            AZURE,      // azure  (100%  blue +  50% green): rgb(000, 128, 255) or #0080ff
            BLUE,       // blue   (100%  blue):              rgb(000, 000, 255) or #0000ff
            PURPLE,     // purple (100%  blue +  50%   red): rgb(128, 000, 255) or #8000ff
            MAGENTA,    // magenta(100%  blue + 100%   red): rgb(255, 000, 255) or #ff00ff
            ROSE,       // rose   (100%   red +  50%  blue): rgb(255, 000, 128) or #ff0080

            LIGHT_RED,     // light_red    (100%   red +  50% green +  50%  blue): rgb(255, 128, 128) or #ff8080
            LIGHT_YELLOW,  // light_yellow (100%   red + 100% green +  50%  blue): rgb(255, 255, 128) or #ffff80
            LIGHT_GREEN,   // light_green  ( 50%   red + 100% green +  50%  blue): rgb(128, 255, 128) or #80ff80
            LIGHT_CYAN,    // light_cyan   ( 50%   red + 100% green + 100%  blue): rgb(128, 255, 255) or #80ffff
            LIGHT_BLUE,    // light_blue   ( 50%   red +  50% green + 100%  blue): rgb(128, 128, 255) or #8080ff
            LIGHT_MAGENTA, // light_magenta(100%   red +  50% green + 100%  blue): rgb(255, 128, 255) or #ff80ff

            DARK_RED,      // dark_red   ( 50%   red):              rgb(128, 000, 000) or #800000
            DARK_YELLOW,   // dark_yellow( 50%   red +  50% green): rgb(128, 128, 000) or #808000
            DARK_GREEN,    // dark_green ( 50% green):              rgb(000, 128, 000) or #008000
            DARK_CYAN,     // dark_cyan  ( 50% green +  50%  blue): rgb(000, 128, 128) or #008080
            DARK_BLUE,     // dark_blue  ( 50%  blue):              rgb(000, 000, 128) or #000080
            DARK_MAGENTA,  // dark_purple( 50%  blue +  50%   red): rgb(128, 000, 128) or #800080


            SILVER     = LIGHT_GRAY,    // same as light_gray,    rgb(192, 192, 192) or #c0c0c0
            SALMON     = LIGHT_RED,     // same as light_red,     rgb(255, 128, 128) or #ff8080
            LEMON      = LIGHT_YELLOW,  // same as light_yellow,  rgb(255, 255, 128) or #ffff80
            MINT       = LIGHT_GREEN,   // same as light_green,   rgb(128, 255, 128) or #80ff80
            SKY        = LIGHT_CYAN,    // same as light_cyan,    rgb(128, 255, 255) or #80ffff
            CORNFLOWER = LIGHT_BLUE,    // same as light_blue,    rgb(128, 128, 255) or #8080ff
            PINK       = LIGHT_MAGENTA, // same as light_magenta, rgb(255, 128, 255) or #ff80ff
            MAROON     = DARK_RED,      // same as dark_red,      rgb(128, 000, 000) or #800000
            OLIVE      = DARK_YELLOW,   // same as dark_yellow,   rgb(128, 128, 000) or #808000
            AVOCADO    = DARK_GREEN,    // same as dark_green,    rgb(000, 128, 000) or #008000
            TEAL       = DARK_CYAN,     // same as dark_cyan,     rgb(000, 128, 128) or #008080
            NAVY       = DARK_BLUE,     // same as dark_blue,     rgb(000, 000, 128) or #000080
            VIOLET     = DARK_MAGENTA,  // same as dark_purple,   rgb(128, 000, 128) or #800080

            BETTER_RED = 50,   // more appealing red:     rgb(255, 048, 048) or #ff3030
            BETTER_ORANGE,     // more appealing orange:  rgb(255, 128, 043) or #ff802b
            BETTER_YELLOW,     // more appealing yellow:  rgb(255, 192, 020) or #ffc014
            BETTER_LIME,       // more appealing lime:    rgb(135, 247, 037) or #87f725
            BETTER_GREEN,      // more appealing green:   rgb(012, 133, 001) or #0c8501
            BETTER_AQUA,       // more appealing aqua:    rgb(040, 206, 247) or #28cef7
            BETTER_CYAN,       // more appealing cyan:    rgb(023, 160, 179) or #17a0b3
            BETTER_BLUE,       // more appealing blue:    rgb(016, 065, 227) or #1041e3
            BETTER_PURPLE,     // more appealing purple:  rgb(120, 024, 237) or #7818ed
            BETTER_MAGENTA,    // more appealing magenta: rgb(203, 042, 235) or #cb2aeb
            BETTER_PINK,       // more appealing pink:    rgb(250, 087, 198) or #fa57c6
            BETTER_BROWN,      // more appealing brown:   rgb(069, 032, 013) or #45200d
            BETTER_BLACK,      // more appealing black:   rgb(022, 025, 029) or #16191d
            BETTER_DARK_GRAY,  // more appealing gray:    rgb(076, 075, 093) or #4c4b5d
            BETTER_GRAY,       // more appealing gray:    rgb(116, 126, 134) or #747e86
            BETTER_LIGHT_GRAY, // more appealing gray:    rgb(175, 186, 193) or #afbac1
            BETTER_WHITE,      // more appealing white:   rgb(232, 247, 249) or #e8f7f9

            CLEAR, // complete transparency. RGBA is (0, 0, 0, 0)
        };
        ColorID GetColorIDFor(Str colorId);
        u32 GetHexFromID(ColorID id);
    }
}

namespace Quasi::Text {
    template <class T, bool A>
    struct Formatter<Math::IColor<T, A>> : Math::VectorFormatter {
        static usize FormatTo(StringWriter sw, const Math::IColor<T, A>& vec, const FormatOptions& options) {
            return VectorFormatter::FormatToInternal(sw, vec.AsSpan(), Str::Slice("rgba", 3 + A), options);
        }
    };
}