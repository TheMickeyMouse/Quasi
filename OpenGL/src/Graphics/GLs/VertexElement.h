#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "VertexBufferLayout.h"
#include <stdu/types.h>

#define GL_VERTEX_T(T) using __vertex_type__ = T

#define GL_VERTEX_LAYOUT_OF(T) T::__VERTEX_LAYOUT__
#define GL_VERTEX_CUSTOM_TRANSFORM(N) __vertex_type__ __vertex_mul__(const Maths::mat3D& N) const // NOLINT(bugprone-macro-parentheses)

#define __GL_VERTEX_TRANSFORM_FIELD__(M) copy.M = _m * copy.M;  // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VERTEX_TRANSFORM_SEQUENCE__(SEQ) STDU_CAT(__GL_VT1__ SEQ, END__) // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VT1__(X) __GL_VERTEX_TRANSFORM_FIELD__(X) __GL_VT2__ // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VT2__(X) __GL_VERTEX_TRANSFORM_FIELD__(X) __GL_VT1__ // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __GL_VT1__END__ // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define __GL_VT2__END__ // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define GL_VERTEX_TRANSFORM_FIELDS(FIELDS) \
    GL_VERTEX_CUSTOM_TRANSFORM(_m) { __vertex_type__ copy = *this; __GL_VERTEX_TRANSFORM_SEQUENCE__(FIELDS) return copy; }
#define GL_VERTEX_MATMUL(V, M) V.__vertex_mul__(M)

#define __GL_VERTEX_ELEMENT__(M) .Join(Graphics::VertexBufferComponent::Type<STDU_DECLTYPE_MEMBER(__vertex_type__, M)>()) // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VERTEX_FIELDS__(SEQ) STDU_CAT(__GL_VE1__ SEQ, END__) // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VE1__(X) __GL_VERTEX_ELEMENT__(X) __GL_VE2__ // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VE2__(X) __GL_VERTEX_ELEMENT__(X) __GL_VE1__ // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VE1__END__ // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define __GL_VE2__END__ // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#define GL_VERTEX_FIELD(X) inline static const auto __VERTEX_LAYOUT__ = Graphics::VertexBufferLayout() __GL_VERTEX_FIELDS__(X)

struct VertexColorTexture3D {
    Maths::fvec3  Position;
    Maths::colorf Color;
    Maths::fvec2  TextureCoordinate;
    int TextureID = 0;

    GL_VERTEX_T(VertexColorTexture3D);
    GL_VERTEX_FIELD((Position)(Color)(TextureCoordinate)(TextureID));
    GL_VERTEX_TRANSFORM_FIELDS((Position))
};

struct VertexColorTextureAtlas3D {
    Maths::fvec3  Position;
    Maths::colorf Color;
    int TextureAtlID = 0;
    int TextureCorner = 0;
    
    GL_VERTEX_T(VertexColorTextureAtlas3D);
    GL_VERTEX_FIELD((Position)(Color)(TextureAtlID)(TextureCorner));
    GL_VERTEX_TRANSFORM_FIELDS((Position))
};

struct VertexColor3D {
    Maths::fvec3  Position;
    Maths::colorf Color;

    GL_VERTEX_T(VertexColor3D);
    GL_VERTEX_FIELD((Position)(Color));
    GL_VERTEX_TRANSFORM_FIELDS((Position))
};