// S187 editor tests: ScriptGraphEditorV1, EventBusEditorV1, AssetBundleEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ScriptGraphEditorV1.h"
#include "NF/Editor/EventBusEditorV1.h"
#include "NF/Editor/AssetBundleEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ScriptGraphEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Sgev1Node validity", "[Editor][S187]") {
    Sgev1Node n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Branch01";
    REQUIRE(n.isValid());
}

TEST_CASE("ScriptGraphEditorV1 addNode and nodeCount", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    REQUIRE(sge.nodeCount() == 0);
    Sgev1Node n; n.id = 1; n.name = "Entry";
    REQUIRE(sge.addNode(n));
    REQUIRE(sge.nodeCount() == 1);
}

TEST_CASE("ScriptGraphEditorV1 addNode invalid fails", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    REQUIRE(!sge.addNode(Sgev1Node{}));
}

TEST_CASE("ScriptGraphEditorV1 addNode duplicate fails", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Node n; n.id = 1; n.name = "A";
    sge.addNode(n);
    REQUIRE(!sge.addNode(n));
}

TEST_CASE("ScriptGraphEditorV1 removeNode", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Node n; n.id = 2; n.name = "B";
    sge.addNode(n);
    REQUIRE(sge.removeNode(2));
    REQUIRE(sge.nodeCount() == 0);
    REQUIRE(!sge.removeNode(2));
}

TEST_CASE("ScriptGraphEditorV1 findNode", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Node n; n.id = 3; n.name = "C";
    sge.addNode(n);
    REQUIRE(sge.findNode(3) != nullptr);
    REQUIRE(sge.findNode(99) == nullptr);
}

TEST_CASE("ScriptGraphEditorV1 addConnection and connectionCount", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Connection c; c.id = 1; c.fromNode = 1; c.toNode = 2;
    REQUIRE(sge.addConnection(c));
    REQUIRE(sge.connectionCount() == 1);
}

TEST_CASE("ScriptGraphEditorV1 addConnection invalid fails", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    REQUIRE(!sge.addConnection(Sgev1Connection{}));
}

TEST_CASE("ScriptGraphEditorV1 addConnection duplicate fails", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Connection c; c.id = 1; c.fromNode = 1; c.toNode = 2;
    sge.addConnection(c);
    REQUIRE(!sge.addConnection(c));
}

TEST_CASE("ScriptGraphEditorV1 removeConnection", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Connection c; c.id = 1; c.fromNode = 1; c.toNode = 2;
    sge.addConnection(c);
    REQUIRE(sge.removeConnection(1));
    REQUIRE(sge.connectionCount() == 0);
}

TEST_CASE("ScriptGraphEditorV1 activeCount", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Node n1; n1.id = 1; n1.name = "A"; n1.state = Sgev1NodeState::Active;
    Sgev1Node n2; n2.id = 2; n2.name = "B";
    sge.addNode(n1); sge.addNode(n2);
    REQUIRE(sge.activeCount() == 1);
}

TEST_CASE("ScriptGraphEditorV1 errorCount", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Node n1; n1.id = 1; n1.name = "A"; n1.state = Sgev1NodeState::Error;
    Sgev1Node n2; n2.id = 2; n2.name = "B"; n2.state = Sgev1NodeState::Active;
    sge.addNode(n1); sge.addNode(n2);
    REQUIRE(sge.errorCount() == 1);
}

TEST_CASE("ScriptGraphEditorV1 countByType", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    Sgev1Node n1; n1.id = 1; n1.name = "A"; n1.nodeType = Sgev1NodeType::Branch;
    Sgev1Node n2; n2.id = 2; n2.name = "B"; n2.nodeType = Sgev1NodeType::Loop;
    Sgev1Node n3; n3.id = 3; n3.name = "C"; n3.nodeType = Sgev1NodeType::Branch;
    sge.addNode(n1); sge.addNode(n2); sge.addNode(n3);
    REQUIRE(sge.countByType(Sgev1NodeType::Branch) == 2);
    REQUIRE(sge.countByType(Sgev1NodeType::Loop) == 1);
}

