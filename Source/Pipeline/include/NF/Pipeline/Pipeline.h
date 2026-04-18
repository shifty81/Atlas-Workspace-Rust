#pragma once
// NF::Pipeline — Shared workspace pipeline for inter-tool communication
//
// All tools (editor, Blender bridge, AtlasAI, ContractScanner,
// ReplayMinimizer) communicate exclusively by reading and writing files under
// the workspace's .atlas/pipeline/ directory tree.  No sockets, no RPC.
//
// Directory layout (relative to workspace root):
//   .atlas/
//     pipeline/
//       changes/       ← .change.json events dropped here by any tool
//       assets/        ← imported/generated assets (meshes, textures)
//       worlds/        ← live world-state snapshots
//       scripts/       ← GraphVM bytecode / logic graph JSON
//       animations/    ← exported rigs, clips, IK configs
//       sessions/      ← AtlasAI session logs (future)
//     manifest.json    ← GUID → asset path registry
//     watch.log        ← append-only event log written by all tools

#include <cstdint>
#include <string>
#include <string_view>
#include <functional>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <filesystem>

namespace NF {

// ── ChangeEventType ──────────────────────────────────────────────

enum class ChangeEventType : uint8_t {
    Unknown,
    AssetImported,
    WorldChanged,
    ScriptUpdated,
    AnimationExported,
    ContractIssue,
    ReplayExported,
    AIAnalysis,          // Emitted by AI tools (AtlasAI) as a response
};

inline const char* changeEventTypeName(ChangeEventType t) noexcept {
    switch (t) {
        case ChangeEventType::AssetImported:     return "AssetImported";
        case ChangeEventType::WorldChanged:      return "WorldChanged";
        case ChangeEventType::ScriptUpdated:     return "ScriptUpdated";
        case ChangeEventType::AnimationExported: return "AnimationExported";
        case ChangeEventType::ContractIssue:     return "ContractIssue";
        case ChangeEventType::ReplayExported:    return "ReplayExported";
        case ChangeEventType::AIAnalysis:        return "AIAnalysis";
        default:                                  return "Unknown";
    }
}

inline ChangeEventType changeEventTypeFromString(std::string_view s) noexcept {
    if (s == "AssetImported")     return ChangeEventType::AssetImported;
    if (s == "WorldChanged")      return ChangeEventType::WorldChanged;
    if (s == "ScriptUpdated")     return ChangeEventType::ScriptUpdated;
    if (s == "AnimationExported") return ChangeEventType::AnimationExported;
    if (s == "ContractIssue")     return ChangeEventType::ContractIssue;
    if (s == "ReplayExported")    return ChangeEventType::ReplayExported;
    if (s == "AIAnalysis")        return ChangeEventType::AIAnalysis;
    return ChangeEventType::Unknown;
}

// ── ChangeEvent ──────────────────────────────────────────────────
// Schema: { "tool": "...", "event_type": "...", "path": "...",
//           "timestamp": <ms>, "metadata": "..." }

struct ChangeEvent {
    std::string     tool;
    ChangeEventType eventType = ChangeEventType::Unknown;
    std::string     path;
    int64_t         timestamp = 0;   // milliseconds since Unix epoch
    std::string     metadata;        // arbitrary string (JSON-compatible)

    // Serialize to a JSON string.
    std::string toJson() const;

    // Parse from a JSON string; returns false on failure.
    static bool fromJson(const std::string& json, ChangeEvent& out);

    // Write to a .change.json file inside changesDir.
    // Filename: <tool>_<eventtype>_<timestamp>.change.json
    bool writeToFile(const std::filesystem::path& changesDir) const;

    // Read from a .change.json file.
    static bool readFromFile(const std::filesystem::path& filePath,
                             ChangeEvent& out);
};

// ── AssetRecord ──────────────────────────────────────────────────

struct AssetRecord {
    std::string guid;
    std::string type;         // "mesh", "texture", "animation", "script", …
    std::string path;         // relative path from workspace root
    int64_t     importDate = 0;
    std::string checksum;
};

// ── Manifest ─────────────────────────────────────────────────────
// Backed by manifest.json.  Stores the project-level GUID registry and
// module list.  All assets imported from any tool get an entry here.

class Manifest {
public:
    Manifest() = default;

