#pragma once
#include "NumTypes.h"
#include <concepts>

#include "Corner.h"
#include "Direction.h"

namespace Maths
{
    template <class TVec>
    concept Vec = requires (TVec vec) {
        sizeof(TVec) % sizeof(decltype(vec.x)) == 0;
        sizeof(TVec) / sizeof(decltype(vec.x)) > 1;
    };
    
    template <class TBase, class TVec>
    concept VecLessThan = requires () {
        Vec<TVec>;
        sizeof(TVec) < sizeof(TBase);
    };

    template <class TBase, class TVec>
    concept VecGreaterThan = requires () {
        Vec<TVec>;
        sizeof(TVec) > sizeof(TBase);
    };
    
    template <class T>
    concept Numeral = requires (T self, T other) {
        { self + other } -> std::convertible_to<T>;
        { self - other } -> std::convertible_to<T>;
        { self * other } -> std::convertible_to<T>;
        { self / other } -> std::convertible_to<T>;
        (T) 0;
        (T) 1;
        (T)-1;
    };

    template <Numeral T> struct Vec2;
    template <Numeral T> struct Vec3;
    template <Numeral T> struct Vec4;
    using Vector2 = Vec2<float>;
    using Vector3 = Vec3<float>;
    using Vector4 = Vec4<float>;
    using Vec2f64 = Vec2<double>;
    using Vec3f64 = Vec3<double>;
    using Vec4f64 = Vec4<double>;
    using Vec2d = Vec2<double>;
    using Vec3d = Vec3<double>;
    using Vec4d = Vec4<double>;
    using Vec2Int = Vec2<int>;
    using Vec3Int = Vec3<int>;
    using Vec4Int = Vec4<int>;
    using Vec2UInt = Vec2<uint>;
    using Vec3UInt = Vec3<uint>;
    using Vec4UInt = Vec4<uint>;

    // TODO: Add non-numeral Vecs
    // TODO: Add math operations like: lerp, length

#pragma region Vec2 Declaration
    template <Numeral T> struct Vec2
    {
        static const Vec2 RIGHT;
        static const Vec2 LEFT ;
        static const Vec2 UP   ;
        static const Vec2 DOWN ;
        
        static const Vec2 ZERO ;
        static const Vec2 ONE  ;
        
        T x, y;
        Vec2(T x = 0, T y = 0) : x(x), y(y) {}
        Vec2(Direction2D dir, T scale = 1);
        Vec2(Corner2D    cor, T scale = 1);
        Vec2(const Vec3<T>& xy);
        Vec2(const Vec4<T>& xy);

        T operator[] (unsigned int i) const { return ((const T*)this)[i]; }
        operator T*() { return (T*)this; }

#define DEF_VEC2_OP(OP) Vec2 operator OP (const Vec2& other) const { return { x OP other.x, y OP other.y }; } \
                        Vec2 operator OP (T other) const { return { x OP other, y OP other }; } \
                        Vec2& operator OP##= (const Vec2& other) { x OP##= other.x; y OP##= other.y; return *this; } \
                        Vec2& operator OP##= (T other) { x OP##= other; y OP##= other; return *this; }

        DEF_VEC2_OP(+)
        DEF_VEC2_OP(-)
        DEF_VEC2_OP(*)
        DEF_VEC2_OP(/)
        DEF_VEC2_OP(%)

        Vec2 operator+() const { return *this; }
        Vec2 operator-() const { return { -x, -y }; }
        
        bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Vec2& other) const { return !(*this == other); }

#undef DEF_VEC2_OP
#define DEF_VEC2_CAST(TYPE)
        template <Numeral TOther> operator Vec2<TOther>() { return { (TOther)x, (TOther)y }; }
        template <VecLessThan<Vec2>    TVec> operator TVec() { return TVec { *this }; }
        template <VecGreaterThan<Vec2> TVec> operator TVec() { return TVec { x, y }; }
    };
#pragma endregion 