TEST_CASE("ScriptGraphEditorV1 onChange callback", "[Editor][S187]") {
    ScriptGraphEditorV1 sge;
    uint64_t notified = 0;
    sge.setOnChange([&](uint64_t id) { notified = id; });
    Sgev1Node n; n.id = 7; n.name = "G";
    sge.addNode(n);
    REQUIRE(notified == 7);
}

TEST_CASE("Sgev1Node state helpers", "[Editor][S187]") {
    Sgev1Node n; n.id = 1; n.name = "X";
    n.state = Sgev1NodeState::Active;     REQUIRE(n.isActive());
    n.state = Sgev1NodeState::Error;      REQUIRE(n.isError());
    n.state = Sgev1NodeState::Breakpoint; REQUIRE(n.isBreakpoint());
}

TEST_CASE("sgev1NodeTypeName all values", "[Editor][S187]") {
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Entry))        == "Entry");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Exit))         == "Exit");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Branch))       == "Branch");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Loop))         == "Loop");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::FunctionCall)) == "FunctionCall");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Variable))     == "Variable");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Event))        == "Event");
    REQUIRE(std::string(sgev1NodeTypeName(Sgev1NodeType::Comment))      == "Comment");
}

// ── EventBusEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Ebev1Channel validity", "[Editor][S187]") {
    Ebev1Channel c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "OnPlayerDeath";
    REQUIRE(c.isValid());
}

TEST_CASE("EventBusEditorV1 addChannel and channelCount", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    REQUIRE(ebe.channelCount() == 0);
    Ebev1Channel c; c.id = 1; c.name = "C1";
    REQUIRE(ebe.addChannel(c));
    REQUIRE(ebe.channelCount() == 1);
}

TEST_CASE("EventBusEditorV1 addChannel invalid fails", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    REQUIRE(!ebe.addChannel(Ebev1Channel{}));
}

TEST_CASE("EventBusEditorV1 addChannel duplicate fails", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Channel c; c.id = 1; c.name = "A";
    ebe.addChannel(c);
    REQUIRE(!ebe.addChannel(c));
}

TEST_CASE("EventBusEditorV1 removeChannel", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Channel c; c.id = 2; c.name = "B";
    ebe.addChannel(c);
    REQUIRE(ebe.removeChannel(2));
    REQUIRE(ebe.channelCount() == 0);
    REQUIRE(!ebe.removeChannel(2));
}

TEST_CASE("EventBusEditorV1 findChannel", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Channel c; c.id = 3; c.name = "C";
    ebe.addChannel(c);
    REQUIRE(ebe.findChannel(3) != nullptr);
    REQUIRE(ebe.findChannel(99) == nullptr);
}

TEST_CASE("EventBusEditorV1 addSubscription and subscriptionCount", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Subscription s; s.id = 1; s.channelId = 10; s.listener = "PlayerSystem";
    REQUIRE(ebe.addSubscription(s));
    REQUIRE(ebe.subscriptionCount() == 1);
}

TEST_CASE("EventBusEditorV1 removeSubscription", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Subscription s; s.id = 1; s.channelId = 10; s.listener = "L";
    ebe.addSubscription(s);
    REQUIRE(ebe.removeSubscription(1));
    REQUIRE(ebe.subscriptionCount() == 0);
}

TEST_CASE("EventBusEditorV1 activeChannelCount", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Channel c1; c1.id = 1; c1.name = "A"; c1.state = Ebev1ChannelState::Active;
    Ebev1Channel c2; c2.id = 2; c2.name = "B";
    ebe.addChannel(c1); ebe.addChannel(c2);
    REQUIRE(ebe.activeChannelCount() == 1);
}

