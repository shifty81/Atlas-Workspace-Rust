#pragma once
// NF::Workspace — Phase 22: Workspace Drag and Drop System
//
// Workspace-level drag-and-drop coordination:
//   DragPayloadType — payload format classification enum
//   DragPayload     — typed content carrier; isValid(), equality
//   DragSession     — lifecycle state machine (Idle/Active/Hovering/Dropped/Cancelled)
//   DropZone        — named zone with accepted-type mask and tryAccept
//   DragDropManager — session orchestrator with zone registry and observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// DragPayloadType — payload format classification
// ═════════════════════════════════════════════════════════════════

enum class DragPayloadType : uint8_t {
    None   = 0,
    Text   = 1,
    Path   = 2,
    Asset  = 3,
    Entity = 4,
    Json   = 5,
    Custom = 6,
};

inline const char* dragPayloadTypeName(DragPayloadType t) {
    switch (t) {
        case DragPayloadType::None:   return "None";
        case DragPayloadType::Text:   return "Text";
        case DragPayloadType::Path:   return "Path";
        case DragPayloadType::Asset:  return "Asset";
        case DragPayloadType::Entity: return "Entity";
        case DragPayloadType::Json:   return "Json";
        case DragPayloadType::Custom: return "Custom";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// DragPayload — typed content carrier
// ═════════════════════════════════════════════════════════════════

struct DragPayload {
    DragPayloadType type    = DragPayloadType::None;
    std::string     content;

    bool isValid() const { return type != DragPayloadType::None && !content.empty(); }

    bool operator==(const DragPayload& o) const {
        return type == o.type && content == o.content;
    }
    bool operator!=(const DragPayload& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// DragSessionState — drag lifecycle state
// ═════════════════════════════════════════════════════════════════

enum class DragSessionState : uint8_t {
    Idle,
    Active,
    Hovering,
    Dropped,
    Cancelled,
};

inline const char* dragSessionStateName(DragSessionState s) {
    switch (s) {
        case DragSessionState::Idle:      return "Idle";
        case DragSessionState::Active:    return "Active";
        case DragSessionState::Hovering:  return "Hovering";
        case DragSessionState::Dropped:   return "Dropped";
        case DragSessionState::Cancelled: return "Cancelled";
        default:                          return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// DragSession — lifecycle state machine
// ═════════════════════════════════════════════════════════════════

class DragSession {
public:
    DragSession() = default;
    explicit DragSession(const DragPayload& payload, std::string sourceZoneId = {})
        : m_payload(payload), m_sourceZoneId(std::move(sourceZoneId)) {}

    bool begin() {
        if (m_state != DragSessionState::Idle) return false;
        if (!m_payload.isValid()) return false;
        m_state = DragSessionState::Active;
        return true;
    }

    bool setHovering(const std::string& zoneId) {
        if (m_state != DragSessionState::Active && m_state != DragSessionState::Hovering)
            return false;
        m_state = DragSessionState::Hovering;
        m_hoverZoneId = zoneId;
        return true;
    }

    bool drop() {
        if (m_state != DragSessionState::Active && m_state != DragSessionState::Hovering)
            return false;
        m_state = DragSessionState::Dropped;
        return true;
    }

    bool cancel() {
        if (m_state == DragSessionState::Dropped || m_state == DragSessionState::Cancelled)
            return false;
        m_state = DragSessionState::Cancelled;
        return true;
    }

    bool isActive()    const { return m_state == DragSessionState::Active || m_state == DragSessionState::Hovering; }
    bool isCompleted() const { return m_state == DragSessionState::Dropped || m_state == DragSessionState::Cancelled; }

    DragSessionState        state()        const { return m_state; }
    const DragPayload&      payload()      const { return m_payload; }
    const std::string&      sourceZoneId() const { return m_sourceZoneId; }
    const std::string&      hoverZoneId()  const { return m_hoverZoneId; }

    void reset() {
        m_state       = DragSessionState::Idle;
        m_hoverZoneId.clear();
    }

private:
    DragPayload      m_payload;
    std::string      m_sourceZoneId;
    std::string      m_hoverZoneId;
    DragSessionState m_state = DragSessionState::Idle;
};

// ═════════════════════════════════════════════════════════════════
// DropZone — named zone with accepted-type mask
// ═════════════════════════════════════════════════════════════════

class DropZone {
public:
    DropZone() = default;
    DropZone(std::string id, std::string label, uint8_t acceptMask = 0xFF)
        : m_id(std::move(id)), m_label(std::move(label)), m_acceptMask(acceptMask) {}

    bool isValid() const { return !m_id.empty(); }

    const std::string& id()    const { return m_id; }
    const std::string& label() const { return m_label; }
    uint8_t            acceptMask() const { return m_acceptMask; }

    bool accepts(DragPayloadType type) const {
        return (m_acceptMask & (1u << static_cast<uint8_t>(type))) != 0;
    }

    void setAcceptMask(uint8_t mask) { m_acceptMask = mask; }

    bool tryAccept(DragSession& session) {
        if (!session.isActive()) return false;
        if (!accepts(session.payload().type)) return false;
        session.setHovering(m_id);
        m_lastAccepted = session.payload();
        m_acceptCount++;
        return true;
    }

    const DragPayload& lastAccepted() const { return m_lastAccepted; }
    int                acceptCount()  const { return m_acceptCount; }

    void clear() {
        m_lastAccepted = {};
        m_acceptCount  = 0;
    }

private:
    std::string m_id;
    std::string m_label;
    uint8_t     m_acceptMask  = 0xFF;
    DragPayload m_lastAccepted;
    int         m_acceptCount = 0;
};

// ═════════════════════════════════════════════════════════════════
// DragDropManager — session orchestrator with zone registry
// ═════════════════════════════════════════════════════════════════

class DragDropManager {
public:
    using Observer = std::function<void(const DragSession&)>;

    static constexpr int MAX_ZONES     = 64;
    static constexpr int MAX_OBSERVERS = 16;

    // Zone registry ────────────────────────────────────────────

    bool registerZone(const DropZone& zone) {
        if (!zone.isValid()) return false;
        if (findZone(zone.id())) return false;
        if ((int)m_zones.size() >= MAX_ZONES) return false;
        m_zones.push_back(zone);
        return true;
    }

    bool unregisterZone(const std::string& id) {
        auto it = std::find_if(m_zones.begin(), m_zones.end(),
            [&](const DropZone& z) { return z.id() == id; });
        if (it == m_zones.end()) return false;
        m_zones.erase(it);
        return true;
    }

    DropZone* findZone(const std::string& id) {
        for (auto& z : m_zones)
            if (z.id() == id) return &z;
        return nullptr;
    }

    const DropZone* findZone(const std::string& id) const {
        for (auto& z : m_zones)
            if (z.id() == id) return &z;
        return nullptr;
    }

    std::vector<std::string> allZoneIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_zones.size());
        for (auto& z : m_zones) ids.push_back(z.id());
        return ids;
    }

    // Drag session lifecycle ────────────────────────────────────

    bool beginDrag(const DragPayload& payload, const std::string& sourceZoneId = {}) {
        if (m_session.isActive()) return false;
        m_session = DragSession(payload, sourceZoneId);
        bool ok = m_session.begin();
        if (ok) notifyObservers();
        return ok;
    }

    bool cancelDrag() {
        if (!m_session.isActive()) return false;
        bool ok = m_session.cancel();
        if (ok) notifyObservers();
        return ok;
    }

    bool commitDrop(const std::string& targetZoneId) {
        if (!m_session.isActive()) return false;
        DropZone* zone = findZone(targetZoneId);
        if (!zone) return false;
        if (!zone->tryAccept(m_session)) return false;
        bool ok = m_session.drop();
        if (ok) {
            notifyObservers();
            m_dropCount++;
        }
        return ok;
    }

    bool hasActiveSession() const { return m_session.isActive(); }
    const DragSession& activeSession() const { return m_session; }

    int dropCount() const { return m_dropCount; }

    void clear() {
        if (m_session.isActive()) m_session.cancel();
        m_session = {};
        m_zones.clear();
        m_dropCount = 0;
    }

    // Observers ────────────────────────────────────────────────

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(
            std::remove_if(m_observers.begin(), m_observers.end(),
                [id](const ObserverEntry& e) { return e.id == id; }),
            m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    void notifyObservers() {
        for (auto& e : m_observers) e.cb(m_session);
    }

    std::vector<DropZone>     m_zones;
    DragSession               m_session;
    int                       m_dropCount       = 0;
    uint32_t                  m_nextObserverId  = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