    // Load from a manifest.json file.  Returns false if the file is missing
    // or cannot be parsed (leaves the object in a default-constructed state).
    bool load(const std::filesystem::path& manifestPath);

    // Persist to a manifest.json file.  Creates the file if it does not exist.
    bool save(const std::filesystem::path& manifestPath) const;

    // Register an asset.  If record.guid is empty a unique GUID is generated.
    // If an asset with the same path already exists it is updated.
    // Returns the final GUID.
    std::string registerAsset(AssetRecord record);

    const AssetRecord* findByGuid(const std::string& guid) const;
    const AssetRecord* findByPath(const std::string& path) const;

    // Remove by GUID.  Returns true if an entry was found and removed.
    bool removeAsset(const std::string& guid);

    const std::vector<AssetRecord>& records() const { return m_records; }
    size_t recordCount() const { return m_records.size(); }

    // Project-level metadata.
    std::string              projectName;
    std::vector<std::string> modules;

private:
    std::vector<AssetRecord>                  m_records;
    std::unordered_map<std::string, size_t>   m_guidIndex;  // guid  → index
    std::unordered_map<std::string, size_t>   m_pathIndex;  // path  → index

    static std::string generateGuid();
    void rebuildIndex();
};

// ── WatchLog ─────────────────────────────────────────────────────
// Append-only event log.  Every tool appends its ChangeEvents here so that
// AtlasAI (S9) can replay the full development timeline.

class WatchLog {
public:
    explicit WatchLog(std::filesystem::path logPath);
    ~WatchLog() = default;

    WatchLog(const WatchLog&)            = delete;
    WatchLog& operator=(const WatchLog&) = delete;

    // Append a ChangeEvent line (thread-safe).
    void append(const ChangeEvent& event);

    // Append a raw text line (thread-safe).
    void appendLine(std::string_view message);

    // Flush buffered writes to disk.
    void flush();

    const std::filesystem::path& path() const { return m_logPath; }

private:
    std::filesystem::path m_logPath;
    std::mutex            m_mutex;
};

// ── PipelineWatcher ──────────────────────────────────────────────
// Watches a directory for new .change.json files and dispatches typed
// ChangeEvents to registered callbacks.
//
// Two modes of use:
//  1. Background thread: call start() — the watcher polls every 100 ms.
//  2. Single-thread / test: call poll() manually.

using PipelineEventCallback = std::function<void(const ChangeEvent&)>;

class PipelineWatcher {
public:
    explicit PipelineWatcher(std::filesystem::path watchDir);
    ~PipelineWatcher();

    PipelineWatcher(const PipelineWatcher&)            = delete;
    PipelineWatcher& operator=(const PipelineWatcher&) = delete;

    // Register a callback to be invoked when a new event is detected.
    void subscribe(PipelineEventCallback cb);

    // Start the background polling thread.  Returns false if already running.
    bool start();

    // Stop the background thread and join it.
    void stop();

    bool isRunning() const noexcept { return m_running.load(); }

    // Manually scan watchDir for unprocessed .change.json files.
    // Dispatches callbacks synchronously.  Returns the count of new events.
    int poll();

    const std::filesystem::path& watchDir() const { return m_watchDir; }

private:
    void watchLoop();
    void processFile(const std::filesystem::path& filePath);

    std::filesystem::path              m_watchDir;
    std::vector<PipelineEventCallback> m_callbacks;
    std::atomic<bool>                  m_running{false};
    std::thread                        m_thread;
    std::mutex                         m_callbackMutex;

    // Tracks processed files: filename → last-write-time in ms.
    std::unordered_map<std::string, int64_t> m_seen;
    std::mutex                               m_seenMutex;
};

// ── PipelineDirectories ──────────────────────────────────────────
// Utility: derive all pipeline paths from a workspace root and optionally
// create the directory tree.

struct PipelineDirectories {
    std::filesystem::path root;
    std::filesystem::path dotAtlas;
    std::filesystem::path pipeline;
    std::filesystem::path changes;
    std::filesystem::path assets;
    std::filesystem::path worlds;
    std::filesystem::path scripts;
    std::filesystem::path animations;
    std::filesystem::path sessions;
    std::filesystem::path manifestFile;
    std::filesystem::path watchLogFile;

