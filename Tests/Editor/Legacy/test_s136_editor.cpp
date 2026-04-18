// S136 editor tests: NetworkTopologyEditor, BandwidthProfileEditor, LatencySimEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LatencySimEditor.h"
#include "NF/Editor/BandwidthProfileEditor.h"
#include "NF/Editor/NetworkTopologyEditor.h"

using namespace NF;

// ── NetworkTopologyEditor ─────────────────────────────────────────────────────

TEST_CASE("NetTopoType names", "[Editor][S136]") {
    REQUIRE(std::string(netTopoTypeName(NetTopoType::P2P))          == "P2P");
    REQUIRE(std::string(netTopoTypeName(NetTopoType::ClientServer))  == "ClientServer");
    REQUIRE(std::string(netTopoTypeName(NetTopoType::RelayServer))   == "RelayServer");
    REQUIRE(std::string(netTopoTypeName(NetTopoType::Mesh))          == "Mesh");
    REQUIRE(std::string(netTopoTypeName(NetTopoType::Hybrid))        == "Hybrid");
}

TEST_CASE("NetTopoProtocol names", "[Editor][S136]") {
    REQUIRE(std::string(netTopoProtocolName(NetTopoProtocol::TCP))       == "TCP");
    REQUIRE(std::string(netTopoProtocolName(NetTopoProtocol::UDP))       == "UDP");
    REQUIRE(std::string(netTopoProtocolName(NetTopoProtocol::WebSocket)) == "WebSocket");
    REQUIRE(std::string(netTopoProtocolName(NetTopoProtocol::WebRTC))    == "WebRTC");
    REQUIRE(std::string(netTopoProtocolName(NetTopoProtocol::Custom))    == "Custom");
}

TEST_CASE("NetworkTopologyDef defaults", "[Editor][S136]") {
    NetworkTopologyDef t(1, "lan_p2p", NetTopoType::P2P, NetTopoProtocol::UDP);
    REQUIRE(t.id()        == 1u);
    REQUIRE(t.name()      == "lan_p2p");
    REQUIRE(t.type()      == NetTopoType::P2P);
    REQUIRE(t.protocol()  == NetTopoProtocol::UDP);
    REQUIRE(t.maxPeers()  == 8u);
    REQUIRE(t.port()      == 7777u);
    REQUIRE(t.isEnabled());
}

TEST_CASE("NetworkTopologyDef mutation", "[Editor][S136]") {
    NetworkTopologyDef t(2, "relay_srv", NetTopoType::RelayServer, NetTopoProtocol::TCP);
    t.setMaxPeers(64u);
    t.setPort(9090u);
    t.setIsEnabled(false);
    REQUIRE(t.maxPeers()  == 64u);
    REQUIRE(t.port()      == 9090u);
    REQUIRE(!t.isEnabled());
}

TEST_CASE("NetworkTopologyEditor defaults", "[Editor][S136]") {
    NetworkTopologyEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.defaultMaxPeers() == 16u);
    REQUIRE(ed.topoCount()       == 0u);
}

TEST_CASE("NetworkTopologyEditor add/remove", "[Editor][S136]") {
    NetworkTopologyEditor ed;
    REQUIRE(ed.addTopology(NetworkTopologyDef(1, "a", NetTopoType::P2P,          NetTopoProtocol::UDP)));
    REQUIRE(ed.addTopology(NetworkTopologyDef(2, "b", NetTopoType::ClientServer,  NetTopoProtocol::TCP)));
    REQUIRE(ed.addTopology(NetworkTopologyDef(3, "c", NetTopoType::Mesh,          NetTopoProtocol::WebRTC)));
    REQUIRE(!ed.addTopology(NetworkTopologyDef(1, "a", NetTopoType::P2P,         NetTopoProtocol::UDP)));
    REQUIRE(ed.topoCount() == 3u);
    REQUIRE(ed.removeTopology(2));
    REQUIRE(ed.topoCount() == 2u);
    REQUIRE(!ed.removeTopology(99));
}

