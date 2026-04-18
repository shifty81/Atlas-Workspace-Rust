#pragma once
// NF::Editor — Level stream editor v1: streaming level/chunk management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lsv1StreamState : uint8_t { Unloaded, Loading, Loaded, Streaming, Error };
enum class Lsv1Priority    : uint8_t { Critical, High, Normal, Low, Background };

inline const char* lsv1StreamStateName(Lsv1StreamState s) {
    switch (s) {
        case Lsv1StreamState::Unloaded:  return "Unloaded";
        case Lsv1StreamState::Loading:   return "Loading";
        case Lsv1StreamState::Loaded:    return "Loaded";
        case Lsv1StreamState::Streaming: return "Streaming";
        case Lsv1StreamState::Error:     return "Error";
    }
    return "Unknown";
}

inline const char* lsv1PriorityName(Lsv1Priority p) {
    switch (p) {
        case Lsv1Priority::Critical:   return "Critical";
        case Lsv1Priority::High:       return "High";
        case Lsv1Priority::Normal:     return "Normal";
        case Lsv1Priority::Low:        return "Low";
        case Lsv1Priority::Background: return "Background";
    }
    return "Unknown";
}

struct Lsv1Chunk {
    uint64_t        id        = 0;
    std::string     name;
    Lsv1StreamState state     = Lsv1StreamState::Unloaded;
    Lsv1Priority    priority  = Lsv1Priority::Normal;
    float           boundsX   = 0.f;
    float           boundsY   = 0.f;
    float           boundsZ   = 0.f;
    float           radius    = 100.f;
    float           sizeMB    = 0.f;   // estimated memory footprint
    std::vector<uint64_t> dependencies; // other chunk ids this depends on

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isLoaded() const { return state == Lsv1StreamState::Loaded; }
};

using Lsv1ChangeCallback = std::function<void(uint64_t)>;

class LevelStreamEditorV1 {
public:
    static constexpr size_t MAX_CHUNKS = 256;

    bool addChunk(const Lsv1Chunk& chunk) {
        if (!chunk.isValid()) return false;
        for (const auto& c : m_chunks) if (c.id == chunk.id) return false;
        if (m_chunks.size() >= MAX_CHUNKS) return false;
        m_chunks.push_back(chunk);
        if (m_onChange) m_onChange(chunk.id);
        return true;
    }

    bool removeChunk(uint64_t id) {
        for (auto it = m_chunks.begin(); it != m_chunks.end(); ++it) {
            if (it->id == id) { m_chunks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lsv1Chunk* findChunk(uint64_t id) {
        for (auto& c : m_chunks) if (c.id == id) return &c;
        return nullptr;
    }

    bool setState(uint64_t id, Lsv1StreamState state) {
        auto* c = findChunk(id);
        if (!c) return false;
        c->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setPriority(uint64_t id, Lsv1Priority priority) {
        auto* c = findChunk(id);
        if (!c) return false;
        c->priority = priority;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool addDependency(uint64_t chunkId, uint64_t depId) {
        auto* c = findChunk(chunkId);
        if (!c) return false;
        for (auto d : c->dependencies) if (d == depId) return false;
        c->dependencies.push_back(depId);
        return true;
    }

    [[nodiscard]] size_t chunkCount() const { return m_chunks.size(); }

    [[nodiscard]] size_t loadedCount() const {
        size_t c = 0;
        for (const auto& ch : m_chunks) if (ch.isLoaded()) ++c;
        return c;
    }

    [[nodiscard]] float totalSizeMB() const {
        float total = 0.f;
        for (const auto& ch : m_chunks) total += ch.sizeMB;
        return total;
    }

    [[nodiscard]] size_t countByPriority(Lsv1Priority prio) const {
        size_t c = 0;
        for (const auto& ch : m_chunks) if (ch.priority == prio) ++c;
        return c;
    }

    void setOnChange(Lsv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lsv1Chunk> m_chunks;
    Lsv1ChangeCallback     m_onChange;
};

} // namespace NF
