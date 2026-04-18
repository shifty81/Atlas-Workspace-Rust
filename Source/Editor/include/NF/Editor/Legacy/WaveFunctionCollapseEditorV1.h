#pragma once
// NF::Editor — Wave function collapse editor v1: WFC tile and pattern management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wfcv1TileCategory  : uint8_t { Ground, Wall, Roof, Detail, Special, Empty };
enum class Wfcv1CollapseState : uint8_t { Superposition, Collapsed, Contradiction, Solved };

inline const char* wfcv1TileCategoryName(Wfcv1TileCategory c) {
    switch (c) {
        case Wfcv1TileCategory::Ground:  return "Ground";
        case Wfcv1TileCategory::Wall:    return "Wall";
        case Wfcv1TileCategory::Roof:    return "Roof";
        case Wfcv1TileCategory::Detail:  return "Detail";
        case Wfcv1TileCategory::Special: return "Special";
        case Wfcv1TileCategory::Empty:   return "Empty";
    }
    return "Unknown";
}

inline const char* wfcv1CollapseStateName(Wfcv1CollapseState s) {
    switch (s) {
        case Wfcv1CollapseState::Superposition:  return "Superposition";
        case Wfcv1CollapseState::Collapsed:      return "Collapsed";
        case Wfcv1CollapseState::Contradiction:  return "Contradiction";
        case Wfcv1CollapseState::Solved:         return "Solved";
    }
    return "Unknown";
}

struct Wfcv1Tile {
    uint64_t            id       = 0;
    std::string         name;
    Wfcv1TileCategory   category = Wfcv1TileCategory::Ground;
    Wfcv1CollapseState  state    = Wfcv1CollapseState::Superposition;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isCollapsed() const { return state == Wfcv1CollapseState::Collapsed; }
    [[nodiscard]] bool isSolved()    const { return state == Wfcv1CollapseState::Solved; }
};

struct Wfcv1Pattern {
    uint64_t    id     = 0;
    uint64_t    tileId = 0;
    std::string data;

    [[nodiscard]] bool isValid() const { return id != 0 && tileId != 0 && !data.empty(); }
};

using Wfcv1ChangeCallback = std::function<void(uint64_t)>;

class WaveFunctionCollapseEditorV1 {
public:
    static constexpr size_t MAX_TILES    = 2048;
    static constexpr size_t MAX_PATTERNS = 8192;

    bool addTile(const Wfcv1Tile& tile) {
        if (!tile.isValid()) return false;
        for (const auto& t : m_tiles) if (t.id == tile.id) return false;
        if (m_tiles.size() >= MAX_TILES) return false;
        m_tiles.push_back(tile);
        if (m_onChange) m_onChange(tile.id);
        return true;
    }

    bool removeTile(uint64_t id) {
        for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
            if (it->id == id) { m_tiles.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Wfcv1Tile* findTile(uint64_t id) {
        for (auto& t : m_tiles) if (t.id == id) return &t;
        return nullptr;
    }

    bool addPattern(const Wfcv1Pattern& pattern) {
        if (!pattern.isValid()) return false;
        for (const auto& p : m_patterns) if (p.id == pattern.id) return false;
        if (m_patterns.size() >= MAX_PATTERNS) return false;
        m_patterns.push_back(pattern);
        if (m_onChange) m_onChange(pattern.tileId);
        return true;
    }

    bool removePattern(uint64_t id) {
        for (auto it = m_patterns.begin(); it != m_patterns.end(); ++it) {
            if (it->id == id) { m_patterns.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t tileCount()    const { return m_tiles.size(); }
    [[nodiscard]] size_t patternCount() const { return m_patterns.size(); }

    [[nodiscard]] size_t collapsedCount() const {
        size_t c = 0; for (const auto& t : m_tiles) if (t.isCollapsed()) ++c; return c;
    }
    [[nodiscard]] size_t solvedCount() const {
        size_t c = 0; for (const auto& t : m_tiles) if (t.isSolved()) ++c; return c;
    }
    [[nodiscard]] size_t countByTileCategory(Wfcv1TileCategory category) const {
        size_t c = 0; for (const auto& t : m_tiles) if (t.category == category) ++c; return c;
    }

    void setOnChange(Wfcv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wfcv1Tile>    m_tiles;
    std::vector<Wfcv1Pattern> m_patterns;
    Wfcv1ChangeCallback       m_onChange;
};

} // namespace NF
