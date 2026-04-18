#pragma once
// NF::Workspace — Phase 15: Workspace Diagnostics and Telemetry
//
// Workspace-level diagnostics and telemetry infrastructure:
//   DiagnosticSeverity    — severity classification
//   DiagnosticCategory    — diagnostic source category
//   DiagnosticEntry       — structured diagnostic record
//   DiagnosticCollector   — collect/query diagnostics with filtering
//   TelemetryEventType    — telemetry event classification
//   TelemetryEvent        — typed telemetry event
//   TelemetryCollector    — accumulate telemetry events with session lifecycle
//   DiagnosticSnapshot    — point-in-time snapshot of collector state

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// DiagnosticSeverity — severity level for diagnostic entries
// ═════════════════════════════════════════════════════════════════

enum class DiagnosticSeverity : uint8_t {
    Info    = 0,   // Informational — no action needed
    Warning = 1,   // Advisory — worth reviewing
    Error   = 2,   // Error — requires attention
    Fatal   = 3,   // Fatal — workspace may be unstable
};

inline const char* diagnosticSeverityName(DiagnosticSeverity s) {
    switch (s) {
        case DiagnosticSeverity::Info:    return "Info";
        case DiagnosticSeverity::Warning: return "Warning";
        case DiagnosticSeverity::Error:   return "Error";
        case DiagnosticSeverity::Fatal:   return "Fatal";
        default:                          return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// DiagnosticCategory — source category classification
// ═════════════════════════════════════════════════════════════════

enum class DiagnosticCategory : uint8_t {
    Build       = 0,   // Build system diagnostics
    Asset       = 1,   // Asset pipeline issues
    Plugin      = 2,   // Plugin system issues
    Project     = 3,   // Project loading/validation
    Tool        = 4,   // Tool lifecycle issues
    Render      = 5,   // Rendering/backend issues
    Performance = 6,   // Performance warnings
    IO          = 7,   // File I/O issues
    Network     = 8,   // Network/connectivity issues
    System      = 9,   // General workspace system
    Custom      = 10,  // User-defined / plugin diagnostics
};

inline const char* diagnosticCategoryName(DiagnosticCategory c) {
    switch (c) {
        case DiagnosticCategory::Build:       return "Build";
        case DiagnosticCategory::Asset:       return "Asset";
        case DiagnosticCategory::Plugin:      return "Plugin";
        case DiagnosticCategory::Project:     return "Project";
        case DiagnosticCategory::Tool:        return "Tool";
        case DiagnosticCategory::Render:      return "Render";
        case DiagnosticCategory::Performance: return "Performance";
        case DiagnosticCategory::IO:          return "IO";
        case DiagnosticCategory::Network:     return "Network";
        case DiagnosticCategory::System:      return "System";
        case DiagnosticCategory::Custom:      return "Custom";
        default:                              return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// DiagnosticEntry — a single structured diagnostic record
// ═════════════════════════════════════════════════════════════════

struct DiagnosticEntry {
    std::string       id;            // Unique diagnostic identifier (e.g. "BUILD_001")
    DiagnosticCategory category = DiagnosticCategory::System;
    DiagnosticSeverity severity = DiagnosticSeverity::Info;
    std::string       source;        // Originating subsystem or tool
    std::string       message;       // Human-readable diagnostic message
    std::string       detail;        // Optional extended detail
    uint64_t          timestampMs = 0; // Milliseconds since session start
    bool              acknowledged = false; // User has seen/dismissed this

    bool isValid() const {
        return !id.empty() && !source.empty() && !message.empty();
    }

    bool isError() const {
        return severity == DiagnosticSeverity::Error || severity == DiagnosticSeverity::Fatal;
    }

    bool operator==(const DiagnosticEntry& o) const { return id == o.id && timestampMs == o.timestampMs; }
    bool operator!=(const DiagnosticEntry& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// DiagnosticCollector — accumulate and query diagnostic entries
// ═════════════════════════════════════════════════════════════════

class DiagnosticCollector {
public:
    static constexpr size_t MAX_ENTRIES = 4096;

    // ── Submission ───────────────────────────────────────────────
    bool submit(const DiagnosticEntry& entry) {
        if (!entry.isValid()) return false;
        if (entries_.size() >= MAX_ENTRIES) return false;
        entries_.push_back(entry);
        notifyObservers(entries_.back());
        return true;
    }

    // ── Quick submit helpers ─────────────────────────────────────
    bool submitInfo(const std::string& id, const std::string& source,
                    const std::string& message, DiagnosticCategory cat = DiagnosticCategory::System,
                    uint64_t ts = 0) {
        return submit({id, cat, DiagnosticSeverity::Info, source, message, "", ts, false});
    }

    bool submitWarning(const std::string& id, const std::string& source,
                       const std::string& message, DiagnosticCategory cat = DiagnosticCategory::System,
                       uint64_t ts = 0) {
        return submit({id, cat, DiagnosticSeverity::Warning, source, message, "", ts, false});
    }

    bool submitError(const std::string& id, const std::string& source,
                     const std::string& message, DiagnosticCategory cat = DiagnosticCategory::System,
                     uint64_t ts = 0) {
        return submit({id, cat, DiagnosticSeverity::Error, source, message, "", ts, false});
    }

    // ── Query ────────────────────────────────────────────────────
    size_t count() const { return entries_.size(); }
    bool   empty() const { return entries_.empty(); }

    const DiagnosticEntry* findById(const std::string& id) const {
        for (auto& e : entries_) {
            if (e.id == id) return &e;
        }
        return nullptr;
    }

    std::vector<DiagnosticEntry> findByCategory(DiagnosticCategory cat) const {
        std::vector<DiagnosticEntry> result;
        for (auto& e : entries_) {
            if (e.category == cat) result.push_back(e);
        }
        return result;
    }

    std::vector<DiagnosticEntry> findBySeverity(DiagnosticSeverity sev) const {
        std::vector<DiagnosticEntry> result;
        for (auto& e : entries_) {
            if (e.severity == sev) result.push_back(e);
        }
        return result;
    }

    std::vector<DiagnosticEntry> findBySource(const std::string& source) const {
        std::vector<DiagnosticEntry> result;
        for (auto& e : entries_) {
            if (e.source == source) result.push_back(e);
        }
        return result;
    }

    size_t countBySeverity(DiagnosticSeverity sev) const {
        size_t n = 0;
        for (auto& e : entries_) {
            if (e.severity == sev) ++n;
        }
        return n;
    }

    size_t countByCategory(DiagnosticCategory cat) const {
        size_t n = 0;
        for (auto& e : entries_) {
            if (e.category == cat) ++n;
        }
        return n;
    }

    size_t errorCount() const {
        size_t n = 0;
        for (auto& e : entries_) {
            if (e.isError()) ++n;
        }
        return n;
    }

    size_t unacknowledgedCount() const {
        size_t n = 0;
        for (auto& e : entries_) {
            if (!e.acknowledged) ++n;
        }
        return n;
    }

    bool hasErrors() const { return errorCount() > 0; }

    const std::vector<DiagnosticEntry>& all() const { return entries_; }

    // ── Acknowledge ──────────────────────────────────────────────
    bool acknowledge(const std::string& id) {
        for (auto& e : entries_) {
            if (e.id == id) { e.acknowledged = true; return true; }
        }
        return false;
    }

    void acknowledgeAll() {
        for (auto& e : entries_) e.acknowledged = true;
    }

    // ── Observer ─────────────────────────────────────────────────
    using DiagnosticCallback = std::function<void(const DiagnosticEntry&)>;

    void addObserver(DiagnosticCallback cb) {
        if (cb) observers_.push_back(std::move(cb));
    }

    void clearObservers() { observers_.clear(); }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() { entries_.clear(); }

private:
    void notifyObservers(const DiagnosticEntry& entry) {
        for (auto& cb : observers_) cb(entry);
    }

    std::vector<DiagnosticEntry>    entries_;
    std::vector<DiagnosticCallback> observers_;
};

// ═════════════════════════════════════════════════════════════════
// TelemetryEventType — telemetry event classification
// ═════════════════════════════════════════════════════════════════

enum class TelemetryEventType : uint8_t {
    FeatureUsage  = 0,  // User invoked a feature / tool
    Performance   = 1,  // Frame time, load time, build time
    Error         = 2,  // Runtime error occurred
    Navigation    = 3,  // Panel/tool navigation
    Session       = 4,  // Session start/end/resume
    Command       = 5,  // Command execution
    Asset         = 6,  // Asset import/export/modify
    Plugin        = 7,  // Plugin load/activate
    Custom        = 8,  // User-defined
};

inline const char* telemetryEventTypeName(TelemetryEventType t) {
    switch (t) {
        case TelemetryEventType::FeatureUsage: return "FeatureUsage";
        case TelemetryEventType::Performance:  return "Performance";
        case TelemetryEventType::Error:        return "Error";
        case TelemetryEventType::Navigation:   return "Navigation";
        case TelemetryEventType::Session:      return "Session";
        case TelemetryEventType::Command:      return "Command";
        case TelemetryEventType::Asset:        return "Asset";
        case TelemetryEventType::Plugin:       return "Plugin";
        case TelemetryEventType::Custom:       return "Custom";
        default:                               return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TelemetryEvent — a single telemetry event
// ═════════════════════════════════════════════════════════════════

struct TelemetryEvent {
    static constexpr size_t MAX_PROPERTIES = 32;

    std::string        name;        // Event name (e.g. "tool.activated")
    TelemetryEventType type = TelemetryEventType::Custom;
    std::string        source;      // Originating subsystem
    uint64_t           timestampMs = 0;
    double             durationMs  = 0.0; // Optional duration measurement

    // Key-value property bag
    struct Property {
        std::string key;
        std::string value;
    };

    bool isValid() const { return !name.empty() && !source.empty(); }

    bool setProperty(const std::string& key, const std::string& value) {
        if (key.empty()) return false;
        for (auto& p : properties_) {
            if (p.key == key) { p.value = value; return true; }
        }
        if (properties_.size() >= MAX_PROPERTIES) return false;
        properties_.push_back({key, value});
        return true;
    }

    std::string getProperty(const std::string& key) const {
        for (auto& p : properties_) {
            if (p.key == key) return p.value;
        }
        return {};
    }

    bool hasProperty(const std::string& key) const {
        for (auto& p : properties_) {
            if (p.key == key) return true;
        }
        return false;
    }

    size_t propertyCount() const { return properties_.size(); }

    const std::vector<Property>& properties() const { return properties_; }

private:
    std::vector<Property> properties_;
};

// ═════════════════════════════════════════════════════════════════
// TelemetryCollector — session-scoped telemetry accumulation
// ═════════════════════════════════════════════════════════════════

class TelemetryCollector {
public:
    static constexpr size_t MAX_EVENTS = 8192;

    // ── Session lifecycle ────────────────────────────────────────
    void beginSession(const std::string& sessionId) {
        sessionId_ = sessionId;
        active_ = true;
        events_.clear();
    }

    void endSession() {
        active_ = false;
    }

    bool isActive() const { return active_; }
    const std::string& sessionId() const { return sessionId_; }

    // ── Recording ────────────────────────────────────────────────
    bool record(const TelemetryEvent& event) {
        if (!active_) return false;
        if (!event.isValid()) return false;
        if (events_.size() >= MAX_EVENTS) return false;
        events_.push_back(event);
        notifyObservers(events_.back());
        return true;
    }

    // ── Quick record helpers ─────────────────────────────────────
    bool recordFeature(const std::string& name, const std::string& source, uint64_t ts = 0) {
        TelemetryEvent e;
        e.name = name;
        e.type = TelemetryEventType::FeatureUsage;
        e.source = source;
        e.timestampMs = ts;
        return record(e);
    }

    bool recordPerformance(const std::string& name, const std::string& source,
                           double durationMs, uint64_t ts = 0) {
        TelemetryEvent e;
        e.name = name;
        e.type = TelemetryEventType::Performance;
        e.source = source;
        e.durationMs = durationMs;
        e.timestampMs = ts;
        return record(e);
    }

    bool recordError(const std::string& name, const std::string& source, uint64_t ts = 0) {
        TelemetryEvent e;
        e.name = name;
        e.type = TelemetryEventType::Error;
        e.source = source;
        e.timestampMs = ts;
        return record(e);
    }

    // ── Query ────────────────────────────────────────────────────
    size_t count() const { return events_.size(); }
    bool   empty() const { return events_.empty(); }

    std::vector<TelemetryEvent> findByType(TelemetryEventType type) const {
        std::vector<TelemetryEvent> result;
        for (auto& e : events_) {
            if (e.type == type) result.push_back(e);
        }
        return result;
    }

    std::vector<TelemetryEvent> findBySource(const std::string& source) const {
        std::vector<TelemetryEvent> result;
        for (auto& e : events_) {
            if (e.source == source) result.push_back(e);
        }
        return result;
    }

    std::vector<TelemetryEvent> findByName(const std::string& name) const {
        std::vector<TelemetryEvent> result;
        for (auto& e : events_) {
            if (e.name == name) result.push_back(e);
        }
        return result;
    }

    size_t countByType(TelemetryEventType type) const {
        size_t n = 0;
        for (auto& e : events_) {
            if (e.type == type) ++n;
        }
        return n;
    }

    const std::vector<TelemetryEvent>& all() const { return events_; }

    // ── Observer ─────────────────────────────────────────────────
    using TelemetryCallback = std::function<void(const TelemetryEvent&)>;

    void addObserver(TelemetryCallback cb) {
        if (cb) observers_.push_back(std::move(cb));
    }

    void clearObservers() { observers_.clear(); }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() {
        events_.clear();
    }

private:
    void notifyObservers(const TelemetryEvent& event) {
        for (auto& cb : observers_) cb(event);
    }

    std::string                     sessionId_;
    bool                            active_ = false;
    std::vector<TelemetryEvent>     events_;
    std::vector<TelemetryCallback>  observers_;
};

// ═════════════════════════════════════════════════════════════════
// DiagnosticSnapshot — point-in-time snapshot of collector state
// ═════════════════════════════════════════════════════════════════

struct DiagnosticSnapshot {
    size_t totalEntries       = 0;
    size_t infoCount          = 0;
    size_t warningCount       = 0;
    size_t errorCount         = 0;
    size_t fatalCount         = 0;
    size_t unacknowledgedCount = 0;
    bool   hasErrors          = false;

    static DiagnosticSnapshot capture(const DiagnosticCollector& collector) {
        DiagnosticSnapshot snap;
        snap.totalEntries        = collector.count();
        snap.infoCount           = collector.countBySeverity(DiagnosticSeverity::Info);
        snap.warningCount        = collector.countBySeverity(DiagnosticSeverity::Warning);
        snap.errorCount          = collector.countBySeverity(DiagnosticSeverity::Error);
        snap.fatalCount          = collector.countBySeverity(DiagnosticSeverity::Fatal);
        snap.unacknowledgedCount = collector.unacknowledgedCount();
        snap.hasErrors           = collector.hasErrors();
        return snap;
    }
};

// ═════════════════════════════════════════════════════════════════
// TelemetrySnapshot — point-in-time snapshot of telemetry state
// ═════════════════════════════════════════════════════════════════

struct TelemetrySnapshot {
    std::string sessionId;
    bool        active       = false;
    size_t      totalEvents  = 0;
    size_t      featureCount = 0;
    size_t      perfCount    = 0;
    size_t      errorCount   = 0;

    static TelemetrySnapshot capture(const TelemetryCollector& collector) {
        TelemetrySnapshot snap;
        snap.sessionId    = collector.sessionId();
        snap.active       = collector.isActive();
        snap.totalEvents  = collector.count();
        snap.featureCount = collector.countByType(TelemetryEventType::FeatureUsage);
        snap.perfCount    = collector.countByType(TelemetryEventType::Performance);
        snap.errorCount   = collector.countByType(TelemetryEventType::Error);
        return snap;
    }
};

} // namespace NF
