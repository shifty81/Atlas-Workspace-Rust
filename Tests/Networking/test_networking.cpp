#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Networking/Networking.h"

using Catch::Matchers::WithinAbs;

// ── PacketType Coverage ──────────────────────────────────────────

TEST_CASE("PacketType enum values", "[Networking][Packet]") {
    REQUIRE(static_cast<uint8_t>(NF::PacketType::Connect) == 0);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::Disconnect) == 1);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::Heartbeat) == 2);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::EntitySpawn) == 3);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::EntityDestroy) == 4);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::EntityUpdate) == 5);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::WorldChunk) == 6);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::VoxelEdit) == 7);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::ChatMessage) == 8);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::RPC) == 9);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::Ping) == 10);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::Pong) == 11);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::AuthRequest) == 12);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::AuthResponse) == 13);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::PlayerInput) == 14);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::StateSnapshot) == 15);
    REQUIRE(static_cast<uint8_t>(NF::PacketType::AckReliable) == 16);
}

// ── Packet Creation and Serialization ────────────────────────────

TEST_CASE("Packet creation and defaults", "[Networking][Packet]") {
    NF::Packet p;
    REQUIRE(p.type == NF::PacketType::Connect);
    REQUIRE(p.senderId == 0);
    REQUIRE(p.sequenceNum == 0);
    REQUIRE(p.reliable == false);
    REQUIRE_THAT(p.timestamp, WithinAbs(0.f, 1e-6));
    REQUIRE(p.payload.isNull());
}

TEST_CASE("Packet serialization round-trip", "[Networking][Packet]") {
    NF::Packet original;
    original.type = NF::PacketType::EntityUpdate;
    original.senderId = 42;
    original.sequenceNum = 100;
    original.reliable = true;
    original.timestamp = 3.14f;
    original.payload = NF::JsonValue(std::string("test_payload"));

    auto json = NF::PacketSerializer::toJson(original);
    auto restored = NF::PacketSerializer::fromJson(json);

    REQUIRE(restored.type == original.type);
    REQUIRE(restored.senderId == original.senderId);
    REQUIRE(restored.sequenceNum == original.sequenceNum);
    REQUIRE(restored.reliable == original.reliable);
    REQUIRE_THAT(restored.timestamp, WithinAbs(3.14f, 1e-4));
    REQUIRE(restored.payload.asString() == "test_payload");
}

TEST_CASE("Packet serialization with complex payload", "[Networking][Packet]") {
    NF::Packet original;
    original.type = NF::PacketType::ChatMessage;
    original.senderId = 7;
    original.sequenceNum = 55;
    original.reliable = true;
    original.timestamp = 1.5f;

    auto payload = NF::JsonValue::object();
    payload.set("message", NF::JsonValue(std::string("hello")));
    payload.set("channel", NF::JsonValue(static_cast<int32_t>(1)));
    original.payload = payload;

    auto json = NF::PacketSerializer::toJson(original);
    auto restored = NF::PacketSerializer::fromJson(json);

    REQUIRE(restored.payload.isObject());
    REQUIRE(restored.payload["message"].asString() == "hello");
    REQUIRE(restored.payload["channel"].asInt() == 1);
}

TEST_CASE("Packet serialization without payload", "[Networking][Packet]") {
    NF::Packet p;
    p.type = NF::PacketType::Ping;

    auto json = NF::PacketSerializer::toJson(p);
    auto restored = NF::PacketSerializer::fromJson(json);

    REQUIRE(restored.type == NF::PacketType::Ping);
    REQUIRE(restored.payload.isNull());
}

// ── Connection State Management ──────────────────────────────────

TEST_CASE("Connection creation and defaults", "[Networking][Connection]") {
    NF::Connection conn(1, "127.0.0.1", 7777);
    REQUIRE(conn.id() == 1);
    REQUIRE(conn.address() == "127.0.0.1");
    REQUIRE(conn.port() == 7777);
    REQUIRE(conn.state() == NF::ConnectionState::Disconnected);
    REQUIRE_THAT(conn.latency(), WithinAbs(0.f, 1e-6));
    REQUIRE(conn.packetsSent() == 0);
    REQUIRE(conn.packetsReceived() == 0);
    REQUIRE(conn.sendQueue().empty());
    REQUIRE(conn.receiveQueue().empty());
    REQUIRE(conn.reliableBuffer().empty());
}

