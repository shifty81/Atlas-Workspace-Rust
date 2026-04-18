#pragma once
#include "UIRenderer.h"
#include "UIDrawList.h"
#include "UIEventRouter.h"
#include "UILayoutSolver.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace atlas::ui {

/// Direction for child layout.
enum class SceneLayoutDir : uint8_t {
    Vertical,
    Horizontal,
    Absolute
};

/// A retained-mode scene graph node.  Nodes form a tree and support
/// layout solving, hit-testing, event bubbling, and deferred drawing.
/// This is the backbone of the custom (non-ImGui) UI system.
class UISceneNode {
public:
    virtual ~UISceneNode() = default;

    // ---- Identification ----
    uint32_t           id     = 0;
    std::string        name;

    // ---- Geometry (resolved by layout pass) ----
    UIRect             bounds {};

    // ---- Layout hints ----
    UIConstraint       constraint {};
    SceneLayoutDir     layoutDir = SceneLayoutDir::Vertical;

    // ---- Visibility ----
    bool               visible = true;

    // ---- Tree API ----
    void               AddChild(std::unique_ptr<UISceneNode> child);
    void               RemoveChild(uint32_t childId);
    UISceneNode*       FindChild(uint32_t childId) const;
    const std::vector<std::unique_ptr<UISceneNode>>& Children() const;
    size_t             ChildCount() const;

    // ---- Lifecycle ----

    /// Solve layout for this node and all descendants given an
    /// available rectangle.
    void               Layout(const UIRect& available);

    /// Emit draw commands into the supplied draw list.
    virtual void       Draw(UIDrawList& drawList) const;

    /// Recursively draw this node and all visible children.
    void               DrawTree(UIDrawList& drawList) const;

    // ---- Events ----

    /// Point-in-bounds check (screen coordinates).
    bool               HitTest(int32_t x, int32_t y) const;

    /// Dispatch an event down the tree (topmost child first).
    /// Returns true if the event was consumed.
    virtual bool       HandleEvent(const UIEvent& event);

private:
    static uint32_t NextGlobalId();

    std::vector<std::unique_ptr<UISceneNode>> m_children;
};

/// Root of the scene graph — thin wrapper that owns the top-level node.
class UISceneGraph {
public:
    void                Init(const std::string& name);
    const std::string&  GetName() const;

    /// Access the root node.
    UISceneNode&        Root();
    const UISceneNode&  Root() const;

    /// Solve layout starting from the root, using the given viewport rect.
    void                Layout(const UIRect& viewport);

    /// Draw the entire tree into a draw list.
    void                DrawAll(UIDrawList& drawList) const;

    /// Dispatch an event from the root.
    bool                DispatchEvent(const UIEvent& event);

private:
    std::string                    m_name;
    std::unique_ptr<UISceneNode>   m_root;
};

} // namespace atlas::ui
