#pragma once
// NF::Editor — Widget kit v1: primitive UI widget descriptors and layout model
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Widget Type ───────────────────────────────────────────────────

enum class WidgetType : uint8_t {
    Button,
    Label,
    TextInput,
    Checkbox,
    Slider,
    Dropdown,
    Separator,
    Spacer,
    Image,
    ProgressBar,
};

inline const char* widgetTypeName(WidgetType t) {
    switch (t) {
        case WidgetType::Button:     return "Button";
        case WidgetType::Label:      return "Label";
        case WidgetType::TextInput:  return "TextInput";
        case WidgetType::Checkbox:   return "Checkbox";
        case WidgetType::Slider:     return "Slider";
        case WidgetType::Dropdown:   return "Dropdown";
        case WidgetType::Separator:  return "Separator";
        case WidgetType::Spacer:     return "Spacer";
        case WidgetType::Image:      return "Image";
        case WidgetType::ProgressBar:return "ProgressBar";
    }
    return "Unknown";
}

// ── Widget State ──────────────────────────────────────────────────

enum class WidgetState : uint8_t {
    Normal,
    Hovered,
    Pressed,
    Focused,
    Disabled,
};

inline const char* widgetStateName(WidgetState s) {
    switch (s) {
        case WidgetState::Normal:   return "Normal";
        case WidgetState::Hovered:  return "Hovered";
        case WidgetState::Pressed:  return "Pressed";
        case WidgetState::Focused:  return "Focused";
        case WidgetState::Disabled: return "Disabled";
    }
    return "Unknown";
}

// ── Widget Layout ─────────────────────────────────────────────────

enum class WidgetLayout : uint8_t {
    Horizontal,   // stack left-to-right
    Vertical,     // stack top-to-bottom
    Absolute,     // absolute position
};

// ── Widget Size Constraint ────────────────────────────────────────

struct WidgetSizeConstraint {
    float minW  = 0.f;
    float minH  = 0.f;
    float maxW  = 99999.f;
    float maxH  = 99999.f;
    float prefW = 0.f;  // 0 = auto
    float prefH = 0.f;

    [[nodiscard]] float clampW(float w) const {
        return w < minW ? minW : (w > maxW ? maxW : w);
    }
    [[nodiscard]] float clampH(float h) const {
        return h < minH ? minH : (h > maxH ? maxH : h);
    }
};

// ── Widget Descriptor ─────────────────────────────────────────────

using WidgetClickCallback  = std::function<void()>;
using WidgetChangeCallback = std::function<void(const std::string& value)>;

struct WidgetDescriptor {
    uint32_t           id       = 0;
    WidgetType         type     = WidgetType::Label;
    WidgetState        state    = WidgetState::Normal;
    std::string        label;
    std::string        tooltip;
    std::string        iconId;
    std::string        value;    // current value (text/checked/float-as-string)
    WidgetSizeConstraint size;
    bool               visible  = true;
    bool               enabled  = true;

    // Type-specific helpers
    float              sliderMin = 0.f;
    float              sliderMax = 1.f;
    std::vector<std::string> dropdownOptions;

    WidgetClickCallback  onClick;
    WidgetChangeCallback onChange;

    [[nodiscard]] bool isValid() const { return id != 0; }
    [[nodiscard]] bool isDisabled() const { return state == WidgetState::Disabled || !enabled; }

    void disable() { state = WidgetState::Disabled; enabled = false; }
    void enable()  { state = WidgetState::Normal;   enabled = true;  }
};

// ── Widget Container ──────────────────────────────────────────────

struct WidgetContainer {
    uint32_t                         id     = 0;
    WidgetLayout                     layout = WidgetLayout::Vertical;
    std::vector<WidgetDescriptor>    widgets;
    float                            paddingPx = 4.f;
    float                            spacingPx = 4.f;

    [[nodiscard]] bool isValid() const { return id != 0; }
    [[nodiscard]] size_t widgetCount() const { return widgets.size(); }

    bool addWidget(const WidgetDescriptor& w) {
        if (!w.isValid()) return false;
        for (const auto& e : widgets) if (e.id == w.id) return false;
        widgets.push_back(w);
        return true;
    }

    bool removeWidget(uint32_t widgetId) {
        for (auto it = widgets.begin(); it != widgets.end(); ++it) {
            if (it->id == widgetId) { widgets.erase(it); return true; }
        }
        return false;
    }

    WidgetDescriptor* findWidget(uint32_t widgetId) {
        for (auto& w : widgets) if (w.id == widgetId) return &w;
        return nullptr;
    }
    const WidgetDescriptor* findWidget(uint32_t widgetId) const {
        for (const auto& w : widgets) if (w.id == widgetId) return &w;
        return nullptr;
    }
};

// ── Widget Kit V1 ─────────────────────────────────────────────────

class WidgetKitV1 {
public:
    static constexpr size_t MAX_CONTAINERS = 256;

    bool addContainer(const WidgetContainer& c) {
        if (!c.isValid()) return false;
        if (m_containers.size() >= MAX_CONTAINERS) return false;
        for (const auto& e : m_containers) if (e.id == c.id) return false;
        m_containers.push_back(c);
        return true;
    }

    bool removeContainer(uint32_t id) {
        for (auto it = m_containers.begin(); it != m_containers.end(); ++it) {
            if (it->id == id) { m_containers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] WidgetContainer* findContainer(uint32_t id) {
        for (auto& c : m_containers) if (c.id == id) return &c;
        return nullptr;
    }
    [[nodiscard]] const WidgetContainer* findContainer(uint32_t id) const {
        for (const auto& c : m_containers) if (c.id == id) return &c;
        return nullptr;
    }

    // Simulate a click on a widget (fires callback, updates state)
    bool simulateClick(uint32_t containerId, uint32_t widgetId) {
        auto* c = findContainer(containerId);
        if (!c) return false;
        auto* w = c->findWidget(widgetId);
        if (!w || w->isDisabled()) return false;
        w->state = WidgetState::Pressed;
        if (w->onClick) w->onClick();
        w->state = WidgetState::Normal;
        ++m_interactionCount;
        return true;
    }

    bool setValue(uint32_t containerId, uint32_t widgetId, const std::string& val) {
        auto* c = findContainer(containerId);
        if (!c) return false;
        auto* w = c->findWidget(widgetId);
        if (!w || w->isDisabled()) return false;
        std::string old = w->value;
        w->value = val;
        if (w->onChange && old != val) w->onChange(val);
        ++m_interactionCount;
        return true;
    }

    bool setWidgetEnabled(uint32_t containerId, uint32_t widgetId, bool enabled) {
        auto* c = findContainer(containerId);
        if (!c) return false;
        auto* w = c->findWidget(widgetId);
        if (!w) return false;
        if (enabled) w->enable();
        else w->disable();
        return true;
    }

    [[nodiscard]] size_t containerCount()    const { return m_containers.size(); }
    [[nodiscard]] size_t interactionCount()  const { return m_interactionCount;  }
    [[nodiscard]] const std::vector<WidgetContainer>& containers() const { return m_containers; }

private:
    std::vector<WidgetContainer> m_containers;
    size_t m_interactionCount = 0;
};

} // namespace NF
