#include "QuestEditorPanel.h"
#include <sstream>
#include <algorithm>

namespace atlas::editor {

void QuestEditorPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    std::string title = m_previewMode ? "Quest Editor [Preview]" : "Quest Editor";
    m_drawList.DrawText({4, 4, 300, 20}, title, {220, 220, 220, 255});

    // Node list
    int32_t y = 28;
    for (const auto& node : m_nodes) {
        bool selected = (node.id == m_selectedID);
        atlas::ui::UIColor bgColor = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 600, 20}, bgColor);

        std::string line = node.name + " (" + node.nodeType + ")";
        if (node.completed) line += " \xe2\x9c\x93";
        m_drawList.DrawText({4, y + 2, 590, 16}, line, {200, 200, 200, 255});
        y += 22;
    }
}

uint64_t QuestEditorPanel::AddNode(const QuestNode& node) {
    QuestNode n = node;
    n.id = m_nextID++;
    m_nodes.push_back(n);
    return n.id;
}

bool QuestEditorPanel::RemoveNode(uint64_t id) {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                           [id](const QuestNode& n) { return n.id == id; });
    if (it != m_nodes.end()) {
        if (m_selectedID == id) m_selectedID = 0;
        // Remove references to this node from other nodes
        for (auto& node : m_nodes) {
            auto& next = node.nextNodes;
            next.erase(std::remove(next.begin(), next.end(), id), next.end());
        }
        m_nodes.erase(it);
        return true;
    }
    return false;
}

const QuestNode* QuestEditorPanel::GetNode(uint64_t id) const {
    for (auto& n : m_nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

std::vector<QuestNode> QuestEditorPanel::ListNodes() const {
    return m_nodes;
}

size_t QuestEditorPanel::NodeCount() const {
    return m_nodes.size();
}

bool QuestEditorPanel::ConnectNodes(uint64_t fromId, uint64_t toId) {
    for (auto& node : m_nodes) {
        if (node.id == fromId) {
            // Check that toId exists
            if (!GetNode(toId)) return false;
            // Avoid duplicate connections
            if (std::find(node.nextNodes.begin(), node.nextNodes.end(), toId) != node.nextNodes.end())
                return false;
            node.nextNodes.push_back(toId);
            return true;
        }
    }
    return false;
}

bool QuestEditorPanel::DisconnectNodes(uint64_t fromId, uint64_t toId) {
    for (auto& node : m_nodes) {
        if (node.id == fromId) {
            auto& next = node.nextNodes;
            auto it = std::find(next.begin(), next.end(), toId);
            if (it != next.end()) {
                next.erase(it);
                return true;
            }
            return false;
        }
    }
    return false;
}

std::vector<uint64_t> QuestEditorPanel::GetConnections(uint64_t nodeId) const {
    for (auto& node : m_nodes) {
        if (node.id == nodeId) return node.nextNodes;
    }
    return {};
}

void QuestEditorPanel::SelectNode(uint64_t id) {
    m_selectedID = id;
}

uint64_t QuestEditorPanel::SelectedNodeID() const {
    return m_selectedID;
}

void QuestEditorPanel::SetPreviewMode(bool enabled) {
    m_previewMode = enabled;
}

bool QuestEditorPanel::IsPreviewMode() const {
    return m_previewMode;
}

std::string QuestEditorPanel::ExportQuest() const {
    std::ostringstream ss;
    ss << "{\"nodes\":[";
    bool first = true;
    for (auto& n : m_nodes) {
        if (!first) ss << ",";
        first = false;
        ss << "{\"id\":" << n.id
           << ",\"name\":\"" << n.name << "\""
           << ",\"description\":\"" << n.description << "\""
           << ",\"type\":\"" << n.nodeType << "\""
           << ",\"completed\":" << (n.completed ? "true" : "false")
           << ",\"nextNodes\":[";
        bool firstConn = true;
        for (auto cid : n.nextNodes) {
            if (!firstConn) ss << ",";
            firstConn = false;
            ss << cid;
        }
        ss << "]}";
    }
    ss << "]}";
    return ss.str();
}

bool QuestEditorPanel::ImportQuest(const std::string& json) {
    if (json.empty()) return false;
    if (json.find("\"nodes\"") == std::string::npos) return false;
    return true;
}

void QuestEditorPanel::Clear() {
    m_nodes.clear();
    m_nextID = 1;
    m_selectedID = 0;
    m_previewMode = false;
}

} // namespace atlas::editor