TEST_CASE("NetworkTopologyEditor counts and find", "[Editor][S136]") {
    NetworkTopologyEditor ed;
    NetworkTopologyDef t1(1, "a", NetTopoType::P2P,          NetTopoProtocol::UDP);
    NetworkTopologyDef t2(2, "b", NetTopoType::P2P,          NetTopoProtocol::TCP);
    NetworkTopologyDef t3(3, "c", NetTopoType::ClientServer,  NetTopoProtocol::TCP);
    NetworkTopologyDef t4(4, "d", NetTopoType::Hybrid,        NetTopoProtocol::WebSocket); t4.setIsEnabled(false);
    ed.addTopology(t1); ed.addTopology(t2); ed.addTopology(t3); ed.addTopology(t4);
    REQUIRE(ed.countByType(NetTopoType::P2P)          == 2u);
    REQUIRE(ed.countByType(NetTopoType::ClientServer)  == 1u);
    REQUIRE(ed.countByType(NetTopoType::Mesh)          == 0u);
    REQUIRE(ed.countByProtocol(NetTopoProtocol::TCP)   == 2u);
    REQUIRE(ed.countByProtocol(NetTopoProtocol::UDP)   == 1u);
    REQUIRE(ed.countEnabled()                          == 3u);
    auto* found = ed.findTopology(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == NetTopoType::ClientServer);
    REQUIRE(ed.findTopology(99) == nullptr);
}

TEST_CASE("NetworkTopologyEditor settings mutation", "[Editor][S136]") {
    NetworkTopologyEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(false);
    ed.setDefaultMaxPeers(32u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.defaultMaxPeers() == 32u);
}

// ── BandwidthProfileEditor ────────────────────────────────────────────────────

TEST_CASE("BwProfileTier names", "[Editor][S136]") {
    REQUIRE(std::string(bwProfileTierName(BwProfileTier::Low))       == "Low");
    REQUIRE(std::string(bwProfileTierName(BwProfileTier::Medium))    == "Medium");
    REQUIRE(std::string(bwProfileTierName(BwProfileTier::High))      == "High");
    REQUIRE(std::string(bwProfileTierName(BwProfileTier::Ultra))     == "Ultra");
    REQUIRE(std::string(bwProfileTierName(BwProfileTier::Unlimited)) == "Unlimited");
}

TEST_CASE("BwProfileDirection names", "[Editor][S136]") {
    REQUIRE(std::string(bwProfileDirectionName(BwProfileDirection::Upload))        == "Upload");
    REQUIRE(std::string(bwProfileDirectionName(BwProfileDirection::Download))      == "Download");
    REQUIRE(std::string(bwProfileDirectionName(BwProfileDirection::Bidirectional)) == "Bidirectional");
}

TEST_CASE("BandwidthProfile defaults", "[Editor][S136]") {
    BandwidthProfile p(1, "home_high", BwProfileTier::High, BwProfileDirection::Bidirectional);
    REQUIRE(p.id()        == 1u);
    REQUIRE(p.name()      == "home_high");
    REQUIRE(p.tier()      == BwProfileTier::High);
    REQUIRE(p.direction() == BwProfileDirection::Bidirectional);
    REQUIRE(p.limitKbps() == 1024u);
    REQUIRE(p.burstKbps() == 2048u);
    REQUIRE(p.isEnabled());
}

TEST_CASE("BandwidthProfile mutation", "[Editor][S136]") {
    BandwidthProfile p(2, "mobile_low", BwProfileTier::Low, BwProfileDirection::Download);
    p.setLimitKbps(256u);
    p.setBurstKbps(512u);
    p.setIsEnabled(false);
    REQUIRE(p.limitKbps() == 256u);
    REQUIRE(p.burstKbps() == 512u);
    REQUIRE(!p.isEnabled());
}

