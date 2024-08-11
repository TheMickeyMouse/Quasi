#pragma once
#include "Math/Transform2D.h"
#include "Math/Transform3D.h"
#include "VertexBufferLayout.h"

#define Q_GL_DEFINE_VERTEX(T, DIM, MEMBS, ... /* may use 'custom transform' */) \
    static constexpr bool IS_GL_VERTEX = true; \
    static constexpr u32 DIMENSION = 0x##DIM >> 4; /* hacking ND into N */ \
    private: struct types { Q_ITERATE_W_SEQUENCE(Q_GL_VERTTYPES_IT, (T), MEMBS) }; public: \
    inline static const auto VERTEX_LAYOUT = Quasi::Graphics::VertexBufferLayout::FromTypes< \
        Q_INVOKE(Q_ARGS_SKIP, Q_ITERATE_SEQUENCE(Q_GL_VERTLAYOUT_IT, MEMBS)) \
    >(); \
    template <Q_INVOKE(Q_ARGS_SKIP, Q_ITERATE_SEQUENCE(Q_GL_VERTBTEMPLIST_IT, MEMBS))> \
    struct Blueprint { \
        Q_ITERATE_SEQUENCE(Q_GL_VERTBMEMBLIST_IT, MEMBS) \
        T operator()(const auto& args) { \
            return T { Q_INVOKE(Q_ARGS_SKIP, Q_ITERATE_SEQUENCE(Q_GL_VERTBUILDLIST_IT, MEMBS)) }; \
        } \
    }; \
    \
    T _VMul(const Math::ITransformer##DIM auto& _tr) const { \
        Q_IF_ARGS_ELSE((__VA_ARGS__), (return __VA_ARGS__(_tr);), ( \
            return T { Q_INVOKE(Q_ARGS_SKIP, Q_ITERATE_SEQUENCE(Q_GL_VERTTRANS_IT, MEMBS)) }; \
        ))\
    } \

#define Q_GL_VERTTRANS_IT(M, ...) , .M = Q_IF_ARGS_ELSE((__VA_ARGS__), (__VA_ARGS__::transform(M, _tr)), (M))
#define Q_GL_VERTLAYOUT_IT(X, ...) , types::_##X
#define Q_GL_VERTTYPES_IT(T, X, ...) using _##X = Quasi::MemberT<&T::X>;
#define Q_GL_VERTBTEMPLIST_IT(X, ...) , class X##_t_ = Quasi::Graphics::VertexBuilder::Default<types::_##X>
#define Q_GL_VERTBMEMBLIST_IT(X, ...) X##_t_ X {};
#define Q_GL_VERTBUILDLIST_IT(X, ...) , .X = X(args)

namespace Quasi::Graphics {
    template <class T> concept IVertex = requires { T::IS_GL_VERTEX; };

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

        Q_GL_DEFINE_VERTEX(Vertex2D, 2D, (Position, PosTf));
    };

    struct VertexColor2D {
        Math::fVector2 Position;
        Math::fColor   Color;

        Q_GL_DEFINE_VERTEX(VertexColor2D, 2D, (Position, PosTf)(Color));
    };

    struct VertexTexture2D {
        Math::fVector2 Position;
        Math::fVector2 TextureCoordinate;

        Q_GL_DEFINE_VERTEX(VertexTexture2D, 2D, (Position, PosTf)(TextureCoordinate));
    };

    struct Vertex3D {
        Math::fVector3 Position;

        Q_GL_DEFINE_VERTEX(Vertex3D, 3D, (Position, PosTf));
    };

    struct VertexNormal3D {
        Math::fVector3 Position;
        Math::fVector3 Normal;

        Q_GL_DEFINE_VERTEX(VertexNormal3D, 3D, (Position, PosTf)(Normal, NormTf));
    };

    struct VertexColor3D {
        Math::fVector3 Position;
        Math::fColor   Color;

        Q_GL_DEFINE_VERTEX(VertexColor3D, 3D, (Position, PosTf)(Color));
    };

    struct VertexColorTexture3D {
        Math::fVector3 Position;
        Math::fColor   Color;
        Math::fVector2 TextureCoordinate;

        Q_GL_DEFINE_VERTEX(VertexColorTexture3D, 3D, (Position, PosTf)(Color)(TextureCoordinate));
    };

    struct VertexColorNormal3D {
        Math::fVector3 Position;
        Math::fColor   Color;
        Math::fVector3 Normal;

        Q_GL_DEFINE_VERTEX(VertexColorNormal3D, 3D, (Position, PosTf)(Color)(Normal, NormTf));
    };

    struct VertexTextureNormal3D {
        Math::fVector3 Position;
        Math::fVector2 TextureCoordinate;
        Math::fVector3 Normal;

        Q_GL_DEFINE_VERTEX(VertexTextureNormal3D, 3D, (Position, PosTf)(TextureCoordinate)(Normal, NormTf));
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