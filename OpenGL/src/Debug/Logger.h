// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier
// ReSharper disable CppClangTidyBugproneReservedIdentifier
#pragma once
#include <chrono>
#include <source_location>
#include <format>
#include <iostream>
#include <vector>
#include <array>

#include "ConsoleColor.h"
#include <core.h>
#include "freetype/internal/ftobjs.h"
#include "stdu/macros.h"
#include "stdu/ref.h"
#include "stdu/types.h"
#include <cassert>

namespace Debug {
    OPENGL_API void DebugBreak();

    enum class Severity {
        OFF,
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL,

        SEVERITY_NUM,
        NEVER = SEVERITY_NUM
    };

    inline STDU_ENUM_TOSTR(Severity, SeverityName,
        (OFF, "OFF")(TRACE, "TRACE")(DEBUG, "DEBUG")(INFO, "INFO")(WARN, "WARN")(ERROR, "ERROR")(CRITICAL, "CRIT"),
        "NULL");
    inline constexpr std::array<ConsoleColor, (int)Severity::SEVERITY_NUM> SEVERITY_COLOR = []{
        std::array<ConsoleColor, (int)Severity::SEVERITY_NUM> arr;
        arr[(int)Severity::OFF]      = ConsoleColor::RESET;
        arr[(int)Severity::TRACE]    = ConsoleColor::FG_GREEN;
        arr[(int)Severity::DEBUG]    = ConsoleColor::FG_CYAN;
        arr[(int)Severity::INFO]     = ConsoleColor::RESET;
        arr[(int)Severity::WARN]     = ConsoleColor::FG_YELLOW;
        arr[(int)Severity::ERROR]    = ConsoleColor::FG_RED;
        arr[(int)Severity::CRITICAL] = Bold(ConsoleColor::FG_RED);
        return arr;
    }();
    inline ConsoleColor SeverityColor(Severity s) { return SEVERITY_COLOR[(int)s]; }
    inline ColoredText SeverityColoredName(Severity s) { return { SeverityColor(s), SeverityName(s) }; }


    using DateTime = std::chrono::time_point<std::chrono::system_clock>;
    using SourceLoc = std::source_location;

    // captures source location
    template <class... Ts>
    struct FmtStr {
        std::format_string<Ts...> fmt;
        SourceLoc loc;
        FmtStr(std::format_string<Ts...> f, const SourceLoc& l = SourceLoc::current()) : fmt(f), loc(l) {}
    };

    struct LogEntry {
        std::string log;
        Severity severity;
        DateTime time;
        std::source_location fileLoc;
    };


    class Logger {
        stdu::ref<std::ostream> logOut;
        ColoredText name = { ConsoleColor::RESET, "LOG" };
        Severity filterLevel = Severity::OFF, breakLevel = Severity::NEVER;
        std::vector<LogEntry> logs;

        bool shortenFileNames : 1 = true;
        bool includeFunction : 1 = true;
        bool alwaysFlush : 1 = false;
        int lPad = 50;

        OPENGL_API static Logger InternalLog;
    public:
#ifdef NDEBUG
        static constexpr bool DEBUG = false;
#else
        static constexpr bool DEBUG = true;
#endif
        explicit Logger(std::ostream& out = std::clog) : logOut(out) {}

        static bool Overrides(Severity filter, Severity log) { return (int)log >= (int)filter; }
        [[nodiscard]] bool Overrides(Severity s) const { return Overrides(filterLevel, s); }
        void SetFilter(Severity s) { filterLevel = s; }
        void SetBreakLevel(Severity s) { breakLevel = s; }

        void SetName(const std::string& newName) { name.string = newName; }
        void SetNameColor(const ConsoleColor col) { name.color = col; }
        void SetShortenFile(const bool flag) { shortenFileNames = flag; }
        void SetIncludeFunc(const bool flag) { includeFunction = flag; }
        void SetAlwaysFlush(const bool flag) { alwaysFlush = flag; }
        void SetLocPad(const int pad) { lPad = pad; }

        OPENGL_API static DateTime Now();

