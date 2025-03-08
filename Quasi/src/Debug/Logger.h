#pragma once
#include <chrono>
#include <iostream>
#include <source_location>

#include "Timer.h"
#include "Utils/Text.h"
#include "Utils/Text/Num.h"
#include "Utils/Enum.h"

namespace Quasi::Debug {
    void DebugBreak();

    struct SeverityData {
        Str name;
        Text::ConsoleColor color;

        Text::ColoredStr ColoredName() const { return { color, name }; }

        QDefineEnum$(Severity,
            (OFF,      ("OFF",      Text::RESET))
            (TRACE,    ("TRACE",    FgColor(Text::GREEN)))
            (DEBUG,    ("DEBUG",    FgColor(Text::CYAN)))
            (INFO,     ("INFO",     Text::RESET))
            (WARN,     ("WARN",     FgColor(Text::YELLOW)))
            (ERROR,    ("ERROR",    FgColor(Text::RED)))
            (CRITICAL, ("CRITICAL", Bold(Text::FG_RED))),
        COMPARABLE, ("NEVER", Text::GRAY))
    };

    using SourceLoc = std::source_location;

    // captures source location
    struct FmtStr {
        Str fmt;
        SourceLoc loc;
        FmtStr(const char* f, const SourceLoc& l = SourceLoc::current()) : fmt(f), loc(l) {}
        FmtStr(Str f, const SourceLoc& l = SourceLoc::current()) : fmt(f), loc(l) {}
        // consteval FmtStr(const char* f, const SourceLoc& l = SourceLoc::current()) : fmt(f), loc(l) {}
    };

    struct LogEntry {
        String log;
        Severity severity;
        DateTime time;
        SourceLoc fileLoc;
    };

    class Logger {
        Text::StringWriter logOut;
        Text::ColoredStr name = { Text::RESET, "LOG" };
        Severity filterLevel = Severity::OFF, breakLevel = Severity::ERROR;
        Vec<LogEntry> logs;

        bool shortenFileNames : 1 = true;
        bool includeFunction : 1 = true;
        bool alwaysFlush : 1 = false;
        bool recordLogs : 1 = false;
        u32 lPad = 50;

    public:
        static Logger InternalLog;
#ifdef NDEBUG
        static constexpr bool DEBUG = false;
#else
        static constexpr bool DEBUG = true;
#endif
        explicit Logger(Text::StringWriter out = Text::StringWriter::WriteToConsole()) : logOut(out) {}

        static bool Overrides(Severity filter, Severity log) { return log >= filter; }
        bool Overrides(Severity s) const { return Overrides(filterLevel, s); }
        void SetFilter(Severity s) { filterLevel = s; }
        void SetBreakLevel(Severity s) { breakLevel = s; }

        void SetName(Str newName) { name.text = newName; }
        void SetNameColor(const Text::ConsoleColor col) { name.color = col; }
        void SetShortenFile(const bool flag) { shortenFileNames = flag; }
        void SetIncludeFunc(const bool flag) { includeFunction = flag; }
        void SetAlwaysFlush(const bool flag) { alwaysFlush = flag; }
        void SetRecordLogs(const bool flag) { recordLogs = flag; }
        void SetLocPad(const u32 pad) { lPad = pad; }

        String FmtLog(const LogEntry& log) const;
        String FmtLog(Str log, Severity severity, DateTime time, const SourceLoc& fileLoc) const;
        Str FmtFile(Str fullname) const;
        String FmtSourceLoc(const SourceLoc& loc) const;
        void LogNoOut  (Severity sv, Str s, const SourceLoc& loc = SourceLoc::current());
        void ConsoleLog(Severity sv, Str s, const SourceLoc& loc = SourceLoc::current());
        void Log       (Severity sv, Str s, const SourceLoc& loc = SourceLoc::current());

        void AssertMsg(bool assert, Str msg, const SourceLoc& loc = SourceLoc::current());

        void WriteAllLogs(Text::StringWriter out, Severity filter = Severity::NONE);

        template <class ...Ts> void LogFmt(Severity s, const FmtStr& fmt, Ts&&... args) {
            this->Log(s, Text::Format(fmt.fmt, std::forward<Ts>(args)...), fmt.loc);
        }

        template <class ...Ts> void Assert(bool assert, const FmtStr& fmt, Ts&&... args) {
            if (assert) return;
            this->AssertMsg(true, Text::Format(fmt.fmt, std::forward<Ts>(args)...), fmt.loc);
        }

        template <class T>
        void AssertEq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) {
            this->Assert(val == cmp,
                { "Left operand {0}({1}) is not equal to Right Operand {0}({2})", loc },
                Text::TypeName<T>(), val, cmp
            );
        }