TEST_CASE("BandwidthProfileEditor defaults", "[Editor][S136]") {
    BandwidthProfileEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByTier());
    REQUIRE(ed.defaultLimitKbps() == 512u);
    REQUIRE(ed.profileCount()     == 0u);
}

TEST_CASE("BandwidthProfileEditor add/remove", "[Editor][S136]") {
    BandwidthProfileEditor ed;
    REQUIRE(ed.addProfile(BandwidthProfile(1, "a", BwProfileTier::Low,  BwProfileDirection::Upload)));
    REQUIRE(ed.addProfile(BandwidthProfile(2, "b", BwProfileTier::High, BwProfileDirection::Download)));
    REQUIRE(ed.addProfile(BandwidthProfile(3, "c", BwProfileTier::Ultra, BwProfileDirection::Bidirectional)));
    REQUIRE(!ed.addProfile(BandwidthProfile(1, "a", BwProfileTier::Low, BwProfileDirection::Upload)));
    REQUIRE(ed.profileCount() == 3u);
    REQUIRE(ed.removeProfile(2));
    REQUIRE(ed.profileCount() == 2u);
    REQUIRE(!ed.removeProfile(99));
}

TEST_CASE("BandwidthProfileEditor counts and find", "[Editor][S136]") {
    BandwidthProfileEditor ed;
    BandwidthProfile p1(1, "a", BwProfileTier::Low,  BwProfileDirection::Upload);
    BandwidthProfile p2(2, "b", BwProfileTier::Low,  BwProfileDirection::Download);
    BandwidthProfile p3(3, "c", BwProfileTier::High, BwProfileDirection::Bidirectional);
    BandwidthProfile p4(4, "d", BwProfileTier::Ultra, BwProfileDirection::Upload); p4.setIsEnabled(false);
    ed.addProfile(p1); ed.addProfile(p2); ed.addProfile(p3); ed.addProfile(p4);
    REQUIRE(ed.countByTier(BwProfileTier::Low)                         == 2u);
    REQUIRE(ed.countByTier(BwProfileTier::High)                        == 1u);
    REQUIRE(ed.countByTier(BwProfileTier::Medium)                      == 0u);
    REQUIRE(ed.countByDirection(BwProfileDirection::Upload)            == 2u);
    REQUIRE(ed.countByDirection(BwProfileDirection::Download)          == 1u);
    REQUIRE(ed.countEnabled()                                          == 3u);
    auto* found = ed.findProfile(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->tier() == BwProfileTier::High);
    REQUIRE(ed.findProfile(99) == nullptr);
}

TEST_CASE("BandwidthProfileEditor settings mutation", "[Editor][S136]") {
    BandwidthProfileEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByTier(false);
    ed.setDefaultLimitKbps(2048u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByTier());
    REQUIRE(ed.defaultLimitKbps() == 2048u);
}

// ── LatencySimEditor ──────────────────────────────────────────────────────────

TEST_CASE("LatSimCondition names", "[Editor][S136]") {
    REQUIRE(std::string(latSimConditionName(LatSimCondition::Perfect))  == "Perfect");
    REQUIRE(std::string(latSimConditionName(LatSimCondition::Good))     == "Good");
    REQUIRE(std::string(latSimConditionName(LatSimCondition::Average))  == "Average");
    REQUIRE(std::string(latSimConditionName(LatSimCondition::Poor))     == "Poor");
    REQUIRE(std::string(latSimConditionName(LatSimCondition::Terrible)) == "Terrible");
}

TEST_CASE("LatSimJitter names", "[Editor][S136]") {
    REQUIRE(std::string(latSimJitterName(LatSimJitter::None))   == "None");
    REQUIRE(std::string(latSimJitterName(LatSimJitter::Low))    == "Low");
    REQUIRE(std::string(latSimJitterName(LatSimJitter::Medium)) == "Medium");
    REQUIRE(std::string(latSimJitterName(LatSimJitter::High))   == "High");
    REQUIRE(std::string(latSimJitterName(LatSimJitter::Spike))  == "Spike");
}

