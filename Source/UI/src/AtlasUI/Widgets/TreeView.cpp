#include "NF/UI/AtlasUI/Widgets/TreeView.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

bool TreeView::selectNode(const std::string& nodeId) {
    if (selectInNodes(m_roots, nodeId)) {
        m_selectedId = nodeId;
        if (m_onSelect) m_onSelect(nodeId);
        m_flatDirty = true;
        return true;
    }
    return false;
}

bool TreeView::toggleNode(const std::string& nodeId) {
    if (toggleInNodes(m_roots, nodeId)) {
        m_flatDirty = true;
        return true;
    }
    return false;
}

bool TreeView::expandAll() {
    setExpandAll(m_roots, true);
    m_flatDirty = true;
    return !m_roots.empty();
}

bool TreeView::collapseAll() {
    setExpandAll(m_roots, false);
    m_flatDirty = true;
    return !m_roots.empty();
}

size_t TreeView::visibleNodeCount() const {
    if (m_flatDirty) {
        // const_cast to rebuild cache; logically const
        const_cast<TreeView*>(this)->rebuildFlat();
    }
    return m_flatNodes.size();
}

void TreeView::rebuildFlat() {
    m_flatNodes.clear();
    flattenNodes(m_roots, 0);
    m_flatDirty = false;
}

void TreeView::flattenNodes(std::vector<TreeNode>& nodes, int depth) {
    for (auto& node : nodes) {
        m_flatNodes.push_back({&node, depth, 0.f});
        if (node.expanded && !node.children.empty()) {
            flattenNodes(node.children, depth + 1);
        }
    }
}

bool TreeView::selectInNodes(std::vector<TreeNode>& nodes, const std::string& id) {
    for (auto& node : nodes) {
        node.selected = (node.id == id);
        selectInNodes(node.children, id);
        if (node.selected) return true;
    }
    return false;
}

bool TreeView::toggleInNodes(std::vector<TreeNode>& nodes, const std::string& id) {
    for (auto& node : nodes) {
        if (node.id == id) {
            node.expanded = !node.expanded;
            if (m_onExpand) m_onExpand(node.id, node.expanded);
            return true;
        }
        if (toggleInNodes(node.children, id)) return true;
    }
    return false;
}

void TreeView::setExpandAll(std::vector<TreeNode>& nodes, bool expanded) {
    for (auto& node : nodes) {
        if (!node.children.empty()) {
            node.expanded = expanded;
        }
        setExpandAll(node.children, expanded);
    }
}

void TreeView::measure(ILayoutContext&) {
    if (m_flatDirty) rebuildFlat();
    float totalHeight = static_cast<float>(m_flatNodes.size()) * m_rowHeight;
    m_bounds.h = totalHeight;
}

void TreeView::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    if (m_flatDirty) rebuildFlat();
    float y = bounds.y;
    for (auto& flat : m_flatNodes) {
        flat.y = y;
        y += m_rowHeight;
    }
}

void TreeView::paint(IPaintContext& context) {
    if (!m_visible) return;
    if (m_flatDirty) rebuildFlat();

    context.pushClip(m_bounds);

    for (const auto& flat : m_flatNodes) {
        const auto* node = flat.node;
        float indent = static_cast<float>(flat.depth) * m_indentWidth;
        NF::Rect rowRect = {m_bounds.x, flat.y, m_bounds.w, m_rowHeight};

        if (node->selected) {
            context.fillRect(rowRect, Theme::ColorToken::Selection);
        }

        // Draw expand/collapse indicator for non-leaf nodes
        if (!node->isLeaf()) {
            NF::Rect arrowRect = {m_bounds.x + indent + 2.f, flat.y + 4.f, 16.f, 16.f};
            const char* arrow = node->expanded ? "v" : ">";
            context.drawText(arrowRect, arrow, 0, Theme::ColorToken::TextMuted);
        }

        // Draw label
        NF::Rect labelRect = {
            m_bounds.x + indent + (node->isLeaf() ? 18.f : 20.f),
            flat.y,
            std::max(0.f, m_bounds.w - indent - 20.f),
            m_rowHeight
        };
        context.drawText(insetRect(labelRect, Theme::Spacing::Tiny, Theme::Spacing::Tiny),
                         node->label, 0, Theme::ColorToken::Text);
    }

    context.popClip();
}

bool TreeView::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) return false;
    if (m_flatDirty) rebuildFlat();

    for (const auto& flat : m_flatNodes) {
        NF::Rect rowRect = {m_bounds.x, flat.y, m_bounds.w, m_rowHeight};
        if (rectContains(rowRect, context.mousePosition()) && context.primaryDown()) {
            float indent = static_cast<float>(flat.depth) * m_indentWidth;

            // Click on expand arrow area
            if (!flat.node->isLeaf()) {
                NF::Rect arrowRect = {m_bounds.x + indent, flat.y, 20.f, m_rowHeight};
                if (rectContains(arrowRect, context.mousePosition())) {
                    toggleNode(flat.node->id);
                    return true;
                }
            }

            // Select node
            selectNode(flat.node->id);
            return true;
        }
    }
    return false;
}

} // namespace NF::UI::AtlasUI