TEST_CASE("Connection state transitions", "[Networking][Connection]") {
    NF::Connection conn(1);
    REQUIRE(conn.state() == NF::ConnectionState::Disconnected);

    conn.setState(NF::ConnectionState::Connecting);
    REQUIRE(conn.state() == NF::ConnectionState::Connecting);

    conn.setState(NF::ConnectionState::Connected);
    REQUIRE(conn.state() == NF::ConnectionState::Connected);

    conn.setState(NF::ConnectionState::Authenticated);
    REQUIRE(conn.state() == NF::ConnectionState::Authenticated);

    conn.setState(NF::ConnectionState::Disconnecting);
    REQUIRE(conn.state() == NF::ConnectionState::Disconnecting);
}

TEST_CASE("Connection send/receive queues", "[Networking][Connection]") {
    NF::Connection conn(1);

    NF::Packet p1;
    p1.type = NF::PacketType::Heartbeat;
    NF::Packet p2;
    p2.type = NF::PacketType::ChatMessage;

    conn.queueSend(p1);
    conn.queueSend(p2);
    REQUIRE(conn.sendQueue().size() == 2);
    REQUIRE(conn.packetsSent() == 2);

    conn.queueReceive(p1);
    REQUIRE(conn.receiveQueue().size() == 1);
    REQUIRE(conn.packetsReceived() == 1);

    conn.clearSendQueue();
    REQUIRE(conn.sendQueue().empty());
    conn.clearReceiveQueue();
    REQUIRE(conn.receiveQueue().empty());
}

TEST_CASE("Connection reliable buffer and ack", "[Networking][Connection]") {
    NF::Connection conn(1);

    NF::Packet p1;
    p1.type = NF::PacketType::EntityUpdate;
    p1.reliable = true;
    p1.sequenceNum = 10;

    NF::Packet p2;
    p2.type = NF::PacketType::EntityUpdate;
    p2.reliable = true;
    p2.sequenceNum = 11;

    NF::Packet p3;
    p3.type = NF::PacketType::Heartbeat;
    p3.reliable = false;
    p3.sequenceNum = 12;

    conn.queueSend(p1);
    conn.queueSend(p2);
    conn.queueSend(p3);

    REQUIRE(conn.reliableBuffer().size() == 2);

    conn.ackReliable(10);
    REQUIRE(conn.reliableBuffer().size() == 1);
    REQUIRE(conn.reliableBuffer()[0].sequenceNum == 11);

    conn.ackReliable(11);
    REQUIRE(conn.reliableBuffer().empty());
}

// ── ConnectionManager ────────────────────────────────────────────

TEST_CASE("ConnectionManager add/remove/find", "[Networking][Connection]") {
    NF::ConnectionManager mgr;
    REQUIRE(mgr.connectionCount() == 0);

    mgr.addConnection(1, "10.0.0.1", 8000);
    mgr.addConnection(2, "10.0.0.2", 8001);
    REQUIRE(mgr.connectionCount() == 2);

    auto* c1 = mgr.findConnection(1);
    REQUIRE(c1 != nullptr);
    REQUIRE(c1->address() == "10.0.0.1");
    REQUIRE(c1->port() == 8000);
    REQUIRE(c1->state() == NF::ConnectionState::Connecting);

    auto* c3 = mgr.findConnection(99);
    REQUIRE(c3 == nullptr);

    mgr.removeConnection(1);
    REQUIRE(mgr.connectionCount() == 1);
    REQUIRE(mgr.findConnection(1) == nullptr);
}

TEST_CASE("ConnectionManager broadcast", "[Networking][Connection]") {
    NF::ConnectionManager mgr;
    mgr.addConnection(1);
    mgr.addConnection(2);
    mgr.addConnection(3);

    // Only Connected/Authenticated receive broadcasts
    mgr.findConnection(1)->setState(NF::ConnectionState::Connected);
    mgr.findConnection(2)->setState(NF::ConnectionState::Authenticated);
    // Connection 3 stays Connecting — should not receive

    NF::Packet p;
    p.type = NF::PacketType::ChatMessage;
    mgr.broadcast(p);

    REQUIRE(mgr.findConnection(1)->sendQueue().size() == 1);
    REQUIRE(mgr.findConnection(2)->sendQueue().size() == 1);
    REQUIRE(mgr.findConnection(3)->sendQueue().empty());
}

