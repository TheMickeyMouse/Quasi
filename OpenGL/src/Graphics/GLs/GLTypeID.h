// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier
// ReSharper disable CppClangTidyClangDiagnosticUnusedMacros
// ReSharper disable CppClangTidyBugproneReservedIdentifier
#pragma once
#include "NumTypes.h"
#include "stdu/macros.h"

namespace Graphics {
    enum class GLTypeID {
        UNDEFINED = 0,
        FLOAT  = 0x1406,
        DOUBLE = 0x140A,
        INT    = 0x1404,
        UINT   = 0x1405,
        BYTE   = 0x1400,
        UBYTE  = 0x1401,
        SHORT  = 0x1402,
        USHORT = 0x1403,
    };

    template <GLTypeID> struct GLTypeOf { using type = void; };
    template <GLTypeID T> using GLTypeOfT = typename GLTypeOf<T>::type;
    template <class T> constexpr GLTypeID GLTypeIDOf         = GLTypeID::UNDEFINED;

#define GL_TYPE_CONVERSION(T, ID) template <> inline constexpr GLTypeID GLTypeIDOf<T>  = GLTypeID::ID; \
                       template <> struct GLTypeOf<GLTypeID::ID> { using type = T; }

    GL_TYPE_CONVERSION(float,  FLOAT);
    GL_TYPE_CONVERSION(double, DOUBLE);
    GL_TYPE_CONVERSION(int,    INT);
    GL_TYPE_CONVERSION(uint,   UINT);
    GL_TYPE_CONVERSION(char,   BYTE);
    GL_TYPE_CONVERSION(uchar,  UBYTE);
    GL_TYPE_CONVERSION(int16,  SHORT);
    GL_TYPE_CONVERSION(uint16, USHORT);

#undef GL_TYPE_CONVERSION

#define GL_SIZEOF_DECL(SEQ) constexpr int SizeOf(const GLTypeID type) { switch(type) { GL_SIZEOF_SEQ(SEQ) default: return 0; } }
#define GL_SIZEOF_SEQ(SEQ) STDU_CAT(__GL_SIZE_T1__ SEQ, END__)
#define GL_SIZEOF_TYPE(M) case GLTypeID::M: return sizeof(GLTypeOfT<GLTypeID::M>);
#define __GL_SIZE_T1__(X) GL_SIZEOF_TYPE(X) __GL_SIZE_T2__
#define __GL_SIZE_T2__(X) GL_SIZEOF_TYPE(X) __GL_SIZE_T1__
#define __GL_SIZE_T1__END__
#define __GL_SIZE_T2__END__

    GL_SIZEOF_DECL((FLOAT)(DOUBLE)(INT)(UINT)(BYTE)(UBYTE)(SHORT)(USHORT))

#undef GL_SIZEOF_DECL
#undef GL_SIZEOF_SEQ
#undef GL_SIZEOF_TYPE
#undef __GL_SIZE_T1__
#undef __GL_SIZE_T2__
#undef __GL_SIZE_T1__END__
#undef __GL_SIZE_T2__END__
}
