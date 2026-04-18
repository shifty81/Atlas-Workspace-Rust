#pragma once
// NF::Editor — Package manager v1: install/uninstall packages with dependency tracking
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class PkgStatus : uint8_t {
    NotInstalled, Installing, Installed, Outdated, Failed, Removed
};

inline const char* pkgStatusName(PkgStatus s) {
    switch(s){
        case PkgStatus::NotInstalled: return "NotInstalled";
        case PkgStatus::Installing:   return "Installing";
        case PkgStatus::Installed:    return "Installed";
        case PkgStatus::Outdated:     return "Outdated";
        case PkgStatus::Failed:       return "Failed";
        case PkgStatus::Removed:      return "Removed";
    }
    return "Unknown";
}

struct PkgEntry {
    uint32_t              id     = 0;
    std::string           name;
    std::string           version;
    std::string           author;
    PkgStatus             status = PkgStatus::NotInstalled;
    std::vector<uint32_t> deps;
    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isInstalled() const { return status == PkgStatus::Installed; }
};

using PkgStatusCallback = std::function<void(uint32_t, PkgStatus)>;

class PackageManagerV1 {
public:
    static constexpr size_t MAX_PACKAGES = 256;

    bool addPackage(const PkgEntry& pkg) {
        if (!pkg.isValid()) return false;
        if (m_packages.size() >= MAX_PACKAGES) return false;
        for (const auto& p : m_packages) if (p.id == pkg.id) return false;
        m_packages.push_back(pkg);
        return true;
    }

    bool removePackage(uint32_t id) {
        for (auto it = m_packages.begin(); it != m_packages.end(); ++it) {
            if (it->id == id) { m_packages.erase(it); return true; }
        }
        return false;
    }

    bool install(uint32_t id) {
        PkgEntry* p = findPackage_(id);
        if (!p) return false;
        p->status = PkgStatus::Installed;
        if (m_onStatusChange) m_onStatusChange(id, PkgStatus::Installed);
        return true;
    }

    bool uninstall(uint32_t id) {
        PkgEntry* p = findPackage_(id);
        if (!p) return false;
        p->status = PkgStatus::Removed;
        if (m_onStatusChange) m_onStatusChange(id, PkgStatus::Removed);
        return true;
    }

    [[nodiscard]] size_t packageCount() const { return m_packages.size(); }

    const PkgEntry* findPackage(const std::string& name) const {
        for (const auto& p : m_packages) if (p.name == name) return &p;
        return nullptr;
    }

    PkgStatus getStatus(uint32_t id) const {
        for (const auto& p : m_packages) if (p.id == id) return p.status;
        return PkgStatus::NotInstalled;
    }

    void setOnStatusChange(PkgStatusCallback cb) { m_onStatusChange = std::move(cb); }

private:
    PkgEntry* findPackage_(uint32_t id) {
        for (auto& p : m_packages) if (p.id == id) return &p;
        return nullptr;
    }

    std::vector<PkgEntry>  m_packages;
    PkgStatusCallback      m_onStatusChange;
};

} // namespace NF
