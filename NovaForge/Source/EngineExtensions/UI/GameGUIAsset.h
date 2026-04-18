#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace atlas::ui {

enum class WidgetType : uint8_t {
    Label,
    Button,
    Panel,
    Image,
    ProgressBar,
    List,
    Grid,
    Slider,
    Toggle,
    TextInput
};

struct WidgetBinding {
    std::string propertyName;
    std::string bindingKey;
};

struct WidgetDef {
    uint64_t id = 0;
    WidgetType type = WidgetType::Panel;
    std::string name;
    float x = 0.0f;
    float y = 0.0f;
    float w = 100.0f;
    float h = 100.0f;
    bool visible = true;
    std::unordered_map<std::string, std::string> properties;
    std::vector<WidgetBinding> bindings;
    std::vector<std::unique_ptr<WidgetDef>> children;
};

class GameGUIAsset {
public:
    uint64_t AddWidget(WidgetType type, const std::string& name);
    uint64_t AddChild(uint64_t parentID, WidgetType type, const std::string& name);
    bool RemoveWidget(uint64_t id);
    WidgetDef* FindWidget(uint64_t id);
    const WidgetDef* FindWidget(uint64_t id) const;
    std::vector<const WidgetDef*> ListWidgetsByType(WidgetType type) const;
    size_t WidgetCount() const;

    void SetProperty(uint64_t id, const std::string& key, const std::string& value);
    std::string GetProperty(uint64_t id, const std::string& key) const;

    void AddBinding(uint64_t id, const std::string& property, const std::string& bindingKey);

    bool Validate() const;
    std::string ExportJSON() const;
    bool ImportJSON(const std::string& json);
    void Clear();

    WidgetDef* Root();
    const WidgetDef* Root() const;

private:
    std::vector<std::unique_ptr<WidgetDef>> m_roots;
    uint64_t m_nextID = 1;

    WidgetDef* FindInTree(WidgetDef* node, uint64_t id);
    const WidgetDef* FindInTree(const WidgetDef* node, uint64_t id) const;
    bool RemoveInTree(WidgetDef* node, uint64_t id);
    void CollectByType(const WidgetDef* node, WidgetType type, std::vector<const WidgetDef*>& out) const;
    size_t CountNodes(const WidgetDef* node) const;
    bool ValidateNode(const WidgetDef* node) const;
};

} // namespace atlas::ui