        template <class T>
        void AssertNeq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) {
            this->Assert(val != cmp,
                { "Left operand {0}({1}) is equal to Right Operand {0}({2})", loc },
                Text::TypeName<T>(), val, cmp
            );
        }

        template <class ...Ts> void Trace   (const FmtStr& fmt, Ts&&... args) { this->LogFmt(Severity::TRACE,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Debug   (const FmtStr& fmt, Ts&&... args) { this->LogFmt(Severity::DEBUG,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Info    (const FmtStr& fmt, Ts&&... args) { this->LogFmt(Severity::INFO,     fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Warn    (const FmtStr& fmt, Ts&&... args) { this->LogFmt(Severity::WARN,     fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Error   (const FmtStr& fmt, Ts&&... args) { this->LogFmt(Severity::ERROR,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Critical(const FmtStr& fmt, Ts&&... args) { this->LogFmt(Severity::CRITICAL, fmt, std::forward<Ts>(args)...); }

        void NoOp() const {}

        static Logger& GetInternalLog();
    };

    inline void SetFilter(Severity s) { Logger::GetInternalLog().SetFilter(s); }
    inline void SetBreakLevel(Severity s) { Logger::GetInternalLog().SetBreakLevel(s); }

    inline void SetName(Str newName) { Logger::GetInternalLog().SetName(newName); }
    inline void SetNameColor(const Text::ConsoleColor col) { Logger::GetInternalLog().SetNameColor(col); }
    inline void SetShortenFile(const bool flag) { Logger::GetInternalLog().SetShortenFile(flag); }
    inline void SetIncludeFunc(const bool flag) { Logger::GetInternalLog().SetIncludeFunc(flag); }
    inline void SetLocPad(const int pad) { Logger::GetInternalLog().SetLocPad(pad); }

    inline void Log(Severity sv, Str s, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().Log(sv, s, loc); }

    inline void AssertMsg(bool assert, Str msg, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertMsg(assert, msg, loc); }

    inline void Write(Text::StringWriter out, Severity filter = Severity::NONE) { Logger::GetInternalLog().WriteAllLogs(out, filter); }

    template <class ...Ts> void LogFmt(Severity s, const FmtStr& fmt, Ts&&... args) {
        Logger::GetInternalLog().LogFmt(s, fmt, std::forward<Ts>(args)...);
    }

    template <class ...Ts> void Assert(bool assert, const FmtStr& fmt, Ts&&... args) {
        Logger::GetInternalLog().Assert(assert, fmt.fmt, std::forward<Ts>(args)...);
    }

    template <class T>
    void AssertEq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertEq(val, cmp, loc); }

    template <class T>
    void AssertNeq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertNeq(val, cmp, loc); }

    template <class ...Ts> void Trace   (const FmtStr& fmt, Ts&&... args) { Logger::GetInternalLog().Trace   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Debug   (const FmtStr& fmt, Ts&&... args) { Logger::GetInternalLog().Debug   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Info    (const FmtStr& fmt, Ts&&... args) { Logger::GetInternalLog().Info    (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Warn    (const FmtStr& fmt, Ts&&... args) { Logger::GetInternalLog().Warn    (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Error   (const FmtStr& fmt, Ts&&... args) { Logger::GetInternalLog().Error   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Critical(const FmtStr& fmt, Ts&&... args) { Logger::GetInternalLog().Critical(fmt, std::forward<Ts>(args)...); }
    inline void NoOp() {}

#ifdef NDEBUG
    #define QTrace$     NoOp() Q_EAT
    #define QDebug$     NoOp() Q_EAT
    #define QInfo$      NoOp() Q_EAT
    #define QWarn$      NoOp() Q_EAT
    #define QError$     NoOp() Q_EAT
    #define QCritical$  NoOp() Q_EAT
    #define QAssert$    NoOp() Q_EAT
    #define QAssertEq$  NoOp() Q_EAT
    #define QAssertNeq$ NoOp() Q_EAT
    #define QAssertMsg$ NoOp() Q_EAT
#else
    #define QTrace$    Trace
    #define QDebug$    Debug
    #define QInfo$     Info
    #define QWarn$     Warn
    #define QError$    Error
    #define QCritical$ Critical
    #define QAssert$    Assert
    #define QAssertEq$  AssertEq
    #define QAssertNeq$ AssertNeq
    #define QAssertMsg$ AssertMsg
#endif

    void Flush();
}

namespace Quasi {
    template <class T, class Super> T& INullable<T, Super>::Assert() { return QGetterMut$(Assert); }
    template <class T, class Super> const T& INullable<T, Super>::Assert() const {
        Debug::Assert(HasValue(), "{} doesn't have a value", Text::TypeName<Super>());
        return Unwrap();
    }

    template <class T, class Super> T& INullable<T, Super>::Assert(Str msg) { return QGetterMut$(Assert, msg); }
    template <class T, class Super> const T& INullable<T, Super>::Assert(Str msg) const {
        Debug::Assert(HasValue(), msg);
        return Unwrap();
    }

    template <class T, class Super> T& INullable<T, Super>::Assert(auto&& assertfn) { return QGetterMut$(Assert, assertfn); }
    template <class T, class Super> const T& INullable<T, Super>::Assert(auto&& assertfn) const {
        if (IsNull()) assertfn();
        return Unwrap();
    }
}

namespace Quasi::Text {
    template <>
    struct Formatter<Debug::DateTime> {
        using FormatOptions = Str;
        static FormatOptions ConfigureOptions(Str x) { return x; }
        static usize FormatTo(StringWriter out, const Debug::DateTime& time, Str fmt);
    };
}