TEST_CASE("ConnectionManager timeout detection", "[Networking][Connection]") {
    NF::ConnectionManager mgr;
    mgr.addConnection(1);
    mgr.addConnection(2);

    mgr.findConnection(1)->setState(NF::ConnectionState::Connected);
    mgr.findConnection(1)->setLastHeartbeat(0.f);

    mgr.findConnection(2)->setState(NF::ConnectionState::Connected);
    mgr.findConnection(2)->setLastHeartbeat(9.0f);

    // At time=10, with threshold=5: conn 1 (last HB at 0) should time out
    mgr.checkTimeouts(10.f, 5.f);

    REQUIRE(mgr.findConnection(1)->state() ==
            NF::ConnectionState::Disconnected);
    REQUIRE(mgr.findConnection(2)->state() ==
            NF::ConnectionState::Connected);
}

// ── Replication System ───────────────────────────────────────────

TEST_CASE("ReplicatedProperty types", "[Networking][Replication]") {
    NF::ReplicatedProperty prop;
    prop.name = "health";
    prop.propertyType = NF::ReplicatedProperty::Type::Float;
    prop.value = NF::JsonValue(100.f);
    prop.dirty = true;
    prop.sequenceNum = 1;

    REQUIRE(prop.name == "health");
    REQUIRE(prop.propertyType == NF::ReplicatedProperty::Type::Float);
    REQUIRE(prop.dirty);
    REQUIRE(prop.sequenceNum == 1);
    REQUIRE_THAT(prop.value.asFloat(), WithinAbs(100.f, 1e-4));
}

TEST_CASE("ReplicationManager register/unregister", "[Networking][Replication]") {
    NF::ReplicationManager mgr;
    REQUIRE(mgr.entityCount() == 0);

    mgr.registerEntity(1, 10, NF::ReplicationRule::ServerAuthority);
    mgr.registerEntity(2, 20, NF::ReplicationRule::ClientAuthority);
    REQUIRE(mgr.entityCount() == 2);

    auto* e1 = mgr.findEntity(1);
    REQUIRE(e1 != nullptr);
    REQUIRE(e1->entityId == 1);
    REQUIRE(e1->ownerId == 10);
    REQUIRE(e1->rule == NF::ReplicationRule::ServerAuthority);

    auto* e2 = mgr.findEntity(2);
    REQUIRE(e2 != nullptr);
    REQUIRE(e2->rule == NF::ReplicationRule::ClientAuthority);

    mgr.unregisterEntity(1);
    REQUIRE(mgr.entityCount() == 1);
    REQUIRE(mgr.findEntity(1) == nullptr);
}

TEST_CASE("ReplicationManager mark dirty and collect snapshots",
          "[Networking][Replication]") {
    NF::ReplicationManager mgr;
    mgr.registerEntity(1, 10);

    auto* entity = mgr.findEntity(1);
    REQUIRE(entity != nullptr);

    NF::ReplicatedProperty hp;
    hp.name = "health";
    hp.propertyType = NF::ReplicatedProperty::Type::Float;
    hp.value = NF::JsonValue(100.f);
    hp.sequenceNum = 1;

    NF::ReplicatedProperty pos;
    pos.name = "position";
    pos.propertyType = NF::ReplicatedProperty::Type::Vec3;
    pos.value = NF::JsonValue(std::string("0,0,0"));
    pos.sequenceNum = 1;

    entity->properties.push_back(hp);
    entity->properties.push_back(pos);

    // Not dirty yet — should produce no snapshots
    auto packets = mgr.collectDirtySnapshots();
    REQUIRE(packets.empty());

    // Mark dirty
    mgr.markDirty(1);
    REQUIRE(entity->dirty);
    REQUIRE(entity->properties[0].dirty);
    REQUIRE(entity->properties[1].dirty);

    packets = mgr.collectDirtySnapshots();
    REQUIRE(packets.size() == 1);
    REQUIRE(packets[0].type == NF::PacketType::EntityUpdate);
    REQUIRE(packets[0].senderId == 10);
    REQUIRE(packets[0].reliable);

    // After collection, should be clean
    REQUIRE_FALSE(entity->dirty);
    REQUIRE_FALSE(entity->properties[0].dirty);
    REQUIRE_FALSE(entity->properties[1].dirty);

    // Second collect should be empty
    packets = mgr.collectDirtySnapshots();
    REQUIRE(packets.empty());
}

