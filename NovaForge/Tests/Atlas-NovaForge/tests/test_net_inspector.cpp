#include "../editor/panels/NetInspectorPanel.h"
#include "../engine/net/NetContext.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

using namespace atlas::editor;
using namespace atlas::net;

void test_net_inspector_standalone() {
    NetContext net;
    net.Init(NetMode::Standalone);

    NetInspectorPanel panel(net);
    panel.Draw();

    auto& snap = panel.Snapshot();
    assert(snap.modeString == "Standalone");
    assert(snap.peerCount == 0);
    assert(snap.connectedCount == 0);
    assert(snap.avgRtt == 0.0f);

    std::cout << "[PASS] test_net_inspector_standalone" << std::endl;
}

void test_net_inspector_server_with_peers() {
    NetContext net;
    net.Init(NetMode::Server);

    net.AddPeer();
    net.AddPeer();

    NetInspectorPanel panel(net);
    panel.Draw();

    auto& snap = panel.Snapshot();
    assert(snap.modeString == "Server");
    assert(snap.peerCount == 2);

    std::cout << "[PASS] test_net_inspector_server_with_peers" << std::endl;
}

void test_net_inspector_mode_names() {
    NetContext net;
    NetInspectorPanel panel(net);

    net.Init(NetMode::Client);
    panel.Draw();
    assert(panel.Snapshot().modeString == "Client");

    net.Init(NetMode::P2P_Host);
    panel.Draw();
    assert(panel.Snapshot().modeString == "P2P_Host");

    net.Init(NetMode::P2P_Peer);
    panel.Draw();
    assert(panel.Snapshot().modeString == "P2P_Peer");

    std::cout << "[PASS] test_net_inspector_mode_names" << std::endl;
}