    static PipelineDirectories fromRoot(const std::filesystem::path& workspaceRoot);

    // Create all directories on disk.  Returns false if any creation fails.
    bool ensureCreated() const;
};

// ── ToolAdapter ──────────────────────────────────────────────────
// Abstract base class for pipeline-aware tools.  Each tool declares which
// event types it accepts and processes incoming events.  Tools may emit
// response events back into the changes directory.

class ToolAdapter {
public:
    virtual ~ToolAdapter() = default;

    // Human-readable tool name (e.g. "BlenderGenerator").
    virtual const char* name() const noexcept = 0;

    // Return true if this tool should receive events of the given type.
    virtual bool acceptsEvent(ChangeEventType type) const noexcept = 0;

    // Process an incoming event.  The adapter may write response events
    // to dirs.changes.  Returns true if the event was handled.
    virtual bool handleEvent(const ChangeEvent& event,
                             const PipelineDirectories& dirs) = 0;

    // Count of events successfully handled since construction.
    size_t handledCount() const noexcept { return m_handledCount; }

protected:
    // Helper: emit a response ChangeEvent back into the pipeline.
    bool emitEvent(ChangeEventType type,
                   const std::string& path,
                   const std::string& metadata,
                   const PipelineDirectories& dirs);

    size_t m_handledCount = 0;
};

// ── Concrete Tool Adapters ───────────────────────────────────────

// BlenderGenerator: accepts AssetImported; imports meshes / animations.
class BlenderGenAdapter final : public ToolAdapter {
public:
    const char* name() const noexcept override;
    bool acceptsEvent(ChangeEventType type) const noexcept override;
    bool handleEvent(const ChangeEvent& event,
                     const PipelineDirectories& dirs) override;
};

// ContractScanner: accepts ScriptUpdated; scans source for violations.
class ContractScannerAdapter final : public ToolAdapter {
public:
    const char* name() const noexcept override;
    bool acceptsEvent(ChangeEventType type) const noexcept override;
    bool handleEvent(const ChangeEvent& event,
                     const PipelineDirectories& dirs) override;
};

// ReplayMinimizer: accepts ReplayExported; minimizes replay files.
class ReplayMinimizerAdapter final : public ToolAdapter {
public:
    const char* name() const noexcept override;
    bool acceptsEvent(ChangeEventType type) const noexcept override;
    bool handleEvent(const ChangeEvent& event,
                     const PipelineDirectories& dirs) override;
};

// AtlasAI: accepts all non-Unknown event types; unified AI analysis broker
// (absorbs the former SwissAgent observer role) with specialised reasoning
// for ContractIssue and WorldChanged events.
class AtlasAIAdapter final : public ToolAdapter {
public:
    const char* name() const noexcept override;
    bool acceptsEvent(ChangeEventType type) const noexcept override;
    bool handleEvent(const ChangeEvent& event,
                     const PipelineDirectories& dirs) override;
};

// ── AssetImportStatus ────────────────────────────────────────────
// Tracks the lifecycle of an asset imported through the pipeline.

enum class AssetImportStatus : uint8_t {
    Pending,       // Import requested, not yet processed
    Validated,     // File exists and has valid format
    Registered,    // Asset registered in Manifest with a GUID
    Failed,        // Import failed (see errorMessage)
};

inline const char* assetImportStatusName(AssetImportStatus s) noexcept {
    switch (s) {
        case AssetImportStatus::Pending:    return "Pending";
        case AssetImportStatus::Validated:  return "Validated";
        case AssetImportStatus::Registered: return "Registered";
        case AssetImportStatus::Failed:     return "Failed";
        default:                             return "Unknown";
    }
}

// ── AssetImportResult ────────────────────────────────────────────
// Result of processing an asset through the BlenderBridge.

struct AssetImportResult {
    std::string       sourcePath;       // Original file path from ChangeEvent
    std::string       guid;             // GUID assigned by Manifest (empty on failure)
    std::string       assetType;        // "mesh", "rig", "clip", or "unknown"
    AssetImportStatus status = AssetImportStatus::Pending;
    std::string       errorMessage;     // Non-empty only on failure
    int64_t           importTimestamp = 0;
};

// ── BlenderBridge ────────────────────────────────────────────────
// S2: Full BlenderGen Bridge that processes exported assets from
// Blender, validates them, registers them in the Manifest, and
// tracks import history.  Designed to be driven by BlenderGenAdapter
// or used standalone in tests.

class BlenderBridge {
public:
    explicit BlenderBridge(Manifest& manifest);

