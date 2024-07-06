#pragma once
#include "VertexElement.h"

namespace Quasi::Graphics::VertexBuilder {
    namespace details {
        template <auto P, auto... Ps>
        constexpr auto first = P;
    }

    struct MeshConstructData3D {
        Math::fVector3 Position, Normal;
    };

    constexpr Math::fVector3 MeshConstructData3D::* PositionArg = &MeshConstructData3D::Position;
    constexpr Math::fVector3 MeshConstructData3D::* NormalArg   = &MeshConstructData3D::Normal;

    struct MeshConstructData2D {
        Math::fVector2 Position;
    };

    constexpr Math::fVector2 MeshConstructData2D::* PositionArg2D = &MeshConstructData2D::Position;

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

    template <class T> struct Constant {
        T value;
        T operator()(const auto&) const {
            return value;
        }
    };

    template <class F, auto ...Ps>
    requires (std::is_same_v<StructureT<details::first<Ps...>>, StructureT<Ps>> && ...)
    struct From {
        std::decay_t<F> func;

        auto operator()(const StructureT<details::first<Ps...>>& args) const {
            return func(args.*Ps...);
        }
    };

    template <class F>
    struct From<F> {
        std::decay_t<F> func;

        auto operator()(const auto&) const {
            return func();
        }
    };

    template <auto ...Ps>
    auto FromArg(auto&& f) -> From<decltype(f), Ps...> { return { std::forward<decltype(f)>(f) }; }
}