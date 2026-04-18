#pragma once
// NF::Editor — Network debugger panel
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class NetConnectionState : uint8_t {
    Disconnected, Connecting, Connected, Reconnecting, Failed
};

inline const char* netConnectionStateName(NetConnectionState s) {
    switch (s) {
        case NetConnectionState::Disconnected: return "Disconnected";
        case NetConnectionState::Connecting:   return "Connecting";
        case NetConnectionState::Connected:    return "Connected";
        case NetConnectionState::Reconnecting: return "Reconnecting";
        case NetConnectionState::Failed:       return "Failed";
    }
    return "Unknown";
}

enum class NetProtocol : uint8_t {
    UDP, TCP, WebSocket, QUIC, Relay
};

inline const char* netProtocolName(NetProtocol p) {
    switch (p) {
        case NetProtocol::UDP:       return "UDP";
        case NetProtocol::TCP:       return "TCP";
        case NetProtocol::WebSocket: return "WebSocket";
        case NetProtocol::QUIC:      return "QUIC";
        case NetProtocol::Relay:     return "Relay";
    }
    return "Unknown";
}

enum class NetMessageType : uint8_t {
    Reliable, Unreliable, ReliableOrdered, UnreliableSequenced, Broadcast
};

inline const char* netMessageTypeName(NetMessageType t) {
    switch (t) {
        case NetMessageType::Reliable:             return "Reliable";
        case NetMessageType::Unreliable:           return "Unreliable";
        case NetMessageType::ReliableOrdered:      return "ReliableOrdered";
        case NetMessageType::UnreliableSequenced:  return "UnreliableSequenced";
        case NetMessageType::Broadcast:            return "Broadcast";
    }
    return "Unknown";
}

class NetPeerEntry {
public:
    explicit NetPeerEntry(const std::string& id, const std::string& address)
        : m_id(id), m_address(address) {}

    void setState(NetConnectionState s) { m_state    = s; }
    void setProtocol(NetProtocol p)     { m_protocol = p; }
    void setLatencyMs(float ms)         { m_latencyMs = ms; }
    void setPacketLoss(float pct)       { m_packetLoss = pct; }
    void setBytesIn(uint64_t b)         { m_bytesIn  = b; }
    void setBytesOut(uint64_t b)        { m_bytesOut = b; }

    [[nodiscard]] const std::string&  id()         const { return m_id;         }
    [[nodiscard]] const std::string&  address()    const { return m_address;    }
    [[nodiscard]] NetConnectionState  state()      const { return m_state;      }
    [[nodiscard]] NetProtocol         protocol()   const { return m_protocol;   }
    [[nodiscard]] float               latencyMs()  const { return m_latencyMs;  }
    [[nodiscard]] float               packetLoss() const { return m_packetLoss; }
    [[nodiscard]] uint64_t            bytesIn()    const { return m_bytesIn;    }
    [[nodiscard]] uint64_t            bytesOut()   const { return m_bytesOut;   }

    [[nodiscard]] bool isConnected() const { return m_state == NetConnectionState::Connected; }
    [[nodiscard]] bool hasBadLatency(float threshold = 100.0f) const { return m_latencyMs > threshold; }

private:
    std::string         m_id;
    std::string         m_address;
    NetConnectionState  m_state      = NetConnectionState::Disconnected;
    NetProtocol         m_protocol   = NetProtocol::UDP;
    float               m_latencyMs  = 0.0f;
    float               m_packetLoss = 0.0f;
    uint64_t            m_bytesIn    = 0;
    uint64_t            m_bytesOut   = 0;
};

class NetworkDebuggerPanel {
public:
    static constexpr size_t MAX_PEERS = 64;

    [[nodiscard]] bool addPeer(const NetPeerEntry& peer) {
        for (auto& p : m_peers) if (p.id() == peer.id()) return false;
        if (m_peers.size() >= MAX_PEERS) return false;
        m_peers.push_back(peer);
        return true;
    }

    [[nodiscard]] bool removePeer(const std::string& id) {
        for (auto it = m_peers.begin(); it != m_peers.end(); ++it) {
            if (it->id() == id) {
                if (m_selectedPeer == id) m_selectedPeer.clear();
                m_peers.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] NetPeerEntry* findPeer(const std::string& id) {
        for (auto& p : m_peers) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool selectPeer(const std::string& id) {
        for (auto& p : m_peers) if (p.id() == id) { m_selectedPeer = id; return true; }
        return false;
    }

    [[nodiscard]] const std::string& selectedPeer() const { return m_selectedPeer; }
    [[nodiscard]] size_t peerCount()     const { return m_peers.size(); }

    [[nodiscard]] size_t connectedCount() const {
        size_t c = 0; for (auto& p : m_peers) if (p.isConnected()) ++c; return c;
    }
    [[nodiscard]] size_t countByState(NetConnectionState s) const {
        size_t c = 0; for (auto& p : m_peers) if (p.state() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByProtocol(NetProtocol proto) const {
        size_t c = 0; for (auto& p : m_peers) if (p.protocol() == proto) ++c; return c;
    }
    [[nodiscard]] size_t highLatencyCount(float threshold = 100.0f) const {
        size_t c = 0; for (auto& p : m_peers) if (p.hasBadLatency(threshold)) ++c; return c;
    }

private:
    std::vector<NetPeerEntry> m_peers;
    std::string               m_selectedPeer;
};

} // namespace NF