        OPENGL_API std::string FmtLog(const LogEntry& log);
        OPENGL_API std::string_view FmtFile(std::string_view fullname) const;
        OPENGL_API std::string FmtSourceLoc(const SourceLoc& loc) const;
        OPENGL_API void LogNoOut  (Severity sv, const std::string& s, const SourceLoc& loc = SourceLoc::current());
        OPENGL_API void ConsoleLog(Severity sv, const std::string& s, const SourceLoc& loc = SourceLoc::current());
        OPENGL_API void Log       (Severity sv, const std::string& s, const SourceLoc& loc = SourceLoc::current());

        OPENGL_API void Assert(bool assert, const std::string& msg, const SourceLoc& loc = SourceLoc::current());

        OPENGL_API void Write(std::ostream& out, Severity filter = Severity::SEVERITY_NUM);

        template <class ...Ts> void LogFmt(Severity s, FmtStr<Ts...> fmt, Ts&&... args) {
            this->Log(s, std::format(fmt.fmt, std::forward<Ts>(args)...), fmt.loc);
        }

        template <class ...Ts> void AssertFmt(bool assert, FmtStr<Ts...> fmt, Ts&&... args) {
            this->Assert(assert, std::format(fmt.fmt, std::forward<Ts>(args)...), fmt.loc);
        }

        template <class T>
        void AssertEq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) {
            this->AssertFmt(val == cmp,
                { "Left operand {0}({1}) is not equal to Right Operand {0}({2})", loc },
                stdu::nameof<T>(), val, cmp
            );
        }

        template <class T>
        void AssertNeq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) {
            this->AssertFmt(val != cmp,
                { "Left operand {0}({1}) is equal to Right Operand {0}({2})", loc },
                stdu::nameof<T>(), val, cmp
            );
        }

        template <class ...Ts> void Trace   (FmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::TRACE,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Debug   (FmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::DEBUG,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Info    (FmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::INFO,     fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Warn    (FmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::WARN,     fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Error   (FmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::ERROR,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Critical(FmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::CRITICAL, fmt, std::forward<Ts>(args)...); }

        OPENGL_API void Flush();

        OPENGL_API static Logger& GetInternalLog();
    };

    inline void SetFilter(Severity s) { Logger::GetInternalLog().SetFilter(s); }
    inline void SetBreakLevel(Severity s) { Logger::GetInternalLog().SetBreakLevel(s); }

    inline void SetName(const std::string& newName) { Logger::GetInternalLog().SetName(newName); }
    inline void SetNameColor(const ConsoleColor col) { Logger::GetInternalLog().SetNameColor(col); }
    inline void SetShortenFile(const bool flag) { Logger::GetInternalLog().SetShortenFile(flag); }
    inline void SetIncludeFunc(const bool flag) { Logger::GetInternalLog().SetIncludeFunc(flag); }
    inline void SetLocPad(const int pad) { Logger::GetInternalLog().SetLocPad(pad); }

    inline void Log(Severity sv, const std::string& s, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().Log(sv, s, loc); }

    inline void Assert(bool assert, const std::string& msg, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().Assert(assert, msg, loc); }

    inline void Write(std::ostream& out, Severity filter = Severity::SEVERITY_NUM) { Logger::GetInternalLog().Write(out, filter); }

    template <class ...Ts> void LogFmt(Severity s, FmtStr<Ts...> fmt, Ts&&... args) {
        Logger::GetInternalLog().LogFmt(s, fmt, std::forward<Ts>(args)...);
    }

    template <class ...Ts> void AssertFmt(bool assert, FmtStr<Ts...> fmt, Ts&&... args) {
        Logger::GetInternalLog().Assert(assert, fmt, std::forward<Ts>(args)...);
    }

    template <class T>
    void AssertEq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertEq(val, cmp, loc); }

    template <class T>
    void AssertNeq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertNeq(val, cmp, loc); }

    template <class ...Ts> void Trace   (FmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Trace   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Debug   (FmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Debug   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Info    (FmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Info    (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Warn    (FmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Warn    (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Error   (FmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Error   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Critical(FmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Critical(fmt, std::forward<Ts>(args)...); }

    OPENGL_API void Flush();
}

template <>
struct std::formatter<Debug::Severity> : formatter<std::string> {
    auto format(Debug::Severity s, std::format_context& ctx) const {
        return formatter<std::string>::format(std::format("{}", SeverityColoredName(s)), ctx);
    }
};