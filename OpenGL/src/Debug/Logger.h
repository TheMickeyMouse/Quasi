#pragma once
#include <chrono>
#include <source_location>
#include <format>
#include <iostream>

#include "ConsoleColor.h"
#include "Utils/Enum.h"
#include "Utils/Option.h"

namespace Quasi::Debug {
    void DebugBreak();

    enum class Severity {
        OFF,
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL,

        NUM,
        NEVER = NUM
    };

    inline Q_ENUM_TOSTR(Severity, SeverityName,
        (OFF, "OFF")(TRACE, "TRACE")(DEBUG, "DEBUG")(INFO, "INFO")(WARN, "WARN")(ERROR, "ERROR")(CRITICAL, "CRIT"),
        "NULL");
    inline static const EnumMap SEVERITY_COLOR = [] {
        EnumMap<Severity, ConsoleColor> arr;
        arr[Severity::OFF]      = ConsoleColor::RESET;
        arr[Severity::TRACE]    = FgColor(ConsoleColor::GREEN);
        arr[Severity::DEBUG]    = FgColor(ConsoleColor::CYAN);
        arr[Severity::INFO]     = ConsoleColor::RESET;
        arr[Severity::WARN]     = FgColor(ConsoleColor::YELLOW);
        arr[Severity::ERROR]    = FgColor(ConsoleColor::RED);
        arr[Severity::CRITICAL] = Bold(ConsoleColor::FG_RED);
        return arr;
    }();
    inline ConsoleColor SeverityColor(Severity s) { return SEVERITY_COLOR[s]; }
    inline ColoredText SeverityColoredName(Severity s) { return { SeverityColor(s), SeverityName(s) }; }


    using DateTime = std::chrono::time_point<std::chrono::system_clock>;
    using SourceLoc = std::source_location;

    // captures source location
    template <class... Ts>
    struct FmtStr {
        std::format_string<Ts...> fmt;
        SourceLoc loc;
        template <class S> requires std::convertible_to<const S&, std::format_string<Ts...>>
        consteval FmtStr(const S& f, const SourceLoc& l = SourceLoc::current())
        : fmt(f), loc(l) {}

        FmtStr(std::format_string<Ts...> f, const SourceLoc& l = SourceLoc::current())
        : fmt(f), loc(l) {}
        // consteval FmtStr(const char* f, const SourceLoc& l = SourceLoc::current()) : fmt(f), loc(l) {}
    };

    template <class... Ts>
    using ImplicitFmtStr = std::type_identity_t<FmtStr<Ts...>>;

    struct LogEntry {
        String log;
        Severity severity;
        DateTime time;
        SourceLoc fileLoc;
    };


    class Logger {
        Ref<OutStream> logOut;
        ColoredText name = { ConsoleColor::RESET, "LOG" };
        Severity filterLevel = Severity::OFF, breakLevel = Severity::NEVER;
        Vec<LogEntry> logs;

        bool shortenFileNames : 1 = true;
        bool includeFunction : 1 = true;
        bool alwaysFlush : 1 = false;
        bool recordLogs : 1 = false;
        int lPad = 50;

        static Logger InternalLog;
    public:
#ifdef NDEBUG
        static constexpr bool DEBUG = false;
#else
        static constexpr bool DEBUG = true;
#endif
        explicit Logger(OutStream& out = std::cout) : logOut(out) {}

        static bool Overrides(Severity filter, Severity log) { return (int)log >= (int)filter; }
        [[nodiscard]] bool Overrides(Severity s) const { return Overrides(filterLevel, s); }
        void SetFilter(Severity s) { filterLevel = s; }
        void SetBreakLevel(Severity s) { breakLevel = s; }

        void SetName(Str newName) { name.string = newName; }
        void SetNameColor(const ConsoleColor col) { name.color = col; }
        void SetShortenFile(const bool flag) { shortenFileNames = flag; }
        void SetIncludeFunc(const bool flag) { includeFunction = flag; }
        void SetAlwaysFlush(const bool flag) { alwaysFlush = flag; }
        void SetRecordLogs(const bool flag) { recordLogs = flag; }
        void SetLocPad(const int pad) { lPad = pad; }

        static DateTime Now();

        String FmtLog(const LogEntry& log);
        String FmtLog(Str log, Severity severity, DateTime time, const SourceLoc& fileLoc);
        [[nodiscard]] Str FmtFile(Str fullname) const;
        [[nodiscard]] String FmtSourceLoc(const SourceLoc& loc) const;
        void LogNoOut  (Severity sv, Str s, const SourceLoc& loc = SourceLoc::current());
        void ConsoleLog(Severity sv, Str s, const SourceLoc& loc = SourceLoc::current());
        void Log       (Severity sv, Str s, const SourceLoc& loc = SourceLoc::current());

        void Assert(bool assert, Str msg, const SourceLoc& loc = SourceLoc::current());

        void Write(OutStream& out, Severity filter = Severity::NEVER);

        template <class ...Ts> void LogFmt(Severity s, ImplicitFmtStr<Ts...> fmt, Ts&&... args) {
            this->Log(s, std::format(fmt.fmt, std::forward<Ts>(args)...), fmt.loc);
        }

