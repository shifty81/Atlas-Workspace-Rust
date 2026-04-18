#include "RuleGraphEditorPanel.h"
#include <algorithm>
#include <cstdio>

namespace atlas::editor {

RuleGraphEditorPanel::RuleGraphEditorPanel() {
    SetVisible(true);
}

const char* RuleGraphEditorPanel::Name() const {
    return "Rule Graph Editor";
}

void RuleGraphEditorPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 800, 600}, {35, 35, 35, 255});

    // Header
    m_drawList.DrawText({4, 4, 200, 20}, "Auto-Tile Rule Graph", {220, 220, 220, 255});

    if (m_debugMode) {
        m_drawList.DrawText({210, 4, 100, 20}, "[DEBUG]", {255, 200, 0, 255});
    }

    // Draw wires first (behind nodes)
    for (auto& wire : m_wires) {
        auto* from = GetNode(wire.fromNode);
        auto* to   = GetNode(wire.toNode);
        if (!from || !to) continue;

        // Simple straight line between node centres (approximate with a rect)
        float fx = (from->x + from->width  * 0.5f + m_panX) * m_zoom;
        float fy = (from->y + from->height * 0.5f + m_panY) * m_zoom;
        float tx = (to->x   + to->width    * 0.5f + m_panX) * m_zoom;
        float ty = (to->y   + to->height   * 0.5f + m_panY) * m_zoom;

        // Draw as a thin horizontal then vertical segment
        int32_t midX = static_cast<int32_t>((fx + tx) * 0.5f);
        m_drawList.DrawRect(
            {static_cast<int32_t>(fx), static_cast<int32_t>(fy), midX - static_cast<int32_t>(fx), 2},
            {180, 180, 180, 200});
        m_drawList.DrawRect(
            {midX, static_cast<int32_t>(fy), 2, static_cast<int32_t>(ty - fy)},
            {180, 180, 180, 200});
        m_drawList.DrawRect(
            {midX, static_cast<int32_t>(ty), static_cast<int32_t>(tx) - midX, 2},
            {180, 180, 180, 200});
    }

    // Draw nodes
    for (auto& node : m_nodes) {
        float nx = (node.x + m_panX) * m_zoom;
        float ny = (node.y + m_panY) * m_zoom;
        float nw = node.width  * m_zoom;
        float nh = node.height * m_zoom;

        atlas::ui::UIRect nodeRect{
            static_cast<int32_t>(nx), static_cast<int32_t>(ny),
            static_cast<int32_t>(nw), static_cast<int32_t>(nh)};

        // Node body
        atlas::ui::UIColor bodyColor = {55, 55, 55, 255};
        if (node.selected) {
            bodyColor = {0, 90, 160, 255};
        }
        m_drawList.DrawRect(nodeRect, bodyColor);

        // Border
        atlas::ui::UIColor borderColor = node.selected
            ? atlas::ui::UIColor{0, 150, 255, 255}
            : atlas::ui::UIColor{80, 80, 80, 255};
        m_drawList.DrawBorder(nodeRect, 1, borderColor);

        // Label
        atlas::ui::UIRect labelRect{
            nodeRect.x + 4, nodeRect.y + 4,
            nodeRect.w - 8, 16};
        m_drawList.DrawText(labelRect, node.label, {220, 220, 220, 255});

        // Debug info
        if (m_debugMode) {
            char dbg[64];
            std::snprintf(dbg, sizeof(dbg), "id:%u", node.id);
            m_drawList.DrawText(
                {nodeRect.x + 4, nodeRect.y + nodeRect.h - 16, nodeRect.w - 8, 12},
                dbg, {255, 200, 0, 180});
        }
    }
}

// --- Node management ---

RuleNodeID RuleGraphEditorPanel::AddNode(const std::string& label, float x, float y) {
    RuleNodeUI node;
    node.id    = m_nextId++;
    node.label = label;
    node.x     = x;
    node.y     = y;
    m_nodes.push_back(node);
    return node.id;
}

void RuleGraphEditorPanel::RemoveNode(RuleNodeID id) {
    RemoveWiresForNode(id);
    m_nodes.erase(
        std::remove_if(m_nodes.begin(), m_nodes.end(),
            [id](const RuleNodeUI& n) { return n.id == id; }),
        m_nodes.end());
    if (m_selected == id) m_selected = 0;
}

RuleNodeUI* RuleGraphEditorPanel::GetNode(RuleNodeID id) {
    for (auto& n : m_nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

const RuleNodeUI* RuleGraphEditorPanel::GetNode(RuleNodeID id) const {
    for (auto& n : m_nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

size_t RuleGraphEditorPanel::NodeCount() const {
    return m_nodes.size();
}

// --- Wire management ---

void RuleGraphEditorPanel::AddWire(
    RuleNodeID from, uint32_t fromPin,
    RuleNodeID to,   uint32_t toPin)
{
    m_wires.push_back({from, fromPin, to, toPin});
}

void RuleGraphEditorPanel::RemoveWiresForNode(RuleNodeID id) {
    m_wires.erase(
        std::remove_if(m_wires.begin(), m_wires.end(),
            [id](const RuleWire& w) {
                return w.fromNode == id || w.toNode == id;
            }),
        m_wires.end());
}

size_t RuleGraphEditorPanel::WireCount() const {
    return m_wires.size();
}

const std::vector<RuleWire>& RuleGraphEditorPanel::Wires() const {
    return m_wires;
}

// --- Selection ---

void RuleGraphEditorPanel::SelectNode(RuleNodeID id) {
    for (auto& n : m_nodes) {
        n.selected = (n.id == id);
    }
    m_selected = id;
}

void RuleGraphEditorPanel::ClearSelection() {
    for (auto& n : m_nodes) {
        n.selected = false;
    }
    m_selected = 0;
}

RuleNodeID RuleGraphEditorPanel::SelectedNodeId() const {
    return m_selected;
}

// --- Pan / Zoom ---

void RuleGraphEditorPanel::SetPan(float x, float y) {
    m_panX = x;
    m_panY = y;
}

float RuleGraphEditorPanel::PanX() const { return m_panX; }
float RuleGraphEditorPanel::PanY() const { return m_panY; }

void RuleGraphEditorPanel::SetZoom(float z) {
    if (z > 0.1f && z < 10.0f) m_zoom = z;
}

float RuleGraphEditorPanel::Zoom() const { return m_zoom; }

// --- Debug mode ---

void RuleGraphEditorPanel::SetDebugMode(bool on) { m_debugMode = on; }
bool RuleGraphEditorPanel::IsDebugMode() const { return m_debugMode; }

// --- Draw commands ---

const atlas::ui::UIDrawList& RuleGraphEditorPanel::GetDrawList() const {
    return m_drawList;
}

} // namespace atlas::editor
