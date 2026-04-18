#pragma once
// NF::Editor — Interaction editor v1: grab, use, examine, talk and push point authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Iev1InteractionType : uint8_t { Grab, Use, Examine, Talk, Push };
enum class Iev1InteractionState: uint8_t { Enabled, Disabled, Locked };
enum class Iev1Priority        : uint8_t { High, Normal, Low };

inline const char* iev1InteractionTypeName(Iev1InteractionType t) {
    switch (t) {
        case Iev1InteractionType::Grab:    return "Grab";
        case Iev1InteractionType::Use:     return "Use";
        case Iev1InteractionType::Examine: return "Examine";
        case Iev1InteractionType::Talk:    return "Talk";
        case Iev1InteractionType::Push:    return "Push";
    }
    return "Unknown";
}

inline const char* iev1InteractionStateName(Iev1InteractionState s) {
    switch (s) {
        case Iev1InteractionState::Enabled:  return "Enabled";
        case Iev1InteractionState::Disabled: return "Disabled";
        case Iev1InteractionState::Locked:   return "Locked";
    }
    return "Unknown";
}

inline const char* iev1PriorityName(Iev1Priority p) {
    switch (p) {
        case Iev1Priority::High:   return "High";
        case Iev1Priority::Normal: return "Normal";
        case Iev1Priority::Low:    return "Low";
    }
    return "Unknown";
}

struct Iev1Interaction {
    uint64_t            id         = 0;
    std::string         name;
    Iev1InteractionType type       = Iev1InteractionType::Use;
    Iev1InteractionState state     = Iev1InteractionState::Enabled;
    std::string         promptText;
    float               range      = 1.f;
    Iev1Priority        priority   = Iev1Priority::Normal;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEnabled() const { return state == Iev1InteractionState::Enabled; }
};

using Iev1ChangeCallback = std::function<void(uint64_t)>;

class InteractionEditorV1 {
public:
    static constexpr size_t MAX_INTERACTIONS = 512;

    bool addInteraction(const Iev1Interaction& interaction) {
        if (!interaction.isValid()) return false;
        for (const auto& i : m_interactions) if (i.id == interaction.id) return false;
        if (m_interactions.size() >= MAX_INTERACTIONS) return false;
        m_interactions.push_back(interaction);
        if (m_onChange) m_onChange(interaction.id);
        return true;
    }

    bool removeInteraction(uint64_t id) {
        for (auto it = m_interactions.begin(); it != m_interactions.end(); ++it) {
            if (it->id == id) { m_interactions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Iev1Interaction* findInteraction(uint64_t id) {
        for (auto& i : m_interactions) if (i.id == id) return &i;
        return nullptr;
    }

    bool setState(uint64_t id, Iev1InteractionState state) {
        auto* i = findInteraction(id);
        if (!i) return false;
        i->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setRange(uint64_t id, float range) {
        auto* i = findInteraction(id);
        if (!i) return false;
        i->range = std::max(0.1f, range);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setPriority(uint64_t id, Iev1Priority priority) {
        auto* i = findInteraction(id);
        if (!i) return false;
        i->priority = priority;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setPromptText(uint64_t id, const std::string& text) {
        auto* i = findInteraction(id);
        if (!i) return false;
        i->promptText = text;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t interactionCount() const { return m_interactions.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0;
        for (const auto& i : m_interactions) if (i.isEnabled()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Iev1InteractionType type) const {
        size_t c = 0;
        for (const auto& i : m_interactions) if (i.type == type) ++c;
        return c;
    }

    void setOnChange(Iev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Iev1Interaction> m_interactions;
    Iev1ChangeCallback           m_onChange;
};

} // namespace NF
