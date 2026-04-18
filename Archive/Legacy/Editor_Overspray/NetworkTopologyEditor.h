#pragma once
// NF::Editor — network topology configuration management
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
#include <string>
#include <vector>
#include <cstdint>

namespace NF {

enum class NetTopoType : uint8_t { P2P, ClientServer, RelayServer, Mesh, Hybrid };
inline const char* netTopoTypeName(NetTopoType v) {
    switch (v) {
        case NetTopoType::P2P:          return "P2P";
        case NetTopoType::ClientServer: return "ClientServer";
        case NetTopoType::RelayServer:  return "RelayServer";
        case NetTopoType::Mesh:         return "Mesh";
        case NetTopoType::Hybrid:       return "Hybrid";
    }
    return "Unknown";
}

enum class NetTopoProtocol : uint8_t { TCP, UDP, WebSocket, WebRTC, Custom };
inline const char* netTopoProtocolName(NetTopoProtocol v) {
    switch (v) {
        case NetTopoProtocol::TCP:       return "TCP";
        case NetTopoProtocol::UDP:       return "UDP";
        case NetTopoProtocol::WebSocket: return "WebSocket";
        case NetTopoProtocol::WebRTC:    return "WebRTC";
        case NetTopoProtocol::Custom:    return "Custom";
    }
    return "Unknown";
}

class NetworkTopologyDef {
public:
    explicit NetworkTopologyDef(uint32_t id, const std::string& name,
                                NetTopoType type, NetTopoProtocol protocol)
        : m_id(id), m_name(name), m_type(type), m_protocol(protocol) {}

    void setMaxPeers(uint32_t v)  { m_maxPeers  = v; }
    void setPort(uint32_t v)      { m_port       = v; }
    void setIsEnabled(bool v)     { m_isEnabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] NetTopoType        type()     const { return m_type;     }
    [[nodiscard]] NetTopoProtocol    protocol() const { return m_protocol; }
    [[nodiscard]] uint32_t           maxPeers() const { return m_maxPeers; }
    [[nodiscard]] uint32_t           port()     const { return m_port;     }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t      m_id;
    std::string   m_name;
    NetTopoType   m_type;
    NetTopoProtocol m_protocol;
    uint32_t      m_maxPeers  = 8u;
    uint32_t      m_port      = 7777u;
    bool          m_isEnabled = true;
};

class NetworkTopologyEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled    = v; }
    void setIsGroupByType(bool v)       { m_isGroupByType     = v; }
    void setDefaultMaxPeers(uint32_t v) { m_defaultMaxPeers   = v; }

    bool addTopology(const NetworkTopologyDef& t) {
        for (auto& x : m_topos) if (x.id() == t.id()) return false;
        m_topos.push_back(t); return true;
    }
    bool removeTopology(uint32_t id) {
        auto it = std::find_if(m_topos.begin(), m_topos.end(),
            [&](const NetworkTopologyDef& t){ return t.id() == id; });
        if (it == m_topos.end()) return false;
        m_topos.erase(it); return true;
    }
    [[nodiscard]] NetworkTopologyDef* findTopology(uint32_t id) {
        for (auto& t : m_topos) if (t.id() == id) return &t;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()  const { return m_isShowDisabled;  }
    [[nodiscard]] bool     isGroupByType()   const { return m_isGroupByType;   }
    [[nodiscard]] uint32_t defaultMaxPeers() const { return m_defaultMaxPeers; }
    [[nodiscard]] size_t   topoCount()       const { return m_topos.size();    }

    [[nodiscard]] size_t countByType(NetTopoType t) const {
        size_t n = 0; for (auto& x : m_topos) if (x.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByProtocol(NetTopoProtocol p) const {
        size_t n = 0; for (auto& x : m_topos) if (x.protocol() == p) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_topos) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<NetworkTopologyDef> m_topos;
    bool     m_isShowDisabled  = false;
    bool     m_isGroupByType   = true;
    uint32_t m_defaultMaxPeers = 16u;
};

} // namespace NF