TEST_CASE("ReplicationManager apply snapshot", "[Networking][Replication]") {
    NF::ReplicationManager mgr;
    mgr.registerEntity(5, 1);

    auto* entity = mgr.findEntity(5);
    NF::ReplicatedProperty hp;
    hp.name = "health";
    hp.propertyType = NF::ReplicatedProperty::Type::Float;
    hp.value = NF::JsonValue(100.f);
    hp.sequenceNum = 0;
    entity->properties.push_back(hp);

    // Create an update packet
    NF::Packet p;
    p.type = NF::PacketType::EntityUpdate;
    auto payload = NF::JsonValue::object();
    payload.set("entityId", NF::JsonValue(static_cast<int32_t>(5)));
    auto props = NF::JsonValue::array();
    auto jp = NF::JsonValue::object();
    jp.set("name", NF::JsonValue(std::string("health")));
    jp.set("value", NF::JsonValue(75.f));
    jp.set("seq", NF::JsonValue(static_cast<int32_t>(2)));
    props.push(std::move(jp));
    payload.set("properties", std::move(props));
    p.payload = std::move(payload);

    mgr.applySnapshot(p);

    REQUIRE_THAT(entity->properties[0].value.asFloat(),
                 WithinAbs(75.f, 1e-4));
    REQUIRE(entity->properties[0].sequenceNum == 2);
}

// ── Session Management ───────────────────────────────────────────

TEST_CASE("Session add/remove players", "[Networking][Session]") {
    NF::Session session;
    session.setMaxPlayers(4);
    session.setSessionName("TestGame");
    session.setHostConnectionId(1);

    REQUIRE(session.sessionName() == "TestGame");
    REQUIRE(session.maxPlayers() == 4);
    REQUIRE(session.playerCount() == 0);

    NF::PlayerInfo p1{1, "Alice", 0, false, 20.f};
    NF::PlayerInfo p2{2, "Bob", 1, true, 30.f};

    REQUIRE(session.addPlayer(p1));
    REQUIRE(session.addPlayer(p2));
    REQUIRE(session.playerCount() == 2);

    auto* found = session.findPlayer(1);
    REQUIRE(found != nullptr);
    REQUIRE(found->displayName == "Alice");

    session.removePlayer(1);
    REQUIRE(session.playerCount() == 1);
    REQUIRE(session.findPlayer(1) == nullptr);
    REQUIRE(session.findPlayer(2) != nullptr);
}

TEST_CASE("Session state transitions", "[Networking][Session]") {
    NF::Session session;
    REQUIRE(session.state() == NF::SessionState::Lobby);

    session.setState(NF::SessionState::Loading);
    REQUIRE(session.state() == NF::SessionState::Loading);

    session.setState(NF::SessionState::InGame);
    REQUIRE(session.state() == NF::SessionState::InGame);

    session.setState(NF::SessionState::Paused);
    REQUIRE(session.state() == NF::SessionState::Paused);

    session.setState(NF::SessionState::Ending);
    REQUIRE(session.state() == NF::SessionState::Ending);
}

TEST_CASE("Session host check", "[Networking][Session]") {
    NF::Session session;
    session.setHostConnectionId(42);

    REQUIRE(session.isHost(42));
    REQUIRE_FALSE(session.isHost(1));
    REQUIRE_FALSE(session.isHost(0));
}

TEST_CASE("Session max players limit", "[Networking][Session]") {
    NF::Session session;
    session.setMaxPlayers(2);

    NF::PlayerInfo p1{1, "A", 0, false, 0.f};
    NF::PlayerInfo p2{2, "B", 0, false, 0.f};
    NF::PlayerInfo p3{3, "C", 0, false, 0.f};

    REQUIRE(session.addPlayer(p1));
    REQUIRE(session.addPlayer(p2));
    REQUIRE_FALSE(session.addPlayer(p3));
    REQUIRE(session.playerCount() == 2);
}

// ── Lockstep System ──────────────────────────────────────────────

TEST_CASE("LockstepManager store and retrieve inputs",
          "[Networking][Lockstep]") {
    NF::LockstepManager mgr;
    REQUIRE(mgr.currentFrame() == 0);
    REQUIRE(mgr.inputDelay() == 2);

    NF::InputFrame f1;
    f1.frameNumber = 0;
    f1.connectionId = 1;
    f1.inputs = {{"move_forward", 1.0f}, {"jump", 0.0f}};

    NF::InputFrame f2;
    f2.frameNumber = 0;
    f2.connectionId = 2;
    f2.inputs = {{"move_forward", 0.0f}, {"shoot", 1.0f}};

    mgr.storeInput(f1);
    mgr.storeInput(f2);

    auto inputs = mgr.getConfirmedInputs(0);
    REQUIRE(inputs.size() == 2);
}

