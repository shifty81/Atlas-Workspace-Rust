#pragma once
// NF::Workspace — Phase 20: Workspace Clipboard System
//
// Multi-channel workspace clipboard infrastructure:
//   ClipboardFormat  — typed clipboard format enum
//   ClipboardEntry   — typed clip with format/data/timestamp
//   ClipboardBuffer  — newest-first ring buffer of entries
//   ClipboardChannel — named channel wrapping a buffer
//   ClipboardManager — workspace-scoped multi-channel clipboard with observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// ClipboardFormat — typed clipboard format
// ═════════════════════════════════════════════════════════════════

enum class ClipboardFormat : uint8_t {
    None     = 0,
    Text     = 1,
    RichText = 2,
    Path     = 3,
    EntityId = 4,
    JsonBlob = 5,
    Binary   = 6,
    Custom   = 7,
};

inline const char* clipboardFormatName(ClipboardFormat f) {
    switch (f) {
        case ClipboardFormat::None:     return "None";
        case ClipboardFormat::Text:     return "Text";
        case ClipboardFormat::RichText: return "RichText";
        case ClipboardFormat::Path:     return "Path";
        case ClipboardFormat::EntityId: return "EntityId";
        case ClipboardFormat::JsonBlob: return "JsonBlob";
        case ClipboardFormat::Binary:   return "Binary";
        case ClipboardFormat::Custom:   return "Custom";
        default:                        return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ClipboardEntry — typed clip with format/data/timestamp
// ═════════════════════════════════════════════════════════════════

struct ClipboardEntry {
    ClipboardFormat format    = ClipboardFormat::None;
    std::string     data;
    uint64_t        timestamp = 0;

    bool isValid()  const { return format != ClipboardFormat::None; }
    bool isEmpty()  const { return data.empty(); }

    bool operator==(const ClipboardEntry& o) const {
        return format == o.format && data == o.data;
    }
    bool operator!=(const ClipboardEntry& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// ClipboardBuffer — newest-first ring buffer
// ═════════════════════════════════════════════════════════════════

class ClipboardBuffer {
public:
    static constexpr int MAX_SLOTS = 32;

    explicit ClipboardBuffer(int capacity = MAX_SLOTS)
        : m_capacity(std::min(std::max(capacity, 1), MAX_SLOTS)) {}

    bool push(const ClipboardEntry& entry) {
        if (!entry.isValid()) return false;
        m_slots.insert(m_slots.begin(), entry);
        if ((int)m_slots.size() > m_capacity)
            m_slots.resize(m_capacity);
        return true;
    }

    const ClipboardEntry* peek() const {
        return m_slots.empty() ? nullptr : &m_slots.front();
    }

    const ClipboardEntry* peekAt(int index) const {
        if (index < 0 || index >= (int)m_slots.size()) return nullptr;
        return &m_slots[index];
    }

    bool pop() {
        if (m_slots.empty()) return false;
        m_slots.erase(m_slots.begin());
        return true;
    }

    int  count()    const { return (int)m_slots.size(); }
    int  capacity() const { return m_capacity; }
    void clear()          { m_slots.clear(); }

private:
    int                        m_capacity;
    std::vector<ClipboardEntry> m_slots;
};

// ═════════════════════════════════════════════════════════════════
// ClipboardChannel — named channel wrapping a buffer
// ═════════════════════════════════════════════════════════════════

class ClipboardChannel {
public:
    ClipboardChannel() = default;
    explicit ClipboardChannel(std::string name, int capacity = 8)
        : m_name(std::move(name)), m_buffer(capacity) {}

    const std::string& name() const { return m_name; }
    bool isValid() const { return !m_name.empty(); }

    bool push(const ClipboardEntry& entry) { return m_buffer.push(entry); }
    const ClipboardEntry* peek() const     { return m_buffer.peek(); }
    bool pop()                             { return m_buffer.pop(); }
    int  count() const                     { return m_buffer.count(); }
    void clear()                           { m_buffer.clear(); }

private:
    std::string    m_name;
    ClipboardBuffer m_buffer{8};
};

// ═════════════════════════════════════════════════════════════════
// ClipboardManager — workspace-scoped multi-channel clipboard
// ═════════════════════════════════════════════════════════════════

class ClipboardManager {
public:
    using Observer = std::function<void(const std::string& channel, const ClipboardEntry&)>;

    static constexpr int MAX_CHANNELS  = 16;
    static constexpr int MAX_OBSERVERS = 16;

    bool registerChannel(const std::string& name, int capacity = 8) {
        if (name.empty()) return false;
        if (isRegistered(name)) return false;
        if ((int)m_channels.size() >= MAX_CHANNELS) return false;
        m_channels.emplace_back(name, capacity);
        return true;
    }

    bool unregisterChannel(const std::string& name) {
        auto it = findChannelIt(name);
        if (it == m_channels.end()) return false;
        m_channels.erase(it);
        return true;
    }

    bool isRegistered(const std::string& name) const {
        return findChannelConst(name) != nullptr;
    }

    ClipboardChannel* findChannel(const std::string& name) {
        auto it = findChannelIt(name);
        return (it != m_channels.end()) ? &*it : nullptr;
    }

    bool push(const std::string& channelName, const ClipboardEntry& entry) {
        auto* ch = findChannel(channelName);
        if (!ch) return false;
        bool ok = ch->push(entry);
        if (ok) notifyObservers(channelName, entry);
        return ok;
    }

    const ClipboardEntry* peek(const std::string& channelName) const {
        auto* ch = findChannelConst(channelName);
        return ch ? ch->peek() : nullptr;
    }

    bool pop(const std::string& channelName) {
        auto* ch = findChannel(channelName);
        return ch ? ch->pop() : false;
    }

    bool copyText(const std::string& ch, const std::string& text) {
        return push(ch, {ClipboardFormat::Text, text, 0});
    }

    bool copyPath(const std::string& ch, const std::string& path) {
        return push(ch, {ClipboardFormat::Path, path, 0});
    }

    bool copyEntity(const std::string& ch, const std::string& entityId) {
        return push(ch, {ClipboardFormat::EntityId, entityId, 0});
    }

    bool copyJson(const std::string& ch, const std::string& json) {
        return push(ch, {ClipboardFormat::JsonBlob, json, 0});
    }

    std::vector<std::string> allChannels() const {
        std::vector<std::string> names;
        names.reserve(m_channels.size());
        for (auto& ch : m_channels) names.push_back(ch.name());
        return names;
    }

    void clear() { m_channels.clear(); }

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

    std::vector<ClipboardChannel>::iterator findChannelIt(const std::string& name) {
        return std::find_if(m_channels.begin(), m_channels.end(),
            [&](const ClipboardChannel& ch) { return ch.name() == name; });
    }

    const ClipboardChannel* findChannelConst(const std::string& name) const {
        for (auto& ch : m_channels)
            if (ch.name() == name) return &ch;
        return nullptr;
    }

    // Non-const version that returns pointer (used internally by peek)
    ClipboardChannel* findChannelConst2(const std::string& name) {
        auto it = findChannelIt(name);
        return (it != m_channels.end()) ? &*it : nullptr;
    }

    void notifyObservers(const std::string& channelName, const ClipboardEntry& entry) {
        for (auto& e : m_observers) e.cb(channelName, entry);
    }

    std::vector<ClipboardChannel> m_channels;
    uint32_t                      m_nextObserverId = 0;
    std::vector<ObserverEntry>    m_observers;
};

// Fix: peek needs const access — add const overload helper
// ClipboardChannel peek() is already const so findChannelConst works fine.

} // namespace NF
