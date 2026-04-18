#pragma once
// NF::Editor — Scene streaming configuration editor
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

enum class StreamingCellType : uint8_t {
    StaticMesh, Landscape, Audio, Navigation, Foliage, Lighting
};

inline const char* streamingCellTypeName(StreamingCellType t) {
    switch (t) {
        case StreamingCellType::StaticMesh:  return "StaticMesh";
        case StreamingCellType::Landscape:   return "Landscape";
        case StreamingCellType::Audio:       return "Audio";
        case StreamingCellType::Navigation:  return "Navigation";
        case StreamingCellType::Foliage:     return "Foliage";
        case StreamingCellType::Lighting:    return "Lighting";
    }
    return "Unknown";
}

enum class StreamingLoadState : uint8_t {
    Unloaded, Loading, Loaded, Unloading, Error
};

inline const char* streamingLoadStateName(StreamingLoadState s) {
    switch (s) {
        case StreamingLoadState::Unloaded:  return "Unloaded";
        case StreamingLoadState::Loading:   return "Loading";
        case StreamingLoadState::Loaded:    return "Loaded";
        case StreamingLoadState::Unloading: return "Unloading";
        case StreamingLoadState::Error:     return "Error";
    }
    return "Unknown";
}

enum class StreamingPriority : uint8_t {
    Low, Normal, High, Critical
};

inline const char* streamingPriorityName(StreamingPriority p) {
    switch (p) {
        case StreamingPriority::Low:      return "Low";
        case StreamingPriority::Normal:   return "Normal";
        case StreamingPriority::High:     return "High";
        case StreamingPriority::Critical: return "Critical";
    }
    return "Unknown";
}

class StreamingCell {
public:
    explicit StreamingCell(const std::string& name, StreamingCellType type)
        : m_name(name), m_type(type) {}

    void setState(StreamingLoadState s)      { m_state    = s; }
    void setPriority(StreamingPriority p)    { m_priority = p; }
    void setLoadRadius(float r)              { m_loadRadius = r; }
    void setUnloadRadius(float r)            { m_unloadRadius = r; }
    void setEnabled(bool v)                  { m_enabled  = v; }

    [[nodiscard]] const std::string& name()         const { return m_name;        }
    [[nodiscard]] StreamingCellType  type()         const { return m_type;        }
    [[nodiscard]] StreamingLoadState state()        const { return m_state;       }
    [[nodiscard]] StreamingPriority  priority()     const { return m_priority;    }
    [[nodiscard]] float              loadRadius()   const { return m_loadRadius;  }
    [[nodiscard]] float              unloadRadius() const { return m_unloadRadius;}
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool               isLoaded()     const { return m_state == StreamingLoadState::Loaded; }

private:
    std::string        m_name;
    StreamingCellType  m_type;
    StreamingLoadState m_state       = StreamingLoadState::Unloaded;
    StreamingPriority  m_priority    = StreamingPriority::Normal;
    float              m_loadRadius  = 500.0f;
    float              m_unloadRadius = 600.0f;
    bool               m_enabled     = true;
};

class SceneStreaming {
public:
    static constexpr size_t MAX_CELLS = 256;

    [[nodiscard]] bool addCell(const StreamingCell& cell) {
        for (auto& c : m_cells) if (c.name() == cell.name()) return false;
        if (m_cells.size() >= MAX_CELLS) return false;
        m_cells.push_back(cell);
        return true;
    }

    [[nodiscard]] bool removeCell(const std::string& name) {
        for (auto it = m_cells.begin(); it != m_cells.end(); ++it) {
            if (it->name() == name) { m_cells.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] StreamingCell* findCell(const std::string& name) {
        for (auto& c : m_cells) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t cellCount()    const { return m_cells.size(); }
    [[nodiscard]] size_t loadedCount()  const {
        size_t c = 0; for (auto& cell : m_cells) if (cell.isLoaded()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(StreamingCellType t) const {
        size_t c = 0; for (auto& cell : m_cells) if (cell.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByState(StreamingLoadState s) const {
        size_t c = 0; for (auto& cell : m_cells) if (cell.state() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByPriority(StreamingPriority p) const {
        size_t c = 0; for (auto& cell : m_cells) if (cell.priority() == p) ++c; return c;
    }

private:
    std::vector<StreamingCell> m_cells;
};

} // namespace NF
