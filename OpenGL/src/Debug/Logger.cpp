#include "Logger.h"

#include "internal_debug_break.h"
#include "stdu/io.h"

namespace Debug {
    OPENGL_API Logger Logger::InternalLog = [] {
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
        else
            throw std::runtime_error("assertion failed.");
    }

    DateTime Logger::Now() {
        return std::chrono::system_clock::now();
    }

    std::string Logger::FmtLog(const LogEntry& log) {
        ConsoleColor scol = SeverityColor(log.severity);
        return std::format(
            "{}[{:%Y-%m-%d %T}]{} {}> {}{:<8} {:<{}} {}{}\n",
            scol, std::chrono::current_zone()->to_local(log.time), ConsoleColor::RESET, name,
            scol, std::format("[{}]:", SeverityName(log.severity)),
            FmtSourceLoc(log.fileLoc), lPad,
            log.log,
            ConsoleColor::RESET
        );
    }

    std::string_view Logger::FmtFile(std::string_view fullname) const {
        return shortenFileNames ? std::get<1>(stdu::getfolder(fullname)) : fullname;
    }

    std::string Logger::FmtSourceLoc(const SourceLoc& loc) const {
        return includeFunction ?
            std::format("{}:{}:{} in {}:", FmtFile(loc.file_name()), loc.line(), loc.column(), loc.function_name()) :
            std::format("{}:{}:{}:", FmtFile(loc.file_name()), loc.line(), loc.column());
    }

    void Logger::LogNoOut(const Severity sv, const std::string& s, const SourceLoc& loc) {
        logs.emplace_back(s, sv, Now(), loc);
    }

    void Logger::ConsoleLog(const Severity sv, const std::string& s, const SourceLoc& loc) {
        *logOut << FmtLog({ s, sv, Now(), loc });
        if (alwaysFlush) Flush();
    }

    void Logger::Log(const Severity sv, const std::string& s, const SourceLoc& loc) {
        LogNoOut(sv, s, loc);
        ConsoleLog(sv, s, loc);
        if (Overrides(breakLevel, sv)) {
            Flush();
            DebugBreak();
        }
    }

    void Logger::Assert(const bool assert, const std::string& msg, const SourceLoc& loc) {
        if (!assert) {
            Log(Severity::ERROR, std::format("Assertion failed: {}", msg), loc);
            Flush();
            DebugBreak();
        }
    }

    void Logger::Write(std::ostream& out, Severity filter) {
        filter = filter == Severity::SEVERITY_NUM ? filterLevel : filter;
        for (const LogEntry& entry : logs) {
            if (Overrides(filter, entry.severity))
                out << FmtLog(entry);
        }
    }

    void Logger::Flush() {
        std::flush(*logOut);
    }
}