TEST_CASE("EventBusEditorV1 countByPriority", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Channel c1; c1.id = 1; c1.name = "A"; c1.priority = Ebev1EventPriority::High;
    Ebev1Channel c2; c2.id = 2; c2.name = "B"; c2.priority = Ebev1EventPriority::Low;
    Ebev1Channel c3; c3.id = 3; c3.name = "C"; c3.priority = Ebev1EventPriority::High;
    ebe.addChannel(c1); ebe.addChannel(c2); ebe.addChannel(c3);
    REQUIRE(ebe.countByPriority(Ebev1EventPriority::High) == 2);
    REQUIRE(ebe.countByPriority(Ebev1EventPriority::Low) == 1);
}

TEST_CASE("EventBusEditorV1 subscriptionsForChannel", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    Ebev1Subscription s1; s1.id = 1; s1.channelId = 10; s1.listener = "A";
    Ebev1Subscription s2; s2.id = 2; s2.channelId = 10; s2.listener = "B";
    Ebev1Subscription s3; s3.id = 3; s3.channelId = 20; s3.listener = "C";
    ebe.addSubscription(s1); ebe.addSubscription(s2); ebe.addSubscription(s3);
    REQUIRE(ebe.subscriptionsForChannel(10) == 2);
    REQUIRE(ebe.subscriptionsForChannel(20) == 1);
}

TEST_CASE("EventBusEditorV1 onChange callback on addChannel", "[Editor][S187]") {
    EventBusEditorV1 ebe;
    uint64_t notified = 0;
    ebe.setOnChange([&](uint64_t id) { notified = id; });
    Ebev1Channel c; c.id = 5; c.name = "E";
    ebe.addChannel(c);
    REQUIRE(notified == 5);
}

TEST_CASE("Ebev1Channel state helpers", "[Editor][S187]") {
    Ebev1Channel c; c.id = 1; c.name = "X";
    c.state = Ebev1ChannelState::Active;     REQUIRE(c.isActive());
    c.state = Ebev1ChannelState::Paused;     REQUIRE(c.isPaused());
    c.state = Ebev1ChannelState::Deprecated; REQUIRE(c.isDeprecated());
}

TEST_CASE("ebev1EventPriorityName all values", "[Editor][S187]") {
    REQUIRE(std::string(ebev1EventPriorityName(Ebev1EventPriority::Low))      == "Low");
    REQUIRE(std::string(ebev1EventPriorityName(Ebev1EventPriority::Normal))   == "Normal");
    REQUIRE(std::string(ebev1EventPriorityName(Ebev1EventPriority::High))     == "High");
    REQUIRE(std::string(ebev1EventPriorityName(Ebev1EventPriority::Critical)) == "Critical");
}

// ── AssetBundleEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Abev1Bundle validity", "[Editor][S187]") {
    Abev1Bundle b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "GameplayBundle";
    REQUIRE(b.isValid());
}

TEST_CASE("AssetBundleEditorV1 addBundle and bundleCount", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    REQUIRE(abe.bundleCount() == 0);
    Abev1Bundle b; b.id = 1; b.name = "B1";
    REQUIRE(abe.addBundle(b));
    REQUIRE(abe.bundleCount() == 1);
}

TEST_CASE("AssetBundleEditorV1 addBundle invalid fails", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    REQUIRE(!abe.addBundle(Abev1Bundle{}));
}

TEST_CASE("AssetBundleEditorV1 addBundle duplicate fails", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1Bundle b; b.id = 1; b.name = "A";
    abe.addBundle(b);
    REQUIRE(!abe.addBundle(b));
}

TEST_CASE("AssetBundleEditorV1 removeBundle", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1Bundle b; b.id = 2; b.name = "B";
    abe.addBundle(b);
    REQUIRE(abe.removeBundle(2));
    REQUIRE(abe.bundleCount() == 0);
    REQUIRE(!abe.removeBundle(2));
}

TEST_CASE("AssetBundleEditorV1 findBundle", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1Bundle b; b.id = 3; b.name = "C";
    abe.addBundle(b);
    REQUIRE(abe.findBundle(3) != nullptr);
    REQUIRE(abe.findBundle(99) == nullptr);
}

