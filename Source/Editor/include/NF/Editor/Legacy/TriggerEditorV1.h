#pragma once
// NF::Editor — Trigger editor v1: event trigger authoring with condition chains
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Tev1TriggerKind  : uint8_t { OnEnter, OnExit, OnStay, OnOverlap, OnCustom };
enum class Tev1ConditionType: uint8_t { Tag, Layer, Name, Always };

struct Tev1Condition {
    Tev1ConditionType type  = Tev1ConditionType::Always;
    std::string       value;
};

struct Tev1Action {
    uint64_t    id    = 0;
    std::string name;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Tev1Trigger {
    uint64_t          id   = 0;
    std::string       label;
    Tev1TriggerKind   kind = Tev1TriggerKind::OnEnter;
    bool              enabled = true;
    std::vector<Tev1Condition> conditions;
    std::vector<Tev1Action>    actions;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty(); }
};

using Tev1FireCallback = std::function<void(const Tev1Trigger&)>;

class TriggerEditorV1 {
public:
    bool addTrigger(const Tev1Trigger& t) {
        if (!t.isValid()) return false;
        for (const auto& tr : m_triggers) if (tr.id == t.id) return false;
        m_triggers.push_back(t);
        return true;
    }

    bool removeTrigger(uint64_t id) {
        for (auto it = m_triggers.begin(); it != m_triggers.end(); ++it) {
            if (it->id == id) { m_triggers.erase(it); return true; }
        }
        return false;
    }

    bool enableTrigger(uint64_t id, bool enable) {
        for (auto& tr : m_triggers) {
            if (tr.id == id) { tr.enabled = enable; return true; }
        }
        return false;
    }

    bool fireTrigger(uint64_t id) {
        for (const auto& tr : m_triggers) {
            if (tr.id == id && tr.enabled) {
                if (m_onFire) m_onFire(tr);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] size_t triggerCount() const { return m_triggers.size(); }

    void setOnFire(Tev1FireCallback cb) { m_onFire = std::move(cb); }

private:
    std::vector<Tev1Trigger> m_triggers;
    Tev1FireCallback         m_onFire;
};

} // namespace NF
