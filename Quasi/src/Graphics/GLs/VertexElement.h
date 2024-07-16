#pragma once
#include "Math/Transform2D.h"
#include "Math/Transform3D.h"
#include "VertexBufferLayout.h"

#define Q_GL_VERTEX_T(T, DIM, MEMBS, ... /* may use 'custom transform' */) \
    using _VSelf = T; static constexpr u32 DIMENSION = 0x##DIM >> 4; /* hacking ND into N */ \
    private: struct types { __Q_GL_VERTEX_TYPES__(MEMBS) }; public: \
    inline static const auto VERTEX_LAYOUT = Quasi::Graphics::VertexBufferLayout::FromTypes<Q_UNARY(Q_DEFER(Q_REMOVE_FIRST)(__Q_GL_VERTEX_LAYOUT_FIELDS__(MEMBS)))>(); \
    template <Q_UNARY(Q_DEFER(Q_REMOVE_FIRST)(__Q_GL_VERTEX_TEMPLATE_LIST__(MEMBS)))> \
    struct Blueprint { \
        __Q_GL_VERTEX_MEMBER_LIST__(MEMBS) \
        _VSelf operator()(const auto& args) { \
            return _VSelf { Q_UNARY(__Q_GL_VERTEX_BUILD_LIST__(MEMBS)) }; \
        } \
    }; \
    \
    _VSelf _VMul(const Math::ITransformer##DIM auto& _tr) const { \
        Q_IF_ELSE(Q_HAS_ARGS(__VA_ARGS__), (return __VA_ARGS__(_tr);), ( \
            return _VSelf { __Q_GL_VERTEX_TRANSFORM_SEQUENCE__(MEMBS) }; \
        ))\
    } \

#define __Q_GL_VERTEX_TRANSFORM_SEQUENCE__(SEQ) Q_DEFER(Q_REMOVE_FIRST)(Q_CAT(__Q_GL_VT1__ SEQ, END__))
#define __Q_GL_VT1__(M, ...) Q_DEFER(Q_COMMA)() .M = Q_IF_ELSE(Q_IS_EMPTY(__VA_ARGS__), (M), (__VA_ARGS__::transform(M, _tr))) __Q_GL_VT2__
#define __Q_GL_VT2__(M, ...) Q_DEFER(Q_COMMA)() .M = Q_IF_ELSE(Q_IS_EMPTY(__VA_ARGS__), (M), (__VA_ARGS__::transform(M, _tr))) __Q_GL_VT1__
#define __Q_GL_VT1__END__
#define __Q_GL_VT2__END__

#define __Q_GL_VERTEX_TYPES__(SEQ) Q_CAT(__Q_GL_VY1__ SEQ, END__)
#define __Q_GL_VY1__(X, ...) using _##X = MemberT<&_VSelf::X>; __Q_GL_VY2__
#define __Q_GL_VY2__(X, ...) using _##X = MemberT<&_VSelf::X>; __Q_GL_VY1__
#define __Q_GL_VY1__END__
#define __Q_GL_VY2__END__

#define __Q_GL_VERTEX_LAYOUT_FIELDS__(SEQ) Q_CAT(__Q_GL_VL1__ SEQ, END__)
#define __Q_GL_VL1__(X, ...) Q_DEFER(Q_COMMA)() types::_##X __Q_GL_VL2__
#define __Q_GL_VL2__(X, ...) Q_DEFER(Q_COMMA)() types::_##X __Q_GL_VL1__
#define __Q_GL_VL1__END__
#define __Q_GL_VL2__END__

#define __Q_GL_VERTEX_TEMPLATE_LIST__(SEQ) Q_CAT(__Q_GL_VTL1__ SEQ, END__)
#define __Q_GL_VTL1__(X, ...) Q_DEFER(Q_COMMA)() class X##_t_ = Quasi::Graphics::VertexBuilder::Default<types::_##X> __Q_GL_VTL2__
#define __Q_GL_VTL2__(X, ...) Q_DEFER(Q_COMMA)() class X##_t_ = Quasi::Graphics::VertexBuilder::Default<types::_##X> __Q_GL_VTL1__
#define __Q_GL_VTL1__END__
#define __Q_GL_VTL2__END__

