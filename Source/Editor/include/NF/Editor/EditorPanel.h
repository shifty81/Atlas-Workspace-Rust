#pragma once
// NF::Editor — Editor panel base class
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Workspace/SelectionService.h"
#include "NF/Editor/EditorTheme.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

class EditorPanel {
public:
    virtual ~EditorPanel() = default;

    [[nodiscard]] virtual const std::string& name() const = 0;
    [[nodiscard]] virtual DockSlot slot() const = 0;
    virtual void update(float dt) = 0;
    virtual void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) = 0;
    virtual void renderUI(UIContext& ctx, const Rect& bounds) { (void)ctx; (void)bounds; }

    [[nodiscard]] bool isVisible() const { return m_visible; }
    void setVisible(bool v) { m_visible = v; }

private:
    bool m_visible = true;
};

} // namespace NF