    // Process an asset from BlenderGenerator.  Validates the file,
    // detects asset type from metadata, registers in Manifest.
    // Returns a result struct with the outcome.
    AssetImportResult importAsset(const ChangeEvent& event,
                                  const PipelineDirectories& dirs);

    // Query import history.
    const std::vector<AssetImportResult>& importHistory() const { return m_history; }
    size_t importCount() const noexcept { return m_importCount; }
    size_t failedCount() const noexcept { return m_failedCount; }

    // Check if an asset path has already been imported.
    bool isImported(const std::string& path) const;

    // Get the GUID for a previously imported asset.
    std::string guidForPath(const std::string& path) const;

    // Access the underlying Manifest.
    Manifest&       manifest()       { return m_manifest; }
    const Manifest& manifest() const { return m_manifest; }

private:
    Manifest&                                 m_manifest;
    std::vector<AssetImportResult>            m_history;
    std::unordered_map<std::string, size_t>   m_pathIndex;  // path → history index
    size_t                                    m_importCount = 0;
    size_t                                    m_failedCount = 0;

    // Detect asset type from event metadata (type=mesh, type=rig, type=clip).
    static std::string detectAssetType(const ChangeEvent& event);

    // Validate that the asset file exists on disk.
    static bool validateAssetFile(const std::filesystem::path& fullPath);

    // Resolve the full filesystem path for an asset from its event path.
    static std::filesystem::path resolveAssetPath(
        const ChangeEvent& event, const PipelineDirectories& dirs);
};

// ── ToolRegistry ─────────────────────────────────────────────────
// Central registry that connects a PipelineWatcher to ToolAdapters.
// When an event arrives, the registry dispatches it to every adapter
// whose acceptsEvent() returns true.

class ToolRegistry {
public:
    ToolRegistry() = default;

    // Register a tool adapter.  Ownership is transferred to the registry.
    void registerTool(std::unique_ptr<ToolAdapter> tool);

    // Attach to a PipelineWatcher.  Subscribes a callback that dispatches
    // events to all registered tools.  Must be called after all tools are
    // registered.  The PipelineDirectories are captured for handler use.
    void attach(PipelineWatcher& watcher, const PipelineDirectories& dirs);

    // Manually dispatch a single event to all matching tools (for testing).
    int dispatch(const ChangeEvent& event, const PipelineDirectories& dirs);

    // Query registered tools.
    size_t toolCount() const noexcept { return m_tools.size(); }
    const ToolAdapter* tool(size_t index) const;

private:
    std::vector<std::unique_ptr<ToolAdapter>> m_tools;
};

// ── AnalysisRequest ──────────────────────────────────────────────
// A pipeline event forwarded to the AI workspace broker for analysis.

struct AnalysisRequest {
    ChangeEvent       event;           // The original pipeline event
    std::string       sessionId;       // Broker session that owns this request
    int64_t           requestTime = 0; // When the request was created
};

// ── AnalysisResult ───────────────────────────────────────────────
// The broker's response after processing an AnalysisRequest.

struct AnalysisResult {
    std::string       sessionId;
    std::string       requestPath;     // Path from the original event
    ChangeEventType   sourceEventType = ChangeEventType::Unknown;
    std::string       summary;         // Human-readable analysis summary
    std::string       recommendation;  // Suggested action (may be empty)
    int64_t           analysisTime = 0;
    bool              success = false;
};

// ── BrokerSession ────────────────────────────────────────────────
// Tracks a single AI analysis session with conversation history.

struct BrokerSession {
    std::string                    id;
    std::string                    projectName;
    int64_t                        createdAt = 0;
    int64_t                        lastActiveAt = 0;
    std::vector<AnalysisRequest>   requests;
    std::vector<AnalysisResult>    results;
    bool                           active = true;
};

// ── WorkspaceBroker ──────────────────────────────────────────────
// S3: AtlasAI workspace broker that manages AI sessions, context
// indexing, and analysis routing.  Processes pipeline events, tracks
// workspace state, and emits analysis results back into the pipeline.
//
// The broker maintains:
//  - Named sessions (create/resume/close)
//  - Per-session event history and analysis results
//  - Workspace context (event index for lookup)
//  - File activity tracking for hot-path detection

class WorkspaceBroker {
public:
    WorkspaceBroker();

