// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier
// ReSharper disable CppClangTidyClangDiagnosticUnusedMacros
// ReSharper disable CppClangTidyBugproneReservedIdentifier
#pragma once
#include "Utils/Enum.h"
#include "Utils/Macros.h"

namespace Quasi::Graphics {
    enum class GLTypeID {
        UNDEFINED = 0,
        FLOAT  = 0x1406,
        DOUBLE = 0x140A,
        INT    = 0x1404,
        UINT   = 0x1405,
        SBYTE  = 0x1400,
        BYTE   = 0x1401,
        SHORT  = 0x1402,
        USHORT = 0x1403,
    };
    inline Q_ENUM_TOSTR(GLTypeID, GLTypeName,
        (UNDEFINED, "null")
        (FLOAT,     "float")
        (DOUBLE,    "double")
        (INT,       "int")
        (UINT,      "uint")
        (SBYTE,     "sbyte")
        (BYTE,      "byte")
        (SHORT,     "short")
        (USHORT,    "ushort"),
    "null")

    constexpr u32 IsValidGLID(GLTypeID id) {
        return 1 * (id == GLTypeID::FLOAT)  +
               2 * (id == GLTypeID::DOUBLE) +
               3 * (id == GLTypeID::INT)    +
               4 * (id == GLTypeID::UINT)   +
               5 * (id == GLTypeID::SBYTE)  +
               6 * (id == GLTypeID::BYTE)   +
               7 * (id == GLTypeID::SHORT)  +
               8 * (id == GLTypeID::USHORT);
    }

    template <class T> constexpr auto GLGetTypeID  = GLTypeID::UNDEFINED;
    template <> constexpr auto GLGetTypeID<float>  = GLTypeID::FLOAT;
    template <> constexpr auto GLGetTypeID<double> = GLTypeID::DOUBLE;
    template <> constexpr auto GLGetTypeID<int>    = GLTypeID::INT;
    template <> constexpr auto GLGetTypeID<uint>   = GLTypeID::UINT;
    template <> constexpr auto GLGetTypeID<sbyte>  = GLTypeID::SBYTE;
    template <> constexpr auto GLGetTypeID<byte>   = GLTypeID::BYTE;
    template <> constexpr auto GLGetTypeID<short>  = GLTypeID::SHORT;
    template <> constexpr auto GLGetTypeID<ushort> = GLTypeID::USHORT;

    template <GLTypeID T> using GLTypeOfID = std::tuple_element_t<IsValidGLID(T), Tuple<void, float, double, int, uint, sbyte, byte, short, ushort>>;

#define GL_SIZEOF_DECL(SEQ) constexpr usize SizeOf(const GLTypeID type) { switch(type) { GL_SIZEOF_SEQ(SEQ) default: return 0; } }
#define GL_SIZEOF_SEQ(SEQ) Q_CAT(__GL_SIZE_T1__ SEQ, END__)
#define GL_SIZEOF_TYPE(M) case GLTypeID::M: return sizeof(GLTypeOfID<GLTypeID::M>);
#define __GL_SIZE_T1__(X) GL_SIZEOF_TYPE(X) __GL_SIZE_T2__
#define __GL_SIZE_T2__(X) GL_SIZEOF_TYPE(X) __GL_SIZE_T1__
#define __GL_SIZE_T1__END__
#define __GL_SIZE_T2__END__

    GL_SIZEOF_DECL((FLOAT)(DOUBLE)(INT)(UINT)(SBYTE)(BYTE)(SHORT)(USHORT))

#undef GL_SIZEOF_DECL
#undef GL_SIZEOF_SEQ
#undef GL_SIZEOF_TYPE
#undef __GL_SIZE_T1__
#undef __GL_SIZE_T2__
#undef __GL_SIZE_T1__END__
#undef __GL_SIZE_T2__END__
}
