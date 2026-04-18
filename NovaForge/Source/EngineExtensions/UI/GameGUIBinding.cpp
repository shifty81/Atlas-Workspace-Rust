#include "GameGUIBinding.h"
#include <algorithm>

namespace atlas::ui {

uint64_t GameGUIBindingSystem::AddBinding(const BindingDef& def) {
    BindingEntry entry;
    entry.id = m_nextID++;
    entry.def = def;
    m_bindings.push_back(entry);
    return entry.id;
}

void GameGUIBindingSystem::RemoveBinding(uint64_t id) {
    m_bindings.erase(
        std::remove_if(m_bindings.begin(), m_bindings.end(),
                       [id](const BindingEntry& e) { return e.id == id; }),
        m_bindings.end());
}

const BindingDef* GameGUIBindingSystem::GetBinding(uint64_t id) const {
    for (auto& entry : m_bindings) {
        if (entry.id == id) return &entry.def;
    }
    return nullptr;
}

std::vector<BindingDef> GameGUIBindingSystem::ListBindings() const {
    std::vector<BindingDef> result;
    for (auto& entry : m_bindings) {
        result.push_back(entry.def);
    }
    return result;
}

size_t GameGUIBindingSystem::BindingCount() const {
    return m_bindings.size();
}

std::vector<BindingUpdate> GameGUIBindingSystem::EvaluateAll(
    const std::unordered_map<std::string, std::string>& data) const {
    std::vector<BindingUpdate> updates;
    for (auto& entry : m_bindings) {
        std::string key = entry.def.sourceComponent + "." + entry.def.sourceField;
        auto it = data.find(key);
        std::string rawValue = (it != data.end()) ? it->second : "";
        std::string finalValue = ApplyFormat(entry.def, rawValue);
        updates.push_back({entry.def.widgetID, entry.def.propertyName, finalValue});
    }
    return updates;
}

BindingUpdate GameGUIBindingSystem::EvaluateBinding(
    uint64_t id,
    const std::unordered_map<std::string, std::string>& data) const {
    for (auto& entry : m_bindings) {
        if (entry.id == id) {
            std::string key = entry.def.sourceComponent + "." + entry.def.sourceField;
            auto it = data.find(key);
            std::string rawValue = (it != data.end()) ? it->second : "";
            std::string finalValue = ApplyFormat(entry.def, rawValue);
            return {entry.def.widgetID, entry.def.propertyName, finalValue};
        }
    }
    return {};
}

void GameGUIBindingSystem::SetFormatter(const std::string& name,
                                         std::function<std::string(const std::string&)> fn) {
    m_formatters[name] = std::move(fn);
}

bool GameGUIBindingSystem::Validate() const {
    for (auto& entry : m_bindings) {
        if (entry.def.widgetID.empty()) return false;
        if (entry.def.propertyName.empty()) return false;
    }
    return true;
}

void GameGUIBindingSystem::Clear() {
    m_bindings.clear();
    m_formatters.clear();
    m_nextID = 1;
}

std::string GameGUIBindingSystem::ApplyFormat(const BindingDef& def, const std::string& rawValue) const {
    if (def.bindingType == BindingType::Direct) {
        return rawValue;
    }
    if (def.bindingType == BindingType::Formatted) {
        auto it = m_formatters.find(def.formatString);
        if (it != m_formatters.end()) {
            return it->second(rawValue);
        }
        // Simple format: replace {} with value
        std::string result = def.formatString;
        auto pos = result.find("{}");
        if (pos != std::string::npos) {
            result.replace(pos, 2, rawValue);
        }
        return result;
    }
    if (def.bindingType == BindingType::Computed) {
        auto it = m_formatters.find(def.formatString);
        if (it != m_formatters.end()) {
            return it->second(rawValue);
        }
        return rawValue;
    }
    return rawValue;
}

} // namespace atlas::ui
