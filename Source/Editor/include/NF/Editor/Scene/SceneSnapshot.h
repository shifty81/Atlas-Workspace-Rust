#pragma once
// NF::Editor — Scene snapshot history + system
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class SceneSnapshotType : uint8_t {
    Full      = 0,
    Delta     = 1,
    Lighting  = 2,
    Physics   = 3,
    AI        = 4,
    Audio     = 5,
    Visual    = 6,
    Meta      = 7,
};

inline const char* sceneSnapshotTypeName(SceneSnapshotType t) {
    switch (t) {
        case SceneSnapshotType::Full:     return "Full";
        case SceneSnapshotType::Delta:    return "Delta";
        case SceneSnapshotType::Lighting: return "Lighting";
        case SceneSnapshotType::Physics:  return "Physics";
        case SceneSnapshotType::AI:       return "AI";
        case SceneSnapshotType::Audio:    return "Audio";
        case SceneSnapshotType::Visual:   return "Visual";
        case SceneSnapshotType::Meta:     return "Meta";
        default:                          return "Unknown";
    }
}

enum class SceneSnapshotState : uint8_t {
    Valid     = 0,
    Outdated  = 1,
    Corrupted = 2,
    Partial   = 3,
};

struct SceneSnapshotFrame {
    std::string        id;
    std::string        label;
    SceneSnapshotType  type  = SceneSnapshotType::Full;
    SceneSnapshotState state = SceneSnapshotState::Valid;
    uint64_t           timestamp = 0;
    size_t             dataSize  = 0;   // bytes captured

    [[nodiscard]] bool isValid()     const { return state == SceneSnapshotState::Valid;     }
    [[nodiscard]] bool isOutdated()  const { return state == SceneSnapshotState::Outdated;  }
    [[nodiscard]] bool isCorrupted() const { return state == SceneSnapshotState::Corrupted; }
    [[nodiscard]] bool isPartial()   const { return state == SceneSnapshotState::Partial;   }

    void markOutdated()  { if (state == SceneSnapshotState::Valid) state = SceneSnapshotState::Outdated;  }
    void markCorrupted() { state = SceneSnapshotState::Corrupted; }
};

class SceneSnapshotHistory {
public:
    static constexpr size_t MAX_FRAMES = 128;

    bool push(const SceneSnapshotFrame& frame) {
        if (m_frames.size() >= MAX_FRAMES) return false;
        for (auto& f : m_frames) if (f.id == frame.id) return false;
        m_frames.push_back(frame);
        return true;
    }

    bool remove(const std::string& id) {
        for (auto it = m_frames.begin(); it != m_frames.end(); ++it) {
            if (it->id == id) { m_frames.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SceneSnapshotFrame* find(const std::string& id) {
        for (auto& f : m_frames) if (f.id == id) return &f;
        return nullptr;
    }

    [[nodiscard]] const SceneSnapshotFrame* find(const std::string& id) const {
        for (auto& f : m_frames) if (f.id == id) return &f;
        return nullptr;
    }

    [[nodiscard]] SceneSnapshotFrame* latest() {
        if (m_frames.empty()) return nullptr;
        return &m_frames.back();
    }

    void markAllOutdated() {
        for (auto& f : m_frames) f.markOutdated();
    }

    [[nodiscard]] size_t frameCount()     const { return m_frames.size(); }
    [[nodiscard]] bool   empty()          const { return m_frames.empty(); }

    [[nodiscard]] size_t validCount() const {
        size_t c = 0;
        for (auto& f : m_frames) if (f.isValid()) c++;
        return c;
    }

    [[nodiscard]] size_t corruptedCount() const {
        size_t c = 0;
        for (auto& f : m_frames) if (f.isCorrupted()) c++;
        return c;
    }

    [[nodiscard]] size_t totalDataSize() const {
        size_t total = 0;
        for (auto& f : m_frames) total += f.dataSize;
        return total;
    }

    [[nodiscard]] const std::vector<SceneSnapshotFrame>& frames() const { return m_frames; }

private:
    std::vector<SceneSnapshotFrame> m_frames;
};

class SceneSnapshotSystem {
public:
    void init()     { m_initialized = true;  m_history = SceneSnapshotHistory{}; }
    void shutdown() { m_initialized = false; m_history = SceneSnapshotHistory{}; }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool capture(const SceneSnapshotFrame& frame) {
        if (!m_initialized) return false;
        return m_history.push(frame);
    }

    bool discard(const std::string& id) {
        if (!m_initialized) return false;
        return m_history.remove(id);
    }

    [[nodiscard]] SceneSnapshotFrame* find(const std::string& id) {
        return m_history.find(id);
    }

    [[nodiscard]] SceneSnapshotFrame* latest() {
        return m_history.latest();
    }

    void invalidateAll() {
        m_history.markAllOutdated();
    }

    [[nodiscard]] size_t frameCount()     const { return m_history.frameCount();     }
    [[nodiscard]] size_t validCount()     const { return m_history.validCount();     }
    [[nodiscard]] size_t corruptedCount() const { return m_history.corruptedCount(); }
    [[nodiscard]] size_t totalDataSize()  const { return m_history.totalDataSize();  }

    [[nodiscard]] SceneSnapshotHistory&       history()       { return m_history; }
    [[nodiscard]] const SceneSnapshotHistory& history() const { return m_history; }

private:
    SceneSnapshotHistory m_history;
    bool                 m_initialized = false;
};

// ============================================================
// S20 — Resource Monitor System
// ============================================================


} // namespace NF
