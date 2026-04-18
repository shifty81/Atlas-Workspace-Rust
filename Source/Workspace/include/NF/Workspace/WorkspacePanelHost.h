#pragma once
// NF::Editor::WorkspacePanelHost
// Manages all 8 core AtlasUI panels for the workspace.
// Maps DockLayout slot bounds → panel arrange/paint/handleInput each frame.
// Drop-in replacement for the legacy EditorPanel render loop.
// Note: EditorApp is deprecated; the canonical path is WorkspaceShell → WorkspaceRenderer.

#include "NF/UI/AtlasUI/PanelHost.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/UI/AtlasUI/DrawListDispatcher.h"

#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include "NF/UI/AtlasUI/Panels/HierarchyPanel.h"
#include "NF/UI/AtlasUI/Panels/InspectorPanel.h"
#include "NF/UI/AtlasUI/Panels/ConsolePanel.h"
#include "NF/UI/AtlasUI/Panels/ContentBrowserPanel.h"
#include "NF/UI/AtlasUI/Panels/IDEPanel.h"
#include "NF/UI/AtlasUI/Panels/GraphEditorPanel.h"
#include "NF/UI/AtlasUI/Panels/PipelineMonitorPanel.h"

#include "NF/Workspace/SelectionService.h"
#include "NF/Editor/DockLayout.h"
#include "NF/Workspace/WorkspaceInputBridge.h"
#include "NF/Input/Input.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace NF {

class WorkspacePanelHost {
public:
    WorkspacePanelHost() {
        // Create all core AtlasUI panels
        m_viewport      = std::make_shared<UI::AtlasUI::ViewportPanel>();
        m_hierarchy     = std::make_shared<UI::AtlasUI::HierarchyPanel>();
        m_inspector     = std::make_shared<UI::AtlasUI::InspectorPanel>();
        m_console       = std::make_shared<UI::AtlasUI::ConsolePanel>();
        m_contentBrowser= std::make_shared<UI::AtlasUI::ContentBrowserPanel>();
        m_ide           = std::make_shared<UI::AtlasUI::IDEPanel>();
        m_graphEditor   = std::make_shared<UI::AtlasUI::GraphEditorPanel>();
        m_pipelineMonitor = std::make_shared<UI::AtlasUI::PipelineMonitorPanel>();

        m_host.attachPanel(m_viewport);
        m_host.attachPanel(m_hierarchy);
        m_host.attachPanel(m_inspector);
        m_host.attachPanel(m_console);
        m_host.attachPanel(m_contentBrowser);
        m_host.attachPanel(m_ide);
        m_host.attachPanel(m_graphEditor);
        m_host.attachPanel(m_pipelineMonitor);

        // Map AtlasUI panel ID → DockLayout panel name
        m_nameMap["atlas.viewport"]        = "Viewport";
        m_nameMap["atlas.hierarchy"]       = "Hierarchy";
        m_nameMap["atlas.inspector"]       = "Inspector";
        m_nameMap["atlas.console"]         = "Console";
        m_nameMap["atlas.content_browser"] = "ContentBrowser";
        m_nameMap["atlas.ide"]             = "IDE";
        m_nameMap["atlas.graph_editor"]    = "GraphEditor";
        m_nameMap["atlas.pipeline_monitor"]= "PipelineMonitor";
    }

    /// Bind the UIRenderer used to dispatch AtlasUI DrawList commands to GDI.
    void setRenderer(UIRenderer* renderer) {
        m_dispatcher.setRenderer(renderer);
    }

    /// Render all visible panels: arrange to DockLayout bounds, paint, dispatch.
    /// Call this from the host render loop between beginFrame and endFrame.
    void renderPanels(DockLayout& dock) {
        for (auto& panel : m_host.panels()) {
            auto it = m_nameMap.find(panel->panelId());
            if (it != m_nameMap.end()) {
                auto* dp = dock.findPanel(it->second);
                if (dp && dp->visible && dock.isPanelActive(it->second)) {
                    panel->setVisible(true);
                    panel->arrange(dock.adjustedBoundsForPanel(*dp));
                } else {
                    panel->setVisible(false);
                    continue;
                }
            }
            if (!panel->isVisible()) continue;

            UI::AtlasUI::BasicPaintContext paintCtx;
            panel->paint(paintCtx);
            m_dispatcher.dispatch(paintCtx.drawList());
        }
    }

    /// Route input to all visible panels.
    /// Call this from the host update loop.
    void handleInput(DockLayout& dock, InputSystem& input) {
        (void)dock;
        UI::AtlasUI::BasicInputContext inputCtx;
        m_inputBridge.sync(input, inputCtx);
        for (auto& panel : m_host.panels()) {
            if (!panel->isVisible()) continue;
            panel->handleInput(inputCtx);
        }
    }

    // Panel accessors (for data binding from host)
    [[nodiscard]] UI::AtlasUI::ViewportPanel&       viewport()        { return *m_viewport; }
    [[nodiscard]] UI::AtlasUI::HierarchyPanel&      hierarchy()       { return *m_hierarchy; }
    [[nodiscard]] UI::AtlasUI::InspectorPanel&      inspector()       { return *m_inspector; }
    [[nodiscard]] UI::AtlasUI::ConsolePanel&        console()         { return *m_console; }
    [[nodiscard]] UI::AtlasUI::ContentBrowserPanel& contentBrowser()  { return *m_contentBrowser; }
    [[nodiscard]] UI::AtlasUI::IDEPanel&            ide()             { return *m_ide; }
    [[nodiscard]] UI::AtlasUI::GraphEditorPanel&    graphEditor()     { return *m_graphEditor; }
    [[nodiscard]] UI::AtlasUI::PipelineMonitorPanel& pipelineMonitor(){ return *m_pipelineMonitor; }

    [[nodiscard]] const UI::AtlasUI::PanelHost& panelHost() const { return m_host; }
    [[nodiscard]] UI::AtlasUI::PanelHost& panelHost() { return m_host; }

private:
    UI::AtlasUI::PanelHost m_host;
    UI::AtlasUI::DrawListDispatcher m_dispatcher;
    WorkspaceInputBridge m_inputBridge;
    std::unordered_map<std::string, std::string> m_nameMap;

    std::shared_ptr<UI::AtlasUI::ViewportPanel>        m_viewport;
    std::shared_ptr<UI::AtlasUI::HierarchyPanel>       m_hierarchy;
    std::shared_ptr<UI::AtlasUI::InspectorPanel>       m_inspector;
    std::shared_ptr<UI::AtlasUI::ConsolePanel>         m_console;
    std::shared_ptr<UI::AtlasUI::ContentBrowserPanel>  m_contentBrowser;
    std::shared_ptr<UI::AtlasUI::IDEPanel>             m_ide;
    std::shared_ptr<UI::AtlasUI::GraphEditorPanel>     m_graphEditor;
    std::shared_ptr<UI::AtlasUI::PipelineMonitorPanel> m_pipelineMonitor;
};

} // namespace NF
