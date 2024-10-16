#pragma once
#include "Utils/Enum.h"
#include "Utils/Macros.h"

namespace Quasi::Graphics {
    struct GLTypeIDData {
        u32 glID, typeSize;

        QDefineEnum$(GLTypeID,
            (FLOAT,  (0x1406, sizeof(float)))
            (DOUBLE, (0x140A, sizeof(double)))
            (INT,    (0x1404, sizeof(int)))
            (UINT,   (0x1405, sizeof(uint)))
            (SBYTE,  (0x1400, sizeof(sbyte)))
            (BYTE,   (0x1401, sizeof(byte)))
            (SHORT,  (0x1402, sizeof(short)))
            (USHORT, (0x1403, sizeof(ushort))),
        NULLABLE, (0, 0))
    };

    template <class T> GLTypeID GLGetTypeID()  { return GLTypeID::NONE; }
    template <> inline GLTypeID GLGetTypeID<float>()  { return GLTypeID::FLOAT; }
    template <> inline GLTypeID GLGetTypeID<double>() { return GLTypeID::DOUBLE; }
    template <> inline GLTypeID GLGetTypeID<int>()    { return GLTypeID::INT; }
    template <> inline GLTypeID GLGetTypeID<uint>()   { return GLTypeID::UINT; }
    template <> inline GLTypeID GLGetTypeID<sbyte>()  { return GLTypeID::SBYTE; }
    template <> inline GLTypeID GLGetTypeID<byte>()   { return GLTypeID::BYTE; }
    template <> inline GLTypeID GLGetTypeID<short>()  { return GLTypeID::SHORT; }
    template <> inline GLTypeID GLGetTypeID<ushort>() { return GLTypeID::USHORT; }
}