    // ── SA-1: Session management ──────────────────────────────────

    // Create a new session.  Returns the generated session ID.
    std::string createSession(const std::string& projectName);

    // Resume an existing session by ID.  Returns true if found.
    bool resumeSession(const std::string& sessionId);

    // Close a session.  Returns true if found and closed.
    bool closeSession(const std::string& sessionId);

    // Get a session by ID (nullptr if not found).
    const BrokerSession* session(const std::string& sessionId) const;

    // List all active session IDs.
    std::vector<std::string> activeSessions() const;

    size_t sessionCount() const noexcept { return m_sessions.size(); }

    // ── SA-2: Context indexing ────────────────────────────────────

    // Index a pipeline event into the workspace context.
    // Tracks file activity and event history per session.
    void indexEvent(const std::string& sessionId, const ChangeEvent& event);

    // Get the number of events indexed for a given file path.
    size_t eventCountForPath(const std::string& path) const;

    // Get the most recent event type for a given file path.
    ChangeEventType lastEventTypeForPath(const std::string& path) const;

    // Get all tracked file paths (hot-path detection).
    std::vector<std::string> trackedPaths() const;

    // ── SA-3: Analysis requests ───────────────────────────────────

    // Submit a pipeline event for AI analysis within a session.
    // Returns the analysis result.
    AnalysisResult analyzeEvent(const std::string& sessionId,
                                const ChangeEvent& event,
                                const PipelineDirectories& dirs);

    // ── SA-4: Broker statistics ───────────────────────────────────

    size_t totalAnalyses() const noexcept { return m_totalAnalyses; }
    size_t totalEventsIndexed() const noexcept { return m_totalEventsIndexed; }

    // ── SA-5: Full pipeline integration ───────────────────────────

    // Subscribe the broker to a PipelineWatcher.  All incoming events
    // are indexed and analyzed under the given session.
    void attachToWatcher(PipelineWatcher& watcher,
                         const std::string& sessionId,
                         const PipelineDirectories& dirs);

private:
    std::unordered_map<std::string, BrokerSession> m_sessions;
    size_t m_totalAnalyses = 0;
    size_t m_totalEventsIndexed = 0;

    // Context index: file path → list of (event type, timestamp) pairs.
    struct FileActivity {
        std::vector<std::pair<ChangeEventType, int64_t>> events;
    };
    std::unordered_map<std::string, FileActivity> m_fileIndex;

