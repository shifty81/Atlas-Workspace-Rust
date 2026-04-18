#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace atlas::ui {

enum class BindingType : uint8_t {
    Direct,
    Formatted,
    Computed
};

struct BindingDef {
    std::string widgetID;
    std::string propertyName;
    std::string sourceEntity;
    std::string sourceComponent;
    std::string sourceField;
    BindingType bindingType = BindingType::Direct;
    std::string formatString;
};

struct BindingUpdate {
    std::string widgetID;
    std::string propertyName;
    std::string newValue;
};

class GameGUIBindingSystem {
public:
    uint64_t AddBinding(const BindingDef& def);
    void RemoveBinding(uint64_t id);
    const BindingDef* GetBinding(uint64_t id) const;
    std::vector<BindingDef> ListBindings() const;
    size_t BindingCount() const;

    std::vector<BindingUpdate> EvaluateAll(
        const std::unordered_map<std::string, std::string>& data) const;
    BindingUpdate EvaluateBinding(
        uint64_t id,
        const std::unordered_map<std::string, std::string>& data) const;

    void SetFormatter(const std::string& name,
                      std::function<std::string(const std::string&)> fn);

    bool Validate() const;
    void Clear();

private:
    struct BindingEntry {
        uint64_t id = 0;
        BindingDef def;
    };
    std::vector<BindingEntry> m_bindings;
    uint64_t m_nextID = 1;
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> m_formatters;

    std::string ApplyFormat(const BindingDef& def, const std::string& rawValue) const;
};

} // namespace atlas::ui
