#pragma once
// NF::Editor — Network sync editor v1: replicated property and sync group authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Nsv1SyncMode     : uint8_t { Reliable, Unreliable, ReliableOrdered };
enum class Nsv1Ownership    : uint8_t { Server, Client, SharedAuthority };
enum class Nsv1Interpolation: uint8_t { None, Linear, Hermite, Extrapolate };

struct Nsv1SyncProperty {
    uint64_t          id           = 0;
    std::string       name;
    Nsv1SyncMode      mode         = Nsv1SyncMode::Reliable;
    Nsv1Interpolation interp       = Nsv1Interpolation::Linear;
    float             sendRate     = 20.f;
    bool              compressed   = false;
    bool              enabled      = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && sendRate > 0.f; }
};

struct Nsv1SyncGroup {
    uint64_t                    id    = 0;
    std::string                 name;
    Nsv1Ownership               owner = Nsv1Ownership::Server;
    std::vector<Nsv1SyncProperty> props;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Nsv1ChangeCallback = std::function<void(uint64_t)>;

class NetworkSyncEditorV1 {
public:
    bool addGroup(const Nsv1SyncGroup& g) {
        if (!g.isValid()) return false;
        for (const auto& eg : m_groups) if (eg.id == g.id) return false;
        m_groups.push_back(g);
        return true;
    }

    bool removeGroup(uint64_t id) {
        for (auto it = m_groups.begin(); it != m_groups.end(); ++it) {
            if (it->id == id) { m_groups.erase(it); return true; }
        }
        return false;
    }

    bool addProperty(uint64_t groupId, const Nsv1SyncProperty& prop) {
        if (!prop.isValid()) return false;
        for (auto& g : m_groups) {
            if (g.id == groupId) {
                g.props.push_back(prop);
                if (m_onChange) m_onChange(groupId);
                return true;
            }
        }
        return false;
    }

    bool removeProperty(uint64_t groupId, uint64_t propId) {
        for (auto& g : m_groups) {
            if (g.id == groupId) {
                for (auto it = g.props.begin(); it != g.props.end(); ++it) {
                    if (it->id == propId) { g.props.erase(it); if (m_onChange) m_onChange(groupId); return true; }
                }
            }
        }
        return false;
    }

    bool setOwnership(uint64_t groupId, Nsv1Ownership owner) {
        for (auto& g : m_groups) {
            if (g.id == groupId) { g.owner = owner; if (m_onChange) m_onChange(groupId); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t groupCount() const { return m_groups.size(); }

    [[nodiscard]] const Nsv1SyncGroup* findGroup(uint64_t id) const {
        for (const auto& g : m_groups) if (g.id == id) return &g;
        return nullptr;
    }

    void setOnChange(Nsv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Nsv1SyncGroup> m_groups;
    Nsv1ChangeCallback         m_onChange;
};

} // namespace NF