    static std::string generateSessionId();
    std::string generateSummary(const ChangeEvent& event) const;
    std::string generateRecommendation(const ChangeEvent& event) const;
};

// ── RuleSeverity ─────────────────────────────────────────────────
// Severity level for AtlasAI rule violations.

enum class RuleSeverity : uint8_t {
    Info,
    Warning,
    Error,
    Critical,
};

inline const char* ruleSeverityName(RuleSeverity s) noexcept {
    switch (s) {
        case RuleSeverity::Info:     return "Info";
        case RuleSeverity::Warning:  return "Warning";
        case RuleSeverity::Error:    return "Error";
        case RuleSeverity::Critical: return "Critical";
        default:                      return "Unknown";
    }
}

// ── AtlasAIRule ─────────────────────────────────────────────────
// A single declarative rule evaluated by the AtlasAIReasoner.
// Schema: { "id": "...", "description": "...", "severity": "...",
//           "event_type": "...", "path_pattern": "...",
//           "condition": "...", "suggestion": "..." }

struct AtlasAIRule {
    std::string       id;            // Unique rule identifier (e.g. "R001")
    std::string       description;   // Human-readable description
    RuleSeverity      severity = RuleSeverity::Warning;
    ChangeEventType   eventType = ChangeEventType::Unknown;  // Which event triggers this rule
    std::string       pathPattern;   // Glob-like path pattern (empty = match all)
    std::string       condition;     // Condition expression (for future use)
    std::string       suggestion;    // Recommended fix if violated
};

// ── RuleViolation ────────────────────────────────────────────────
// Result of evaluating an AtlasAIRule against a pipeline event.

struct RuleViolation {
    std::string       ruleId;
    std::string       description;
    RuleSeverity      severity = RuleSeverity::Warning;
    std::string       path;          // File path that triggered the violation
    std::string       suggestion;    // Recommended fix
    int64_t           timestamp = 0;
};

// ── AtlasAIReasoner ─────────────────────────────────────────────
// S4: AtlasAI reasoning engine that evaluates declarative rules
// against pipeline events.  Manages a rule set, evaluates events,
// tracks violations, and emits findings back into the pipeline.
//
// AB-1: Rule definition and loading
// AB-2: Rule evaluation engine
// AB-3: Game balance rule set (pre-built rules)
// AB-4: Editor integration (violation reporting)
// AB-5: CI gate integration (pass/fail summary)

class AtlasAIReasoner {
public:
    AtlasAIReasoner();

    // ── AB-1: Rule management ─────────────────────────────────────

    // Add a rule to the rule set.
    void addRule(AtlasAIRule rule);

    // Load rules from a JSON string (array of rule objects).
    size_t loadRulesFromJson(const std::string& json);

    // Get all registered rules.
    const std::vector<AtlasAIRule>& rules() const { return m_rules; }
    size_t ruleCount() const noexcept { return m_rules.size(); }

    // Find a rule by ID (nullptr if not found).
    const AtlasAIRule* findRule(const std::string& id) const;

    // ── AB-2: Rule evaluation ─────────────────────────────────────

    // Evaluate all applicable rules against a pipeline event.
    // Returns the list of violations found.
    std::vector<RuleViolation> evaluate(const ChangeEvent& event) const;

    // ── AB-3: Built-in game balance rules ─────────────────────────

    // Load a pre-built set of game balance and pipeline quality rules.
    void loadDefaultRules();

    // ── AB-4: Violation tracking ──────────────────────────────────

    // Process an event: evaluate rules, track violations, emit findings.
    // Returns the number of violations found.
    size_t processEvent(const ChangeEvent& event,
                        const PipelineDirectories& dirs);

    // Get all accumulated violations.
    const std::vector<RuleViolation>& violations() const { return m_violations; }
    size_t violationCount() const noexcept { return m_violations.size(); }

    // Get violations for a specific path.
    std::vector<RuleViolation> violationsForPath(const std::string& path) const;

    // ── AB-5: CI gate summary ─────────────────────────────────────

    // Returns true if there are no Error or Critical violations.
    bool passesGate() const;

    // Produce a human-readable summary of all violations.
    std::string summary() const;

    // Total events processed.
    size_t eventsProcessed() const noexcept { return m_eventsProcessed; }

    // ── Pipeline integration ──────────────────────────────────────

    // Subscribe to a PipelineWatcher.  Incoming events are evaluated
    // against the rule set and violations are emitted as AIAnalysis events.
    void attachToWatcher(PipelineWatcher& watcher,
                         const PipelineDirectories& dirs);

private:
    std::vector<AtlasAIRule>    m_rules;
    std::vector<RuleViolation>  m_violations;
    size_t                      m_eventsProcessed = 0;

    bool matchesPath(const std::string& pattern,
                     const std::string& path) const;
};

} // namespace NF
