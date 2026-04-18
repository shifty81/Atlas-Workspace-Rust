#pragma once
// NF::Editor — UI design editor v1: UI element and binding management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Uidv1ElementType  : uint8_t { Panel, Button, Label, Image, InputField, Dropdown };
enum class Uidv1ElementState : uint8_t { Draft, Active, Hidden, Locked, Deprecated };

inline const char* uidv1ElementTypeName(Uidv1ElementType t) {
    switch (t) {
        case Uidv1ElementType::Panel:      return "Panel";
        case Uidv1ElementType::Button:     return "Button";
        case Uidv1ElementType::Label:      return "Label";
        case Uidv1ElementType::Image:      return "Image";
        case Uidv1ElementType::InputField: return "InputField";
        case Uidv1ElementType::Dropdown:   return "Dropdown";
    }
    return "Unknown";
}

inline const char* uidv1ElementStateName(Uidv1ElementState s) {
    switch (s) {
        case Uidv1ElementState::Draft:      return "Draft";
        case Uidv1ElementState::Active:     return "Active";
        case Uidv1ElementState::Hidden:     return "Hidden";
        case Uidv1ElementState::Locked:     return "Locked";
        case Uidv1ElementState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

struct Uidv1Element {
    uint64_t           id    = 0;
    std::string        name;
    Uidv1ElementType   type  = Uidv1ElementType::Panel;
    Uidv1ElementState  state = Uidv1ElementState::Draft;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Uidv1ElementState::Active; }
    [[nodiscard]] bool isLocked() const { return state == Uidv1ElementState::Locked; }
};

struct Uidv1Binding {
    uint64_t    id        = 0;
    uint64_t    elementId = 0;
    std::string property;
    std::string source;

    [[nodiscard]] bool isValid() const { return id != 0 && elementId != 0 && !property.empty(); }
};

using Uidv1ChangeCallback = std::function<void(uint64_t)>;

class UIDesignEditorV1 {
public:
    static constexpr size_t MAX_ELEMENTS = 2048;
    static constexpr size_t MAX_BINDINGS = 8192;

    bool addElement(const Uidv1Element& element) {
        if (!element.isValid()) return false;
        for (const auto& e : m_elements) if (e.id == element.id) return false;
        if (m_elements.size() >= MAX_ELEMENTS) return false;
        m_elements.push_back(element);
        if (m_onChange) m_onChange(element.id);
        return true;
    }

    bool removeElement(uint64_t id) {
        for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
            if (it->id == id) { m_elements.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Uidv1Element* findElement(uint64_t id) {
        for (auto& e : m_elements) if (e.id == id) return &e;
        return nullptr;
    }

    bool addBinding(const Uidv1Binding& binding) {
        if (!binding.isValid()) return false;
        for (const auto& b : m_bindings) if (b.id == binding.id) return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        m_bindings.push_back(binding);
        if (m_onChange) m_onChange(binding.elementId);
        return true;
    }

    bool removeBinding(uint64_t id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t elementCount() const { return m_elements.size(); }
    [[nodiscard]] size_t bindingCount() const { return m_bindings.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& e : m_elements) if (e.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& e : m_elements) if (e.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByElementType(Uidv1ElementType type) const {
        size_t c = 0; for (const auto& e : m_elements) if (e.type == type) ++c; return c;
    }

    void setOnChange(Uidv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Uidv1Element> m_elements;
    std::vector<Uidv1Binding> m_bindings;
    Uidv1ChangeCallback       m_onChange;
};

} // namespace NF
