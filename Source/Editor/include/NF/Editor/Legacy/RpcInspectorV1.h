#pragma once
// NF::Editor — RPC inspector v1: remote procedure call definition and call log
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Riv1RpcTarget    : uint8_t { Server, Client, AllClients, Owner, Others };
enum class Riv1RpcReliability: uint8_t { Reliable, Unreliable, ReliableBuffered };

struct Riv1RpcParam {
    std::string name;
    std::string typeName;
};

struct Riv1RpcDefinition {
    uint64_t                  id          = 0;
    std::string               name;
    Riv1RpcTarget             target      = Riv1RpcTarget::Server;
    Riv1RpcReliability        reliability = Riv1RpcReliability::Reliable;
    std::vector<Riv1RpcParam> params;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Riv1CallRecord {
    uint64_t     id         = 0;
    uint64_t     rpcId      = 0;
    uint64_t     timestampMs= 0;
    bool         success    = true;
    [[nodiscard]] bool isValid() const { return id != 0 && rpcId != 0; }
};

using Riv1CallCallback = std::function<void(const Riv1CallRecord&)>;

class RpcInspectorV1 {
public:
    bool addRpc(const Riv1RpcDefinition& rpc) {
        if (!rpc.isValid()) return false;
        for (const auto& er : m_rpcs) if (er.id == rpc.id) return false;
        m_rpcs.push_back(rpc);
        return true;
    }

    bool removeRpc(uint64_t id) {
        for (auto it = m_rpcs.begin(); it != m_rpcs.end(); ++it) {
            if (it->id == id) { m_rpcs.erase(it); return true; }
        }
        return false;
    }

    bool logCall(const Riv1CallRecord& rec) {
        if (!rec.isValid()) return false;
        m_calls.push_back(rec);
        if (m_onCall) m_onCall(rec);
        return true;
    }

    bool clearLog() {
        if (m_calls.empty()) return false;
        m_calls.clear();
        return true;
    }

    [[nodiscard]] size_t rpcCount()  const { return m_rpcs.size();  }
    [[nodiscard]] size_t callCount() const { return m_calls.size(); }

    [[nodiscard]] const Riv1RpcDefinition* findRpc(uint64_t id) const {
        for (const auto& r : m_rpcs) if (r.id == id) return &r;
        return nullptr;
    }

    void setOnCall(Riv1CallCallback cb) { m_onCall = std::move(cb); }

private:
    std::vector<Riv1RpcDefinition> m_rpcs;
    std::vector<Riv1CallRecord>    m_calls;
    Riv1CallCallback               m_onCall;
};

} // namespace NF
