#pragma once
// NF::Editor — Level design toolkit
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

enum class LevelDesignTool : uint8_t {
    Select, Place, Erase, Paint, Fill, Transform, Snap, Align
};

inline const char* levelDesignToolName(LevelDesignTool t) {
    switch (t) {
        case LevelDesignTool::Select:    return "Select";
        case LevelDesignTool::Place:     return "Place";
        case LevelDesignTool::Erase:     return "Erase";
        case LevelDesignTool::Paint:     return "Paint";
        case LevelDesignTool::Fill:      return "Fill";
        case LevelDesignTool::Transform: return "Transform";
        case LevelDesignTool::Snap:      return "Snap";
        case LevelDesignTool::Align:     return "Align";
    }
    return "Unknown";
}

enum class LevelObjectCategory : uint8_t {
    Structure, Prop, Light, Trigger, Spawn, Pickup, Hazard, Decoration
};

inline const char* levelObjectCategoryName(LevelObjectCategory c) {
    switch (c) {
        case LevelObjectCategory::Structure:  return "Structure";
        case LevelObjectCategory::Prop:       return "Prop";
        case LevelObjectCategory::Light:      return "Light";
        case LevelObjectCategory::Trigger:    return "Trigger";
        case LevelObjectCategory::Spawn:      return "Spawn";
        case LevelObjectCategory::Pickup:     return "Pickup";
        case LevelObjectCategory::Hazard:     return "Hazard";
        case LevelObjectCategory::Decoration: return "Decoration";
    }
    return "Unknown";
}

enum class LevelValidationResult : uint8_t {
    Valid, MissingSpawn, OverlappingGeometry, InvalidNavMesh, UnreachableArea, ExceededBudget
};

inline const char* levelValidationResultName(LevelValidationResult r) {
    switch (r) {
        case LevelValidationResult::Valid:               return "Valid";
        case LevelValidationResult::MissingSpawn:        return "MissingSpawn";
        case LevelValidationResult::OverlappingGeometry: return "OverlappingGeometry";
        case LevelValidationResult::InvalidNavMesh:      return "InvalidNavMesh";
        case LevelValidationResult::UnreachableArea:     return "UnreachableArea";
        case LevelValidationResult::ExceededBudget:      return "ExceededBudget";
    }
    return "Unknown";
}

class LevelObject {
public:
    explicit LevelObject(uint32_t id, const std::string& name, LevelObjectCategory cat)
        : m_id(id), m_name(name), m_category(cat) {}

    void setLocked(bool v)    { m_locked   = v; }
    void setVisible(bool v)   { m_visible  = v; }
    void setSelected(bool v)  { m_selected = v; }
    void setLayer(uint32_t v) { m_layer    = v; }

    [[nodiscard]] uint32_t               id()         const { return m_id;       }
    [[nodiscard]] const std::string&     name()       const { return m_name;     }
    [[nodiscard]] LevelObjectCategory    category()   const { return m_category; }
    [[nodiscard]] bool                   isLocked()   const { return m_locked;   }
    [[nodiscard]] bool                   isVisible()  const { return m_visible;  }
    [[nodiscard]] bool                   isSelected() const { return m_selected; }
    [[nodiscard]] uint32_t               layer()      const { return m_layer;    }

private:
    uint32_t              m_id;
    std::string           m_name;
    LevelObjectCategory   m_category;
    bool                  m_locked   = false;
    bool                  m_visible  = true;
    bool                  m_selected = false;
    uint32_t              m_layer    = 0;
};

class LevelDesignToolkit {
public:
    void setActiveTool(LevelDesignTool t)           { m_activeTool    = t; }
    void setSnapEnabled(bool v)                      { m_snapEnabled   = v; }
    void setGridSize(float v)                        { m_gridSize      = v; }
    void setValidationResult(LevelValidationResult r){ m_lastValidation = r; }

    bool addObject(const LevelObject& obj) {
        for (auto& o : m_objects) if (o.id() == obj.id()) return false;
        m_objects.push_back(obj); return true;
    }
    bool removeObject(uint32_t id) {
        auto it = std::find_if(m_objects.begin(), m_objects.end(),
            [&](const LevelObject& o){ return o.id() == id; });
        if (it == m_objects.end()) return false;
        m_objects.erase(it); return true;
    }
    [[nodiscard]] LevelObject* findObject(uint32_t id) {
        for (auto& o : m_objects) if (o.id() == id) return &o;
        return nullptr;
    }

    [[nodiscard]] LevelDesignTool      activeTool()      const { return m_activeTool;    }
    [[nodiscard]] bool                 isSnapEnabled()   const { return m_snapEnabled;   }
    [[nodiscard]] float                gridSize()        const { return m_gridSize;      }
    [[nodiscard]] LevelValidationResult lastValidation() const { return m_lastValidation;}
    [[nodiscard]] size_t               objectCount()     const { return m_objects.size();}

    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& o : m_objects) if (o.isSelected()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(LevelObjectCategory cat) const {
        size_t c = 0; for (auto& o : m_objects) if (o.category() == cat) ++c; return c;
    }

private:
    std::vector<LevelObject>  m_objects;
    LevelDesignTool           m_activeTool    = LevelDesignTool::Select;
    bool                      m_snapEnabled   = true;
    float                     m_gridSize      = 1.0f;
    LevelValidationResult     m_lastValidation = LevelValidationResult::Valid;
};

} // namespace NF
