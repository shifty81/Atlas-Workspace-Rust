#include "LayerTagSystem.h"
#include <algorithm>

namespace atlas::tools {

// ----- Layers -----

bool LayerTagSystem::CreateLayer(const std::string& name) {
    if (m_layers.count(name)) return false;
    LayerInfo info;
    info.name = name;
    m_layers[name] = std::move(info);
    return true;
}

bool LayerTagSystem::RemoveLayer(const std::string& name) {
    auto it = m_layers.find(name);
    if (it == m_layers.end()) return false;
    m_layers.erase(it);

    // Reassign entities to default layer ""
    for (auto& [eid, layer] : m_entityLayer) {
        if (layer == name) layer = "";
    }
    return true;
}

void LayerTagSystem::SetLayerVisible(const std::string& name, bool visible) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) it->second.visible = visible;
}

bool LayerTagSystem::IsLayerVisible(const std::string& name) const {
    auto it = m_layers.find(name);
    if (it == m_layers.end()) return true;  // unknown layer → visible
    return it->second.visible;
}

void LayerTagSystem::SetLayerLocked(const std::string& name, bool locked) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) it->second.locked = locked;
}

bool LayerTagSystem::IsLayerLocked(const std::string& name) const {
    auto it = m_layers.find(name);
    if (it == m_layers.end()) return false;
    return it->second.locked;
}

const LayerInfo* LayerTagSystem::GetLayer(const std::string& name) const {
    auto it = m_layers.find(name);
    if (it == m_layers.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> LayerTagSystem::AllLayers() const {
    std::vector<std::string> out;
    out.reserve(m_layers.size());
    for (const auto& [name, _] : m_layers) {
        out.push_back(name);
    }
    return out;
}

// ----- Entity-to-layer assignment -----

void LayerTagSystem::AssignLayer(uint32_t entityId, const std::string& layer) {
    if (!m_layers.count(layer) && !layer.empty()) {
        CreateLayer(layer);
    }
    m_entityLayer[entityId] = layer;
}

std::string LayerTagSystem::EntityLayer(uint32_t entityId) const {
    auto it = m_entityLayer.find(entityId);
    if (it == m_entityLayer.end()) return "";
    return it->second;
}

std::vector<uint32_t> LayerTagSystem::EntitiesInLayer(const std::string& layer) const {
    std::vector<uint32_t> out;
    for (const auto& [eid, l] : m_entityLayer) {
        if (l == layer) out.push_back(eid);
    }
    return out;
}

// ----- Tags -----

void LayerTagSystem::AddTag(uint32_t entityId, const std::string& tag) {
    m_entityTags[entityId].insert(tag);
}

void LayerTagSystem::RemoveTag(uint32_t entityId, const std::string& tag) {
    auto it = m_entityTags.find(entityId);
    if (it != m_entityTags.end()) {
        it->second.erase(tag);
    }
}

bool LayerTagSystem::HasTag(uint32_t entityId, const std::string& tag) const {
    auto it = m_entityTags.find(entityId);
    if (it == m_entityTags.end()) return false;
    return it->second.count(tag) > 0;
}

std::vector<std::string> LayerTagSystem::EntityTags(uint32_t entityId) const {
    auto it = m_entityTags.find(entityId);
    if (it == m_entityTags.end()) return {};
    return {it->second.begin(), it->second.end()};
}

std::vector<uint32_t> LayerTagSystem::EntitiesWithTag(const std::string& tag) const {
    std::vector<uint32_t> out;
    for (const auto& [eid, tags] : m_entityTags) {
        if (tags.count(tag)) out.push_back(eid);
    }
    return out;
}

// ----- Visibility -----

bool LayerTagSystem::IsEntityVisible(uint32_t entityId) const {
    auto it = m_entityLayer.find(entityId);
    if (it == m_entityLayer.end()) return true;  // no layer → visible
    return IsLayerVisible(it->second);
}

// ----- Housekeeping -----

void LayerTagSystem::RemoveEntity(uint32_t entityId) {
    m_entityLayer.erase(entityId);
    m_entityTags.erase(entityId);
}

void LayerTagSystem::Clear() {
    m_layers.clear();
    m_entityLayer.clear();
    m_entityTags.clear();
}

} // namespace atlas::tools
