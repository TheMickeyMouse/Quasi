// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier
// ReSharper disable CppClangTidyClangDiagnosticUnusedMacros
// ReSharper disable CppClangTidyBugproneReservedIdentifier
#pragma once
#include "enum_utils.h"
#include "NumTypes.h"
#include "stdu/macros.h"
#include "stdu/types.h"

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
    inline STDU_ENUM_TOSTR(GLTypeID, GLTypeName,
        (UNDEFINED, "null")
        (FLOAT, "float")
        (DOUBLE, "double")
        (INT, "int")
        (UINT, "uint")
        (BYTE, "byte")
        (UBYTE, "ubyte")
        (SHORT, "short")
        (USHORT, "ushort"),
        "null")

    using GLTypeMap = stdu::typemap<
        stdu::typelist           <float,           double,           int,           uint,           char,           uchar,           int16,           uint16>,
        stdu::valuelist<GLTypeID, GLTypeID::FLOAT, GLTypeID::DOUBLE, GLTypeID::INT, GLTypeID::UINT, GLTypeID::BYTE, GLTypeID::UBYTE, GLTypeID::SHORT, GLTypeID::USHORT>
    >;

    template <class T> constexpr GLTypeID GLTypeIDOf = stdu::query_map<T, GLTypeMap>;
    template <GLTypeID T> using GLTypeOfT = stdu::reverse_query_map<T, GLTypeMap>;

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
