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

    template <class T> GLTypeID GetTypeIDFor()  { return GLTypeID::NONE; }
    template <> inline GLTypeID GetTypeIDFor<float>()  { return GLTypeID::FLOAT; }
    template <> inline GLTypeID GetTypeIDFor<double>() { return GLTypeID::DOUBLE; }
    template <> inline GLTypeID GetTypeIDFor<int>()    { return GLTypeID::INT; }
    template <> inline GLTypeID GetTypeIDFor<uint>()   { return GLTypeID::UINT; }
    template <> inline GLTypeID GetTypeIDFor<sbyte>()  { return GLTypeID::SBYTE; }
    template <> inline GLTypeID GetTypeIDFor<byte>()   { return GLTypeID::BYTE; }
    template <> inline GLTypeID GetTypeIDFor<short>()  { return GLTypeID::SHORT; }
    template <> inline GLTypeID GetTypeIDFor<ushort>() { return GLTypeID::USHORT; }
}
