#pragma once
#include <cstdio>

#include "Utils/Func.h"
#include "Utils/Str.h"

namespace Quasi::Text {
    enum ConsoleColor : u32;
    template <class... Ts> struct FormatResult;

    struct StringWriter {
    private:
        FuncRef<void(Str)> writer;

        StringWriter(FuncRef<void(Str)> customWriter) : writer(customWriter) {}
    public:
        static StringWriter WriteTo(String& string);
        static StringWriter WriteToFile(std::FILE* file);
        static StringWriter WriteToConsole();
        static StringWriter WriteToError();

        static void StringWriteCallback(void* s, Str str);
        static void FileWriteCallback(void* file, Str str);

        usize Write(Str str);
        usize Write(char c);
        usize WriteRepeat(char c, usize n);
        usize SetColor(ConsoleColor col);

        usize operator()(Str str) { return Write(str); }
        usize operator()(char c)  { return Write(c); }
    };
}
