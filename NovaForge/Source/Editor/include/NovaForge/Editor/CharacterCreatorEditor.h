#pragma once
// NF::Editor — character creator editor
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

enum class BodyPartCategory : uint8_t {
    Head, Torso, Arms, Legs, Feet, Hands, Face, Hair
};

inline const char* bodyPartCategoryName(BodyPartCategory c) {
    switch (c) {
        case BodyPartCategory::Head:  return "Head";
        case BodyPartCategory::Torso: return "Torso";
        case BodyPartCategory::Arms:  return "Arms";
        case BodyPartCategory::Legs:  return "Legs";
        case BodyPartCategory::Feet:  return "Feet";
        case BodyPartCategory::Hands: return "Hands";
        case BodyPartCategory::Face:  return "Face";
        case BodyPartCategory::Hair:  return "Hair";
    }
    return "Unknown";
}

enum class SkinTonePreset : uint8_t {
    Pale, Fair, Medium, Tan, Brown, Dark
};

inline const char* skinTonePresetName(SkinTonePreset s) {
    switch (s) {
        case SkinTonePreset::Pale:   return "Pale";
        case SkinTonePreset::Fair:   return "Fair";
        case SkinTonePreset::Medium: return "Medium";
        case SkinTonePreset::Tan:    return "Tan";
        case SkinTonePreset::Brown:  return "Brown";
        case SkinTonePreset::Dark:   return "Dark";
    }
    return "Unknown";
}

enum class CharacterGender : uint8_t {
    Male, Female, NonBinary, Undefined
};

inline const char* characterGenderName(CharacterGender g) {
    switch (g) {
        case CharacterGender::Male:      return "Male";
        case CharacterGender::Female:    return "Female";
        case CharacterGender::NonBinary: return "NonBinary";
        case CharacterGender::Undefined: return "Undefined";
    }
    return "Unknown";
}

class CharacterPreset {
public:
    explicit CharacterPreset(uint32_t id, const std::string& name, CharacterGender gender)
        : m_id(id), m_name(name), m_gender(gender) {}

    void setSkinTone(SkinTonePreset v)  { m_skinTone    = v; }
    void setHeight(float v)             { m_height      = v; }
    void setWeight(float v)             { m_weight      = v; }
    void setCustomized(bool v)          { m_isCustomized = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;          }
    [[nodiscard]] const std::string& name()         const { return m_name;        }
    [[nodiscard]] CharacterGender    gender()       const { return m_gender;      }
    [[nodiscard]] SkinTonePreset     skinTone()     const { return m_skinTone;    }
    [[nodiscard]] float              height()       const { return m_height;      }
    [[nodiscard]] float              weight()       const { return m_weight;      }
    [[nodiscard]] bool               isCustomized() const { return m_isCustomized;}

private:
    uint32_t        m_id;
    std::string     m_name;
    CharacterGender m_gender;
    SkinTonePreset  m_skinTone    = SkinTonePreset::Medium;
    float           m_height      = 1.75f;
    float           m_weight      = 70.0f;
    bool            m_isCustomized = false;
};

class CharacterCreatorEditor {
public:
    void setActiveBodyPart(BodyPartCategory v) { m_activeBodyPart = v; }
    void setShowSymmetry(bool v)               { m_showSymmetry   = v; }
    void setShowGrid(bool v)                   { m_showGrid       = v; }
    void setZoom(float v)                      { m_zoom           = v; }

    bool addPreset(const CharacterPreset& p) {
        for (auto& e : m_presets) if (e.id() == p.id()) return false;
        m_presets.push_back(p); return true;
    }
    bool removePreset(uint32_t id) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
            [&](const CharacterPreset& e){ return e.id() == id; });
        if (it == m_presets.end()) return false;
        m_presets.erase(it); return true;
    }
    [[nodiscard]] CharacterPreset* findPreset(uint32_t id) {
        for (auto& e : m_presets) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] BodyPartCategory activeBodyPart()  const { return m_activeBodyPart; }
    [[nodiscard]] bool             isShowSymmetry()  const { return m_showSymmetry;   }
    [[nodiscard]] bool             isShowGrid()      const { return m_showGrid;       }
    [[nodiscard]] float            zoom()            const { return m_zoom;           }
    [[nodiscard]] size_t           presetCount()     const { return m_presets.size(); }

    [[nodiscard]] size_t countByGender(CharacterGender g) const {
        size_t c = 0; for (auto& e : m_presets) if (e.gender() == g) ++c; return c;
    }
    [[nodiscard]] size_t countCustomized() const {
        size_t c = 0; for (auto& e : m_presets) if (e.isCustomized()) ++c; return c;
    }

private:
    std::vector<CharacterPreset> m_presets;
    BodyPartCategory m_activeBodyPart = BodyPartCategory::Face;
    bool             m_showSymmetry   = true;
    bool             m_showGrid       = false;
    float            m_zoom           = 1.0f;
};

} // namespace NF