        template <class ...Ts> void AssertFmt(bool assert, ImplicitFmtStr<Ts...> fmt, Ts&&... args) {
            this->Assert(assert, std::format(fmt.fmt, std::forward<Ts>(args)...), fmt.loc);
        }

        template <class T>
        void AssertEq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) {
            this->AssertFmt(val == cmp,
                { "Left operand {0}({1}) is not equal to Right Operand {0}({2})", loc },
                TypeName<T>(), val, cmp
            );
        }

        template <class T>
        void AssertNeq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) {
            this->AssertFmt(val != cmp,
                { "Left operand {0}({1}) is equal to Right Operand {0}({2})", loc },
                TypeName<T>(), val, cmp
            );
        }

        template <class ...Ts> void Trace   (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::TRACE,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Debug   (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::DEBUG,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Info    (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::INFO,     fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Warn    (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::WARN,     fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Error   (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::ERROR,    fmt, std::forward<Ts>(args)...); }
        template <class ...Ts> void Critical(ImplicitFmtStr<Ts...> fmt, Ts&&... args) { this->LogFmt(Severity::CRITICAL, fmt, std::forward<Ts>(args)...); }

        void Flush();

        static Logger& GetInternalLog();
    };

    inline void SetFilter(Severity s) { Logger::GetInternalLog().SetFilter(s); }
    inline void SetBreakLevel(Severity s) { Logger::GetInternalLog().SetBreakLevel(s); }

    inline void SetName(std::string newName) { Logger::GetInternalLog().SetName(std::move(newName)); }
    inline void SetNameColor(const ConsoleColor col) { Logger::GetInternalLog().SetNameColor(col); }
    inline void SetShortenFile(const bool flag) { Logger::GetInternalLog().SetShortenFile(flag); }
    inline void SetIncludeFunc(const bool flag) { Logger::GetInternalLog().SetIncludeFunc(flag); }
    inline void SetLocPad(const int pad) { Logger::GetInternalLog().SetLocPad(pad); }

    inline void Log(Severity sv, Str s, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().Log(sv, s, loc); }

    inline void Assert(bool assert, Str msg, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().Assert(assert, msg, loc); }

    inline void Write(OutStream& out, Severity filter = Severity::NEVER) { Logger::GetInternalLog().Write(out, filter); }

    template <class ...Ts> void LogFmt(Severity s, ImplicitFmtStr<Ts...> fmt, Ts&&... args) {
        Logger::GetInternalLog().LogFmt(s, fmt, std::forward<Ts>(args)...);
    }

    template <class ...Ts> void AssertFmt(bool assert, ImplicitFmtStr<Ts...> fmt, Ts&&... args) {
        Logger::GetInternalLog().Assert(assert, std::format(fmt.fmt, std::forward<Ts>(args)...));
    }

    template <class T>
    void AssertEq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertEq(val, cmp, loc); }

    template <class T>
    void AssertNeq(const T& val, const T& cmp, const SourceLoc& loc = SourceLoc::current()) { Logger::GetInternalLog().AssertNeq(val, cmp, loc); }

    template <class ...Ts> void Trace   (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Trace   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Debug   (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Debug   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Info    (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Info    (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Warn    (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Warn    (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Error   (ImplicitFmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Error   (fmt, std::forward<Ts>(args)...); }
    template <class ...Ts> void Critical(ImplicitFmtStr<Ts...> fmt, Ts&&... args) { Logger::GetInternalLog().Critical(fmt, std::forward<Ts>(args)...); }

    void Flush();
}

namespace Quasi {
    template <class T> T& Option<T>::Assert() { return Q_GETTER_MUT(Assert); }
    template <class T> const T& Option<T>::Assert() const {
        Debug::AssertFmt(HasValue(), "Option<{}> doesn't have a value", Text::TypeName<T>());
        return value;
    }

    template <class T> template <class Asrt> T& Option<T>::Assert(Asrt&& assertfn) { return Q_GETTER_MUT(Assert, assertfn); }
    template <class T> template <class Asrt> const T& Option<T>::Assert(Asrt&& assertfn) const {
        if (IsNull()) assertfn();
        return value;
    }

    template <class T> T& RefImpl<T>::Assert() { return Q_GETTER_MUT(Assert); }
    template <class T> const T& RefImpl<T>::Assert() const {
        Debug::AssertFmt(HasValue(), "Ref<{}> doesn't have a value", Text::TypeName<T>());
        return *obj;
    }

    template <class T> template <class Asrt> T& RefImpl<T>::Assert(Asrt&& assertfn) { return Q_GETTER_MUT(Assert, assertfn); }
    template <class T> template <class Asrt> const T& RefImpl<T>::Assert(Asrt&& assertfn) const {
        if (IsNull()) assertfn();
        return *obj;
    }
}

template <>
struct std::formatter<Quasi::Debug::Severity> : formatter<std::string> {
    auto format(Quasi::Debug::Severity s, std::format_context& ctx) const {
        return formatter<std::string>::format(std::format("{}", SeverityColoredName(s)), ctx);
    }
};