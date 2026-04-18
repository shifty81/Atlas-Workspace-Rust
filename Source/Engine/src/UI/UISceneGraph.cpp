#include "UISceneGraph.h"
#include <algorithm>

namespace atlas::ui {

// ---- UISceneNode ----

uint32_t UISceneNode::NextGlobalId() {
    static uint32_t s_nextId = 1;
    return s_nextId++;
}

void UISceneNode::AddChild(std::unique_ptr<UISceneNode> child) {
    if (!child) return;
    child->id = NextGlobalId();
    m_children.push_back(std::move(child));
}

void UISceneNode::RemoveChild(uint32_t childId) {
    m_children.erase(
        std::remove_if(m_children.begin(), m_children.end(),
            [childId](const std::unique_ptr<UISceneNode>& c) {
                return c->id == childId;
            }),
        m_children.end());
}

UISceneNode* UISceneNode::FindChild(uint32_t childId) const {
    for (auto& c : m_children) {
        if (c->id == childId) return c.get();
        auto* found = c->FindChild(childId);
        if (found) return found;
    }
    return nullptr;
}

const std::vector<std::unique_ptr<UISceneNode>>& UISceneNode::Children() const {
    return m_children;
}

size_t UISceneNode::ChildCount() const {
    return m_children.size();
}

void UISceneNode::Layout(const UIRect& available) {
    bounds = available;

    if (m_children.empty()) return;

    if (layoutDir == SceneLayoutDir::Absolute) {
        // Children use their own constraint position/size
        for (auto& child : m_children) {
            if (!child->visible) continue;
            UIRect childRect;
            childRect.x = available.x + child->constraint.minWidth;
            childRect.y = available.y + child->constraint.minHeight;
            childRect.w = child->constraint.preferredWidth;
            childRect.h = child->constraint.preferredHeight;
            child->Layout(childRect);
        }
        return;
    }

    // Compute total weight and count visible children
    float totalWeight = 0.0f;
    int visibleCount = 0;
    for (auto& child : m_children) {
        if (!child->visible) continue;
        totalWeight += child->constraint.weight;
        ++visibleCount;
    }
    if (visibleCount == 0) return;

    bool horizontal = (layoutDir == SceneLayoutDir::Horizontal);
    int32_t totalSize = horizontal ? available.w : available.h;
    int32_t offset = 0;

    for (auto& child : m_children) {
        if (!child->visible) continue;

        int32_t childSize = (totalWeight > 0.0f)
            ? static_cast<int32_t>((child->constraint.weight / totalWeight) * totalSize)
            : (totalSize / visibleCount);

        UIRect childRect;
        if (horizontal) {
            childRect = {available.x + offset, available.y, childSize, available.h};
        } else {
            childRect = {available.x, available.y + offset, available.w, childSize};
        }

        child->Layout(childRect);
        offset += childSize;
    }
}

void UISceneNode::Draw(UIDrawList& /*drawList*/) const {
    // Base node draws nothing — subclasses override.
}

void UISceneNode::DrawTree(UIDrawList& drawList) const {
    if (!visible) return;
    Draw(drawList);
    for (auto& child : m_children) {
        child->DrawTree(drawList);
    }
}

bool UISceneNode::HitTest(int32_t x, int32_t y) const {
    return x >= bounds.x && x < bounds.x + bounds.w &&
           y >= bounds.y && y < bounds.y + bounds.h;
}

bool UISceneNode::HandleEvent(const UIEvent& event) {
    // Dispatch to children in reverse order (topmost first)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto& child = *it;
        if (!child->visible) continue;
        if (child->HitTest(event.x, event.y)) {
            if (child->HandleEvent(event))
                return true;
        }
    }
    return false;
}

// ---- UISceneGraph ----

void UISceneGraph::Init(const std::string& name) {
    m_name = name;
    m_root = std::make_unique<UISceneNode>();
    m_root->name = "Root";
}

const std::string& UISceneGraph::GetName() const {
    return m_name;
}

UISceneNode& UISceneGraph::Root() {
    return *m_root;
}

const UISceneNode& UISceneGraph::Root() const {
    return *m_root;
}

void UISceneGraph::Layout(const UIRect& viewport) {
    if (m_root) {
        m_root->Layout(viewport);
    }
}

void UISceneGraph::DrawAll(UIDrawList& drawList) const {
    if (m_root) {
        m_root->DrawTree(drawList);
    }
}

bool UISceneGraph::DispatchEvent(const UIEvent& event) {
    if (m_root) {
        return m_root->HandleEvent(event);
    }
    return false;
}

} // namespace atlas::ui
