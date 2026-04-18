#include "GameGUIAsset.h"
#include <sstream>

namespace atlas::ui {

uint64_t GameGUIAsset::AddWidget(WidgetType type, const std::string& name) {
    auto w = std::make_unique<WidgetDef>();
    w->id = m_nextID++;
    w->type = type;
    w->name = name;
    uint64_t id = w->id;
    m_roots.push_back(std::move(w));
    return id;
}

uint64_t GameGUIAsset::AddChild(uint64_t parentID, WidgetType type, const std::string& name) {
    WidgetDef* parent = FindWidget(parentID);
    if (!parent) return 0;
    auto w = std::make_unique<WidgetDef>();
    w->id = m_nextID++;
    w->type = type;
    w->name = name;
    uint64_t id = w->id;
    parent->children.push_back(std::move(w));
    return id;
}

bool GameGUIAsset::RemoveWidget(uint64_t id) {
    for (auto it = m_roots.begin(); it != m_roots.end(); ++it) {
        if ((*it)->id == id) {
            m_roots.erase(it);
            return true;
        }
        if (RemoveInTree(it->get(), id)) return true;
    }
    return false;
}

WidgetDef* GameGUIAsset::FindWidget(uint64_t id) {
    for (auto& root : m_roots) {
        WidgetDef* found = FindInTree(root.get(), id);
        if (found) return found;
    }
    return nullptr;
}

const WidgetDef* GameGUIAsset::FindWidget(uint64_t id) const {
    for (auto& root : m_roots) {
        const WidgetDef* found = FindInTree(root.get(), id);
        if (found) return found;
    }
    return nullptr;
}

std::vector<const WidgetDef*> GameGUIAsset::ListWidgetsByType(WidgetType type) const {
    std::vector<const WidgetDef*> result;
    for (auto& root : m_roots) {
        CollectByType(root.get(), type, result);
    }
    return result;
}

size_t GameGUIAsset::WidgetCount() const {
    size_t count = 0;
    for (auto& root : m_roots) {
        count += CountNodes(root.get());
    }
    return count;
}

void GameGUIAsset::SetProperty(uint64_t id, const std::string& key, const std::string& value) {
    WidgetDef* w = FindWidget(id);
    if (w) w->properties[key] = value;
}

std::string GameGUIAsset::GetProperty(uint64_t id, const std::string& key) const {
    const WidgetDef* w = FindWidget(id);
    if (w) {
        auto it = w->properties.find(key);
        if (it != w->properties.end()) return it->second;
    }
    return "";
}

void GameGUIAsset::AddBinding(uint64_t id, const std::string& property, const std::string& bindingKey) {
    WidgetDef* w = FindWidget(id);
    if (w) {
        w->bindings.push_back({property, bindingKey});
    }
}

bool GameGUIAsset::Validate() const {
    if (m_roots.empty()) return true;
    for (auto& root : m_roots) {
        if (!ValidateNode(root.get())) return false;
    }
    return true;
}

std::string GameGUIAsset::ExportJSON() const {
    std::ostringstream ss;
    ss << "{\"widgets\":[";
    bool first = true;
    for (auto& root : m_roots) {
        if (!first) ss << ",";
        first = false;
        ss << "{\"id\":" << root->id
           << ",\"name\":\"" << root->name << "\""
           << ",\"type\":" << static_cast<int>(root->type)
           << "}";
    }
    ss << "]}";
    return ss.str();
}

bool GameGUIAsset::ImportJSON(const std::string& json) {
    if (json.empty()) return false;
    if (json.find("\"widgets\"") == std::string::npos) return false;
    return true;
}

void GameGUIAsset::Clear() {
    m_roots.clear();
    m_nextID = 1;
}

WidgetDef* GameGUIAsset::Root() {
    if (m_roots.empty()) return nullptr;
    return m_roots[0].get();
}

const WidgetDef* GameGUIAsset::Root() const {
    if (m_roots.empty()) return nullptr;
    return m_roots[0].get();
}

WidgetDef* GameGUIAsset::FindInTree(WidgetDef* node, uint64_t id) {
    if (!node) return nullptr;
    if (node->id == id) return node;
    for (auto& child : node->children) {
        WidgetDef* found = FindInTree(child.get(), id);
        if (found) return found;
    }
    return nullptr;
}

const WidgetDef* GameGUIAsset::FindInTree(const WidgetDef* node, uint64_t id) const {
    if (!node) return nullptr;
    if (node->id == id) return node;
    for (auto& child : node->children) {
        const WidgetDef* found = FindInTree(child.get(), id);
        if (found) return found;
    }
    return nullptr;
}

bool GameGUIAsset::RemoveInTree(WidgetDef* node, uint64_t id) {
    for (auto it = node->children.begin(); it != node->children.end(); ++it) {
        if ((*it)->id == id) {
            node->children.erase(it);
            return true;
        }
        if (RemoveInTree(it->get(), id)) return true;
    }
    return false;
}

void GameGUIAsset::CollectByType(const WidgetDef* node, WidgetType type, std::vector<const WidgetDef*>& out) const {
    if (!node) return;
    if (node->type == type) out.push_back(node);
    for (auto& child : node->children) {
        CollectByType(child.get(), type, out);
    }
}

size_t GameGUIAsset::CountNodes(const WidgetDef* node) const {
    if (!node) return 0;
    size_t count = 1;
    for (auto& child : node->children) {
        count += CountNodes(child.get());
    }
    return count;
}

bool GameGUIAsset::ValidateNode(const WidgetDef* node) const {
    if (!node) return false;
    if (node->id == 0) return false;
    if (node->name.empty()) return false;
    for (auto& child : node->children) {
        if (!ValidateNode(child.get())) return false;
    }
    return true;
}

} // namespace atlas::ui
