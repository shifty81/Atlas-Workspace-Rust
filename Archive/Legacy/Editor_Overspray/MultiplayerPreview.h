#pragma once
// NF::Editor — Multiplayer preview and session simulator
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

enum class MultiplayerRole : uint8_t {
    Host, Client, DedicatedServer, ListenServer, Spectator
};

inline const char* multiplayerRoleName(MultiplayerRole r) {
    switch (r) {
        case MultiplayerRole::Host:            return "Host";
        case MultiplayerRole::Client:          return "Client";
        case MultiplayerRole::DedicatedServer: return "DedicatedServer";
        case MultiplayerRole::ListenServer:    return "ListenServer";
        case MultiplayerRole::Spectator:       return "Spectator";
    }
    return "Unknown";
}

enum class MultiplayerSimMode : uint8_t {
    Local, LAN, WAN, Simulated, Relay
};

inline const char* multiplayerSimModeName(MultiplayerSimMode m) {
    switch (m) {
        case MultiplayerSimMode::Local:     return "Local";
        case MultiplayerSimMode::LAN:       return "LAN";
        case MultiplayerSimMode::WAN:       return "WAN";
        case MultiplayerSimMode::Simulated: return "Simulated";
        case MultiplayerSimMode::Relay:     return "Relay";
    }
    return "Unknown";
}

enum class PlayerSlotState : uint8_t {
    Empty, Occupied, Bot, Reserved, Kicked
};

inline const char* playerSlotStateName(PlayerSlotState s) {
    switch (s) {
        case PlayerSlotState::Empty:    return "Empty";
        case PlayerSlotState::Occupied: return "Occupied";
        case PlayerSlotState::Bot:      return "Bot";
        case PlayerSlotState::Reserved: return "Reserved";
        case PlayerSlotState::Kicked:   return "Kicked";
    }
    return "Unknown";
}

class PlayerSlot {
public:
    explicit PlayerSlot(uint8_t index) : m_index(index) {}

    void setState(PlayerSlotState s)       { m_state    = s;     }
    void setRole(MultiplayerRole r)        { m_role     = r;     }
    void setName(const std::string& name)  { m_name     = name;  }
    void setPing(uint16_t ms)              { m_ping     = ms;    }
    void setReady(bool v)                  { m_ready    = v;     }

    [[nodiscard]] uint8_t              index()   const { return m_index;  }
    [[nodiscard]] PlayerSlotState      state()   const { return m_state;  }
    [[nodiscard]] MultiplayerRole      role()    const { return m_role;   }
    [[nodiscard]] const std::string&   name()    const { return m_name;   }
    [[nodiscard]] uint16_t             ping()    const { return m_ping;   }
    [[nodiscard]] bool                 isReady() const { return m_ready;  }

    [[nodiscard]] bool isEmpty()    const { return m_state == PlayerSlotState::Empty;    }
    [[nodiscard]] bool isOccupied() const { return m_state == PlayerSlotState::Occupied; }
    [[nodiscard]] bool isBot()      const { return m_state == PlayerSlotState::Bot;      }

private:
    uint8_t          m_index;
    PlayerSlotState  m_state  = PlayerSlotState::Empty;
    MultiplayerRole  m_role   = MultiplayerRole::Client;
    std::string      m_name;
    uint16_t         m_ping   = 0;
    bool             m_ready  = false;
};

class MultiplayerPreviewPanel {
public:
    static constexpr uint8_t MAX_SLOTS = 16;

    explicit MultiplayerPreviewPanel() {
        for (uint8_t i = 0; i < MAX_SLOTS; ++i) m_slots.emplace_back(i);
    }

    [[nodiscard]] bool setSlot(uint8_t index, PlayerSlotState state, MultiplayerRole role) {
        if (index >= MAX_SLOTS) return false;
        m_slots[index].setState(state);
        m_slots[index].setRole(role);
        return true;
    }

    [[nodiscard]] PlayerSlot* slot(uint8_t index) {
        if (index >= MAX_SLOTS) return nullptr;
        return &m_slots[index];
    }

    void setSimMode(MultiplayerSimMode m) { m_simMode  = m; }
    void setRunning(bool v)               { m_running  = v; }
    void setSimLatencyMs(uint16_t ms)     { m_simLatency = ms; }

    [[nodiscard]] MultiplayerSimMode simMode()      const { return m_simMode;    }
    [[nodiscard]] bool               isRunning()    const { return m_running;    }
    [[nodiscard]] uint16_t           simLatencyMs() const { return m_simLatency; }
    [[nodiscard]] size_t             slotCount()    const { return m_slots.size(); }

    [[nodiscard]] size_t occupiedCount() const {
        size_t c = 0; for (auto& s : m_slots) if (s.isOccupied()) ++c; return c;
    }
    [[nodiscard]] size_t botCount() const {
        size_t c = 0; for (auto& s : m_slots) if (s.isBot()) ++c; return c;
    }
    [[nodiscard]] size_t readyCount() const {
        size_t c = 0; for (auto& s : m_slots) if (s.isReady()) ++c; return c;
    }
    [[nodiscard]] size_t countByRole(MultiplayerRole r) const {
        size_t c = 0; for (auto& s : m_slots) if (s.role() == r) ++c; return c;
    }

private:
    std::vector<PlayerSlot> m_slots;
    MultiplayerSimMode      m_simMode   = MultiplayerSimMode::Local;
    uint16_t                m_simLatency = 0;
    bool                    m_running   = false;
};

} // namespace NF
