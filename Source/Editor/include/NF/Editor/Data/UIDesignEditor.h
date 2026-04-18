#pragma once
// NF::Editor — UI design editor
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

enum class UIDesignElementType : uint8_t {
    Panel, Button, Label, Image, ProgressBar, Slider, InputField, Checkbox, Dropdown, List
};

inline const char* uiDesignElementTypeName(UIDesignElementType t) {
    switch (t) {
        case UIDesignElementType::Panel:       return "Panel";
        case UIDesignElementType::Button:      return "Button";
        case UIDesignElementType::Label:       return "Label";
        case UIDesignElementType::Image:       return "Image";
        case UIDesignElementType::ProgressBar: return "ProgressBar";
        case UIDesignElementType::Slider:      return "Slider";
        case UIDesignElementType::InputField:  return "InputField";
        case UIDesignElementType::Checkbox:    return "Checkbox";
        case UIDesignElementType::Dropdown:    return "Dropdown";
        case UIDesignElementType::List:        return "List";
    }
    return "Unknown";
}

enum class UIAnchorPreset : uint8_t {
    TopLeft, TopCenter, TopRight,
    MiddleLeft, MiddleCenter, MiddleRight,
    BottomLeft, BottomCenter, BottomRight,
    FullStretch
};

inline const char* uiAnchorPresetName(UIAnchorPreset p) {
    switch (p) {
        case UIAnchorPreset::TopLeft:      return "TopLeft";
        case UIAnchorPreset::TopCenter:    return "TopCenter";
        case UIAnchorPreset::TopRight:     return "TopRight";
        case UIAnchorPreset::MiddleLeft:   return "MiddleLeft";
        case UIAnchorPreset::MiddleCenter: return "MiddleCenter";
        case UIAnchorPreset::MiddleRight:  return "MiddleRight";
        case UIAnchorPreset::BottomLeft:   return "BottomLeft";
        case UIAnchorPreset::BottomCenter: return "BottomCenter";
        case UIAnchorPreset::BottomRight:  return "BottomRight";
        case UIAnchorPreset::FullStretch:  return "FullStretch";
    }
    return "Unknown";
}

enum class UIScreenResolution : uint8_t {
    R1920x1080, R2560x1440, R3840x2160, R1280x720, R1366x768, R375x812
};

inline const char* uiScreenResolutionName(UIScreenResolution r) {
    switch (r) {
        case UIScreenResolution::R1920x1080: return "1920x1080";
        case UIScreenResolution::R2560x1440: return "2560x1440";
        case UIScreenResolution::R3840x2160: return "3840x2160";
        case UIScreenResolution::R1280x720:  return "1280x720";
        case UIScreenResolution::R1366x768:  return "1366x768";
        case UIScreenResolution::R375x812:   return "375x812";
    }
    return "Unknown";
}

class UIDesignElement {
public:
    explicit UIDesignElement(uint32_t id, const std::string& name, UIDesignElementType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setAnchor(UIAnchorPreset p) { m_anchor   = p; }
    void setVisible(bool v)          { m_visible  = v; }
    void setLocked(bool v)           { m_locked   = v; }
    void setSelected(bool v)         { m_selected = v; }
    void setWidth(float v)           { m_width    = v; }
    void setHeight(float v)          { m_height   = v; }
    void setZOrder(int32_t v)        { m_zOrder   = v; }

    [[nodiscard]] uint32_t             id()         const { return m_id;       }
    [[nodiscard]] const std::string&   name()       const { return m_name;     }
    [[nodiscard]] UIDesignElementType  type()       const { return m_type;     }
    [[nodiscard]] UIAnchorPreset       anchor()     const { return m_anchor;   }
    [[nodiscard]] bool                 isVisible()  const { return m_visible;  }
    [[nodiscard]] bool                 isLocked()   const { return m_locked;   }
    [[nodiscard]] bool                 isSelected() const { return m_selected; }
    [[nodiscard]] float                width()      const { return m_width;    }
    [[nodiscard]] float                height()     const { return m_height;   }
    [[nodiscard]] int32_t              zOrder()     const { return m_zOrder;   }

private:
    uint32_t            m_id;
    std::string         m_name;
    UIDesignElementType m_type;
    UIAnchorPreset      m_anchor   = UIAnchorPreset::MiddleCenter;
    bool                m_visible  = true;
    bool                m_locked   = false;
    bool                m_selected = false;
    float               m_width    = 100.0f;
    float               m_height   = 30.0f;
    int32_t             m_zOrder   = 0;
};

class UIDesignEditor {
public:
    void setPreviewResolution(UIScreenResolution r) { m_resolution  = r; }
    void setShowGrid(bool v)                        { m_showGrid    = v; }
    void setShowGuides(bool v)                      { m_showGuides  = v; }
    void setSnapToGrid(bool v)                      { m_snapToGrid  = v; }

    bool addElement(const UIDesignElement& el) {
        for (auto& e : m_elements) if (e.id() == el.id()) return false;
        m_elements.push_back(el); return true;
    }
    bool removeElement(uint32_t id) {
        auto it = std::find_if(m_elements.begin(), m_elements.end(),
            [&](const UIDesignElement& e){ return e.id() == id; });
        if (it == m_elements.end()) return false;
        m_elements.erase(it); return true;
    }
    [[nodiscard]] UIDesignElement* findElement(uint32_t id) {
        for (auto& e : m_elements) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] UIScreenResolution previewResolution() const { return m_resolution; }
    [[nodiscard]] bool               isShowGrid()        const { return m_showGrid;   }
    [[nodiscard]] bool               isShowGuides()      const { return m_showGuides; }
    [[nodiscard]] bool               isSnapToGrid()      const { return m_snapToGrid; }
    [[nodiscard]] size_t             elementCount()      const { return m_elements.size(); }

    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& e : m_elements) if (e.isSelected()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(UIDesignElementType t) const {
        size_t c = 0; for (auto& e : m_elements) if (e.type() == t) ++c; return c;
    }

private:
    std::vector<UIDesignElement> m_elements;
    UIScreenResolution           m_resolution  = UIScreenResolution::R1920x1080;
    bool                         m_showGrid    = true;
    bool                         m_showGuides  = true;
    bool                         m_snapToGrid  = true;
};

} // namespace NF
