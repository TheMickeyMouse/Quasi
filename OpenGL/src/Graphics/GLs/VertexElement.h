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

#define __GL_VERTEX_ELEMENT__(M) .Join(Graphics::VertexBufferComponent::Type<STDU_DECLTYPE_MEMBER(__vertex_type__, M)>())
#define __GL_VERTEX_FIELDS__(SEQ) STDU_CAT(__GL_VE1__ SEQ, END__)
#define __GL_VE1__(X) __GL_VERTEX_ELEMENT__(X) __GL_VE2__
#define __GL_VE2__(X) __GL_VERTEX_ELEMENT__(X) __GL_VE1__
#define __GL_VE1__END__
#define __GL_VE2__END__

#define __GL_VERTEX_ELEMENT_LIST__(M) ::join<&__vertex_type__::M>
#define __GL_VERTEX_FIELD_LIST__(SEQ) STDU_CAT(__GL_VEL1__ SEQ, END__)
#define __GL_VEL1__(X) __GL_VERTEX_ELEMENT_LIST__(X) __GL_VEL2__
#define __GL_VEL2__(X) __GL_VERTEX_ELEMENT_LIST__(X) __GL_VEL1__
#define __GL_VEL1__END__
#define __GL_VEL2__END__

#define GL_VERTEX_FIELD(X) \
    using __vertex_params__ = stdu::anylist<> __GL_VERTEX_FIELD_LIST__(X); \
    inline static const auto __VERTEX_LAYOUT__ = Graphics::VertexBufferLayout() __GL_VERTEX_FIELDS__(X)

namespace Graphics {
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

    template <class T>
    concept InstanceofVertex = T::_internalVertexFlag;

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

    template <InstanceofVertex T, stdu::anylist_instance Fill>
    struct VertexConverter {
        template <class... Args> static std::tuple<Args...> tuple_get(stdu::typelist<Args...>) { return {}; }
        using Tup = decltype(tuple_get(typename Fill::type {}));
        Tup fills;

        VertexConverter(const Tup& f) : fills(f) {}

        template <class P1, class C1, class P2, class C2>
        static constexpr bool prop_eq(P1 C1::* p1, P2 C2::* p2) {
            if constexpr (std::is_same_v<P1, P2> && std::is_same_v<C1, C2>)
                return p1 == p2;
            else return false;
        }

        template <auto, class>
        static constexpr bool IsFill = false;
        template <auto Match, auto... Args>
        static constexpr bool IsFill<Match, stdu::anylist<Args...>> = (prop_eq(Match, Args) || ...);

        template <stdu::anylist_instance Accum, auto First, auto ...C> struct CompLeftoverT;
        template <stdu::anylist_instance Accum, auto First> struct CompLeftoverT<Accum, First> {
            using type = std::conditional_t<IsFill<First, Fill>, Accum, typename Accum::template join<First>>;
        };
        template <stdu::anylist_instance Accum, auto First, auto ...C> requires IsFill<First, Fill>
        struct CompLeftoverT<Accum, First, C...> {
            using type = typename CompLeftoverT<Accum, C...>::type;
        };
        template <stdu::anylist_instance Accum, auto First, auto ...C> requires (!IsFill<First, Fill>)
        struct CompLeftoverT<Accum, First, C...> {
            using type = typename CompLeftoverT<typename Accum::template join<First>, C...>::type;
        };
        template <auto... P> static auto params_get(stdu::anylist<P...>) {
            return typename CompLeftoverT<stdu::anylist<>, P...>::type {};
        }
        using ParamComp = decltype(params_get(VertexComponents<T> {}));
        using Params = decltype(tuple_get(typename ParamComp::type {}));

        template <auto... F>
        static constexpr auto props_of(stdu::anylist<F...>) {
            return std::make_tuple(F...);
        }

        template <class Tp, class Vp, usize... I>
        void set_props(T& t, const Tp& props, const Vp& vals, std::integer_sequence<usize, I...>) {
            [&] {
                t.*(std::get<I>(props)) = std::get<I>(vals);
            }();
        }

        T operator()(const Params& in) {
            T base;
            set_props(base, props_of(Fill {}), fills, std::make_index_sequence<Fill::size> {});
            set_props(base, props_of(ParamComp {}), in, std::make_index_sequence<ParamComp::size> {});
            return base;
        }
    };
}