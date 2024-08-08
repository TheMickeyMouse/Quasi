#include "Logger.h"

#include <chrono>
#include "internal_debug_break.h"
#include "Utils/Text.h"

namespace Quasi::Debug {
    Logger Logger::InternalLog = [] {
        Logger log { std::cout };
        log.SetName("Internal");

        log.SetBreakLevel(Severity::ERROR);
        log.SetShortenFile(true);
        log.SetIncludeFunc(false);
        log.SetAlwaysFlush(DEBUG);
        log.SetLocPad(0);
        return log;
    }();

    Logger& Logger::GetInternalLog() {
        return InternalLog;
    }

    void DebugBreak() {
        if constexpr (Logger::DEBUG)
            debug_break();
    }

    DateTime Logger::Now() {
        return std::chrono::system_clock::now();
    }

    String Logger::FmtLog(const LogEntry& log) const {
        return FmtLog(log.log, log.severity, log.time, log.fileLoc);
    }

    String Logger::FmtLog(Str log, Severity severity, DateTime time, const SourceLoc& fileLoc) const {
        const ConsoleColor scol = severity->color;
        return Text::Format(
            "{}[{:%y-%M-%d %H:%m:%s.%u}]{} {}> {}{:<8} {:<{}} {}{}\n",
            scol, time, RESET, name,
            scol, Text::Format("[{}]:", severity->name),
            FmtSourceLoc(fileLoc), lPad,
            log,
            RESET
        );
    }

    Str Logger::FmtFile(Str fullname) const {
        return shortenFileNames ? std::get<1>(Text::SplitDirectory(fullname)) : fullname;
    }

    String Logger::FmtSourceLoc(const SourceLoc& loc) const {
        return includeFunction ?
            Text::Format("{}:{}:{} in {}:", FmtFile(loc.file_name()), loc.line(), loc.column(), loc.function_name()) :
            Text::Format("{}:{}:{}:", FmtFile(loc.file_name()), loc.line(), loc.column());
    }

    void Logger::LogNoOut(const Severity sv, const Str s, const SourceLoc& loc) {
        if (recordLogs)
            logs.emplace_back(LogEntry { String { s }, sv, Now(), loc });
    }

    void Logger::ConsoleLog(const Severity sv, const Str s, const SourceLoc& loc) {
        *logOut << FmtLog(s, sv, Now(), loc );
        if (alwaysFlush) Flush();
    }

    void Logger::Log(const Severity sv, const Str s, const SourceLoc& loc) {
        LogNoOut(sv, s, loc);
        ConsoleLog(sv, s, loc);
        if (Overrides(breakLevel, sv)) {
            Flush();
            DebugBreak();
        }
    }

    void Logger::Assert(const bool assert, Str msg, const SourceLoc& loc) {
        if (!assert) {
            Log(Severity::ERROR, Text::Format("Assertion failed: {}", msg), loc);
            Flush();
            DebugBreak();
        }
    }

    void Logger::Write(OutStream& out, Severity filter) {
        filter = filter == Severity::NONE ? filterLevel : filter;
        for (const LogEntry& entry : logs) {
            if (Overrides(filter, entry.severity))
                out << FmtLog(entry);
        }
    }

    void Logger::Flush() {
        std::flush(*logOut);
    }
}
