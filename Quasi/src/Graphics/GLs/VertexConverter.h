#pragma once
#include "VertexElement.h"

namespace Quasi::Graphics::VertexBuilder {
    namespace details {
        template <class T>
        using tuple_from = decltype(typelist2tuple(T {}));
    }

    struct MeshConstructData3D {
        Math::fVector3 Position, Normal;
        u32 VertexIndex;
    };

    constexpr Math::fVector3 MeshConstructData3D::* PositionArg = &MeshConstructData3D::Position;
    constexpr Math::fVector3 MeshConstructData3D::* NormalArg   = &MeshConstructData3D::Normal;
    constexpr u32            MeshConstructData3D::* VIndexArg   = &MeshConstructData3D::VertexIndex;

    struct MeshConstructData2D {
        Math::fVector2 Position;
        u32 VertexIndex;
    };

    constexpr Math::fVector2 MeshConstructData2D::* PositionArg2D = &MeshConstructData2D::Position;
    constexpr u32            MeshConstructData2D::* VIndexArg2D   = &MeshConstructData2D::VertexIndex;

    template <InstanceofVertex T>
    using Blueprint = typename T::__internal_blueprint__;

    template <auto P, class F>
    struct BlueprintMember {
        F inst;
        auto operator()(const auto& args) const {
            return inst(args);
        }
        auto prop() const { return P; }
    };

    template <InstanceofVertex T, class... Ms>
    T BlueprintBuilder(const auto& args, Ms... members) {
        T base;
        Empty _ { (base.*(members.prop()) = members(args))... };
        return base;
    }

    template <class T> struct Default {
        T operator()(const auto&) const {
            return T {};
        }
    };

    template <auto P> struct Get {
        MemberT<P> operator()(const StructureT<P>& args) const {
            return args.*P;
        }
    };
#define PROPERTY_GET(P) struct Get##P { auto operator()(const auto& args) const { return args.P; } }
    PROPERTY_GET(Position);
    PROPERTY_GET(Normal);
    PROPERTY_GET(Color);
    PROPERTY_GET(TexCoord);
    PROPERTY_GET(TextureCoord);
    PROPERTY_GET(TextureCoordinate);
#undef PROPERTY_GET
    using GetVIndex   = Get<VIndexArg>;
    using GetVIndex2D = Get<VIndexArg2D>;

    template <class T, auto P> struct Cast {
        T operator()(const StructureT<P>& args) const {
            return (T)args.*P;
        }
    };
#define PROPERTY_CAST(P) template <class T> struct Cast##P { T operator()(const auto& args) const { return (T)args.P; } }
    PROPERTY_CAST(Position);
    PROPERTY_CAST(Normal);
    PROPERTY_CAST(Color);
    PROPERTY_CAST(TexCoord);
    PROPERTY_CAST(TextureCoord);
    PROPERTY_CAST(TextureCoordinate);
#undef PROPERTY_CAST
    template <class T> using CastVIndex   = Cast<T, VIndexArg>;
    template <class T> using CastVIndex2D = Cast<T, VIndexArg2D>;

    template <class T> struct Constant {
        T value;
        T operator()(const auto&) const {
            return value;
        }
    };

    template <class F, auto P, auto ...Ps>
    requires (std::is_same_v<StructureT<P>, StructureT<Ps>> && ...)
    struct From {
        F func;
        auto operator()(const StructureT<P>& args) const {
            return func(args.*P, args.*Ps...);
        }
    };

    template <auto ...Ps>
    auto FromArg(auto&& f) -> From<decltype(f), Ps...> { return { std::forward<decltype(f)>(f) }; }
}