#pragma once
// NF::Editor — LOD editor v1: LOD level and threshold management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lodev1LodStrategy : uint8_t { Distance, ScreenSize, Manual, Automatic };
enum class Lodev1LodState    : uint8_t { Inactive, Active, Override, Disabled };

inline const char* lodev1LodStrategyName(Lodev1LodStrategy s) {
    switch (s) {
        case Lodev1LodStrategy::Distance:   return "Distance";
        case Lodev1LodStrategy::ScreenSize: return "ScreenSize";
        case Lodev1LodStrategy::Manual:     return "Manual";
        case Lodev1LodStrategy::Automatic:  return "Automatic";
    }
    return "Unknown";
}

inline const char* lodev1LodStateName(Lodev1LodState s) {
    switch (s) {
        case Lodev1LodState::Inactive: return "Inactive";
        case Lodev1LodState::Active:   return "Active";
        case Lodev1LodState::Override: return "Override";
        case Lodev1LodState::Disabled: return "Disabled";
    }
    return "Unknown";
}

struct Lodev1Level {
    uint64_t           id         = 0;
    std::string        name;
    Lodev1LodStrategy  strategy   = Lodev1LodStrategy::Distance;
    Lodev1LodState     state      = Lodev1LodState::Inactive;
    float              threshold  = 0.0f;
    uint32_t           polyCount  = 0;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Lodev1LodState::Active; }
    [[nodiscard]] bool isOverride() const { return state == Lodev1LodState::Override; }
    [[nodiscard]] bool isDisabled() const { return state == Lodev1LodState::Disabled; }
};

struct Lodev1Group {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Lodev1ChangeCallback = std::function<void(uint64_t)>;

class LODEditorV1 {
public:
    static constexpr size_t MAX_LEVELS = 256;
    static constexpr size_t MAX_GROUPS = 64;

    bool addLevel(const Lodev1Level& level) {
        if (!level.isValid()) return false;
        for (const auto& l : m_levels) if (l.id == level.id) return false;
        if (m_levels.size() >= MAX_LEVELS) return false;
        m_levels.push_back(level);
        if (m_onChange) m_onChange(level.id);
        return true;
    }

    bool removeLevel(uint64_t id) {
        for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
            if (it->id == id) { m_levels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lodev1Level* findLevel(uint64_t id) {
        for (auto& l : m_levels) if (l.id == id) return &l;
        return nullptr;
    }

    bool addGroup(const Lodev1Group& group) {
        if (!group.isValid()) return false;
        for (const auto& g : m_groups) if (g.id == group.id) return false;
        if (m_groups.size() >= MAX_GROUPS) return false;
        m_groups.push_back(group);
        return true;
    }

    bool removeGroup(uint64_t id) {
        for (auto it = m_groups.begin(); it != m_groups.end(); ++it) {
            if (it->id == id) { m_groups.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t levelCount() const { return m_levels.size(); }
    [[nodiscard]] size_t groupCount() const { return m_groups.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& l : m_levels) if (l.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t countByStrategy(Lodev1LodStrategy strategy) const {
        size_t c = 0; for (const auto& l : m_levels) if (l.strategy == strategy) ++c; return c;
    }
    [[nodiscard]] uint32_t totalPolyCount() const {
        uint32_t sum = 0; for (const auto& l : m_levels) sum += l.polyCount; return sum;
    }

    void setOnChange(Lodev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lodev1Level> m_levels;
    std::vector<Lodev1Group> m_groups;
    Lodev1ChangeCallback     m_onChange;
};

} // namespace NF
