#pragma once
// NF::Workspace — ProjectManager: session-level project lifecycle coordinator.
//
// ProjectManager is the single authority for opening, closing, saving, and
// tracking workspace projects during a session.  It is distinct from
// ProjectRegistry (the plugin/adapter factory model) — ProjectManager owns the
// runtime state of whichever project is currently active.
//
// Provides:
//   ProjectManagerState — lifecycle state machine (Idle/Opening/Open/Saving/Closing/Error)
//   RecentProjectEntry  — timestamped path descriptor for the recent-projects list
//   ProjectManagerConfig — tunable knobs (max recent, auto-save interval, etc.)
//   ProjectManager      — owns state machine, dirty flag, auto-save clock, recent list
//
// Key behaviours:
//   - Only one project may be open at a time (newProject/openProject reject if already open).
//   - markDirty() flags unsaved changes; save() clears it.
//   - Auto-save triggers when tickAutoSave(dt) accumulates >= config.autoSaveIntervalSec.
//   - Recent list is capped at config.maxRecentProjects (default 10); duplicates are bumped to front.
//   - closeProject() sets state back to Idle and clears the active path + dirty flag.
//   - setError(msg) transitions to Error state; clearError() returns to Idle.

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace NF {

// ── ProjectManagerState ───────────────────────────────────────────────────────

enum class ProjectManagerState : uint8_t {
    Idle,     // no project active
    Opening,  // in-progress open
    Open,     // project ready
    Saving,   // in-progress save
    Closing,  // in-progress close
    Error,    // error condition
};

inline const char* projectManagerStateName(ProjectManagerState s) {
    switch (s) {
        case ProjectManagerState::Idle:    return "Idle";
        case ProjectManagerState::Opening: return "Opening";
        case ProjectManagerState::Open:    return "Open";
        case ProjectManagerState::Saving:  return "Saving";
        case ProjectManagerState::Closing: return "Closing";
        case ProjectManagerState::Error:   return "Error";
        default:                           return "Unknown";
    }
}

// ── RecentProjectEntry ────────────────────────────────────────────────────────

struct RecentProjectEntry {
    std::string path;
    std::string displayName;
    uint64_t    lastOpenedMs = 0;  // Unix time in milliseconds

    [[nodiscard]] bool isValid() const { return !path.empty(); }
};

// ── ProjectManagerConfig ──────────────────────────────────────────────────────

struct ProjectManagerConfig {
    size_t   maxRecentProjects    = 10;
    float    autoSaveIntervalSec  = 300.f; // 5 minutes; 0 = disabled
    bool     autoSaveEnabled      = true;
};

// ── Auto-save callback ────────────────────────────────────────────────────────

using AutoSaveCallback = std::function<void(const std::string& projectPath)>;

// ── ProjectManager ────────────────────────────────────────────────────────────

class ProjectManager {
public:
    explicit ProjectManager(ProjectManagerConfig cfg = {}) : m_config(cfg) {}

    // ── State queries ──────────────────────────────────────────────

    [[nodiscard]] ProjectManagerState state()        const { return m_state; }
    [[nodiscard]] bool isIdle()                      const { return m_state == ProjectManagerState::Idle; }
    [[nodiscard]] bool isOpen()                      const { return m_state == ProjectManagerState::Open; }
    [[nodiscard]] bool hasError()                    const { return m_state == ProjectManagerState::Error; }
    [[nodiscard]] const std::string& errorMessage()  const { return m_errorMsg; }
    [[nodiscard]] const std::string& activePath()    const { return m_activePath; }
    [[nodiscard]] const std::string& activeDisplay() const { return m_activeDisplay; }

    // ── Dirty / unsaved state ──────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }

    void markDirty()  { if (isOpen()) m_dirty = true; }
    void markClean()  { m_dirty = false; }

    // ── New project ────────────────────────────────────────────────
    // Creates an in-memory project at the given path/displayName.
    // Fails if a project is already open or path is empty.

    bool newProject(const std::string& path, const std::string& displayName = "") {
        if (!isIdle()) return false;
        if (path.empty()) return false;

        m_state         = ProjectManagerState::Opening;
        m_activePath    = path;
        m_activeDisplay = displayName.empty() ? path : displayName;
        m_dirty         = false;
        m_autoSaveAccum = 0.f;
        m_state         = ProjectManagerState::Open;

        pushRecent({path, m_activeDisplay, currentMs()});
        return true;
    }

    // ── Open project ───────────────────────────────────────────────
    // Opens an existing project.  Semantically identical to newProject
    // but represents loading from disk (state passes through Opening).

    bool openProject(const std::string& path, const std::string& displayName = "") {
        return newProject(path, displayName);  // same state machine path
    }

    // ── Save ───────────────────────────────────────────────────────
    // Clears dirty flag.  Fails if not Open.

    bool save() {
        if (!isOpen()) return false;
        m_state = ProjectManagerState::Saving;
        m_dirty = false;
        m_autoSaveAccum = 0.f;
        m_state = ProjectManagerState::Open;
        ++m_saveCount;
        return true;
    }

    [[nodiscard]] uint32_t saveCount() const { return m_saveCount; }

    // ── Close ──────────────────────────────────────────────────────
    // Transitions through Closing → Idle; clears active project info.

    bool closeProject() {
        if (isIdle()) return false;
        if (hasError()) {
            // Allow closing from error state too.
            m_state         = ProjectManagerState::Idle;
            m_activePath.clear();
            m_activeDisplay.clear();
            m_dirty         = false;
            m_errorMsg.clear();
            m_autoSaveAccum = 0.f;
            return true;
        }
        if (!isOpen()) return false;

        m_state         = ProjectManagerState::Closing;
        m_activePath.clear();
        m_activeDisplay.clear();
        m_dirty         = false;
        m_autoSaveAccum = 0.f;
        m_state         = ProjectManagerState::Idle;
        return true;
    }

    // ── Error handling ─────────────────────────────────────────────

    void setError(const std::string& message) {
        m_state    = ProjectManagerState::Error;
        m_errorMsg = message;
    }

    void clearError() {
        if (hasError()) {
            m_state    = ProjectManagerState::Idle;
            m_errorMsg.clear();
        }
    }

    // ── Auto-save ──────────────────────────────────────────────────
    // Call each frame/tick with elapsed seconds.  Fires callback when
    // auto-save interval is reached and the project is dirty.
    // Returns true if auto-save was triggered this tick.

    bool tickAutoSave(float dtSec) {
        if (!isOpen()) return false;
        if (!m_config.autoSaveEnabled) return false;
        if (m_config.autoSaveIntervalSec <= 0.f) return false;
        if (!m_dirty) return false;

        m_autoSaveAccum += dtSec;
        if (m_autoSaveAccum >= m_config.autoSaveIntervalSec) {
            m_autoSaveAccum = 0.f;
            if (m_autoSaveCallback) m_autoSaveCallback(m_activePath);
            save();
            return true;
        }
        return false;
    }

    [[nodiscard]] float autoSaveAccum() const { return m_autoSaveAccum; }

    void setAutoSaveCallback(AutoSaveCallback cb) { m_autoSaveCallback = std::move(cb); }

    // ── Recent projects ────────────────────────────────────────────

    [[nodiscard]] const std::vector<RecentProjectEntry>& recentProjects() const {
        return m_recent;
    }

    [[nodiscard]] size_t recentCount() const { return m_recent.size(); }

    void clearRecent() { m_recent.clear(); }

    bool removeRecent(const std::string& path) {
        for (auto it = m_recent.begin(); it != m_recent.end(); ++it) {
            if (it->path == path) { m_recent.erase(it); return true; }
        }
        return false;
    }

    // ── Config ────────────────────────────────────────────────────

    [[nodiscard]] const ProjectManagerConfig& config() const { return m_config; }

    void setConfig(ProjectManagerConfig cfg) {
        m_config = cfg;
        // clamp accumulator if interval changed
        if (m_config.autoSaveIntervalSec > 0.f &&
            m_autoSaveAccum > m_config.autoSaveIntervalSec) {
            m_autoSaveAccum = m_config.autoSaveIntervalSec;
        }
    }

private:
    // Push a recent entry, deduplicating and capping the list.
    void pushRecent(RecentProjectEntry entry) {
        // Remove existing entry with same path.
        for (auto it = m_recent.begin(); it != m_recent.end(); ++it) {
            if (it->path == entry.path) { m_recent.erase(it); break; }
        }
        // Insert at front (most recent).
        m_recent.insert(m_recent.begin(), std::move(entry));
        // Cap list size.
        while (m_recent.size() > m_config.maxRecentProjects) {
            m_recent.pop_back();
        }
    }

    static uint64_t currentMs() {
        using namespace std::chrono;
        return static_cast<uint64_t>(
            duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()).count());
    }

    ProjectManagerState    m_state         = ProjectManagerState::Idle;
    std::string            m_activePath;
    std::string            m_activeDisplay;
    std::string            m_errorMsg;
    bool                   m_dirty         = false;
    uint32_t               m_saveCount     = 0;
    float                  m_autoSaveAccum = 0.f;
    ProjectManagerConfig   m_config;
    AutoSaveCallback       m_autoSaveCallback;
    std::vector<RecentProjectEntry> m_recent;
};

} // namespace NF