TEST_CASE("LatencySimPreset defaults", "[Editor][S136]") {
    LatencySimPreset p(1, "perfect_link", LatSimCondition::Perfect, LatSimJitter::None);
    REQUIRE(p.id()            == 1u);
    REQUIRE(p.name()          == "perfect_link");
    REQUIRE(p.condition()     == LatSimCondition::Perfect);
    REQUIRE(p.jitter()        == LatSimJitter::None);
    REQUIRE(p.latencyMs()     == 50u);
    REQUIRE(p.packetLossPct() == 0.0f);
    REQUIRE(p.isEnabled());
}

TEST_CASE("LatencySimPreset mutation", "[Editor][S136]") {
    LatencySimPreset p(2, "terrible", LatSimCondition::Terrible, LatSimJitter::Spike);
    p.setLatencyMs(400u);
    p.setPacketLossPct(15.0f);
    p.setIsEnabled(false);
    REQUIRE(p.latencyMs()     == 400u);
    REQUIRE(p.packetLossPct() == 15.0f);
    REQUIRE(!p.isEnabled());
}

TEST_CASE("LatencySimEditor defaults", "[Editor][S136]") {
    LatencySimEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByCondition());
    REQUIRE(ed.defaultLatencyMs() == 100u);
    REQUIRE(ed.presetCount()      == 0u);
}

TEST_CASE("LatencySimEditor add/remove", "[Editor][S136]") {
    LatencySimEditor ed;
    REQUIRE(ed.addPreset(LatencySimPreset(1, "a", LatSimCondition::Perfect,  LatSimJitter::None)));
    REQUIRE(ed.addPreset(LatencySimPreset(2, "b", LatSimCondition::Good,     LatSimJitter::Low)));
    REQUIRE(ed.addPreset(LatencySimPreset(3, "c", LatSimCondition::Terrible, LatSimJitter::Spike)));
    REQUIRE(!ed.addPreset(LatencySimPreset(1, "a", LatSimCondition::Perfect, LatSimJitter::None)));
    REQUIRE(ed.presetCount() == 3u);
    REQUIRE(ed.removePreset(2));
    REQUIRE(ed.presetCount() == 2u);
    REQUIRE(!ed.removePreset(99));
}

TEST_CASE("LatencySimEditor counts and find", "[Editor][S136]") {
    LatencySimEditor ed;
    LatencySimPreset p1(1, "a", LatSimCondition::Perfect,  LatSimJitter::None);
    LatencySimPreset p2(2, "b", LatSimCondition::Perfect,  LatSimJitter::Low);
    LatencySimPreset p3(3, "c", LatSimCondition::Poor,     LatSimJitter::High);
    LatencySimPreset p4(4, "d", LatSimCondition::Terrible, LatSimJitter::Spike); p4.setIsEnabled(false);
    ed.addPreset(p1); ed.addPreset(p2); ed.addPreset(p3); ed.addPreset(p4);
    REQUIRE(ed.countByCondition(LatSimCondition::Perfect)  == 2u);
    REQUIRE(ed.countByCondition(LatSimCondition::Poor)     == 1u);
    REQUIRE(ed.countByCondition(LatSimCondition::Good)     == 0u);
    REQUIRE(ed.countByJitter(LatSimJitter::None)           == 1u);
    REQUIRE(ed.countByJitter(LatSimJitter::Low)            == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findPreset(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->condition() == LatSimCondition::Poor);
    REQUIRE(ed.findPreset(99) == nullptr);
}

TEST_CASE("LatencySimEditor settings mutation", "[Editor][S136]") {
    LatencySimEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByCondition(true);
    ed.setDefaultLatencyMs(200u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByCondition());
    REQUIRE(ed.defaultLatencyMs() == 200u);
}