#pragma region Vec3 Declaration
    template <Numeral T> struct Vec3
    {
        static const Vec3 RIGHT;
        static const Vec3 LEFT ;
        static const Vec3 UP   ;
        static const Vec3 DOWN ;
        static const Vec3 FRONT;
        static const Vec3 BACK ;
        
        static const Vec3 ZERO ;
        static const Vec3 ONE  ;
        
        T x, y, z;
        Vec3(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}
        Vec3(Direction3D dir, T scale = 1);
        Vec3(Corner3D    cor, T scale = 1);
        Vec3(const Vec2<T>& xy, float z);
        Vec3(const Vec4<T>& xyz);

        T operator[] (unsigned int i) const { return ((const T*)this)[i]; }
        operator T*() { return (T*)this; }

#define DEF_VEC3_OP(OP) Vec3 operator OP (const Vec3& other) const { return { x OP other.x, y OP other.y, z OP other.z }; } \
                        Vec3 operator OP (T other) const { return { x OP other, y OP other, z OP other }; } \
                        Vec3& operator OP##= (const Vec3& other) { x OP##= other.x; y OP##= other.y; z OP##= other.z; return *this; } \
                        Vec3& operator OP##= (T other) { x OP##= other; y OP##= other; z OP##= other; return *this; }
        
        DEF_VEC3_OP(+)
        DEF_VEC3_OP(-)
        DEF_VEC3_OP(*)
        DEF_VEC3_OP(/)
        DEF_VEC3_OP(%)

        Vec3 operator+() const { return *this; }
        Vec3 operator-() const { return { -x, -y, -z }; }

        bool operator==(const Vec3& other) const { return x == other.x && y == other.y && z == other.z; }
        bool operator!=(const Vec3& other) const { return !(*this == other); }

#undef DEF_VEC3_OP
        template <Numeral TOther> operator Vec3<TOther>() { return { (TOther)x, (TOther)y, (TOther)z }; }
        template <VecLessThan<Vec3>    TVec> operator TVec() { return TVec { *this }; }
        template <VecGreaterThan<Vec3> TVec> operator TVec() { return TVec { x, y, z }; }
    };
#pragma endregion

