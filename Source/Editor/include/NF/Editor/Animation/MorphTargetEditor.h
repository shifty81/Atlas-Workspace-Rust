#pragma once
// NF::Editor — morph target editor
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

enum class MorphCategory : uint8_t {
    Expression, BodyShape, Facial, Corrective, Blend, Custom
};

inline const char* morphCategoryName(MorphCategory c) {
    switch (c) {
        case MorphCategory::Expression: return "Expression";
        case MorphCategory::BodyShape:  return "BodyShape";
        case MorphCategory::Facial:     return "Facial";
        case MorphCategory::Corrective: return "Corrective";
        case MorphCategory::Blend:      return "Blend";
        case MorphCategory::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class MorphBlendMode : uint8_t {
    Additive, Replace, Multiply, Average
};

inline const char* morphBlendModeName(MorphBlendMode m) {
    switch (m) {
        case MorphBlendMode::Additive:  return "Additive";
        case MorphBlendMode::Replace:   return "Replace";
        case MorphBlendMode::Multiply:  return "Multiply";
        case MorphBlendMode::Average:   return "Average";
    }
    return "Unknown";
}

class MorphTarget {
public:
    explicit MorphTarget(uint32_t id, const std::string& name, MorphCategory category)
        : m_id(id), m_name(name), m_category(category) {}

    void setBlendMode(MorphBlendMode v)  { m_blendMode   = v; }
    void setWeight(float v)              { m_weight      = v; }
    void setMinWeight(float v)           { m_minWeight   = v; }
    void setMaxWeight(float v)           { m_maxWeight   = v; }
    void setSymmetric(bool v)            { m_isSymmetric = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;         }
    [[nodiscard]] const std::string& name()        const { return m_name;       }
    [[nodiscard]] MorphCategory      category()    const { return m_category;   }
    [[nodiscard]] MorphBlendMode     blendMode()   const { return m_blendMode;  }
    [[nodiscard]] float              weight()      const { return m_weight;     }
    [[nodiscard]] float              minWeight()   const { return m_minWeight;  }
    [[nodiscard]] float              maxWeight()   const { return m_maxWeight;  }
    [[nodiscard]] bool               isSymmetric() const { return m_isSymmetric;}

private:
    uint32_t      m_id;
    std::string   m_name;
    MorphCategory m_category;
    MorphBlendMode m_blendMode   = MorphBlendMode::Additive;
    float          m_weight      = 0.0f;
    float          m_minWeight   = -1.0f;
    float          m_maxWeight   = 1.0f;
    bool           m_isSymmetric = false;
};

class MorphTargetEditor {
public:
    void setLivePreview(bool v)        { m_livePreview     = v; }
    void setShowMesh(bool v)           { m_showMesh        = v; }
    void setNormalize(bool v)          { m_normalize       = v; }
    void setPreviewFrameRate(float v)  { m_previewFrameRate = v; }

    bool addTarget(const MorphTarget& t) {
        for (auto& e : m_targets) if (e.id() == t.id()) return false;
        m_targets.push_back(t); return true;
    }
    bool removeTarget(uint32_t id) {
        auto it = std::find_if(m_targets.begin(), m_targets.end(),
            [&](const MorphTarget& e){ return e.id() == id; });
        if (it == m_targets.end()) return false;
        m_targets.erase(it); return true;
    }
    [[nodiscard]] MorphTarget* findTarget(uint32_t id) {
        for (auto& e : m_targets) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isLivePreview()      const { return m_livePreview;      }
    [[nodiscard]] bool   isShowMesh()         const { return m_showMesh;         }
    [[nodiscard]] bool   isNormalize()        const { return m_normalize;        }
    [[nodiscard]] float  previewFrameRate()   const { return m_previewFrameRate; }
    [[nodiscard]] size_t targetCount()        const { return m_targets.size();   }

    [[nodiscard]] size_t countByCategory(MorphCategory c) const {
        size_t n = 0; for (auto& e : m_targets) if (e.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByBlendMode(MorphBlendMode m) const {
        size_t n = 0; for (auto& e : m_targets) if (e.blendMode() == m) ++n; return n;
    }

private:
    std::vector<MorphTarget> m_targets;
    bool  m_livePreview      = true;
    bool  m_showMesh         = true;
    bool  m_normalize        = false;
    float m_previewFrameRate = 30.0f;
};

} // namespace NF