#define __Q_GL_VERTEX_MEMBER_LIST__(SEQ) Q_CAT(__Q_GL_VML1__ SEQ, END__)
#define __Q_GL_VML1__(X, ...) X##_t_ X {}; __Q_GL_VML2__
#define __Q_GL_VML2__(X, ...) X##_t_ X {}; __Q_GL_VML1__
#define __Q_GL_VML1__END__
#define __Q_GL_VML2__END__

#define __Q_GL_VERTEX_BUILD_LIST__(SEQ) Q_DEFER(Q_REMOVE_FIRST)(Q_CAT(__Q_GL_VBL1__ SEQ, END__))
#define __Q_GL_VBL1__(X, ...) Q_DEFER(Q_COMMA)() .X = X(args) __Q_GL_VBL2__
#define __Q_GL_VBL2__(X, ...) Q_DEFER(Q_COMMA)() .X = X(args) __Q_GL_VBL1__
#define __Q_GL_VBL1__END__
#define __Q_GL_VBL2__END__

namespace Quasi::Graphics {
    template <class T> concept IVertex = requires { typename T::_VSelf; };

    namespace VertexBuilder {
        struct MeshConstructData2D;
        struct MeshConstructData3D;

        template <class T>
        struct Default;
    }

    struct PosTf {
        static Math::fVector2 transform(const Math::fVector2& vec, const Math::ITransformer2D auto& transform) {
            return transform.Transform(vec);
        }

        static Math::fVector3 transform(const Math::fVector3& vec, const Math::ITransformer3D auto& transform) {
            return transform.Transform(vec);
        }
    };
    struct NormTf {
        static Math::fVector2 transform(const Math::fVector2& vec, const Math::ITransformer2D auto& transform) {
            return transform.TransformNormal(vec);
        }

        static Math::fVector3 transform(const Math::fVector3& vec, const Math::ITransformer3D auto& transform) {
            return transform.TransformNormal(vec);
        }
    };

    struct Vertex2D {
        Math::fVector2 Position;

        Q_GL_VERTEX_T(Vertex2D, 2D, (Position, PosTf));
    };

    struct VertexColor2D {
        Math::fVector2 Position;
        Math::fColor   Color;

        Q_GL_VERTEX_T(VertexColor2D, 2D, (Position, PosTf)(Color));
    };

    struct VertexTexture2D {
        Math::fVector2 Position;
        Math::fVector2 TextureCoordinate;

        Q_GL_VERTEX_T(VertexTexture2D, 2D, (Position, PosTf)(TextureCoordinate));
    };

    struct Vertex3D {
        Math::fVector3 Position;

        Q_GL_VERTEX_T(Vertex3D, 3D, (Position, PosTf));
    };

    struct VertexNormal3D {
        Math::fVector3 Position;
        Math::fVector3 Normal;

        Q_GL_VERTEX_T(VertexNormal3D, 3D, (Position, PosTf)(Normal, NormTf));
    };

    struct VertexColor3D {
        Math::fVector3 Position;
        Math::fColor   Color;

        Q_GL_VERTEX_T(VertexColor3D, 3D, (Position, PosTf)(Color));
    };

    struct VertexColorTexture3D {
        Math::fVector3 Position;
        Math::fColor   Color;
        Math::fVector2 TextureCoordinate;

        Q_GL_VERTEX_T(VertexColorTexture3D, 3D, (Position, PosTf)(Color)(TextureCoordinate));
    };

    struct VertexColorNormal3D {
        Math::fVector3 Position;
        Math::fColor   Color;
        Math::fVector3 Normal;

        Q_GL_VERTEX_T(VertexColorNormal3D, 3D, (Position, PosTf)(Color)(Normal, NormTf));
    };

    struct VertexTextureNormal3D {
        Math::fVector3 Position;
        Math::fVector2 TextureCoordinate;
        Math::fVector3 Normal;

        Q_GL_VERTEX_T(VertexTextureNormal3D, 3D, (Position, PosTf)(TextureCoordinate)(Normal, NormTf));
    };

    template <IVertex T>
    const VertexBufferLayout& VertexLayoutOf() { return T::VERTEX_LAYOUT; }

    template <IVertex T> requires (T::DIMENSION == 2)
    T VertexMul(const T& v, const Math::ITransformer2D auto& transform) {
        return v._VMul(transform);
    }

    template <IVertex T> requires (T::DIMENSION == 3)
    T VertexMul(const T& v, const Math::ITransformer3D auto& transform) {
        return v._VMul(transform);
    }
}