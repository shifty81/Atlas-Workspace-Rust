#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/MultiplayerPreview.h"
#include "NF/Editor/PacketCapture.h"
#include "NF/Editor/NetworkDebugger.h"

using namespace NF;

// ── S86: NetworkDebugger + PacketCapture + MultiplayerPreview ────

// ── NetworkDebugger ──────────────────────────────────────────────

TEST_CASE("NetConnectionState names are correct", "[Editor][S86]") {
    REQUIRE(std::string(netConnectionStateName(NetConnectionState::Disconnected)) == "Disconnected");
    REQUIRE(std::string(netConnectionStateName(NetConnectionState::Connecting))   == "Connecting");
    REQUIRE(std::string(netConnectionStateName(NetConnectionState::Connected))    == "Connected");
    REQUIRE(std::string(netConnectionStateName(NetConnectionState::Reconnecting)) == "Reconnecting");
    REQUIRE(std::string(netConnectionStateName(NetConnectionState::Failed))       == "Failed");
}

TEST_CASE("NetProtocol names are correct", "[Editor][S86]") {
    REQUIRE(std::string(netProtocolName(NetProtocol::UDP))       == "UDP");
    REQUIRE(std::string(netProtocolName(NetProtocol::TCP))       == "TCP");
    REQUIRE(std::string(netProtocolName(NetProtocol::WebSocket)) == "WebSocket");
    REQUIRE(std::string(netProtocolName(NetProtocol::QUIC))      == "QUIC");
    REQUIRE(std::string(netProtocolName(NetProtocol::Relay))     == "Relay");
}

TEST_CASE("NetMessageType names are correct", "[Editor][S86]") {
    REQUIRE(std::string(netMessageTypeName(NetMessageType::Reliable))            == "Reliable");
    REQUIRE(std::string(netMessageTypeName(NetMessageType::Unreliable))          == "Unreliable");
    REQUIRE(std::string(netMessageTypeName(NetMessageType::ReliableOrdered))     == "ReliableOrdered");
    REQUIRE(std::string(netMessageTypeName(NetMessageType::UnreliableSequenced)) == "UnreliableSequenced");
    REQUIRE(std::string(netMessageTypeName(NetMessageType::Broadcast))           == "Broadcast");
}

TEST_CASE("NetPeerEntry stores properties", "[Editor][S86]") {
    NetPeerEntry peer("p1", "192.168.1.1");
    peer.setState(NetConnectionState::Connected);
    peer.setProtocol(NetProtocol::UDP);
    peer.setLatencyMs(42.0f);
    peer.setPacketLoss(0.5f);
    peer.setBytesIn(1024);
    peer.setBytesOut(2048);

    REQUIRE(peer.id() == "p1");
    REQUIRE(peer.address() == "192.168.1.1");
    REQUIRE(peer.state() == NetConnectionState::Connected);
    REQUIRE(peer.protocol() == NetProtocol::UDP);
    REQUIRE(peer.latencyMs() == 42.0f);
    REQUIRE(peer.isConnected());
    REQUIRE(peer.bytesIn()  == 1024);
    REQUIRE(peer.bytesOut() == 2048);
}

TEST_CASE("NetPeerEntry hasBadLatency threshold", "[Editor][S86]") {
    NetPeerEntry peer("p2", "10.0.0.1");
    peer.setLatencyMs(150.0f);
    REQUIRE(peer.hasBadLatency());
    REQUIRE_FALSE(peer.hasBadLatency(200.0f));
}

TEST_CASE("NetworkDebuggerPanel add and find peer", "[Editor][S86]") {
    NetworkDebuggerPanel panel;
    NetPeerEntry peer("peer1", "127.0.0.1");
    peer.setState(NetConnectionState::Connected);
    REQUIRE(panel.addPeer(peer));
    REQUIRE(panel.peerCount() == 1);
    REQUIRE(panel.findPeer("peer1") != nullptr);
}

TEST_CASE("NetworkDebuggerPanel rejects duplicate peer id", "[Editor][S86]") {
    NetworkDebuggerPanel panel;
    NetPeerEntry p("dup", "1.2.3.4");
    panel.addPeer(p);
    REQUIRE_FALSE(panel.addPeer(p));
}

TEST_CASE("NetworkDebuggerPanel remove and select peer", "[Editor][S86]") {
    NetworkDebuggerPanel panel;
    NetPeerEntry p1("a", "1.1.1.1");
    NetPeerEntry p2("b", "2.2.2.2");
    p1.setState(NetConnectionState::Connected);
    panel.addPeer(p1);
    panel.addPeer(p2);
    REQUIRE(panel.selectPeer("a"));
    REQUIRE(panel.selectedPeer() == "a");
    panel.removePeer("a");
    REQUIRE(panel.peerCount() == 1);
    REQUIRE(panel.selectedPeer().empty());
}

