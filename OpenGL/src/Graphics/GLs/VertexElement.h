#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "VertexBufferLayout.h"
#include <stdu/types.h>

#define GL_VERTEX_T(T) using __vertex_type__ = T; static constexpr bool _internalVertexFlag = true

#define GL_VERTEX_CUSTOM_TRANSFORM(M, N) __vertex_type__ __vertex_mul__(const Maths::mat3D& M, const Maths::mat3D& N) const

#define __GL_GET_THIRD__(X, Y, Z, ...) Z
#define __GL_VERTEX_TRANSFORM_FIELD_DEFAULT__(M) __GL_VERTEX_TRANSFORM_FIELD_CUSTOM__(M, Graphics::PositionTransformer)
#define __GL_VERTEX_TRANSFORM_FIELD_CUSTOM__(M, H) copy.M = (H {})(copy.M, _m, _nm)
#define __GL_VERTEX_TRANSFORM_FIELD__(...) __GL_GET_THIRD__(__VA_ARGS__, __GL_VERTEX_TRANSFORM_FIELD_CUSTOM__, __GL_VERTEX_TRANSFORM_FIELD_DEFAULT__)(__VA_ARGS__);
#define __GL_VERTEX_TRANSFORM_SEQUENCE__(SEQ) STDU_CAT(__GL_VT1__ SEQ, END__)
#define __GL_VT1__(...) __GL_VERTEX_TRANSFORM_FIELD__(__VA_ARGS__) __GL_VT2__
#define __GL_VT2__(...) __GL_VERTEX_TRANSFORM_FIELD__(__VA_ARGS__) __GL_VT1__
#define __GL_VT1__END__
#define __GL_VT2__END__
#define GL_VERTEX_TRANSFORM_FIELDS(FIELDS) \
    GL_VERTEX_CUSTOM_TRANSFORM(_m, _nm) { __vertex_type__ copy = *this; __GL_VERTEX_TRANSFORM_SEQUENCE__(FIELDS) return copy; }

#define __REMOVE_FIRST__(FIRST, ...) __VA_ARGS__

#define __GL_VERTEX_TYPES__(SEQ) STDU_CAT(__GL_VY1__ SEQ, END__)
#define __GL_VY1__(X) using _##X = stdu::member_t<&__vertex_type__::X>; __GL_VY2__
#define __GL_VY2__(X) using _##X = stdu::member_t<&__vertex_type__::X>; __GL_VY1__
#define __GL_VY1__END__
#define __GL_VY2__END__

#define __GL_VERTEX_FIELDS__(SEQ) STDU_CAT(__GL_VE1__ SEQ, END__)
#define __GL_VE1__(X) STDU_DEFER(STDU_COMMA)() Graphics::VertexBufferComponent::Type<types::_##X>() __GL_VE2__
#define __GL_VE2__(X) STDU_DEFER(STDU_COMMA)() Graphics::VertexBufferComponent::Type<types::_##X>() __GL_VE1__
#define __GL_VE1__END__
#define __GL_VE2__END__

#define __GL_VERTEX_TEMPLATE_LIST__(SEQ) STDU_CAT(__GL_VTL1__ SEQ, END__)
#define __GL_VTL1__(X) STDU_DEFER(STDU_COMMA)() class _type_##X##_ = Graphics::VertexBuilder::Default<types::_##X> __GL_VTL2__
#define __GL_VTL2__(X) STDU_DEFER(STDU_COMMA)() class _type_##X##_ = Graphics::VertexBuilder::Default<types::_##X> __GL_VTL1__
#define __GL_VTL1__END__
#define __GL_VTL2__END__

#define __GL_VERTEX_PARAM_LIST__(SEQ) STDU_CAT(__GL_VPL1__ SEQ, END__)
#define __GL_VPL1__(X) _type_##X##_ X {}; __GL_VPL2__
#define __GL_VPL2__(X) _type_##X##_ X {}; __GL_VPL1__
#define __GL_VPL1__END__
#define __GL_VPL2__END__

#define __GL_VERTEX_BUILD_LIST__(SEQ) STDU_CAT(__GL_VBL1__ SEQ, END__)
#define __GL_VBL1__(X) STDU_DEFER(STDU_COMMA)() Graphics::VertexBuilder::BlueprintMember<&__vertex_type__::X STDU_DEFER(STDU_COMMA)() _type_##X##_> { std::move(X) } __GL_VBL2__
#define __GL_VBL2__(X) STDU_DEFER(STDU_COMMA)() Graphics::VertexBuilder::BlueprintMember<&__vertex_type__::X STDU_DEFER(STDU_COMMA)() _type_##X##_> { std::move(X) } __GL_VBL1__
#define __GL_VBL1__END__
#define __GL_VBL2__END__

