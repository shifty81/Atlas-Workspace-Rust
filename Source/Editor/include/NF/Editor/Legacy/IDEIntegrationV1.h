#pragma once
// NF::Editor — IDE integration v1: project file sync, language server connection, file watchers
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Idev1FileState   : uint8_t { Unknown, Synced, Modified, Deleted, Conflict };
enum class Idev1ServerState : uint8_t { Disconnected, Connecting, Connected, Error };

inline const char* idev1FileStateName(Idev1FileState s) {
    switch (s) {
        case Idev1FileState::Unknown:  return "Unknown";
        case Idev1FileState::Synced:   return "Synced";
        case Idev1FileState::Modified: return "Modified";
        case Idev1FileState::Deleted:  return "Deleted";
        case Idev1FileState::Conflict: return "Conflict";
    }
    return "Unknown";
}

inline const char* idev1ServerStateName(Idev1ServerState s) {
    switch (s) {
        case Idev1ServerState::Disconnected: return "Disconnected";
        case Idev1ServerState::Connecting:   return "Connecting";
        case Idev1ServerState::Connected:    return "Connected";
        case Idev1ServerState::Error:        return "Error";
    }
    return "Unknown";
}

struct Idev1FileWatcher {
    std::string watchPath;
    bool        recursive = true;
    bool isValid() const { return !watchPath.empty(); }
};

struct Idev1LanguageServer {
    std::string      name;
    std::string      endpoint;
    Idev1ServerState state = Idev1ServerState::Disconnected;
    bool isValid() const { return !name.empty() && !endpoint.empty(); }
    bool isConnected() const { return state == Idev1ServerState::Connected; }
    bool hasError()    const { return state == Idev1ServerState::Error; }
};

struct Idev1ProjectFile {
    uint64_t        id    = 0;
    std::string     path;
    Idev1FileState  state = Idev1FileState::Unknown;
    bool            watched = false;

    [[nodiscard]] bool isValid()    const { return id != 0 && !path.empty(); }
    [[nodiscard]] bool isSynced()   const { return state == Idev1FileState::Synced; }
    [[nodiscard]] bool isModified() const { return state == Idev1FileState::Modified; }
    [[nodiscard]] bool hasConflict() const { return state == Idev1FileState::Conflict; }
};

using Idev1SyncCallback = std::function<void(uint64_t)>;

class IDEIntegrationV1 {
public:
    static constexpr size_t MAX_FILES = 2048;

    bool addFile(const Idev1ProjectFile& file) {
        if (!file.isValid()) return false;
        for (const auto& f : m_files) if (f.id == file.id) return false;
        if (m_files.size() >= MAX_FILES) return false;
        m_files.push_back(file);
        return true;
    }

    bool removeFile(uint64_t id) {
        for (auto it = m_files.begin(); it != m_files.end(); ++it) {
            if (it->id == id) { m_files.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Idev1ProjectFile* findFile(uint64_t id) {
        for (auto& f : m_files) if (f.id == id) return &f;
        return nullptr;
    }

    bool setState(uint64_t id, Idev1FileState state) {
        auto* f = findFile(id);
        if (!f) return false;
        f->state = state;
        if (m_onSync) m_onSync(id);
        return true;
    }

    bool connectServer(const Idev1LanguageServer& server) {
        if (!server.isValid()) return false;
        m_server = server;
        m_server.state = Idev1ServerState::Connected;
        return true;
    }

    bool disconnectServer() {
        if (m_server.state == Idev1ServerState::Disconnected) return false;
        m_server.state = Idev1ServerState::Disconnected;
        return true;
    }

    [[nodiscard]] Idev1ServerState serverState() const { return m_server.state; }

    [[nodiscard]] size_t fileCount()     const { return m_files.size(); }
    [[nodiscard]] size_t syncedCount()   const {
        size_t c = 0; for (const auto& f : m_files) if (f.isSynced())   ++c; return c;
    }
    [[nodiscard]] size_t modifiedCount() const {
        size_t c = 0; for (const auto& f : m_files) if (f.isModified()) ++c; return c;
    }

    void setOnSync(Idev1SyncCallback cb) { m_onSync = std::move(cb); }

private:
    std::vector<Idev1ProjectFile> m_files;
    Idev1LanguageServer           m_server;
    Idev1SyncCallback             m_onSync;
};

} // namespace NF