TEST_CASE("NetworkDebuggerPanel connectedCount and countByProtocol", "[Editor][S86]") {
    NetworkDebuggerPanel panel;
    NetPeerEntry p1("x", "1.1.1.1"); p1.setState(NetConnectionState::Connected); p1.setProtocol(NetProtocol::UDP);
    NetPeerEntry p2("y", "2.2.2.2"); p2.setState(NetConnectionState::Connecting); p2.setProtocol(NetProtocol::TCP);
    NetPeerEntry p3("z", "3.3.3.3"); p3.setState(NetConnectionState::Connected); p3.setProtocol(NetProtocol::UDP);
    panel.addPeer(p1); panel.addPeer(p2); panel.addPeer(p3);
    REQUIRE(panel.connectedCount() == 2);
    REQUIRE(panel.countByProtocol(NetProtocol::UDP) == 2);
    REQUIRE(panel.countByProtocol(NetProtocol::TCP) == 1);
}

TEST_CASE("NetworkDebuggerPanel MAX_PEERS is 64", "[Editor][S86]") {
    REQUIRE(NetworkDebuggerPanel::MAX_PEERS == 64);
}

// ── PacketCapture ────────────────────────────────────────────────

TEST_CASE("PacketDirection names are correct", "[Editor][S86]") {
    REQUIRE(std::string(packetDirectionName(PacketDirection::Inbound))  == "Inbound");
    REQUIRE(std::string(packetDirectionName(PacketDirection::Outbound)) == "Outbound");
    REQUIRE(std::string(packetDirectionName(PacketDirection::Dropped))  == "Dropped");
}

TEST_CASE("PacketCategory names are correct", "[Editor][S86]") {
    REQUIRE(std::string(packetCategoryName(PacketCategory::State))   == "State");
    REQUIRE(std::string(packetCategoryName(PacketCategory::Input))   == "Input");
    REQUIRE(std::string(packetCategoryName(PacketCategory::RPC))     == "RPC");
    REQUIRE(std::string(packetCategoryName(PacketCategory::Spawn))   == "Spawn");
    REQUIRE(std::string(packetCategoryName(PacketCategory::Despawn)) == "Despawn");
    REQUIRE(std::string(packetCategoryName(PacketCategory::Custom))  == "Custom");
}

TEST_CASE("CaptureFilterMode names are correct", "[Editor][S86]") {
    REQUIRE(std::string(captureFilterModeName(CaptureFilterMode::All))           == "All");
    REQUIRE(std::string(captureFilterModeName(CaptureFilterMode::DirectionOnly)) == "DirectionOnly");
    REQUIRE(std::string(captureFilterModeName(CaptureFilterMode::CategoryOnly))  == "CategoryOnly");
    REQUIRE(std::string(captureFilterModeName(CaptureFilterMode::PeerOnly))      == "PeerOnly");
    REQUIRE(std::string(captureFilterModeName(CaptureFilterMode::Custom))        == "Custom");
}

TEST_CASE("PacketCapturePanel start stop and record", "[Editor][S86]") {
    PacketCapturePanel panel;
    REQUIRE_FALSE(panel.isCapturing());
    REQUIRE_FALSE(panel.record(PacketDirection::Inbound, PacketCategory::State, 100));
    panel.start();
    REQUIRE(panel.isCapturing());
    REQUIRE(panel.record(PacketDirection::Inbound, PacketCategory::State, 100));
    REQUIRE(panel.recordCount() == 1);
    panel.stop();
    REQUIRE_FALSE(panel.isCapturing());
}

TEST_CASE("PacketCapturePanel clear resets records", "[Editor][S86]") {
    PacketCapturePanel panel;
    panel.start();
    panel.record(PacketDirection::Outbound, PacketCategory::RPC, 50);
    panel.record(PacketDirection::Inbound, PacketCategory::Input, 32);
    REQUIRE(panel.recordCount() == 2);
    panel.clear();
    REQUIRE(panel.recordCount() == 0);
}

TEST_CASE("PacketCapturePanel countByDirection and totalBytesInbound", "[Editor][S86]") {
    PacketCapturePanel panel;
    panel.start();
    panel.record(PacketDirection::Inbound,  PacketCategory::State, 200);
    panel.record(PacketDirection::Inbound,  PacketCategory::Input, 100);
    panel.record(PacketDirection::Outbound, PacketCategory::RPC,   50);
    REQUIRE(panel.countByDirection(PacketDirection::Inbound)  == 2);
    REQUIRE(panel.countByDirection(PacketDirection::Outbound) == 1);
    REQUIRE(panel.totalBytesInbound() == 300);
}

