#pragma once
// NF::Editor — multiplayer lobby management editor
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

enum class LobbyVisibility : uint8_t { Public, Private, FriendsOnly, InviteOnly };
inline const char* lobbyVisibilityName(LobbyVisibility v) {
    switch (v) {
        case LobbyVisibility::Public:      return "Public";
        case LobbyVisibility::Private:     return "Private";
        case LobbyVisibility::FriendsOnly: return "FriendsOnly";
        case LobbyVisibility::InviteOnly:  return "InviteOnly";
    }
    return "Unknown";
}

enum class LobbyState : uint8_t { Open, Full, InGame, Closed, Disbanded };
inline const char* lobbyStateName(LobbyState v) {
    switch (v) {
        case LobbyState::Open:      return "Open";
        case LobbyState::Full:      return "Full";
        case LobbyState::InGame:    return "InGame";
        case LobbyState::Closed:    return "Closed";
        case LobbyState::Disbanded: return "Disbanded";
    }
    return "Unknown";
}

class LobbyEntry {
public:
    explicit LobbyEntry(uint32_t id, const std::string& name, LobbyVisibility visibility)
        : m_id(id), m_name(name), m_visibility(visibility) {}

    void setState(LobbyState v)       { m_state          = v; }
    void setMaxSlots(uint32_t v)      { m_maxSlots        = v; }
    void setIsVoiceEnabled(bool v)    { m_isVoiceEnabled  = v; }
    void setIsEnabled(bool v)         { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] LobbyVisibility    visibility()     const { return m_visibility;     }
    [[nodiscard]] LobbyState         state()          const { return m_state;          }
    [[nodiscard]] uint32_t           maxSlots()       const { return m_maxSlots;        }
    [[nodiscard]] bool               isVoiceEnabled() const { return m_isVoiceEnabled;  }
    [[nodiscard]] bool               isEnabled()      const { return m_isEnabled;      }

private:
    uint32_t        m_id;
    std::string     m_name;
    LobbyVisibility m_visibility;
    LobbyState      m_state          = LobbyState::Open;
    uint32_t        m_maxSlots        = 8u;
    bool            m_isVoiceEnabled  = false;
    bool            m_isEnabled      = true;
};

class LobbyEditor {
public:
    void setIsShowClosed(bool v)        { m_isShowClosed       = v; }
    void setIsGroupByVisibility(bool v) { m_isGroupByVisibility = v; }
    void setDefaultMaxSlots(uint32_t v) { m_defaultMaxSlots    = v; }

    bool addLobby(const LobbyEntry& e) {
        for (auto& x : m_lobbies) if (x.id() == e.id()) return false;
        m_lobbies.push_back(e); return true;
    }
    bool removeLobby(uint32_t id) {
        auto it = std::find_if(m_lobbies.begin(), m_lobbies.end(),
            [&](const LobbyEntry& e){ return e.id() == id; });
        if (it == m_lobbies.end()) return false;
        m_lobbies.erase(it); return true;
    }
    [[nodiscard]] LobbyEntry* findLobby(uint32_t id) {
        for (auto& e : m_lobbies) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowClosed()        const { return m_isShowClosed;        }
    [[nodiscard]] bool     isGroupByVisibility() const { return m_isGroupByVisibility; }
    [[nodiscard]] uint32_t defaultMaxSlots()     const { return m_defaultMaxSlots;     }
    [[nodiscard]] size_t   lobbyCount()          const { return m_lobbies.size();      }

    [[nodiscard]] size_t countByVisibility(LobbyVisibility v) const {
        size_t n = 0; for (auto& e : m_lobbies) if (e.visibility() == v) ++n; return n;
    }
    [[nodiscard]] size_t countByState(LobbyState s) const {
        size_t n = 0; for (auto& e : m_lobbies) if (e.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_lobbies) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<LobbyEntry> m_lobbies;
    bool     m_isShowClosed        = false;
    bool     m_isGroupByVisibility = false;
    uint32_t m_defaultMaxSlots     = 4u;
};

} // namespace NF
