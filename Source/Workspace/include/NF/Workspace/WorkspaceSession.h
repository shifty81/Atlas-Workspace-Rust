#pragma once
// NF::Workspace — Phase 19: Workspace Session Management
//
// Workspace-level session lifecycle with history and observers:
//   SessionState    — session lifecycle state enum
//   RecentItem      — recently opened item descriptor
//   SessionRecord   — session snapshot with tool list and duration
//   SessionHistory  — capped recent items and session records
//   SessionManager  — workspace-scoped session lifecycle with observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// SessionState — session lifecycle state
// ═════════════════════════════════════════════════════════════════

enum class SessionState : uint8_t {
    Idle    = 0,   // No session active
    Starting = 1,  // Session is being started
    Running  = 2,  // Session is active
    Saving   = 3,  // Session is being saved
    Closing  = 4,  // Session is being closed
    Closed   = 5,  // Session has been closed
};

inline const char* sessionStateName(SessionState s) {
    switch (s) {
        case SessionState::Idle:     return "Idle";
        case SessionState::Starting: return "Starting";
        case SessionState::Running:  return "Running";
        case SessionState::Saving:   return "Saving";
        case SessionState::Closing:  return "Closing";
        case SessionState::Closed:   return "Closed";
        default:                     return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// RecentItem — recently opened item descriptor
// ═════════════════════════════════════════════════════════════════

struct RecentItem {
    std::string path;
    std::string label;
    std::string type;
    uint64_t    timestamp = 0;

    bool isValid() const { return !path.empty(); }
    bool operator==(const RecentItem& o) const { return path == o.path; }
    bool operator!=(const RecentItem& o) const { return path != o.path; }
};

// ═════════════════════════════════════════════════════════════════
// SessionRecord — session snapshot with tool list and duration
// ═════════════════════════════════════════════════════════════════

class SessionRecord {
public:
    SessionRecord() = default;
    explicit SessionRecord(std::string id, std::string name = {})
        : m_id(std::move(id)), m_name(std::move(name)) {}

    const std::string& id()        const { return m_id; }
    const std::string& name()      const { return m_name; }
    SessionState       state()     const { return m_state; }
    uint64_t           startTime() const { return m_startTime; }
    uint64_t           endTime()   const { return m_endTime; }

    void setState(SessionState s)    { m_state = s; }
    void setStartTime(uint64_t t)    { m_startTime = t; }
    void setEndTime(uint64_t t)      { m_endTime = t; }

    uint64_t duration() const {
        return (m_endTime >= m_startTime) ? (m_endTime - m_startTime) : 0;
    }

    void addTool(const std::string& toolId) {
        if (!toolId.empty() && !hasTool(toolId))
            m_toolIds.push_back(toolId);
    }

    bool hasTool(const std::string& toolId) const {
        return std::find(m_toolIds.begin(), m_toolIds.end(), toolId) != m_toolIds.end();
    }

    const std::vector<std::string>& toolIds() const { return m_toolIds; }

    bool isValid() const { return !m_id.empty(); }

    bool operator==(const SessionRecord& o) const { return m_id == o.m_id; }
    bool operator!=(const SessionRecord& o) const { return m_id != o.m_id; }

private:
    std::string              m_id;
    std::string              m_name;
    SessionState             m_state     = SessionState::Idle;
    uint64_t                 m_startTime = 0;
    uint64_t                 m_endTime   = 0;
    std::vector<std::string> m_toolIds;
};

// ═════════════════════════════════════════════════════════════════
// SessionHistory — capped recent items and session records
// ═════════════════════════════════════════════════════════════════

class SessionHistory {
public:
    static constexpr int MAX_ITEMS   = 64;
    static constexpr int MAX_RECORDS = 32;

    // Recent items — prepend; dedup by path (move existing to front)
    void addItem(const RecentItem& item) {
        if (!item.isValid()) return;
        // Remove existing entry with same path
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const RecentItem& r) { return r.path == item.path; });
        if (it != m_items.end()) m_items.erase(it);
        m_items.insert(m_items.begin(), item);
        if ((int)m_items.size() > MAX_ITEMS)
            m_items.resize(MAX_ITEMS);
    }

    void removeItem(const std::string& path) {
        m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
            [&](const RecentItem& r) { return r.path == path; }), m_items.end());
    }

    const RecentItem* findItem(const std::string& path) const {
        for (auto& item : m_items)
            if (item.path == path) return &item;
        return nullptr;
    }

    const std::vector<RecentItem>& items() const { return m_items; }

    // Session records — append; capped at MAX_RECORDS
    void addRecord(const SessionRecord& record) {
        if (!record.isValid()) return;
        m_records.push_back(record);
        if ((int)m_records.size() > MAX_RECORDS)
            m_records.erase(m_records.begin());
    }

    const SessionRecord* findRecord(const std::string& id) const {
        for (auto& r : m_records)
            if (r.id() == id) return &r;
        return nullptr;
    }

    const std::vector<SessionRecord>& records() const { return m_records; }

    void clear() { m_items.clear(); m_records.clear(); }

private:
    std::vector<RecentItem>    m_items;
    std::vector<SessionRecord> m_records;
};

// ═════════════════════════════════════════════════════════════════
// SessionManager — workspace-scoped session lifecycle with observers
// ═════════════════════════════════════════════════════════════════

class SessionManager {
public:
    using Observer = std::function<void(SessionState)>;

    static constexpr int MAX_OBSERVERS = 16;

    SessionState state()     const { return m_state; }
    bool         isRunning() const { return m_state == SessionState::Running; }

    bool start(const std::string& name) {
        if (m_state == SessionState::Running) return false;
        m_current = SessionRecord(generateId(), name);
        m_current.setStartTime(m_clock++);
        setStateAndNotify(SessionState::Starting);
        setStateAndNotify(SessionState::Running);
        return true;
    }

    bool stop() {
        if (m_state != SessionState::Running) return false;
        setStateAndNotify(SessionState::Saving);
        m_current.setEndTime(m_clock++);
        m_current.setState(SessionState::Closed);
        m_history.addRecord(m_current);
        m_current = SessionRecord{};
        setStateAndNotify(SessionState::Closed);
        setStateAndNotify(SessionState::Idle);
        return true;
    }

    bool save() {
        if (m_state != SessionState::Running) return false;
        return true; // Simulated save
    }

    const SessionRecord& currentRecord() const { return m_current; }

    void addRecentItem(const RecentItem& item) { m_history.addItem(item); }

    const std::vector<RecentItem>& recentItems() const { return m_history.items(); }

    void clearRecent() {
        // Rebuild history without items
        std::vector<SessionRecord> recs = m_history.records();
        m_history.clear();
        for (auto& r : recs) m_history.addRecord(r);
    }

    const SessionHistory& history() const { return m_history; }

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(std::remove_if(m_observers.begin(), m_observers.end(),
            [id](const ObserverEntry& e) { return e.id == id; }), m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    void setStateAndNotify(SessionState s) {
        m_state = s;
        for (auto& e : m_observers) e.cb(s);
    }

    std::string generateId() {
        return "session_" + std::to_string(m_idCounter++);
    }

    SessionState              m_state         = SessionState::Idle;
    SessionRecord             m_current;
    SessionHistory            m_history;
    uint64_t                  m_clock         = 0;
    uint32_t                  m_idCounter     = 1;
    uint32_t                  m_nextObserverId = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