#pragma region Vec4 Declaration
    template <Numeral T> struct Vec4
    {
        static const Vec4 RIGHT;
        static const Vec4 LEFT ;
        static const Vec4 UP   ;
        static const Vec4 DOWN ;
        static const Vec4 FRONT;
        static const Vec4 BACK ;
        static const Vec4 IN   ;
        static const Vec4 OUT  ;
        
        static const Vec4 ZERO ;
        static const Vec4 ONE  ;
        
        T x, y, z, w;
        Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : x(x), y(y), z(z), w(w) {}
        Vec4(Direction4D dir, T scale = 1);
        Vec4(Corner4D    cor, T scale = 1);
        Vec4(const Vec2<T>& xy, float z, float w);
        Vec4(const Vec2<T>& xy, const Vec2<T>& zw);
        Vec4(const Vec3<T>& xyz, float w);

        T operator[] (unsigned int i) const { return ((const T*)this)[i]; }
        operator T*() { return (T*)this; }

#define DEF_VEC4_OP(OP) Vec4 operator OP (const Vec4& other) const { return { x OP other.x, y OP other.y, z OP other.z, w OP other.w }; } \
                        Vec4 operator OP (T other) const { return { x OP other, y OP other, z OP other, w OP other }; } \
                        Vec4& operator OP##= (const Vec4& other) { x OP##= other.x; y OP##= other.y; z OP##= other.z; w OP##= other.w; return *this; } \
                        Vec4& operator OP##= (T other) { x OP##= other; y OP##= other; z OP##= other; w OP##= other; return *this; }
        
        DEF_VEC4_OP(+)
        DEF_VEC4_OP(-)
        DEF_VEC4_OP(*)
        DEF_VEC4_OP(/)
        DEF_VEC4_OP(%)

        Vec4 operator+() const { return *this; }
        Vec4 operator-() const { return { -x, -y, -z, -w }; }
        
        bool operator==(const Vec4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
        bool operator!=(const Vec4& other) const { return !(*this == other); }

#undef DEF_VEC3_OP
        template <Numeral TOther> operator Vec4<TOther>() { return { (TOther)x, (TOther)y, (TOther)z, (TOther)w }; }
        template <VecLessThan<Vec4>    TVec> operator TVec() { return TVec { *this }; }
        template <VecGreaterThan<Vec4> TVec> operator TVec() { return TVec { x, y, z, w }; }
    };
#pragma endregion

#pragma region Vec2 Definition
    template <Numeral T> const Vec2<T> Vec2<T>::RIGHT = {  1,  0 };
    template <Numeral T> const Vec2<T> Vec2<T>::LEFT  = { -1,  0 };
    template <Numeral T> const Vec2<T> Vec2<T>::UP    = {  0,  1 };
    template <Numeral T> const Vec2<T> Vec2<T>::DOWN  = {  0, -1 };
    
    template <Numeral T> const Vec2<T> Vec2<T>::ZERO  = {  0,  0 };
    template <Numeral T> const Vec2<T> Vec2<T>::ONE   = {  1,  1 };

    template <Numeral T>
    Vec2<T>::Vec2(Direction2D dir, T scale) {
        using enum Direction2D;
        switch (dir) {
            case RIGHT: x =  scale; y =  0;     return;
            case LEFT : x = -scale; y =  0;     return;
            case UP   : x =  0;     y =  scale; return;
            case DOWN : x =  0;     y = -scale; return;
            
            case ZERO : x =  0;     y =  0;     return;
            case UNIT : x =  scale; y =  scale; return;
            default: x = 0; y = 0;
        }
    }

    template <Numeral T>
    Vec2<T>::Vec2(Corner2D cor, T scale) {
        using enum Corner2D;
        // using namespace stdu;
        // if (cor < 0 || cor >= CENTER) {
        //     x = 0; y = 0;
        //     return;
        // }
        //
        // x = cor & SIDE_LEFT   ? -scale : scale;
        // y = cor & SIDE_BOTTOM ? -scale : scale;

        // default method
        switch (cor) {
            case TOP_RIGHT   : x =  scale; y =  scale; return;
            case TOP_LEFT    : x = -scale; y =  scale; return;
            case BOTTOM_RIGHT: x =  scale; y = -scale; return;
            case BOTTOM_LEFT : x = -scale; y = -scale; return;
            
            case CENTER: x = 0; y = 0;  return;
            default: x = 0; y = 0;
        }
    }

    template <Numeral T> Vec2<T>::Vec2(const Vec3<T>& xy) : x(xy.x), y(xy.y) {}
    template <Numeral T> Vec2<T>::Vec2(const Vec4<T>& xy) : x(xy.x), y(xy.y) {}
#pragma endregion

#pragma region Vec3 Definition
    template <Numeral T> const Vec3<T> Vec3<T>::RIGHT = {  1,  0,  0 };
    template <Numeral T> const Vec3<T> Vec3<T>::LEFT  = { -1,  0,  0 };
    template <Numeral T> const Vec3<T> Vec3<T>::UP    = {  0,  1,  0 };
    template <Numeral T> const Vec3<T> Vec3<T>::DOWN  = {  0, -1,  0 };
    template <Numeral T> const Vec3<T> Vec3<T>::FRONT = {  0,  0,  1 };
    template <Numeral T> const Vec3<T> Vec3<T>::BACK  = {  0,  0, -1 };
    
    template <Numeral T> const Vec3<T> Vec3<T>::ZERO  = {  0,  0,  0 };
    template <Numeral T> const Vec3<T> Vec3<T>::ONE   = {  1,  1,  1 };

    template <Numeral T>
    Vec3<T>::Vec3(Direction3D dir, T scale) {
        using enum Direction3D;
        switch (dir) {
            case RIGHT: x =  scale; y =  0;     z =  0;     return;
            case LEFT : x = -scale; y =  0;     z =  0;     return;
            case UP   : x =  0;     y =  scale; z =  0;     return;
            case DOWN : x =  0;     y = -scale; z =  0;     return;
            case FRONT: x =  0;     y =  0;     z =  scale; return;
            case BACK : x =  0;     y =  0;     z = -scale; return;
            case ZERO : x =  0;     y =  0;     z =  0;     return;
            case UNIT : x =  scale; y =  scale; z =  scale; return;
            default: x = 0; y = 0; z = 0;
        }
    }

    template <Numeral T>
    Vec3<T>::Vec3(Corner3D cor, T scale) {
        using enum Corner3D;
        switch (cor) {
            case FRONT_TOP_RIGHT   : x =  scale; y =  scale; z =  scale; return;
            case FRONT_TOP_LEFT    : x = -scale; y =  scale; z =  scale; return;
            case FRONT_BOTTOM_RIGHT: x =  scale; y = -scale; z =  scale; return;
            case FRONT_BOTTOM_LEFT : x = -scale; y = -scale; z =  scale; return;
            case BACK_TOP_RIGHT    : x =  scale; y =  scale; z = -scale; return;
            case BACK_TOP_LEFT     : x = -scale; y =  scale; z = -scale; return;
            case BACK_BOTTOM_RIGHT : x =  scale; y = -scale; z = -scale; return;
            case BACK_BOTTOM_LEFT  : x = -scale; y = -scale; z = -scale; return;
            case CENTER: x = 0; y = 0; z = 0; return;
            default: x = 0; y = 0; z = 0;
        }
    }
    
    template <Numeral T> Vec3<T>::Vec3(const Vec4<T>& xyz)         : x(xyz.x), y(xyz.y), z(xyz.z) {}
    template <Numeral T> Vec3<T>::Vec3(const Vec2<T>& xy, float z) : x(xy.x),  y(xy.y),  z(z) {}
#pragma endregion

#pragma region Vec4 Definition
    template <Numeral T> const Vec4<T> Vec4<T>::RIGHT = {  1,  0,  0,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::LEFT  = { -1,  0,  0,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::UP    = {  0,  1,  0,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::DOWN  = {  0, -1,  0,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::FRONT = {  0,  0,  1,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::BACK  = {  0,  0, -1,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::IN    = {  0,  0,  0,  1 };
    template <Numeral T> const Vec4<T> Vec4<T>::OUT   = {  0,  0,  0, -1 };
    
    template <Numeral T> const Vec4<T> Vec4<T>::ZERO  = {  0,  0,  0,  0 };
    template <Numeral T> const Vec4<T> Vec4<T>::ONE   = {  1,  1,  1,  1 };

    template <Numeral T>
    Vec4<T>::Vec4(Direction4D dir, T scale) {
        using enum Direction4D;
        switch (dir) {
            case RIGHT: x =  scale; y =  0;     z =  0;     w =  0;     return;
            case LEFT : x = -scale; y =  0;     z =  0;     w =  0;     return;
            case UP   : x =  0;     y =  scale; z =  0;     w =  0;     return;
            case DOWN : x =  0;     y = -scale; z =  0;     w =  0;     return;
            case FRONT: x =  0;     y =  0;     z =  scale; w =  0;     return;
            case BACK : x =  0;     y =  0;     z = -scale; w =  0;     return;
            case IN   : x =  0;     y =  0;     z =  0;     w =  scale; return;
            case OUT  : x =  0;     y =  0;     z =  0;     w = -scale; return;
            case ZERO : x =  0;     y =  0;     z =  0;     w =  0;     return;
            case UNIT : x =  scale; y =  scale; z =  scale; w =  scale; return;
            default: x = 0; y = 0;
        }
    }

    template <Numeral T>
    Vec4<T>::Vec4(Corner4D cor, T scale) {
        using enum Corner4D;
        switch (cor) {
            case INNER_FRONT_TOP_RIGHT   : x =  scale; y =  scale; z =  scale; w =  scale; return;
            case INNER_FRONT_TOP_LEFT    : x = -scale; y =  scale; z =  scale; w =  scale; return;
            case INNER_FRONT_BOTTOM_RIGHT: x =  scale; y = -scale; z =  scale; w =  scale; return;
            case INNER_FRONT_BOTTOM_LEFT : x = -scale; y = -scale; z =  scale; w =  scale; return;
            case INNER_BACK_TOP_RIGHT    : x =  scale; y =  scale; z = -scale; w =  scale; return;
            case INNER_BACK_TOP_LEFT     : x = -scale; y =  scale; z = -scale; w =  scale; return;
            case INNER_BACK_BOTTOM_RIGHT : x =  scale; y = -scale; z = -scale; w =  scale; return;
            case INNER_BACK_BOTTOM_LEFT  : x = -scale; y = -scale; z = -scale; w =  scale; return;
            case OUTER_FRONT_TOP_RIGHT   : x =  scale; y =  scale; z =  scale; w = -scale; return;
            case OUTER_FRONT_TOP_LEFT    : x = -scale; y =  scale; z =  scale; w = -scale; return;
            case OUTER_FRONT_BOTTOM_RIGHT: x =  scale; y = -scale; z =  scale; w = -scale; return;
            case OUTER_FRONT_BOTTOM_LEFT : x = -scale; y = -scale; z =  scale; w = -scale; return;
            case OUTER_BACK_TOP_RIGHT    : x =  scale; y =  scale; z = -scale; w = -scale; return;
            case OUTER_BACK_TOP_LEFT     : x = -scale; y =  scale; z = -scale; w = -scale; return;
            case OUTER_BACK_BOTTOM_RIGHT : x =  scale; y = -scale; z = -scale; w = -scale; return;
            case OUTER_BACK_BOTTOM_LEFT  : x = -scale; y = -scale; z = -scale; w = -scale; return;
            case CENTER: x = 0; y = 0; z = 0; return;
            default: x = 0; y = 0; z = 0;
        }
    }

    template <Numeral T> Vec4<T>::Vec4(const Vec2<T>& xy, float z, float w)  : x(xy.x),  y(xy.y),  z(z),     w(w) {}
    template <Numeral T> Vec4<T>::Vec4(const Vec2<T>& xy, const Vec2<T>& zw) : x(xy.x),  y(xy.y),  z(zw.z),  w(zw.w) {}
    template <Numeral T> Vec4<T>::Vec4(const Vec3<T>& xyz, float w)          : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
#pragma endregion
}
