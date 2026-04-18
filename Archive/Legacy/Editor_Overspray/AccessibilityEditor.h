#pragma once
// NF::Editor — accessibility editor
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

enum class AccessibilityFeatureType : uint8_t {
    HighContrast, LargeText, ReduceMotion, ScreenReader, ClosedCaptions,
    ColorblindMode, AudioDescriptions, StickyKeys, OneButtonMode, HapticFeedback
};

inline const char* accessibilityFeatureTypeName(AccessibilityFeatureType t) {
    switch (t) {
        case AccessibilityFeatureType::HighContrast:      return "HighContrast";
        case AccessibilityFeatureType::LargeText:         return "LargeText";
        case AccessibilityFeatureType::ReduceMotion:      return "ReduceMotion";
        case AccessibilityFeatureType::ScreenReader:      return "ScreenReader";
        case AccessibilityFeatureType::ClosedCaptions:    return "ClosedCaptions";
        case AccessibilityFeatureType::ColorblindMode:    return "ColorblindMode";
        case AccessibilityFeatureType::AudioDescriptions: return "AudioDescriptions";
        case AccessibilityFeatureType::StickyKeys:        return "StickyKeys";
        case AccessibilityFeatureType::OneButtonMode:     return "OneButtonMode";
        case AccessibilityFeatureType::HapticFeedback:    return "HapticFeedback";
    }
    return "Unknown";
}

enum class AccessibilityCategory : uint8_t {
    Visual, Auditory, Motor, Cognitive, Speech
};

inline const char* accessibilityCategoryName(AccessibilityCategory c) {
    switch (c) {
        case AccessibilityCategory::Visual:    return "Visual";
        case AccessibilityCategory::Auditory:  return "Auditory";
        case AccessibilityCategory::Motor:     return "Motor";
        case AccessibilityCategory::Cognitive: return "Cognitive";
        case AccessibilityCategory::Speech:    return "Speech";
    }
    return "Unknown";
}

enum class AccessibilityComplianceLevel : uint8_t {
    None, PartialA, LevelA, LevelAA, LevelAAA
};

inline const char* accessibilityComplianceLevelName(AccessibilityComplianceLevel l) {
    switch (l) {
        case AccessibilityComplianceLevel::None:     return "None";
        case AccessibilityComplianceLevel::PartialA: return "PartialA";
        case AccessibilityComplianceLevel::LevelA:   return "LevelA";
        case AccessibilityComplianceLevel::LevelAA:  return "LevelAA";
        case AccessibilityComplianceLevel::LevelAAA: return "LevelAAA";
    }
    return "Unknown";
}

class AccessibilityFeature {
public:
    explicit AccessibilityFeature(uint32_t id, const std::string& name,
                                  AccessibilityFeatureType type, AccessibilityCategory category)
        : m_id(id), m_name(name), m_type(type), m_category(category) {}

    void setIsEnabled(bool v)                            { m_isEnabled       = v; }
    void setComplianceLevel(AccessibilityComplianceLevel v) { m_complianceLevel = v; }
    void setIsRequired(bool v)                           { m_isRequired      = v; }

    [[nodiscard]] uint32_t                      id()              const { return m_id;              }
    [[nodiscard]] const std::string&            name()            const { return m_name;            }
    [[nodiscard]] AccessibilityFeatureType      type()            const { return m_type;            }
    [[nodiscard]] AccessibilityCategory         category()        const { return m_category;        }
    [[nodiscard]] bool                          isEnabled()       const { return m_isEnabled;       }
    [[nodiscard]] AccessibilityComplianceLevel  complianceLevel() const { return m_complianceLevel; }
    [[nodiscard]] bool                          isRequired()      const { return m_isRequired;      }

private:
    uint32_t                     m_id;
    std::string                  m_name;
    AccessibilityFeatureType     m_type;
    AccessibilityCategory        m_category;
    bool                         m_isEnabled       = false;
    AccessibilityComplianceLevel m_complianceLevel  = AccessibilityComplianceLevel::None;
    bool                         m_isRequired       = false;
};

class AccessibilityEditor {
public:
    void setShowDisabled(bool v)      { m_showDisabled  = v; }
    void setFilterCategory(AccessibilityCategory v) { m_filterCategory = v; }
    void setTargetCompliance(AccessibilityComplianceLevel v) { m_targetCompliance = v; }

    bool addFeature(const AccessibilityFeature& f) {
        for (auto& x : m_features) if (x.id() == f.id()) return false;
        m_features.push_back(f); return true;
    }
    bool removeFeature(uint32_t id) {
        auto it = std::find_if(m_features.begin(), m_features.end(),
            [&](const AccessibilityFeature& f){ return f.id() == id; });
        if (it == m_features.end()) return false;
        m_features.erase(it); return true;
    }
    [[nodiscard]] AccessibilityFeature* findFeature(uint32_t id) {
        for (auto& f : m_features) if (f.id() == id) return &f;
        return nullptr;
    }

    [[nodiscard]] bool                       isShowDisabled()    const { return m_showDisabled;    }
    [[nodiscard]] AccessibilityCategory      filterCategory()    const { return m_filterCategory;  }
    [[nodiscard]] AccessibilityComplianceLevel targetCompliance() const { return m_targetCompliance; }
    [[nodiscard]] size_t                     featureCount()      const { return m_features.size(); }

    [[nodiscard]] size_t countByCategory(AccessibilityCategory c) const {
        size_t n = 0; for (auto& f : m_features) if (f.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& f : m_features) if (f.isEnabled()) ++n; return n;
    }
    [[nodiscard]] size_t countRequired() const {
        size_t n = 0; for (auto& f : m_features) if (f.isRequired()) ++n; return n;
    }

private:
    std::vector<AccessibilityFeature>   m_features;
    bool                                m_showDisabled    = true;
    AccessibilityCategory               m_filterCategory  = AccessibilityCategory::Visual;
    AccessibilityComplianceLevel        m_targetCompliance = AccessibilityComplianceLevel::LevelAA;
};

} // namespace NF