TEST_CASE("LockstepManager advance frame", "[Networking][Lockstep]") {
    NF::LockstepManager mgr;
    REQUIRE(mgr.currentFrame() == 0);
    mgr.advanceFrame();
    REQUIRE(mgr.currentFrame() == 1);
    mgr.advanceFrame();
    mgr.advanceFrame();
    REQUIRE(mgr.currentFrame() == 3);
}

TEST_CASE("LockstepManager check all inputs for frame",
          "[Networking][Lockstep]") {
    NF::LockstepManager mgr;
    std::vector<uint32_t> connIds = {1, 2, 3};

    REQUIRE_FALSE(mgr.hasAllInputsForFrame(0, connIds));

    NF::InputFrame f1{0, 1, {{"move", 1.0f}}};
    NF::InputFrame f2{0, 2, {{"move", 0.5f}}};
    mgr.storeInput(f1);
    mgr.storeInput(f2);

    REQUIRE_FALSE(mgr.hasAllInputsForFrame(0, connIds));

    NF::InputFrame f3{0, 3, {{"move", 0.0f}}};
    mgr.storeInput(f3);

    REQUIRE(mgr.hasAllInputsForFrame(0, connIds));
    REQUIRE_FALSE(mgr.hasAllInputsForFrame(1, connIds));
}

TEST_CASE("LockstepManager input delay setting", "[Networking][Lockstep]") {
    NF::LockstepManager mgr;
    mgr.setInputDelay(5);
    REQUIRE(mgr.inputDelay() == 5);
}

// ── Rollback System ──────────────────────────────────────────────

TEST_CASE("RollbackManager save and load state", "[Networking][Rollback]") {
    NF::RollbackManager mgr;

    int gameState = 42;

    mgr.setSaveStateCallback([&]() {
        return NF::JsonValue(static_cast<int32_t>(gameState));
    });
    mgr.setLoadStateCallback([&](const NF::JsonValue& v) {
        gameState = v.asInt();
    });

    mgr.setCurrentFrame(5);
    mgr.saveState(5);

    gameState = 100;
    REQUIRE(mgr.loadState(5));
    REQUIRE(gameState == 42);

    REQUIRE_FALSE(mgr.loadState(99));
}

TEST_CASE("RollbackManager rollback depth", "[Networking][Rollback]") {
    NF::RollbackManager mgr;
    mgr.setMaxRollbackFrames(4);
    REQUIRE(mgr.maxRollbackFrames() == 4);

    int state = 0;
    mgr.setSaveStateCallback([&]() {
        return NF::JsonValue(static_cast<int32_t>(state));
    });
    mgr.setLoadStateCallback([&](const NF::JsonValue& v) {
        state = v.asInt();
    });

    // Save states for frames 0-5
    for (uint32_t i = 0; i <= 5; ++i) {
        state = static_cast<int>(i * 10);
        mgr.setCurrentFrame(i);
        mgr.saveState(i);
    }

    mgr.setCurrentFrame(5);

    // Rollback by 3 frames (within limit of 4)
    REQUIRE(mgr.rollbackToFrame(2));
    REQUIRE(mgr.currentRollbackDepth() == 3);
    REQUIRE(state == 20);

    // Rollback too far (more than maxRollbackFrames)
    mgr.setCurrentFrame(10);
    REQUIRE_FALSE(mgr.rollbackToFrame(1));
}

TEST_CASE("RollbackManager confirm frame", "[Networking][Rollback]") {
    NF::RollbackManager mgr;
    REQUIRE(mgr.confirmedFrame() == 0);
    REQUIRE(mgr.currentRollbackDepth() == 0);

    mgr.confirmFrame(5);
    REQUIRE(mgr.confirmedFrame() == 5);
    REQUIRE(mgr.currentRollbackDepth() == 0);
}

TEST_CASE("RollbackManager rollback to future frame fails",
          "[Networking][Rollback]") {
    NF::RollbackManager mgr;
    mgr.setCurrentFrame(5);
    REQUIRE_FALSE(mgr.rollbackToFrame(10));
}

// ── RPC System ───────────────────────────────────────────────────

