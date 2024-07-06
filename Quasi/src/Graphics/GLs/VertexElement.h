#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "VertexBufferLayout.h"

#define Q_GL_VERTEX_T(T) using __vertex_type__ = T; static constexpr bool _internalVertexFlag = true

#define Q_GL_VERTEX_CUSTOM_TRANSFORM(M, N) __vertex_type__ __vertex_mul__(const Math::Matrix3D& M, const Math::Matrix3D& N) const

#define __Q_GL_GET_THIRD__(X, Y, Z, ...) Z
#define __Q_GL_VERTEX_TRANSFORM_FIELD_DEFAULT__(M) __Q_GL_VERTEX_TRANSFORM_FIELD_CUSTOM__(M, Quasi::Graphics::PositionTransformer)
#define __Q_GL_VERTEX_TRANSFORM_FIELD_CUSTOM__(M, H) copy.M = (H {})(copy.M, _m, _nm)
#define __Q_GL_VERTEX_TRANSFORM_FIELD__(...) __Q_GL_GET_THIRD__(__VA_ARGS__, __Q_GL_VERTEX_TRANSFORM_FIELD_CUSTOM__, __Q_GL_VERTEX_TRANSFORM_FIELD_DEFAULT__)(__VA_ARGS__);
#define __Q_GL_VERTEX_TRANSFORM_SEQUENCE__(SEQ) Q_CAT(__Q_GL_VT1__ SEQ, END__)
#define __Q_GL_VT1__(...) __Q_GL_VERTEX_TRANSFORM_FIELD__(__VA_ARGS__) __Q_GL_VT2__
#define __Q_GL_VT2__(...) __Q_GL_VERTEX_TRANSFORM_FIELD__(__VA_ARGS__) __Q_GL_VT1__
#define __Q_GL_VT1__END__
#define __Q_GL_VT2__END__
#define Q_GL_VERTEX_TRANSFORM_FIELDS(FIELDS) \
    Q_GL_VERTEX_CUSTOM_TRANSFORM(_m, _nm) { __vertex_type__ copy = *this; __Q_GL_VERTEX_TRANSFORM_SEQUENCE__(FIELDS) return copy; }

#define __REMOVE_FIRST__(FIRST, ...) __VA_ARGS__

#define __Q_GL_VERTEX_TYPES__(SEQ) Q_CAT(__Q_GL_VY1__ SEQ, END__)
#define __Q_GL_VY1__(X) using _##X = MemberT<&__vertex_type__::X>; __Q_GL_VY2__
#define __Q_GL_VY2__(X) using _##X = MemberT<&__vertex_type__::X>; __Q_GL_VY1__
#define __Q_GL_VY1__END__
#define __Q_GL_VY2__END__

#define __Q_GL_VERTEX_FIELDS__(SEQ) Q_CAT(__Q_GL_VE1__ SEQ, END__)
#define __Q_GL_VE1__(X) Q_DEFER(Q_COMMA)() Quasi::Graphics::VertexBufferComponent::Type<types::_##X>() __Q_GL_VE2__
#define __Q_GL_VE2__(X) Q_DEFER(Q_COMMA)() Quasi::Graphics::VertexBufferComponent::Type<types::_##X>() __Q_GL_VE1__
#define __Q_GL_VE1__END__
#define __Q_GL_VE2__END__

#define __Q_GL_VERTEX_TEMPLATE_LIST__(SEQ) Q_CAT(__Q_GL_VTL1__ SEQ, END__)
#define __Q_GL_VTL1__(X) Q_DEFER(Q_COMMA)() class _type_##X##_ = Quasi::Graphics::VertexBuilder::Default<types::_##X> __Q_GL_VTL2__
#define __Q_GL_VTL2__(X) Q_DEFER(Q_COMMA)() class _type_##X##_ = Quasi::Graphics::VertexBuilder::Default<types::_##X> __Q_GL_VTL1__
#define __Q_GL_VTL1__END__
#define __Q_GL_VTL2__END__

#define __Q_GL_VERTEX_MEMBER_LIST__(SEQ) Q_CAT(__Q_GL_VML1__ SEQ, END__)
#define __Q_GL_VML1__(X) _type_##X##_ X {}; __Q_GL_VML2__
#define __Q_GL_VML2__(X) _type_##X##_ X {}; __Q_GL_VML1__
#define __Q_GL_VML1__END__
#define __Q_GL_VML2__END__

#define __Q_GL_VERTEX_BUILD_LIST__(SEQ) Q_CAT(__Q_GL_VBL1__ SEQ, END__)
#define __Q_GL_VBL1__(X) Q_DEFER(Q_COMMA)() Quasi::Graphics::VertexBuilder::BlueprintMember<&__vertex_type__::X Q_DEFER(Q_COMMA)() _type_##X##_> { std::move(X) } __Q_GL_VBL2__
#define __Q_GL_VBL2__(X) Q_DEFER(Q_COMMA)() Quasi::Graphics::VertexBuilder::BlueprintMember<&__vertex_type__::X Q_DEFER(Q_COMMA)() _type_##X##_> { std::move(X) } __Q_GL_VBL1__
#define __Q_GL_VBL1__END__
#define __Q_GL_VBL2__END__

#define __Q_GL_VERTEX_STRING_LIST__(SEQ) Q_CAT(__Q_GL_VSL1__ SEQ, END__)
#define __Q_GL_VSL1__(X) #X "\0" __Q_GL_VSL2__
#define __Q_GL_VSL2__(X) #X "\0" __Q_GL_VSL1__
#define __Q_GL_VSL1__END__
#define __Q_GL_VSL2__END__