TEST_CASE("AssetBundleEditorV1 addEntry and entryCount", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1AssetEntry e; e.id = 1; e.bundleId = 10; e.assetPath = "textures/wood.png";
    REQUIRE(abe.addEntry(e));
    REQUIRE(abe.entryCount() == 1);
}

TEST_CASE("AssetBundleEditorV1 removeEntry", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1AssetEntry e; e.id = 1; e.bundleId = 10; e.assetPath = "mesh.fbx";
    abe.addEntry(e);
    REQUIRE(abe.removeEntry(1));
    REQUIRE(abe.entryCount() == 0);
}

TEST_CASE("AssetBundleEditorV1 readyCount", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1Bundle b1; b1.id = 1; b1.name = "A"; b1.state = Abev1BundleState::Ready;
    Abev1Bundle b2; b2.id = 2; b2.name = "B";
    abe.addBundle(b1); abe.addBundle(b2);
    REQUIRE(abe.readyCount() == 1);
}

TEST_CASE("AssetBundleEditorV1 countByMode", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1Bundle b1; b1.id = 1; b1.name = "A"; b1.mode = Abev1BundleMode::Streaming;
    Abev1Bundle b2; b2.id = 2; b2.name = "B"; b2.mode = Abev1BundleMode::Explicit;
    Abev1Bundle b3; b3.id = 3; b3.name = "C"; b3.mode = Abev1BundleMode::Streaming;
    abe.addBundle(b1); abe.addBundle(b2); abe.addBundle(b3);
    REQUIRE(abe.countByMode(Abev1BundleMode::Streaming) == 2);
    REQUIRE(abe.countByMode(Abev1BundleMode::Explicit) == 1);
}

TEST_CASE("AssetBundleEditorV1 entriesForBundle", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1AssetEntry e1; e1.id = 1; e1.bundleId = 10; e1.assetPath = "a.png";
    Abev1AssetEntry e2; e2.id = 2; e2.bundleId = 10; e2.assetPath = "b.png";
    Abev1AssetEntry e3; e3.id = 3; e3.bundleId = 20; e3.assetPath = "c.png";
    abe.addEntry(e1); abe.addEntry(e2); abe.addEntry(e3);
    REQUIRE(abe.entriesForBundle(10) == 2);
    REQUIRE(abe.entriesForBundle(20) == 1);
}

TEST_CASE("AssetBundleEditorV1 totalSizeBytes", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    Abev1Bundle b1; b1.id = 1; b1.name = "A"; b1.sizeBytes = 1024;
    Abev1Bundle b2; b2.id = 2; b2.name = "B"; b2.sizeBytes = 2048;
    abe.addBundle(b1); abe.addBundle(b2);
    REQUIRE(abe.totalSizeBytes() == 3072);
}

TEST_CASE("AssetBundleEditorV1 onChange callback", "[Editor][S187]") {
    AssetBundleEditorV1 abe;
    uint64_t notified = 0;
    abe.setOnChange([&](uint64_t id) { notified = id; });
    Abev1Bundle b; b.id = 8; b.name = "H";
    abe.addBundle(b);
    REQUIRE(notified == 8);
}

TEST_CASE("Abev1Bundle state helpers", "[Editor][S187]") {
    Abev1Bundle b; b.id = 1; b.name = "X";
    b.state = Abev1BundleState::Ready;     REQUIRE(b.isReady());
    b.state = Abev1BundleState::Published; REQUIRE(b.isPublished());
    b.state = Abev1BundleState::Stale;     REQUIRE(b.isStale());
}

TEST_CASE("abev1BundleModeName all values", "[Editor][S187]") {
    REQUIRE(std::string(abev1BundleModeName(Abev1BundleMode::Explicit))    == "Explicit");
    REQUIRE(std::string(abev1BundleModeName(Abev1BundleMode::Automatic))   == "Automatic");
    REQUIRE(std::string(abev1BundleModeName(Abev1BundleMode::Addressable)) == "Addressable");
    REQUIRE(std::string(abev1BundleModeName(Abev1BundleMode::Streaming))   == "Streaming");
}
