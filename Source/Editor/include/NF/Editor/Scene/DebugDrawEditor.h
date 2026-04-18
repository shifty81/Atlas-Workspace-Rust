#pragma once
// NF::Editor — In-world debug draw overlay editor
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

enum class DebugDrawPrimitiveType : uint8_t {
    Line, Arrow, Box, Sphere, Capsule, Cylinder, Cone, Text, Grid, Axis
};

inline const char* debugDrawPrimitiveTypeName(DebugDrawPrimitiveType t) {
    switch (t) {
        case DebugDrawPrimitiveType::Line:     return "Line";
        case DebugDrawPrimitiveType::Arrow:    return "Arrow";
        case DebugDrawPrimitiveType::Box:      return "Box";
        case DebugDrawPrimitiveType::Sphere:   return "Sphere";
        case DebugDrawPrimitiveType::Capsule:  return "Capsule";
        case DebugDrawPrimitiveType::Cylinder: return "Cylinder";
        case DebugDrawPrimitiveType::Cone:     return "Cone";
        case DebugDrawPrimitiveType::Text:     return "Text";
        case DebugDrawPrimitiveType::Grid:     return "Grid";
        case DebugDrawPrimitiveType::Axis:     return "Axis";
    }
    return "Unknown";
}

enum class DebugDrawLifetime : uint8_t {
    Frame, Persistent, Timed, OnSelected
};

inline const char* debugDrawLifetimeName(DebugDrawLifetime l) {
    switch (l) {
        case DebugDrawLifetime::Frame:      return "Frame";
        case DebugDrawLifetime::Persistent: return "Persistent";
        case DebugDrawLifetime::Timed:      return "Timed";
        case DebugDrawLifetime::OnSelected: return "OnSelected";
    }
    return "Unknown";
}

enum class DebugDrawCategory : uint8_t {
    Physics, AI, Navigation, Animation, UI, Network, Gameplay, Custom
};

inline const char* debugDrawCategoryName(DebugDrawCategory c) {
    switch (c) {
        case DebugDrawCategory::Physics:    return "Physics";
        case DebugDrawCategory::AI:         return "AI";
        case DebugDrawCategory::Navigation: return "Navigation";
        case DebugDrawCategory::Animation:  return "Animation";
        case DebugDrawCategory::UI:         return "UI";
        case DebugDrawCategory::Network:    return "Network";
        case DebugDrawCategory::Gameplay:   return "Gameplay";
        case DebugDrawCategory::Custom:     return "Custom";
    }
    return "Unknown";
}

class DebugDrawEntry {
public:
    explicit DebugDrawEntry(const std::string& name,
                             DebugDrawPrimitiveType type,
                             DebugDrawCategory category)
        : m_name(name), m_type(type), m_category(category) {}

    void setLifetime(DebugDrawLifetime l) { m_lifetime = l; }
    void setDuration(float d)             { m_duration = d; }
    void setThickness(float t)            { m_thickness = t; }
    void setEnabled(bool v)               { m_enabled  = v; }
    void setDepthTest(bool v)             { m_depthTest = v; }

    [[nodiscard]] const std::string&    name()      const { return m_name;      }
    [[nodiscard]] DebugDrawPrimitiveType type()      const { return m_type;      }
    [[nodiscard]] DebugDrawCategory     category()  const { return m_category;  }
    [[nodiscard]] DebugDrawLifetime     lifetime()  const { return m_lifetime;  }
    [[nodiscard]] float                 duration()  const { return m_duration;  }
    [[nodiscard]] float                 thickness() const { return m_thickness; }
    [[nodiscard]] bool                  isEnabled() const { return m_enabled;   }
    [[nodiscard]] bool                  depthTest() const { return m_depthTest; }

private:
    std::string            m_name;
    DebugDrawPrimitiveType m_type;
    DebugDrawCategory      m_category;
    DebugDrawLifetime      m_lifetime  = DebugDrawLifetime::Frame;
    float                  m_duration  = 0.0f;
    float                  m_thickness = 1.0f;
    bool                   m_enabled   = true;
    bool                   m_depthTest = true;
};

class DebugDrawEditor {
public:
    static constexpr size_t MAX_ENTRIES = 512;

    [[nodiscard]] bool addEntry(const DebugDrawEntry& entry) {
        for (auto& e : m_entries) if (e.name() == entry.name()) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        return true;
    }

    [[nodiscard]] bool removeEntry(const std::string& name) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->name() == name) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DebugDrawEntry* findEntry(const std::string& name) {
        for (auto& e : m_entries) if (e.name() == name) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t entryCount()   const { return m_entries.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(DebugDrawCategory cat) const {
        size_t c = 0; for (auto& e : m_entries) if (e.category() == cat) ++c; return c;
    }
    [[nodiscard]] size_t countByType(DebugDrawPrimitiveType t) const {
        size_t c = 0; for (auto& e : m_entries) if (e.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByLifetime(DebugDrawLifetime l) const {
        size_t c = 0; for (auto& e : m_entries) if (e.lifetime() == l) ++c; return c;
    }

    void setVisible(bool v)                     { m_visible   = v; }
    [[nodiscard]] bool isVisible() const        { return m_visible; }

private:
    std::vector<DebugDrawEntry> m_entries;
    bool                        m_visible = true;
};

} // namespace NF
