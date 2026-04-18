#pragma once
// NF::Workspace — BuildLogRouter: routes build/compile logs into AtlasAI pipeline.
//
// Phase 4 component that connects LoggingRouteV1 (structured log system) to the
// AtlasAI analysis pipeline. Build errors, warnings, and diagnostics are
// captured as pipeline ChangeEvents and fed into AtlasAIIntegration for
// insight generation.
//
// Usage:
//   LoggingRouteV1 logger;
//   BuildLogRouter router;
//   router.bind(&logger, &aiIntegration);
//   router.install();   // registers a log sink that captures build logs
//   // ... build logs flow through logger → sink → AtlasAI analysis

#include "NF/Workspace/LoggingRouteV1.h"
#include "NF/Workspace/AIIntegration.h"
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace NF {

// ── Build Log Classification ──────────────────────────────────────

enum class BuildLogClass : uint8_t {
    CompileError,
    CompileWarning,
    LinkerError,
    LinkerWarning,
    ConfigError,
    TestFailure,
    BuildSuccess,
    Other,
};

inline const char* buildLogClassName(BuildLogClass c) {
    switch (c) {
        case BuildLogClass::CompileError:    return "CompileError";
        case BuildLogClass::CompileWarning:  return "CompileWarning";
        case BuildLogClass::LinkerError:     return "LinkerError";
        case BuildLogClass::LinkerWarning:   return "LinkerWarning";
        case BuildLogClass::ConfigError:     return "ConfigError";
        case BuildLogClass::TestFailure:     return "TestFailure";
        case BuildLogClass::BuildSuccess:    return "BuildSuccess";
        case BuildLogClass::Other:           return "Other";
    }
    return "Unknown";
}

// ── Build Log Entry ───────────────────────────────────────────────

struct BuildLogEntry {
    uint64_t        seq         = 0;
    BuildLogClass   classification = BuildLogClass::Other;
    LogLevel        level       = LogLevel::Info;
    std::string     tag;
    std::string     message;
    std::string     source;
    std::string     filePath;      // extracted file path (if any)
    int             lineNumber  = 0; // extracted line number (if any)

    [[nodiscard]] bool isError()   const { return level >= LogLevel::Error; }
    [[nodiscard]] bool isWarning() const { return level >= LogLevel::Warn && level < LogLevel::Error; }
};

// ── Build Log Router Config ───────────────────────────────────────

struct BuildLogRouterConfig {
    LogLevel    captureMinLevel = LogLevel::Warn;    // only capture warn+
    std::string buildTag        = "build";           // log tag filter
    uint32_t    sinkId          = 900;               // log sink id
    std::string sinkName        = "BuildLogRouter";  // log sink name
    bool        routeToAI       = true;              // feed into AtlasAI
    size_t      maxEntries      = 512;               // ring buffer size
};

// ── Build Log Router ──────────────────────────────────────────────

class BuildLogRouter {
public:
    void bind(LoggingRouteV1* logger, AtlasAIIntegration* ai = nullptr) {
        m_logger = logger;
        m_ai     = ai;
    }

    void setConfig(const BuildLogRouterConfig& cfg) { m_config = cfg; }
    [[nodiscard]] const BuildLogRouterConfig& config() const { return m_config; }

    // Install a log sink that captures build-related logs.
    bool install() {
        if (!m_logger) return false;
        if (m_installed) return false;

        LogSink sink;
        sink.id       = m_config.sinkId;
        sink.name     = m_config.sinkName;
        sink.minLevel = m_config.captureMinLevel;
        sink.tagFilter = m_config.buildTag;
        sink.callback = [this](const LogEntry& entry) {
            onLogEntry(entry);
        };

        if (!m_logger->addSink(std::move(sink))) return false;
        m_installed = true;
        return true;
    }

    // Uninstall the log sink.
    bool uninstall() {
        if (!m_logger || !m_installed) return false;
        m_logger->removeSink(m_config.sinkId);
        m_installed = false;
        return true;
    }

    // Classify a log message based on content heuristics.
    static BuildLogClass classify(const LogEntry& entry) {
        const auto& msg = entry.message;
        if (entry.level >= LogLevel::Error) {
            if (msg.find("link") != std::string::npos ||
                msg.find("undefined reference") != std::string::npos ||
                msg.find("unresolved") != std::string::npos)
                return BuildLogClass::LinkerError;
            if (msg.find("test") != std::string::npos ||
                msg.find("FAILED") != std::string::npos ||
                msg.find("assertion") != std::string::npos)
                return BuildLogClass::TestFailure;
            if (msg.find("cmake") != std::string::npos ||
                msg.find("config") != std::string::npos)
                return BuildLogClass::ConfigError;
            return BuildLogClass::CompileError;
        }
        if (entry.level >= LogLevel::Warn) {
            if (msg.find("link") != std::string::npos)
                return BuildLogClass::LinkerWarning;
            return BuildLogClass::CompileWarning;
        }
        if (msg.find("succeeded") != std::string::npos ||
            msg.find("Build complete") != std::string::npos ||
            msg.find("passed") != std::string::npos)
            return BuildLogClass::BuildSuccess;
        return BuildLogClass::Other;
    }

    // Extract file path from a typical compiler message (e.g. "foo.cpp:42: error: ...")
    static std::string extractFilePath(const std::string& msg) {
        // Look for pattern: path:line: or path(line)
        auto colonPos = msg.find(':');
        if (colonPos != std::string::npos && colonPos > 0) {
            auto secondColon = msg.find(':', colonPos + 1);
            if (secondColon != std::string::npos) {
                std::string candidate = msg.substr(0, colonPos);
                // Validate it looks like a file path
                if (candidate.find('.') != std::string::npos &&
                    candidate.find(' ') == std::string::npos) {
                    return candidate;
                }
            }
        }
        return {};
    }

    // Extract line number from a typical compiler message
    static int extractLineNumber(const std::string& msg) {
        auto colonPos = msg.find(':');
        if (colonPos == std::string::npos) return 0;
        auto secondColon = msg.find(':', colonPos + 1);
        if (secondColon == std::string::npos) return 0;
        std::string numStr = msg.substr(colonPos + 1, secondColon - colonPos - 1);
        try { return std::stoi(numStr); } catch (...) { return 0; }
    }

    // ── Accessors ─────────────────────────────────────────────────

    [[nodiscard]] bool        isInstalled()        const { return m_installed; }
    [[nodiscard]] size_t      capturedCount()      const { return m_capturedCount; }
    [[nodiscard]] size_t      errorCount()         const { return m_errorCount; }
    [[nodiscard]] size_t      warningCount()       const { return m_warningCount; }
    [[nodiscard]] size_t      aiRoutedCount()      const { return m_aiRoutedCount; }
    [[nodiscard]] const std::deque<BuildLogEntry>& entries() const { return m_entries; }

    [[nodiscard]] std::vector<BuildLogEntry> errors() const {
        std::vector<BuildLogEntry> result;
        for (const auto& e : m_entries) if (e.isError()) result.push_back(e);
        return result;
    }

    [[nodiscard]] std::vector<BuildLogEntry> warnings() const {
        std::vector<BuildLogEntry> result;
        for (const auto& e : m_entries) if (e.isWarning()) result.push_back(e);
        return result;
    }

    void clearEntries() {
        m_entries.clear();
        m_capturedCount = 0;
        m_errorCount = 0;
        m_warningCount = 0;
    }

private:
    void onLogEntry(const LogEntry& entry) {
        BuildLogEntry buildEntry;
        buildEntry.seq            = entry.seq;
        buildEntry.classification = classify(entry);
        buildEntry.level          = entry.level;
        buildEntry.tag            = entry.tag;
        buildEntry.message        = entry.message;
        buildEntry.source         = entry.source;
        buildEntry.filePath       = extractFilePath(entry.message);
        buildEntry.lineNumber     = extractLineNumber(entry.message);

        // Ring buffer
        if (m_entries.size() >= m_config.maxEntries) {
            m_entries.pop_front();
        }
        m_entries.push_back(buildEntry);
        ++m_capturedCount;

        if (buildEntry.isError()) ++m_errorCount;
        else if (buildEntry.isWarning()) ++m_warningCount;

        // Route to AtlasAI
        if (m_ai && m_config.routeToAI && buildEntry.isError()) {
            std::string eventType = "BuildError";
            std::string path = buildEntry.filePath.empty() ? "build" : buildEntry.filePath;
            m_ai->processEvent(eventType, path, buildEntry.message);
            ++m_aiRoutedCount;
        }
    }

    LoggingRouteV1*       m_logger    = nullptr;
    AtlasAIIntegration*   m_ai        = nullptr;
    BuildLogRouterConfig  m_config;
    bool                  m_installed     = false;
    size_t                m_capturedCount = 0;
    size_t                m_errorCount    = 0;
    size_t                m_warningCount  = 0;
    size_t                m_aiRoutedCount = 0;
    // Ring buffer (deque for O(1) front removal)
    std::deque<BuildLogEntry> m_entries;
};

} // namespace NF