TEST_CASE("RPCRegistry register and invoke", "[Networking][RPC]") {
    NF::RPCRegistry registry;

    int callCount = 0;
    std::string lastFunction;

    registry.registerRPC("spawn_item",
                         [&](const NF::RPCCall& call) {
                             callCount++;
                             lastFunction = call.functionName;
                         });

    REQUIRE(registry.rpcCount() == 1);
    REQUIRE(registry.hasRPC("spawn_item"));

    NF::RPCCall call;
    call.functionName = "spawn_item";
    call.target = NF::RPCTarget::Server;
    call.sourceConnectionId = 1;

    REQUIRE(registry.invoke(call));
    REQUIRE(callCount == 1);
    REQUIRE(lastFunction == "spawn_item");
}

TEST_CASE("RPCRegistry has/count", "[Networking][RPC]") {
    NF::RPCRegistry registry;
    REQUIRE(registry.rpcCount() == 0);
    REQUIRE_FALSE(registry.hasRPC("test"));

    registry.registerRPC("rpc_a", [](const NF::RPCCall&) {});
    registry.registerRPC("rpc_b", [](const NF::RPCCall&) {});
    REQUIRE(registry.rpcCount() == 2);
    REQUIRE(registry.hasRPC("rpc_a"));
    REQUIRE(registry.hasRPC("rpc_b"));
}

TEST_CASE("RPCRegistry invoke unknown RPC", "[Networking][RPC]") {
    NF::RPCRegistry registry;

    NF::RPCCall call;
    call.functionName = "nonexistent";
    REQUIRE_FALSE(registry.invoke(call));
}

TEST_CASE("RPCTarget enum values", "[Networking][RPC]") {
    REQUIRE(static_cast<uint8_t>(NF::RPCTarget::Server) == 0);
    REQUIRE(static_cast<uint8_t>(NF::RPCTarget::Client) == 1);
    REQUIRE(static_cast<uint8_t>(NF::RPCTarget::AllClients) == 2);
    REQUIRE(static_cast<uint8_t>(NF::RPCTarget::AllClientsExcept) == 3);
}

// ── NetworkManager ───────────────────────────────────────────────

TEST_CASE("NetworkManager initialization", "[Networking][Manager]") {
    NF::NetworkManager mgr;
    REQUIRE(mgr.role() == NF::NetRole::None);

    mgr.init(NF::NetRole::Server);
    REQUIRE(mgr.role() == NF::NetRole::Server);

    mgr.shutdown();
    REQUIRE(mgr.role() == NF::NetRole::Server);
}

TEST_CASE("NetworkManager packet routing — Connect", "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(5, "10.0.0.1", 9000);

    NF::Packet p;
    p.type = NF::PacketType::Connect;
    p.senderId = 5;
    mgr.processPacket(p);

    REQUIRE(mgr.connectionManager().findConnection(5)->state() ==
            NF::ConnectionState::Connected);
    REQUIRE(mgr.totalPacketsReceived() == 1);
}

TEST_CASE("NetworkManager packet routing — Disconnect",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(5);
    mgr.connectionManager().findConnection(5)->setState(
        NF::ConnectionState::Connected);

    NF::Packet p;
    p.type = NF::PacketType::Disconnect;
    p.senderId = 5;
    mgr.processPacket(p);

    REQUIRE(mgr.connectionManager().findConnection(5)->state() ==
            NF::ConnectionState::Disconnected);
}

TEST_CASE("NetworkManager packet routing — Heartbeat",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(5);

    NF::Packet p;
    p.type = NF::PacketType::Heartbeat;
    p.senderId = 5;
    p.timestamp = 42.5f;
    mgr.processPacket(p);

    REQUIRE_THAT(
        mgr.connectionManager().findConnection(5)->lastHeartbeat(),
        WithinAbs(42.5f, 1e-4));
}

TEST_CASE("NetworkManager packet routing — AuthRequest",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(5);
    mgr.connectionManager().findConnection(5)->setState(
        NF::ConnectionState::Connected);

    NF::Packet p;
    p.type = NF::PacketType::AuthRequest;
    p.senderId = 5;
    mgr.processPacket(p);

    REQUIRE(mgr.connectionManager().findConnection(5)->state() ==
            NF::ConnectionState::Authenticated);
}

