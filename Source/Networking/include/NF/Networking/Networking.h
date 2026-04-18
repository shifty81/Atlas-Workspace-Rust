#pragma once
// NF::Networking — Sockets, replication, sessions, lockstep/rollback
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace NF {

// ── Net Role ─────────────────────────────────────────────────────

enum class NetRole : uint8_t {
    None,
    Client,
    Server,
    ListenServer
};

// ── Packet System ────────────────────────────────────────────────

enum class PacketType : uint8_t {
    Connect,
    Disconnect,
    Heartbeat,
    EntitySpawn,
    EntityDestroy,
    EntityUpdate,
    WorldChunk,
    VoxelEdit,
    ChatMessage,
    RPC,
    Ping,
    Pong,
    AuthRequest,
    AuthResponse,
    PlayerInput,
    StateSnapshot,
    AckReliable
};

struct Packet {
    PacketType type = PacketType::Connect;
    uint32_t senderId = 0;
    uint32_t sequenceNum = 0;
    bool reliable = false;
    float timestamp = 0.f;
    JsonValue payload;
};

class PacketSerializer {
public:
    static JsonValue toJson(const Packet& p) {
        auto j = JsonValue::object();
        j.set("type", JsonValue(static_cast<int32_t>(p.type)));
        j.set("senderId", JsonValue(static_cast<int32_t>(p.senderId)));
        j.set("sequenceNum", JsonValue(static_cast<int32_t>(p.sequenceNum)));
        j.set("reliable", JsonValue(p.reliable));
        j.set("timestamp", JsonValue(p.timestamp));
        j.set("payload", p.payload);
        return j;
    }

    static Packet fromJson(const JsonValue& j) {
        Packet p;
        p.type = static_cast<PacketType>(j["type"].asInt());
        p.senderId = static_cast<uint32_t>(j["senderId"].asInt());
        p.sequenceNum = static_cast<uint32_t>(j["sequenceNum"].asInt());
        p.reliable = j["reliable"].asBool();
        p.timestamp = j["timestamp"].asFloat();
        if (j.hasKey("payload")) {
            p.payload = j["payload"];
        }
        return p;
    }
};

// ── Connection Management ────────────────────────────────────────

enum class ConnectionState : uint8_t {
    Disconnected,
    Connecting,
    Connected,
    Authenticated,
    Disconnecting
};

class Connection {
public:
    Connection() = default;
    explicit Connection(uint32_t id, const std::string& address = "",
                        uint16_t port = 0)
        : m_id(id), m_address(address), m_port(port) {}

    [[nodiscard]] uint32_t id() const { return m_id; }
    [[nodiscard]] ConnectionState state() const { return m_state; }
    [[nodiscard]] const std::string& address() const { return m_address; }
    [[nodiscard]] uint16_t port() const { return m_port; }
    [[nodiscard]] float latency() const { return m_latency; }
    [[nodiscard]] float lastHeartbeat() const { return m_lastHeartbeat; }
    [[nodiscard]] uint32_t packetsSent() const { return m_packetsSent; }
    [[nodiscard]] uint32_t packetsReceived() const { return m_packetsReceived; }
    [[nodiscard]] const std::vector<Packet>& sendQueue() const {
        return m_sendQueue;
    }
    [[nodiscard]] const std::vector<Packet>& receiveQueue() const {
        return m_receiveQueue;
    }
    [[nodiscard]] const std::vector<Packet>& reliableBuffer() const {
        return m_reliableBuffer;
    }

    void setState(ConnectionState s) { m_state = s; }
    void setLatency(float lat) { m_latency = lat; }
    void setLastHeartbeat(float t) { m_lastHeartbeat = t; }

    void queueSend(Packet p) {
        m_packetsSent++;
        if (p.reliable) {
            m_reliableBuffer.push_back(p);
        }
        m_sendQueue.push_back(std::move(p));
    }

    void queueReceive(Packet p) {
        m_packetsReceived++;
        m_receiveQueue.push_back(std::move(p));
    }

    void clearSendQueue() { m_sendQueue.clear(); }
    void clearReceiveQueue() { m_receiveQueue.clear(); }

    void ackReliable(uint32_t seqNum) {
        m_reliableBuffer.erase(
            std::remove_if(m_reliableBuffer.begin(), m_reliableBuffer.end(),
                           [seqNum](const Packet& p) {
                               return p.sequenceNum == seqNum;
                           }),
            m_reliableBuffer.end());
    }

private:
    uint32_t m_id = 0;
    ConnectionState m_state = ConnectionState::Disconnected;
    std::string m_address;
    uint16_t m_port = 0;
    float m_latency = 0.f;
    float m_lastHeartbeat = 0.f;
    uint32_t m_packetsSent = 0;
    uint32_t m_packetsReceived = 0;
    std::vector<Packet> m_sendQueue;
    std::vector<Packet> m_receiveQueue;
    std::vector<Packet> m_reliableBuffer;
};

class ConnectionManager {
public:
    void addConnection(uint32_t id, const std::string& address = "",
                       uint16_t port = 0) {
        m_connections.emplace(id, Connection(id, address, port));
        m_connections.at(id).setState(ConnectionState::Connecting);
        NF_LOG_INFO("Networking",
                    "Connection added: " + std::to_string(id));
    }

    void removeConnection(uint32_t id) {
        m_connections.erase(id);
        NF_LOG_INFO("Networking",
                    "Connection removed: " + std::to_string(id));
    }

    Connection* findConnection(uint32_t id) {
        auto it = m_connections.find(id);
        return it != m_connections.end() ? &it->second : nullptr;
    }

    [[nodiscard]] const Connection* findConnection(uint32_t id) const {
        auto it = m_connections.find(id);
        return it != m_connections.end() ? &it->second : nullptr;
    }

    void broadcast(const Packet& p) {
        for (auto& [id, conn] : m_connections) {
            if (conn.state() == ConnectionState::Connected ||
                conn.state() == ConnectionState::Authenticated) {
                conn.queueSend(p);
            }
        }
    }

    void checkTimeouts(float currentTime, float timeoutThreshold) {
        std::vector<uint32_t> timedOut;
        for (auto& [id, conn] : m_connections) {
            if (conn.state() != ConnectionState::Disconnected &&
                (currentTime - conn.lastHeartbeat()) > timeoutThreshold) {
                timedOut.push_back(id);
            }
        }
        for (auto id : timedOut) {
            NF_LOG_WARN("Networking",
                        "Connection timed out: " + std::to_string(id));
            m_connections.at(id).setState(ConnectionState::Disconnected);
        }
    }

    [[nodiscard]] size_t connectionCount() const {
        return m_connections.size();
    }

    [[nodiscard]] const std::unordered_map<uint32_t, Connection>&
    connections() const {
        return m_connections;
    }

    std::unordered_map<uint32_t, Connection>& connections() {
        return m_connections;
    }

private:
    std::unordered_map<uint32_t, Connection> m_connections;
};

// ── Replication System ───────────────────────────────────────────

struct ReplicatedProperty {
    enum class Type : uint8_t { Int, Float, Bool, String, Vec3 };

    std::string name;
    Type propertyType = Type::Float;
    bool dirty = false;
    uint32_t sequenceNum = 0;
    JsonValue value;
};

enum class ReplicationRule : uint8_t {
    ServerAuthority,
    ClientAuthority,
    PredictedOnClient
};

struct ReplicatedEntity {
    EntityID entityId = INVALID_ENTITY;
    uint32_t ownerId = 0;
    ReplicationRule rule = ReplicationRule::ServerAuthority;
    std::vector<ReplicatedProperty> properties;
    bool dirty = false;
};

class ReplicationManager {
public:
    void registerEntity(EntityID entityId, uint32_t ownerId,
                        ReplicationRule rule =
                            ReplicationRule::ServerAuthority) {
        ReplicatedEntity re;
        re.entityId = entityId;
        re.ownerId = ownerId;
        re.rule = rule;
        m_entities[entityId] = std::move(re);
    }

    void unregisterEntity(EntityID entityId) { m_entities.erase(entityId); }

    ReplicatedEntity* findEntity(EntityID entityId) {
        auto it = m_entities.find(entityId);
        return it != m_entities.end() ? &it->second : nullptr;
    }

    [[nodiscard]] const ReplicatedEntity*
    findEntity(EntityID entityId) const {
        auto it = m_entities.find(entityId);
        return it != m_entities.end() ? &it->second : nullptr;
    }

    void markDirty(EntityID entityId) {
        auto it = m_entities.find(entityId);
        if (it != m_entities.end()) {
            it->second.dirty = true;
            for (auto& prop : it->second.properties) {
                prop.dirty = true;
            }
        }
    }

    std::vector<Packet> collectDirtySnapshots() {
        std::vector<Packet> packets;
        for (auto& [id, entity] : m_entities) {
            if (!entity.dirty) continue;

            Packet p;
            p.type = PacketType::EntityUpdate;
            p.senderId = entity.ownerId;
            p.reliable = true;

            auto payload = JsonValue::object();
            payload.set("entityId",
                        JsonValue(static_cast<int32_t>(entity.entityId)));
            payload.set("ownerId",
                        JsonValue(static_cast<int32_t>(entity.ownerId)));
            payload.set("rule",
                        JsonValue(static_cast<int32_t>(entity.rule)));

            auto props = JsonValue::array();
            for (auto& prop : entity.properties) {
                if (!prop.dirty) continue;
                auto jp = JsonValue::object();
                jp.set("name", JsonValue(prop.name));
                jp.set("type",
                       JsonValue(static_cast<int32_t>(prop.propertyType)));
                jp.set("value", prop.value);
                jp.set("seq",
                       JsonValue(static_cast<int32_t>(prop.sequenceNum)));
                props.push(std::move(jp));
                prop.dirty = false;
            }
            payload.set("properties", std::move(props));
            p.payload = std::move(payload);

            entity.dirty = false;
            packets.push_back(std::move(p));
        }
        return packets;
    }

    void applySnapshot(const Packet& p) {
        if (p.type != PacketType::EntityUpdate &&
            p.type != PacketType::StateSnapshot)
            return;

        auto entityId =
            static_cast<EntityID>(p.payload["entityId"].asInt());
        auto it = m_entities.find(entityId);
        if (it == m_entities.end()) return;

        auto& entity = it->second;
        const auto& propsJson = p.payload["properties"];
        for (size_t i = 0; i < propsJson.size(); ++i) {
            const auto& jp = propsJson[i];
            auto name = jp["name"].asString();
            for (auto& prop : entity.properties) {
                if (prop.name == name) {
                    prop.value = jp["value"];
                    prop.sequenceNum =
                        static_cast<uint32_t>(jp["seq"].asInt());
                    prop.dirty = false;
                    break;
                }
            }
        }
    }

    [[nodiscard]] size_t entityCount() const { return m_entities.size(); }

    [[nodiscard]] const std::unordered_map<EntityID, ReplicatedEntity>&
    entities() const {
        return m_entities;
    }

private:
    std::unordered_map<EntityID, ReplicatedEntity> m_entities;
};

// ── Session Management ───────────────────────────────────────────

enum class SessionState : uint8_t {
    Lobby,
    Loading,
    InGame,
    Paused,
    Ending
};

struct PlayerInfo {
    uint32_t connectionId = 0;
    std::string displayName;
    uint8_t team = 0;
    bool ready = false;
    float ping = 0.f;
};

class Session {
public:
    Session() = default;

    void setState(SessionState s) { m_state = s; }
    [[nodiscard]] SessionState state() const { return m_state; }

    void setSessionName(const std::string& name) { m_sessionName = name; }
    [[nodiscard]] const std::string& sessionName() const {
        return m_sessionName;
    }

    void setMaxPlayers(uint32_t max) { m_maxPlayers = max; }
    [[nodiscard]] uint32_t maxPlayers() const { return m_maxPlayers; }

    void setHostConnectionId(uint32_t id) { m_hostConnectionId = id; }
    [[nodiscard]] uint32_t hostConnectionId() const {
        return m_hostConnectionId;
    }

    bool addPlayer(const PlayerInfo& player) {
        if (m_players.size() >= m_maxPlayers) return false;
        m_players.push_back(player);
        return true;
    }

    void removePlayer(uint32_t connectionId) {
        m_players.erase(
            std::remove_if(
                m_players.begin(), m_players.end(),
                [connectionId](const PlayerInfo& p) {
                    return p.connectionId == connectionId;
                }),
            m_players.end());
    }

    PlayerInfo* findPlayer(uint32_t connectionId) {
        for (auto& p : m_players) {
            if (p.connectionId == connectionId) return &p;
        }
        return nullptr;
    }

    [[nodiscard]] const PlayerInfo*
    findPlayer(uint32_t connectionId) const {
        for (const auto& p : m_players) {
            if (p.connectionId == connectionId) return &p;
        }
        return nullptr;
    }

    [[nodiscard]] bool isHost(uint32_t connectionId) const {
        return connectionId == m_hostConnectionId;
    }

    [[nodiscard]] size_t playerCount() const { return m_players.size(); }

    [[nodiscard]] const std::vector<PlayerInfo>& players() const {
        return m_players;
    }

private:
    SessionState m_state = SessionState::Lobby;
    std::vector<PlayerInfo> m_players;
    uint32_t m_maxPlayers = 16;
    std::string m_sessionName;
    uint32_t m_hostConnectionId = 0;
};

// ── Lockstep / Input ─────────────────────────────────────────────

struct InputFrame {
    uint32_t frameNumber = 0;
    uint32_t connectionId = 0;
    std::vector<std::pair<std::string, float>> inputs;
};

class LockstepManager {
public:
    [[nodiscard]] uint32_t currentFrame() const { return m_currentFrame; }

    void setInputDelay(uint32_t delay) { m_inputDelay = delay; }
    [[nodiscard]] uint32_t inputDelay() const { return m_inputDelay; }

    void storeInput(const InputFrame& frame) {
        m_confirmedInputs[frame.frameNumber][frame.connectionId] = frame;
    }

    void advanceFrame() { m_currentFrame++; }

    [[nodiscard]] bool hasAllInputsForFrame(
        uint32_t frame,
        const std::vector<uint32_t>& connectionIds) const {
        auto it = m_confirmedInputs.find(frame);
        if (it == m_confirmedInputs.end()) return false;
        for (auto id : connectionIds) {
            if (it->second.find(id) == it->second.end()) return false;
        }
        return true;
    }

    [[nodiscard]] std::vector<InputFrame>
    getConfirmedInputs(uint32_t frame) const {
        std::vector<InputFrame> result;
        auto it = m_confirmedInputs.find(frame);
        if (it != m_confirmedInputs.end()) {
            for (const auto& [connId, inputFrame] : it->second) {
                result.push_back(inputFrame);
            }
        }
        return result;
    }

private:
    uint32_t m_currentFrame = 0;
    uint32_t m_inputDelay = 2;
    std::unordered_map<uint32_t,
                       std::unordered_map<uint32_t, InputFrame>>
        m_confirmedInputs;
};

// ── Rollback ─────────────────────────────────────────────────────

class RollbackManager {
public:
    using SaveStateFunc = std::function<JsonValue()>;
    using LoadStateFunc = std::function<void(const JsonValue&)>;

    void setSaveStateCallback(SaveStateFunc cb) {
        m_saveState = std::move(cb);
    }
    void setLoadStateCallback(LoadStateFunc cb) {
        m_loadState = std::move(cb);
    }

    void setMaxRollbackFrames(uint32_t max) { m_maxRollbackFrames = max; }
    [[nodiscard]] uint32_t maxRollbackFrames() const {
        return m_maxRollbackFrames;
    }

    void saveState(uint32_t frame) {
        if (m_saveState) {
            m_savedStates[frame] = m_saveState();
            // Prune old states beyond the rollback window
            if (m_confirmedFrame > m_maxRollbackFrames) {
                uint32_t pruneBelow =
                    m_confirmedFrame - m_maxRollbackFrames;
                for (auto it = m_savedStates.begin();
                     it != m_savedStates.end();) {
                    if (it->first < pruneBelow)
                        it = m_savedStates.erase(it);
                    else
                        ++it;
                }
            }
        }
    }

    bool loadState(uint32_t frame) {
        auto it = m_savedStates.find(frame);
        if (it != m_savedStates.end() && m_loadState) {
            m_loadState(it->second);
            return true;
        }
        return false;
    }

    bool rollbackToFrame(uint32_t frame) {
        if (frame > m_currentFrame) return false;
        m_currentRollbackDepth = m_currentFrame - frame;
        if (m_currentRollbackDepth > m_maxRollbackFrames) return false;
        return loadState(frame);
    }

    void confirmFrame(uint32_t frame) {
        m_confirmedFrame = frame;
        m_currentRollbackDepth = 0;
    }

    void setCurrentFrame(uint32_t frame) { m_currentFrame = frame; }
    [[nodiscard]] uint32_t currentFrame() const { return m_currentFrame; }
    [[nodiscard]] uint32_t confirmedFrame() const {
        return m_confirmedFrame;
    }
    [[nodiscard]] uint32_t currentRollbackDepth() const {
        return m_currentRollbackDepth;
    }

private:
    uint32_t m_maxRollbackFrames = 8;
    SaveStateFunc m_saveState;
    LoadStateFunc m_loadState;
    std::unordered_map<uint32_t, JsonValue> m_savedStates;
    uint32_t m_currentRollbackDepth = 0;
    uint32_t m_confirmedFrame = 0;
    uint32_t m_currentFrame = 0;
};

// ── RPC System ───────────────────────────────────────────────────

enum class RPCTarget : uint8_t {
    Server,
    Client,
    AllClients,
    AllClientsExcept
};

struct RPCCall {
    std::string functionName;
    JsonValue args;
    RPCTarget target = RPCTarget::Server;
    uint32_t sourceConnectionId = 0;
};

class RPCRegistry {
public:
    using RPCHandler = std::function<void(const RPCCall&)>;

    void registerRPC(const std::string& name, RPCHandler handler) {
        m_handlers[name] = std::move(handler);
    }

    bool invoke(const RPCCall& call) {
        auto it = m_handlers.find(call.functionName);
        if (it != m_handlers.end()) {
            it->second(call);
            return true;
        }
        NF_LOG_WARN("Networking",
                    "RPC not found: " + call.functionName);
        return false;
    }

    [[nodiscard]] bool hasRPC(const std::string& name) const {
        return m_handlers.count(name) > 0;
    }

    [[nodiscard]] size_t rpcCount() const { return m_handlers.size(); }

private:
    std::unordered_map<std::string, RPCHandler> m_handlers;
};

// ── Network Manager ──────────────────────────────────────────────

class NetworkManager {
public:
    void init(NetRole role) {
        m_role = role;
        NF_LOG_INFO("Networking", "Network manager initialized");
    }

    void shutdown() {
        NF_LOG_INFO("Networking", "Network manager shutdown");
    }

    void update(float dt) { tick(dt); }

    [[nodiscard]] NetRole role() const { return m_role; }

    // ── Packet processing ────────────────────────────────────────

    void processPacket(const Packet& p) {
        m_totalPacketsReceived++;
        switch (p.type) {
        case PacketType::Connect: {
            auto* conn = m_connectionMgr.findConnection(p.senderId);
            if (conn) conn->setState(ConnectionState::Connected);
            break;
        }
        case PacketType::Disconnect: {
            auto* conn = m_connectionMgr.findConnection(p.senderId);
            if (conn) conn->setState(ConnectionState::Disconnected);
            break;
        }
        case PacketType::Heartbeat: {
            auto* conn = m_connectionMgr.findConnection(p.senderId);
            if (conn) conn->setLastHeartbeat(p.timestamp);
            break;
        }
        case PacketType::EntitySpawn:
        case PacketType::EntityDestroy:
        case PacketType::EntityUpdate:
        case PacketType::StateSnapshot:
            m_replicationMgr.applySnapshot(p);
            break;
        case PacketType::RPC: {
            RPCCall call;
            call.functionName =
                p.payload["function"].asString();
            call.args = p.payload["args"];
            call.sourceConnectionId = p.senderId;
            call.target = static_cast<RPCTarget>(
                p.payload["target"].asInt());
            m_rpcRegistry.invoke(call);
            break;
        }
        case PacketType::PlayerInput: {
            InputFrame frame;
            frame.frameNumber =
                static_cast<uint32_t>(p.payload["frame"].asInt());
            frame.connectionId = p.senderId;
            const auto& inputs = p.payload["inputs"];
            for (size_t i = 0; i < inputs.size(); ++i) {
                const auto& inp = inputs[i];
                frame.inputs.emplace_back(
                    inp["action"].asString(),
                    inp["value"].asFloat());
            }
            m_lockstepMgr.storeInput(frame);
            break;
        }
        case PacketType::Ping: {
            Packet pong;
            pong.type = PacketType::Pong;
            pong.timestamp = p.timestamp;
            send(p.senderId, pong);
            break;
        }
        case PacketType::Pong: {
            auto* conn = m_connectionMgr.findConnection(p.senderId);
            if (conn) conn->setLatency(p.timestamp);
            break;
        }
        case PacketType::AuthRequest: {
            auto* conn = m_connectionMgr.findConnection(p.senderId);
            if (conn) conn->setState(ConnectionState::Authenticated);
            break;
        }
        case PacketType::AuthResponse:
        case PacketType::ChatMessage:
        case PacketType::WorldChunk:
        case PacketType::VoxelEdit:
            break;
        case PacketType::AckReliable: {
            auto* conn = m_connectionMgr.findConnection(p.senderId);
            if (conn) {
                uint32_t ackedSeq =
                    static_cast<uint32_t>(p.payload["seq"].asInt());
                conn->ackReliable(ackedSeq);
            }
            break;
        }
        }
    }

    void tick(float dt) {
        (void)dt;
        for (auto& [id, conn] : m_connectionMgr.connections()) {
            for (const auto& pkt : conn.receiveQueue()) {
                processPacket(pkt);
            }
            conn.clearReceiveQueue();
        }
    }

    void send(uint32_t connectionId, Packet p) {
        auto* conn = m_connectionMgr.findConnection(connectionId);
        if (conn) {
            conn->queueSend(std::move(p));
            m_totalPacketsSent++;
        }
    }

    void broadcast(Packet p) {
        for (auto& [id, conn] : m_connectionMgr.connections()) {
            if (conn.state() == ConnectionState::Connected ||
                conn.state() == ConnectionState::Authenticated) {
                conn.queueSend(p);
                m_totalPacketsSent++;
            }
        }
    }

    void registerRPC(const std::string& name,
                     RPCRegistry::RPCHandler handler) {
        m_rpcRegistry.registerRPC(name, std::move(handler));
    }

    void invokeRPC(const RPCCall& call) { m_rpcRegistry.invoke(call); }

    // ── Accessors ────────────────────────────────────────────────

    ConnectionManager& connectionManager() { return m_connectionMgr; }
    [[nodiscard]] const ConnectionManager& connectionManager() const {
        return m_connectionMgr;
    }

    ReplicationManager& replicationManager() {
        return m_replicationMgr;
    }
    [[nodiscard]] const ReplicationManager& replicationManager() const {
        return m_replicationMgr;
    }

    Session& session() { return m_session; }
    [[nodiscard]] const Session& session() const { return m_session; }

    LockstepManager& lockstep() { return m_lockstepMgr; }
    [[nodiscard]] const LockstepManager& lockstep() const {
        return m_lockstepMgr;
    }

    RollbackManager& rollback() { return m_rollbackMgr; }
    [[nodiscard]] const RollbackManager& rollback() const {
        return m_rollbackMgr;
    }

    RPCRegistry& rpcRegistry() { return m_rpcRegistry; }
    [[nodiscard]] const RPCRegistry& rpcRegistry() const {
        return m_rpcRegistry;
    }

    // ── Statistics ───────────────────────────────────────────────

    [[nodiscard]] uint32_t totalPacketsSent() const {
        return m_totalPacketsSent;
    }
    [[nodiscard]] uint32_t totalPacketsReceived() const {
        return m_totalPacketsReceived;
    }
    [[nodiscard]] float averageLatency() const {
        const auto& conns = m_connectionMgr.connections();
        if (conns.empty()) return 0.f;
        float total = 0.f;
        for (const auto& [id, conn] : conns) {
            total += conn.latency();
        }
        return total / static_cast<float>(conns.size());
    }

private:
    NetRole m_role = NetRole::None;
    ConnectionManager m_connectionMgr;
    ReplicationManager m_replicationMgr;
    Session m_session;
    LockstepManager m_lockstepMgr;
    RollbackManager m_rollbackMgr;
    RPCRegistry m_rpcRegistry;
    uint32_t m_totalPacketsSent = 0;
    uint32_t m_totalPacketsReceived = 0;
};

} // namespace NF