#define Q_GL_VERTEX_FIELD(X) \
    private: struct types { __Q_GL_VERTEX_TYPES__(X) }; public: \
    inline static const auto __VERTEX_LAYOUT__ = Quasi::Graphics::VertexBufferLayout({Q_UNARY(Q_DEFER(__REMOVE_FIRST__)(__Q_GL_VERTEX_FIELDS__(X)))}); \
    template <Q_UNARY(Q_DEFER(__REMOVE_FIRST__)(__Q_GL_VERTEX_TEMPLATE_LIST__(X)))> \
    struct Blueprint { \
        __Q_GL_VERTEX_MEMBER_LIST__(X) \
        __vertex_type__ operator()(const auto& args) { \
            return Quasi::Graphics::VertexBuilder::BlueprintBuilder<__vertex_type__>(args __Q_GL_VERTEX_BUILD_LIST__(X)); \
        } \
    }; \
    static usize __sizeof_self__() { return sizeof(__vertex_type__); } \
    inline static Quasi::Graphics::VertexDebugTypeInfo __typeinfo__ = { \
        __sizeof_self__(), \
        __VERTEX_LAYOUT__, \
        Text::TypeName<__vertex_type__>(), \
        { __Q_GL_VERTEX_STRING_LIST__(X), sizeof(__Q_GL_VERTEX_STRING_LIST__(X)) - 2 } };

namespace Quasi::Graphics {
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
        template <Math::VectorLike V>
        V operator()(const V& vec, const Math::Matrix3D& mat, const Math::Matrix3D& /* normmat */) {
            if constexpr (requires (V t, Math::Matrix3D m) { { m * t } -> std::same_as<V>; })
                return mat * vec;
            else return (V)(mat * (Math::fVector3)vec);
        }
    };
    struct NormalTransformer {
        template <Math::VectorLike V>
        V operator()(const V& vec, const Math::Matrix3D& /* mat */, const Math::Matrix3D& normmat) {
            if constexpr (requires (V t, Math::Matrix3D m) { { m * t } -> std::same_as<V>; })
                return normmat * vec;
            else return (V)(normmat * (Math::fVector3)vec);
        }
    };

    struct Vertex2D {
        Math::fVector2 Position;

        Q_GL_VERTEX_T(Vertex2D);
        Q_GL_VERTEX_FIELD((Position));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position));
    };

    struct VertexColor2D {
        Math::fVector2 Position;
        Math::fColor   Color;

        Q_GL_VERTEX_T(VertexColor2D);
        Q_GL_VERTEX_FIELD((Position)(Color));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position))
    };

    struct VertexTexture2D {
        Math::fVector2 Position;
        Math::fVector2 TextureCoordinate;

        Q_GL_VERTEX_T(VertexTexture2D);
        Q_GL_VERTEX_FIELD((Position)(TextureCoordinate));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position));
    };

    struct Vertex3D {
        Math::fVector3 Position;

        Q_GL_VERTEX_T(Vertex3D);
        Q_GL_VERTEX_FIELD((Position));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position));
    };

    struct VertexNormal3D {
        Math::fVector3 Position;
        Math::fVector3 Normal;

        Q_GL_VERTEX_T(VertexNormal3D);
        Q_GL_VERTEX_FIELD((Position)(Normal));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position)(Normal, NormalTransformer));
    };

    struct VertexColor3D {
        Math::fVector3 Position;
        Math::fColor   Color;

        Q_GL_VERTEX_T(VertexColor3D);
        Q_GL_VERTEX_FIELD((Position)(Color));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position));
    };

    struct VertexColorTexture3D {
        Math::fVector3 Position;
        Math::fColor   Color;
        Math::fVector2 TextureCoordinate;

        Q_GL_VERTEX_T(VertexColorTexture3D);
        Q_GL_VERTEX_FIELD((Position)(Color)(TextureCoordinate));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position));
    };

    struct VertexColorNormal3D {
        Math::fVector3 Position;
        Math::fColor   Color;
        Math::fVector3 Normal;

        Q_GL_VERTEX_T(VertexColorNormal3D);
        Q_GL_VERTEX_FIELD((Position)(Color)(Normal));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position)(Normal, NormalTransformer));
    };

    struct VertexTextureNormal3D {
        Math::fVector3 Position;
        Math::fVector2 TextureCoordinate;
        Math::fVector3 Normal;

        Q_GL_VERTEX_T(VertexTextureNormal3D);
        Q_GL_VERTEX_FIELD((Position)(TextureCoordinate)(Normal));
        Q_GL_VERTEX_TRANSFORM_FIELDS((Position)(Normal, NormalTransformer));
    };

    template <InstanceofVertex T>
    using VertexComponents = typename T::__vertex_params__;

    template <InstanceofVertex T>
    const VertexBufferLayout& VertexLayoutOf() { return T::__VERTEX_LAYOUT__; }

    template <class T>
    T VertexMul(const T& v, const Math::Matrix3D& mat, const Math::Matrix3D& nmat) {
        if constexpr (InstanceofVertex<T>)
            return v.__vertex_mul__(mat, nmat);
        else if constexpr (requires (T t, Math::Matrix3D m) { { m * t } -> std::same_as<T>; })
            return mat * v;
        else return mat * (Math::fVector3)v;
    }
}