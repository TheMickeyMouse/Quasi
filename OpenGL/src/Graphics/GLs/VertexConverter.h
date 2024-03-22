#pragma once
#include "VertexElement.h"

namespace Graphics::VertexBuilder {
    namespace tmp {
        template <class ...Ts>
        auto typelist2tuple(stdu::typelist<Ts...>) -> std::tuple<Ts...> { return {}; }

        template <class T>
        using tuple_from = decltype(typelist2tuple(T {}));
    }

    struct MeshConstructData3D {
        Maths::fvec3 Position, Normal;
        uint VertexIndex;
    };

    constexpr Maths::fvec3 MeshConstructData3D::* PositionArg = &MeshConstructData3D::Position;
    constexpr Maths::fvec3 MeshConstructData3D::* NormalArg   = &MeshConstructData3D::Normal;
    constexpr uint         MeshConstructData3D::* VIndexArg   = &MeshConstructData3D::VertexIndex;

    struct MeshConstructData2D {
        Maths::fvec2 Position;
        uint VertexIndex;
    };

    constexpr Maths::fvec2 MeshConstructData2D::* PositionArg2D = &MeshConstructData2D::Position;
    constexpr uint         MeshConstructData2D::* VIndexArg2D   = &MeshConstructData2D::VertexIndex;

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
        ([&] {
            base.*((members).prop()) = members(args);
        }(), ...);
        return base;
    }

    template <class T> struct Default {
        T operator()(const auto&) const {
            return T {};
        }
    };

    template <auto P> struct Get {
        stdu::member_t<P> operator()(const stdu::structure_t<P>& args) const {
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
        T operator()(const stdu::structure_t<P>& args) const {
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
    template <class T> using CastVIndex2D   = Cast<T, VIndexArg2D>;

    template <class T> struct Constant {
        T value;
        T operator()(const auto&) const {
            return value;
        }
    };

    template <class F, auto P, auto ...Ps>
    requires (std::is_same_v<stdu::structure_t<P>, stdu::structure_t<Ps>> && ...)
    struct From {
        F func;
        auto operator()(const stdu::structure_t<P>& args) const {
            return func(args.*P, args.*Ps...);
        }
    };

    template <auto ...Ps>
    auto FromArg(auto&& f) -> From<decltype(f), Ps...> { return { std::forward<decltype(f)>(f) }; }
}