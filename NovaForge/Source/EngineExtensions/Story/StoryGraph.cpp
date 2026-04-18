#include "StoryGraph.h"
#include <algorithm>

namespace atlas::story {

void StoryGraph::Init() {
    m_nodes.clear();
    m_nextId = 1;
}

uint32_t StoryGraph::AddNode(StoryNodeType type, const std::string& name, const std::string& description) {
    uint32_t id = m_nextId++;
    StoryNode node;
    node.id = id;
    node.type = type;
    node.name = name;
    node.description = description;
    m_nodes[id] = node;
    return id;
}

void StoryGraph::RemoveNode(uint32_t id) {
    m_nodes.erase(id);
}

const StoryNode* StoryGraph::GetNode(uint32_t id) const {
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        return &it->second;
    }
    return nullptr;
}

size_t StoryGraph::NodeCount() const {
    return m_nodes.size();
}

void StoryGraph::Connect(uint32_t fromId, uint32_t toId) {
    auto it = m_nodes.find(fromId);
    if (it != m_nodes.end()) {
        it->second.connections.push_back(toId);
    }
}

std::vector<uint32_t> StoryGraph::GetConnections(uint32_t fromId) const {
    auto it = m_nodes.find(fromId);
    if (it != m_nodes.end()) {
        return it->second.connections;
    }
    return {};
}

void StoryGraph::AddTag(uint32_t id, const std::string& tag) {
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        it->second.tags.push_back(tag);
    }
}

std::vector<const StoryNode*> StoryGraph::GetNodesByTag(const std::string& tag) const {
    std::vector<const StoryNode*> result;
    for (const auto& [id, node] : m_nodes) {
        for (const auto& t : node.tags) {
            if (t == tag) {
                result.push_back(&node);
                break;
            }
        }
    }
    return result;
}

std::vector<const StoryNode*> StoryGraph::GetNodesByType(StoryNodeType type) const {
    std::vector<const StoryNode*> result;
    for (const auto& [id, node] : m_nodes) {
        if (node.type == type) {
            result.push_back(&node);
        }
    }
    return result;
}

} // namespace atlas::story