TEST_CASE("NetworkManager packet routing — RPC", "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);

    bool called = false;
    mgr.registerRPC("test_rpc", [&](const NF::RPCCall& call) {
        called = true;
        REQUIRE(call.functionName == "test_rpc");
        REQUIRE(call.sourceConnectionId == 3);
    });

    NF::Packet p;
    p.type = NF::PacketType::RPC;
    p.senderId = 3;
    auto payload = NF::JsonValue::object();
    payload.set("function", NF::JsonValue(std::string("test_rpc")));
    payload.set("args", NF::JsonValue::array());
    payload.set("target",
                NF::JsonValue(static_cast<int32_t>(NF::RPCTarget::Server)));
    p.payload = std::move(payload);

    mgr.processPacket(p);
    REQUIRE(called);
}

TEST_CASE("NetworkManager packet routing — PlayerInput",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);

    NF::Packet p;
    p.type = NF::PacketType::PlayerInput;
    p.senderId = 2;
    auto payload = NF::JsonValue::object();
    payload.set("frame", NF::JsonValue(static_cast<int32_t>(7)));
    auto inputs = NF::JsonValue::array();
    auto inp = NF::JsonValue::object();
    inp.set("action", NF::JsonValue(std::string("jump")));
    inp.set("value", NF::JsonValue(1.0f));
    inputs.push(std::move(inp));
    payload.set("inputs", std::move(inputs));
    p.payload = std::move(payload);

    mgr.processPacket(p);

    auto frames = mgr.lockstep().getConfirmedInputs(7);
    REQUIRE(frames.size() == 1);
    REQUIRE(frames[0].connectionId == 2);
    REQUIRE(frames[0].inputs.size() == 1);
    REQUIRE(frames[0].inputs[0].first == "jump");
    REQUIRE_THAT(frames[0].inputs[0].second, WithinAbs(1.0f, 1e-4));
}

TEST_CASE("NetworkManager packet routing — Ping sends Pong",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(5);
    mgr.connectionManager().findConnection(5)->setState(
        NF::ConnectionState::Connected);

    NF::Packet ping;
    ping.type = NF::PacketType::Ping;
    ping.senderId = 5;
    ping.timestamp = 99.f;

    mgr.processPacket(ping);

    // A Pong should have been queued for connection 5
    auto* conn = mgr.connectionManager().findConnection(5);
    REQUIRE(conn->sendQueue().size() == 1);
    REQUIRE(conn->sendQueue()[0].type == NF::PacketType::Pong);
    REQUIRE_THAT(conn->sendQueue()[0].timestamp, WithinAbs(99.f, 1e-4));
}

TEST_CASE("NetworkManager packet routing — AckReliable",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(5);

    // Seed the reliable buffer with a packet
    auto* conn = mgr.connectionManager().findConnection(5);
    NF::Packet rel;
    rel.type = NF::PacketType::EntityUpdate;
    rel.reliable = true;
    rel.sequenceNum = 77;
    conn->queueSend(rel);
    REQUIRE(conn->reliableBuffer().size() == 1);

    // Send AckReliable
    NF::Packet ack;
    ack.type = NF::PacketType::AckReliable;
    ack.senderId = 5;
    auto ackPayload = NF::JsonValue::object();
    ackPayload.set("seq", NF::JsonValue(static_cast<int32_t>(77)));
    ack.payload = std::move(ackPayload);

    mgr.processPacket(ack);
    REQUIRE(conn->reliableBuffer().empty());
}

TEST_CASE("NetworkManager send and broadcast", "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(1);
    mgr.connectionManager().addConnection(2);
    mgr.connectionManager().findConnection(1)->setState(
        NF::ConnectionState::Connected);
    mgr.connectionManager().findConnection(2)->setState(
        NF::ConnectionState::Connected);

    NF::Packet p;
    p.type = NF::PacketType::ChatMessage;

    mgr.send(1, p);
    REQUIRE(mgr.connectionManager().findConnection(1)->sendQueue().size() == 1);
    REQUIRE(mgr.connectionManager().findConnection(2)->sendQueue().empty());
    REQUIRE(mgr.totalPacketsSent() == 1);

    mgr.broadcast(p);
    REQUIRE(mgr.connectionManager().findConnection(1)->sendQueue().size() == 2);
    REQUIRE(mgr.connectionManager().findConnection(2)->sendQueue().size() == 1);
    REQUIRE(mgr.totalPacketsSent() == 3);
}

TEST_CASE("NetworkManager tick processes receive queues",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);
    mgr.connectionManager().addConnection(1);

    auto* conn = mgr.connectionManager().findConnection(1);

    NF::Packet p;
    p.type = NF::PacketType::Connect;
    p.senderId = 1;
    conn->queueReceive(p);
    REQUIRE(conn->receiveQueue().size() == 1);

    mgr.tick(0.016f);

    REQUIRE(conn->receiveQueue().empty());
    REQUIRE(conn->state() == NF::ConnectionState::Connected);
    REQUIRE(mgr.totalPacketsReceived() == 1);
}