TEST_CASE("PacketCapturePanel filterMode", "[Editor][S86]") {
    PacketCapturePanel panel;
    REQUIRE(panel.filterMode() == CaptureFilterMode::All);
    panel.setFilterMode(CaptureFilterMode::CategoryOnly);
    REQUIRE(panel.filterMode() == CaptureFilterMode::CategoryOnly);
}

TEST_CASE("PacketCapturePanel MAX_RECORDS is 4096", "[Editor][S86]") {
    REQUIRE(PacketCapturePanel::MAX_RECORDS == 4096);
}

// ── MultiplayerPreview ───────────────────────────────────────────

TEST_CASE("MultiplayerRole names are correct", "[Editor][S86]") {
    REQUIRE(std::string(multiplayerRoleName(MultiplayerRole::Host))            == "Host");
    REQUIRE(std::string(multiplayerRoleName(MultiplayerRole::Client))          == "Client");
    REQUIRE(std::string(multiplayerRoleName(MultiplayerRole::DedicatedServer)) == "DedicatedServer");
    REQUIRE(std::string(multiplayerRoleName(MultiplayerRole::ListenServer))    == "ListenServer");
    REQUIRE(std::string(multiplayerRoleName(MultiplayerRole::Spectator))       == "Spectator");
}

TEST_CASE("MultiplayerSimMode names are correct", "[Editor][S86]") {
    REQUIRE(std::string(multiplayerSimModeName(MultiplayerSimMode::Local))     == "Local");
    REQUIRE(std::string(multiplayerSimModeName(MultiplayerSimMode::LAN))       == "LAN");
    REQUIRE(std::string(multiplayerSimModeName(MultiplayerSimMode::WAN))       == "WAN");
    REQUIRE(std::string(multiplayerSimModeName(MultiplayerSimMode::Simulated)) == "Simulated");
    REQUIRE(std::string(multiplayerSimModeName(MultiplayerSimMode::Relay))     == "Relay");
}

TEST_CASE("PlayerSlotState names are correct", "[Editor][S86]") {
    REQUIRE(std::string(playerSlotStateName(PlayerSlotState::Empty))    == "Empty");
    REQUIRE(std::string(playerSlotStateName(PlayerSlotState::Occupied)) == "Occupied");
    REQUIRE(std::string(playerSlotStateName(PlayerSlotState::Bot))      == "Bot");
    REQUIRE(std::string(playerSlotStateName(PlayerSlotState::Reserved)) == "Reserved");
    REQUIRE(std::string(playerSlotStateName(PlayerSlotState::Kicked))   == "Kicked");
}

TEST_CASE("MultiplayerPreviewPanel slot count is MAX_SLOTS", "[Editor][S86]") {
    MultiplayerPreviewPanel panel;
    REQUIRE(panel.slotCount() == MultiplayerPreviewPanel::MAX_SLOTS);
}

TEST_CASE("MultiplayerPreviewPanel setSlot and occupiedCount", "[Editor][S86]") {
    MultiplayerPreviewPanel panel;
    REQUIRE(panel.setSlot(0, PlayerSlotState::Occupied, MultiplayerRole::Host));
    REQUIRE(panel.setSlot(1, PlayerSlotState::Bot,      MultiplayerRole::Client));
    REQUIRE(panel.setSlot(2, PlayerSlotState::Occupied, MultiplayerRole::Client));
    REQUIRE(panel.occupiedCount() == 2);
    REQUIRE(panel.botCount()      == 1);
}

TEST_CASE("MultiplayerPreviewPanel setSlot rejects out-of-range index", "[Editor][S86]") {
    MultiplayerPreviewPanel panel;
    REQUIRE_FALSE(panel.setSlot(MultiplayerPreviewPanel::MAX_SLOTS, PlayerSlotState::Occupied, MultiplayerRole::Client));
}

TEST_CASE("MultiplayerPreviewPanel simMode and running", "[Editor][S86]") {
    MultiplayerPreviewPanel panel;
    panel.setSimMode(MultiplayerSimMode::LAN);
    panel.setSimLatencyMs(50);
    panel.setRunning(true);
    REQUIRE(panel.simMode()      == MultiplayerSimMode::LAN);
    REQUIRE(panel.simLatencyMs() == 50);
    REQUIRE(panel.isRunning());
}

TEST_CASE("MultiplayerPreviewPanel MAX_SLOTS is 16", "[Editor][S86]") {
    REQUIRE(MultiplayerPreviewPanel::MAX_SLOTS == 16);
}
