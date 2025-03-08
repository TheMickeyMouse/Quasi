#include "StringWriter.h"

#include <cstdio>

#include "Utils/String.h"

namespace Quasi::Text {
    StringWriter StringWriter::WriteTo(String& string) {
        return { Qfn$(string.AppendStr) };
    }

    StringWriter StringWriter::WriteToFile(std::FILE* file) {
        return { FuncRefs::FromRaw(file, FileWriteCallback) };
    }

    StringWriter StringWriter::WriteToConsole() {
        return WriteToFile(stdout);
    }

    void StringWriter::FileWriteCallback(std::FILE* file, Str str) {
        std::fwrite(str.Data(), 1, str.Length(), file);
    }

    usize StringWriter::Write(Str str) {
        writer(str);
        return str.Length();
    }

    usize StringWriter::Write(char c) {
        writer(Str::Slice(&c, 1));
        return 1;
    }

    usize StringWriter::WriteRepeat(char c, usize n) {
        const u64 repeated8bytes = c * 0x0101'0101'0101'0101;
        const Str rep8 = Str::Slice((const char*)&repeated8bytes, 8);
        for (; n > 8; n -= 8) {
            Write(rep8);
        }
        Write(rep8.Substr(0, n));
        return n;
    }
}
