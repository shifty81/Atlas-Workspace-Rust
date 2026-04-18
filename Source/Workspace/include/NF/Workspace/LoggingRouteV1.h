#pragma once
// NF::Editor — Logging route v1: structured log routing with levels, tags, and sinks
// NB: LogLevel enum is defined in NF/Core/Core.h (Trace/Debug/Info/Warn/Error/Fatal)
#include "NF/Core/Core.h"
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Log Level helpers (using Core LogLevel) ───────────────────────

inline const char* logLevelName(LogLevel l) {
    switch (l) {
        case LogLevel::Trace: return "Trace";
        case LogLevel::Debug: return "Debug";
        case LogLevel::Info:  return "Info";
        case LogLevel::Warn:  return "Warn";
        case LogLevel::Error: return "Error";
        case LogLevel::Fatal: return "Fatal";
    }
    return "Unknown";
}

inline bool logLevelAtLeast(LogLevel msg, LogLevel threshold) {
    return static_cast<uint8_t>(msg) >= static_cast<uint8_t>(threshold);
}

// ── Log Entry ─────────────────────────────────────────────────────

struct LogEntry {
    uint64_t    seq       = 0;
    LogLevel    level     = LogLevel::Info;
    std::string tag;
    std::string message;
    std::string source;    // file:line or module name
    uint64_t    timestampMs = 0;

    [[nodiscard]] bool isValid()   const { return seq != 0 && !message.empty(); }
    [[nodiscard]] bool isError()   const { return level >= LogLevel::Error;     }
    [[nodiscard]] bool isWarning() const { return level >= LogLevel::Warn;   }
};

// ── Log Sink ──────────────────────────────────────────────────────
// Callback-based sink: receives filtered log entries.

using LogSinkCallback = std::function<void(const LogEntry&)>;

struct LogSink {
    uint32_t          id        = 0;
    std::string       name;
    LogLevel          minLevel  = LogLevel::Debug;
    std::string       tagFilter;  // empty = accept all tags
    LogSinkCallback   callback;
    bool              enabled   = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && callback != nullptr; }

    [[nodiscard]] bool accepts(const LogEntry& e) const {
        if (!enabled) return false;
        if (!logLevelAtLeast(e.level, minLevel)) return false;
        if (!tagFilter.empty() && e.tag != tagFilter) return false;
        return true;
    }
};

// ── Log Route ─────────────────────────────────────────────────────
// Maps a source pattern to a set of sink ids.

struct LogRoute {
    uint32_t               id        = 0;
    std::string            name;
    std::string            sourcePattern;  // prefix match; empty = all sources
    std::vector<uint32_t>  sinkIds;
    bool                   passThrough = true; // also pass to default sink

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool matchesSource(const std::string& src) const {
        if (sourcePattern.empty()) return true;
        return src.substr(0, sourcePattern.size()) == sourcePattern;
    }
};

// ── Logging Route V1 ──────────────────────────────────────────────

class LoggingRouteV1 {
public:
    static constexpr size_t MAX_BUFFER   = 4096;
    static constexpr size_t MAX_SINKS    = 32;
    static constexpr size_t MAX_ROUTES   = 64;

    bool addSink(LogSink sink) {
        if (!sink.isValid()) return false;
        if (m_sinks.size() >= MAX_SINKS) return false;
        for (const auto& s : m_sinks) if (s.id == sink.id) return false;
        m_sinks.push_back(std::move(sink));
        return true;
    }

    bool removeSink(uint32_t id) {
        for (auto it = m_sinks.begin(); it != m_sinks.end(); ++it) {
            if (it->id == id) { m_sinks.erase(it); return true; }
        }
        return false;
    }

    bool addRoute(const LogRoute& route) {
        if (!route.isValid()) return false;
        if (m_routes.size() >= MAX_ROUTES) return false;
        for (const auto& r : m_routes) if (r.id == route.id) return false;
        m_routes.push_back(route);
        return true;
    }

    bool removeRoute(uint32_t id) {
        for (auto it = m_routes.begin(); it != m_routes.end(); ++it) {
            if (it->id == id) { m_routes.erase(it); return true; }
        }
        return false;
    }

    void log(LogLevel level, const std::string& tag,
             const std::string& message, const std::string& source = "") {
        LogEntry entry;
        entry.seq         = ++m_seq;
        entry.level       = level;
        entry.tag         = tag;
        entry.message     = message;
        entry.source      = source;
        entry.timestampMs = m_seq;  // simplified monotonic timestamp

        // Buffer
        m_buffer.push_back(entry);
        if (m_buffer.size() > MAX_BUFFER) m_buffer.pop_front();

        ++m_logCount;

        // Route to sinks
        bool routed = false;
        for (const auto& route : m_routes) {
            if (!route.matchesSource(source)) continue;
            routed = true;
            for (uint32_t sinkId : route.sinkIds) {
                auto* sink = findSink(sinkId);
                if (sink && sink->accepts(entry)) sink->callback(entry);
            }
            if (!route.passThrough) return;
        }

        // Pass-through to default sink if not routed exclusively
        if (!routed || true) {
            for (auto& sink : m_sinks) {
                if (sink.accepts(entry)) sink.callback(entry);
            }
        }
    }

    // Convenience helpers
    void trace(const std::string& tag, const std::string& msg, const std::string& src = "") {
        log(LogLevel::Trace,   tag, msg, src);
    }
    void debug(const std::string& tag, const std::string& msg, const std::string& src = "") {
        log(LogLevel::Debug,   tag, msg, src);
    }
    void info(const std::string& tag, const std::string& msg, const std::string& src = "") {
        log(LogLevel::Info,    tag, msg, src);
    }
    void warn(const std::string& tag, const std::string& msg, const std::string& src = "") {
        log(LogLevel::Warn,    tag, msg, src);
    }
    void error(const std::string& tag, const std::string& msg, const std::string& src = "") {
        log(LogLevel::Error,   tag, msg, src);
    }
    void fatal(const std::string& tag, const std::string& msg, const std::string& src = "") {
        log(LogLevel::Fatal,   tag, msg, src);
    }

    void setSinkEnabled(uint32_t id, bool enabled) {
        auto* s = findSink(id);
        if (s) s->enabled = enabled;
    }

    void setMinLevel(uint32_t sinkId, LogLevel level) {
        auto* s = findSink(sinkId);
        if (s) s->minLevel = level;
    }

    [[nodiscard]] const std::deque<LogEntry>& buffer() const { return m_buffer; }
    [[nodiscard]] size_t bufferSize()  const { return m_buffer.size();  }
    [[nodiscard]] size_t logCount()    const { return m_logCount;       }
    [[nodiscard]] size_t sinkCount()   const { return m_sinks.size();   }
    [[nodiscard]] size_t routeCount()  const { return m_routes.size();  }

    [[nodiscard]] size_t countByLevel(LogLevel level) const {
        size_t n = 0;
        for (const auto& e : m_buffer) if (e.level == level) ++n;
        return n;
    }

    void clearBuffer() { m_buffer.clear(); }

    [[nodiscard]] const LogSink* findSink(uint32_t id) const {
        for (const auto& s : m_sinks) if (s.id == id) return &s;
        return nullptr;
    }

private:
    LogSink* findSink(uint32_t id) {
        for (auto& s : m_sinks) if (s.id == id) return &s;
        return nullptr;
    }

    std::deque<LogEntry>   m_buffer;
    std::vector<LogSink>   m_sinks;
    std::vector<LogRoute>  m_routes;
    uint64_t               m_seq      = 0;
    size_t                 m_logCount = 0;
};

} // namespace NF