#define GL_VERTEX_FIELD(X) \
    private: struct types { __GL_VERTEX_TYPES__(X) }; public: \
    inline static const auto __VERTEX_LAYOUT__ = Graphics::VertexBufferLayout({STDU_UNARY(STDU_DEFER(__REMOVE_FIRST__)(__GL_VERTEX_FIELDS__(X)))}); \
    template <STDU_UNARY(STDU_DEFER(__REMOVE_FIRST__)(__GL_VERTEX_TEMPLATE_LIST__(X)))> \
    struct Blueprint { \
        __GL_VERTEX_PARAM_LIST__(X) \
        __vertex_type__ operator()(const auto& args) { \
            return Graphics::VertexBuilder::BlueprintBuilder<__vertex_type__>(args __GL_VERTEX_BUILD_LIST__(X)); \
        } \
    };

namespace Graphics {
    template <class T> concept InstanceofVertex = T::_internalVertexFlag;

    namespace VertexBuilder {
        struct MeshConstructData2D;
        struct MeshConstructData3D;

        template <class T>
        struct Default;

        template <auto P, class F>
        struct BlueprintMember;

        template <InstanceofVertex T, class... Ms>
        T BlueprintBuilder(const auto& args, Ms... members);
    }

    struct PositionTransformer {
        template <Maths::vec_t V>
        V operator()(const V& vec, const Maths::mat3D& mat, const Maths::mat3D& /* normmat */) {
            if constexpr (requires (V t, Maths::mat4x4 m) { { m * t } -> std::same_as<V>; })
                return mat * vec;
            else return (V)(mat * (Maths::fvec3)vec);
        }
    };
    struct NormalTransformer {
        template <Maths::vec_t V>
        V operator()(const V& vec, const Maths::mat3D& /* mat */, const Maths::mat3D& normmat) {
            if constexpr (requires (V t, Maths::mat4x4 m) { { m * t } -> std::same_as<V>; })
                return normmat * vec;
            else return (V)(normmat * (Maths::fvec3)vec);
        }
    };

    struct Vertex2D {
        Maths::fvec2 Position;

        GL_VERTEX_T(Vertex2D);
        GL_VERTEX_FIELD((Position));
        GL_VERTEX_TRANSFORM_FIELDS((Position))
    };

    struct Vertex3D {
        Maths::fvec3 Position;

        GL_VERTEX_T(Vertex3D);
        GL_VERTEX_FIELD((Position));
        GL_VERTEX_TRANSFORM_FIELDS((Position))
    };

    struct VertexColorTexture3D {
        Maths::fvec3  Position;
        Maths::colorf Color;
        Maths::fvec2  TextureCoordinate;

        GL_VERTEX_T(VertexColorTexture3D);
        GL_VERTEX_FIELD((Position)(Color)(TextureCoordinate));
        GL_VERTEX_TRANSFORM_FIELDS((Position))
    };

    struct VertexTexture2D {
        Maths::fvec2 Position;
        Maths::fvec2 TextureCoordinate;

        GL_VERTEX_T(VertexTexture2D);
        GL_VERTEX_FIELD((Position)(TextureCoordinate));
        GL_VERTEX_TRANSFORM_FIELDS((Position))
    };

    struct VertexColor3D {
        Maths::fvec3  Position;
        Maths::colorf Color;

        GL_VERTEX_T(VertexColor3D);
        GL_VERTEX_FIELD((Position)(Color));
        GL_VERTEX_TRANSFORM_FIELDS((Position))
    };

    struct VertexTextureNormal3D {
        Maths::fvec3 Position;
        Maths::fvec2 TextureCoordinate;
        Maths::fvec3 Normal;

        GL_VERTEX_T(VertexTextureNormal3D);
        GL_VERTEX_FIELD((Position)(TextureCoordinate)(Normal));
        GL_VERTEX_TRANSFORM_FIELDS((Position)(Normal, NormalTransformer))
    };

    struct VertexNormal3D {
        Maths::fvec3 Position;
        Maths::fvec3 Normal;

        GL_VERTEX_T(VertexNormal3D);
        GL_VERTEX_FIELD((Position)(Normal));
        GL_VERTEX_TRANSFORM_FIELDS((Position)(Normal, NormalTransformer))
    };

    template <InstanceofVertex T>
    using VertexComponents = typename T::__vertex_params__;

    template <InstanceofVertex T>
    const VertexBufferLayout& VertexLayoutOf = T::__VERTEX_LAYOUT__;

    template <class T>
    T VertexMul(const T& v, const Maths::mat4x4& mat, const Maths::mat4x4& nmat) {
        if constexpr (InstanceofVertex<T>)
            return v.__vertex_mul__(mat, nmat);
        else if constexpr (requires (T t, Maths::mat4x4 m) { { m * t } -> std::same_as<T>; })
            return mat * v;
        else return mat * (Maths::fvec3)v;
    }
}