TEST_CASE("NetworkManager statistics", "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);

    REQUIRE(mgr.totalPacketsSent() == 0);
    REQUIRE(mgr.totalPacketsReceived() == 0);
    REQUIRE_THAT(mgr.averageLatency(), WithinAbs(0.f, 1e-6));

    mgr.connectionManager().addConnection(1);
    mgr.connectionManager().addConnection(2);
    mgr.connectionManager().findConnection(1)->setLatency(50.f);
    mgr.connectionManager().findConnection(2)->setLatency(100.f);

    REQUIRE_THAT(mgr.averageLatency(), WithinAbs(75.f, 1e-4));
}

TEST_CASE("NetworkManager RPC delegation", "[Networking][Manager]") {
    NF::NetworkManager mgr;
    mgr.init(NF::NetRole::Server);

    int invocations = 0;
    mgr.registerRPC("my_rpc", [&](const NF::RPCCall&) { invocations++; });

    REQUIRE(mgr.rpcRegistry().hasRPC("my_rpc"));

    NF::RPCCall call;
    call.functionName = "my_rpc";
    mgr.invokeRPC(call);
    REQUIRE(invocations == 1);
}

TEST_CASE("NetworkManager accessors return correct subsystems",
          "[Networking][Manager]") {
    NF::NetworkManager mgr;

    mgr.connectionManager().addConnection(1);
    REQUIRE(mgr.connectionManager().connectionCount() == 1);

    mgr.replicationManager().registerEntity(10, 1);
    REQUIRE(mgr.replicationManager().entityCount() == 1);

    mgr.session().setSessionName("MySession");
    REQUIRE(mgr.session().sessionName() == "MySession");

    mgr.lockstep().advanceFrame();
    REQUIRE(mgr.lockstep().currentFrame() == 1);

    mgr.rollback().setMaxRollbackFrames(16);
    REQUIRE(mgr.rollback().maxRollbackFrames() == 16);

    mgr.rpcRegistry().registerRPC("test", [](const NF::RPCCall&) {});
    REQUIRE(mgr.rpcRegistry().rpcCount() == 1);
}

// ── NetRole and ReplicationRule ──────────────────────────────────

TEST_CASE("NetRole enum values", "[Networking]") {
    REQUIRE(static_cast<uint8_t>(NF::NetRole::None) == 0);
    REQUIRE(static_cast<uint8_t>(NF::NetRole::Client) == 1);
    REQUIRE(static_cast<uint8_t>(NF::NetRole::Server) == 2);
    REQUIRE(static_cast<uint8_t>(NF::NetRole::ListenServer) == 3);
}

TEST_CASE("ReplicationRule enum values", "[Networking][Replication]") {
    REQUIRE(static_cast<uint8_t>(NF::ReplicationRule::ServerAuthority) == 0);
    REQUIRE(static_cast<uint8_t>(NF::ReplicationRule::ClientAuthority) == 1);
    REQUIRE(static_cast<uint8_t>(NF::ReplicationRule::PredictedOnClient) == 2);
}

TEST_CASE("ConnectionState enum values", "[Networking][Connection]") {
    REQUIRE(static_cast<uint8_t>(NF::ConnectionState::Disconnected) == 0);
    REQUIRE(static_cast<uint8_t>(NF::ConnectionState::Connecting) == 1);
    REQUIRE(static_cast<uint8_t>(NF::ConnectionState::Connected) == 2);
    REQUIRE(static_cast<uint8_t>(NF::ConnectionState::Authenticated) == 3);
    REQUIRE(static_cast<uint8_t>(NF::ConnectionState::Disconnecting) == 4);
}

TEST_CASE("SessionState enum values", "[Networking][Session]") {
    REQUIRE(static_cast<uint8_t>(NF::SessionState::Lobby) == 0);
    REQUIRE(static_cast<uint8_t>(NF::SessionState::Loading) == 1);
    REQUIRE(static_cast<uint8_t>(NF::SessionState::InGame) == 2);
    REQUIRE(static_cast<uint8_t>(NF::SessionState::Paused) == 3);
    REQUIRE(static_cast<uint8_t>(NF::SessionState::Ending) == 4);
}
