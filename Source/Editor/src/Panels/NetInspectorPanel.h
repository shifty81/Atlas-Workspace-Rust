#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/net/NetContext.h"
#include "../../engine/ui/UIDrawList.h"

namespace atlas::editor {

struct NetInspectorSnapshot {
    std::string modeString;
    uint32_t peerCount = 0;
    float avgRtt = 0.0f;
    uint32_t connectedCount = 0;
};

struct NetStats {
    std::string modeString;
    uint32_t connectedPeerCount = 0;
};

class NetInspectorPanel : public EditorPanel {
public:
    explicit NetInspectorPanel(net::NetContext& net) : m_net(net) {}

    const char* Name() const override { return "Network"; }
    void Draw() override;

    const NetInspectorSnapshot& Snapshot() const { return m_snapshot; }

    uint32_t SelectedPeer() const { return m_selectedPeer; }
    void SelectPeer(uint32_t id);
    void ClearPeerSelection() { m_selectedPeer = 0; }

    NetStats GetStats() const;

    static std::string ModeToString(net::NetMode mode);

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    net::NetContext& m_net;
    NetInspectorSnapshot m_snapshot;
    uint32_t m_selectedPeer = 0;
    atlas::ui::UIDrawList m_drawList;
};

}
