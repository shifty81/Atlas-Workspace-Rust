#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

struct TreeNode {
    std::string id;
    std::string label;
    bool expanded = false;
    bool selected = false;
    std::vector<TreeNode> children;

    [[nodiscard]] bool isLeaf() const { return children.empty(); }
    [[nodiscard]] size_t childCount() const { return children.size(); }
};

class TreeView final : public WidgetBase {
public:
    using SelectHandler = std::function<void(const std::string& nodeId)>;
    using ExpandHandler = std::function<void(const std::string& nodeId, bool expanded)>;

    TreeView() = default;

    void setRoots(std::vector<TreeNode> roots) { m_roots = std::move(roots); }
    void addRoot(TreeNode node) { m_roots.push_back(std::move(node)); }
    void clear() { m_roots.clear(); m_selectedId.clear(); }

    void setOnSelect(SelectHandler handler) { m_onSelect = std::move(handler); }
    void setOnExpand(ExpandHandler handler) { m_onExpand = std::move(handler); }

    [[nodiscard]] const std::vector<TreeNode>& roots() const { return m_roots; }
    [[nodiscard]] const std::string& selectedId() const { return m_selectedId; }
    [[nodiscard]] size_t rootCount() const { return m_roots.size(); }

    bool selectNode(const std::string& nodeId);
    bool toggleNode(const std::string& nodeId);
    bool expandAll();
    bool collapseAll();

    [[nodiscard]] size_t visibleNodeCount() const;

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    struct FlatNode {
        TreeNode* node = nullptr;
        int depth = 0;
        float y = 0.f;
    };

    void rebuildFlat();
    void flattenNodes(std::vector<TreeNode>& nodes, int depth);
    bool selectInNodes(std::vector<TreeNode>& nodes, const std::string& id);
    bool toggleInNodes(std::vector<TreeNode>& nodes, const std::string& id);
    void setExpandAll(std::vector<TreeNode>& nodes, bool expanded);

    std::vector<TreeNode> m_roots;
    std::vector<FlatNode> m_flatNodes;
    std::string m_selectedId;
    SelectHandler m_onSelect;
    ExpandHandler m_onExpand;
    float m_rowHeight = 24.f;
    float m_indentWidth = 20.f;
    float m_scrollOffset = 0.f;
    bool m_flatDirty = true;
};

} // namespace NF::UI::AtlasUI
