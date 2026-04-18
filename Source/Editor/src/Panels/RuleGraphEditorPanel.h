#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::editor {

/// Unique identifier for a rule node in the graph editor.
using RuleNodeID = uint32_t;

/// Visual state of a single auto-tile rule node.
struct RuleNodeUI {
    RuleNodeID id        = 0;
    std::string label;
    float      x         = 0.0f;
    float      y         = 0.0f;
    float      width     = 120.0f;
    float      height    = 60.0f;
    bool       selected  = false;
    bool       collapsed = false;
};

/// Visual state of a connection (wire) between two rule nodes.
struct RuleWire {
    RuleNodeID fromNode = 0;
    uint32_t   fromPin  = 0;
    RuleNodeID toNode   = 0;
    uint32_t   toPin    = 0;
};

/// Node-based auto-tile rule editor panel.
/// Provides a visual graph where designers can create, connect, and
/// preview auto-tiling rules without writing code.
class RuleGraphEditorPanel : public EditorPanel {
public:
    RuleGraphEditorPanel();

    const char* Name() const override;
    void Draw() override;

    // --- Node management ---

    RuleNodeID AddNode(const std::string& label, float x, float y);
    void       RemoveNode(RuleNodeID id);
    RuleNodeUI* GetNode(RuleNodeID id);
    const RuleNodeUI* GetNode(RuleNodeID id) const;
    size_t     NodeCount() const;

    // --- Wire management ---

    void   AddWire(RuleNodeID from, uint32_t fromPin,
                   RuleNodeID to,   uint32_t toPin);
    void   RemoveWiresForNode(RuleNodeID id);
    size_t WireCount() const;
    const std::vector<RuleWire>& Wires() const;

    // --- Selection ---

    void   SelectNode(RuleNodeID id);
    void   ClearSelection();
    RuleNodeID SelectedNodeId() const;

    // --- Pan / Zoom ---

    void  SetPan(float x, float y);
    float PanX() const;
    float PanY() const;

    void  SetZoom(float z);
    float Zoom() const;

    // --- Debug mode ---

    void SetDebugMode(bool on);
    bool IsDebugMode() const;

    // --- Draw commands ---

    const atlas::ui::UIDrawList& GetDrawList() const;

private:
    std::vector<RuleNodeUI> m_nodes;
    std::vector<RuleWire>   m_wires;
    RuleNodeID              m_nextId    = 1;
    RuleNodeID              m_selected  = 0;
    float                   m_panX      = 0.0f;
    float                   m_panY      = 0.0f;
    float                   m_zoom      = 1.0f;
    bool                    m_debugMode = false;
    atlas::ui::UIDrawList   m_drawList;
};

} // namespace atlas::editor
