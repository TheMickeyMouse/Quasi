#pragma once
#include "Math/Transform2D.h"
#include "Math/Transform3D.h"
#include "VertexBufferLayout.h"

#define Q_GL_DEFINE_VERTEX(T, DIM, MEMBS, ... /* may use 'custom transform' */) \
    static constexpr bool IS_GL_VERTEX = true; \
    using Self = T; \
    static constexpr u32 DIMENSION = 0x##DIM >> 4; /* hacking ND into N */ \
    public: \
    inline static const auto VERTEX_LAYOUT = Quasi::Graphics::VertexBufferLayout::FromTypes< \
        Q_INVOKE(Q_ARGS_SKIP, Q_ITERATE_SEQUENCE(Q_GL_VERTLAYOUT_IT, MEMBS)) \
    >(); \
    \
    T _VMul(const Math::ITransformer##DIM auto& _tr) const { \
        Q_IF_ARGS_ELSE((__VA_ARGS__), (return __VA_ARGS__(_tr);), ( \
            return T { Q_INVOKE(Q_ARGS_SKIP, Q_ITERATE_SEQUENCE(Q_GL_VERTTRANS_IT, MEMBS)) }; \
        ))\
    }

#define Q_GL_VERTTRANS_IT(MX) , .Q_ARGS_FIRST MX = Q_GL_VERTTRANS_WHEN_T MX
#define Q_GL_VERTTRANS_WHEN_T(M, ...) __VA_OPT__(__VA_ARGS__::transform Q_LPAREN() ) M __VA_OPT__(, _tr Q_RPAREN())
#define Q_GL_VERTLAYOUT_IT(X_) , decltype(Self:: Q_ARGS_FIRST X_)

#define QuasiDefineVertex$(...) Q_GL_DEFINE_VERTEX(__VA_ARGS__)

namespace Quasi::Graphics {
    template <class T> concept IVertex = requires { T::IS_GL_VERTEX; };

    namespace VertexBuilder {
        struct MeshConstructData2D { Math::fv2 Position; };
        struct MeshConstructData3D { Math::fv3 Position, Normal; };
    }

    struct PosTf {
        static Math::fv2 transform(const Math::fv2& vec, const Math::ITransformer2D auto& transform) {
            return transform.Transform(vec);
        }

        static Math::fv3 transform(const Math::fv3& vec, const Math::ITransformer3D auto& transform) {
            return transform.Transform(vec);
        }
    };
    struct NormTf {
        static Math::fv2 transform(const Math::fv2& vec, const Math::ITransformer2D auto& transform) {
            return transform.TransformNormal(vec);
        }

        static Math::fv3 transform(const Math::fv3& vec, const Math::ITransformer3D auto& transform) {
            return transform.TransformNormal(vec);
        }
    };

    struct Vertex2D {
        Math::fv2 Position;

        QuasiDefineVertex$(Vertex2D, 2D, (Position, PosTf));
    };

    struct VertexColor2D {
        Math::fv2    Position;
        Math::fColor Color;

        QuasiDefineVertex$(VertexColor2D, 2D, (Position, PosTf)(Color));
    };

    struct VertexTexture2D {
        Math::fv2 Position;
        Math::fv2 TextureCoordinate;

        QuasiDefineVertex$(VertexTexture2D, 2D, (Position, PosTf)(TextureCoordinate));
    };

    struct Vertex3D {
        Math::fv3 Position;

        QuasiDefineVertex$(Vertex3D, 3D, (Position, PosTf));
    };

    struct VertexNormal3D {
        Math::fv3 Position;
        Math::fv3 Normal;

        QuasiDefineVertex$(VertexNormal3D, 3D, (Position, PosTf)(Normal, NormTf));
    };

    struct VertexColor3D {
        Math::fv3    Position;
        Math::fColor Color;

        QuasiDefineVertex$(VertexColor3D, 3D, (Position, PosTf)(Color));
    };

    struct VertexColorTexture3D {
        Math::fv3    Position;
        Math::fColor Color;
        Math::fv2    TextureCoordinate;

        QuasiDefineVertex$(VertexColorTexture3D, 3D, (Position, PosTf)(Color)(TextureCoordinate));
    };

    struct VertexColorNormal3D {
        Math::fv3    Position;
        Math::fColor Color;
        Math::fv3    Normal;

        QuasiDefineVertex$(VertexColorNormal3D, 3D, (Position, PosTf)(Color)(Normal, NormTf));
    };

    struct VertexTextureNormal3D {
        Math::fv3 Position;
        Math::fv2 TextureCoordinate;
        Math::fv3 Normal;

        QuasiDefineVertex$(VertexTextureNormal3D, 3D, (Position, PosTf)(TextureCoordinate)(Normal, NormTf));
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