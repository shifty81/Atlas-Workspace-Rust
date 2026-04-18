#pragma once
// NF::Editor — packaging editor
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

enum class PackageFormat : uint8_t {
    Zip, Tar, Pak, Iso, Flatpak, Custom
};

inline const char* packageFormatName(PackageFormat f) {
    switch (f) {
        case PackageFormat::Zip:     return "Zip";
        case PackageFormat::Tar:     return "Tar";
        case PackageFormat::Pak:     return "Pak";
        case PackageFormat::Iso:     return "Iso";
        case PackageFormat::Flatpak: return "Flatpak";
        case PackageFormat::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class PackagePlatform : uint8_t {
    Windows, MacOS, Linux, Android, iOS, Console, Web
};

inline const char* packagePlatformName(PackagePlatform p) {
    switch (p) {
        case PackagePlatform::Windows: return "Windows";
        case PackagePlatform::MacOS:   return "MacOS";
        case PackagePlatform::Linux:   return "Linux";
        case PackagePlatform::Android: return "Android";
        case PackagePlatform::iOS:     return "iOS";
        case PackagePlatform::Console: return "Console";
        case PackagePlatform::Web:     return "Web";
    }
    return "Unknown";
}

class PackageEntry {
public:
    explicit PackageEntry(uint32_t id, const std::string& name,
                          PackageFormat format, PackagePlatform platform)
        : m_id(id), m_name(name), m_format(format), m_platform(platform) {}

    void setVersionString(const std::string& v) { m_versionString = v; }
    void setIsRelease(bool v)                    { m_isRelease     = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;            }
    [[nodiscard]] const std::string& name()          const { return m_name;          }
    [[nodiscard]] PackageFormat      format()        const { return m_format;        }
    [[nodiscard]] PackagePlatform    platform()      const { return m_platform;      }
    [[nodiscard]] const std::string& versionString() const { return m_versionString; }
    [[nodiscard]] bool               isRelease()     const { return m_isRelease;     }

private:
    uint32_t        m_id;
    std::string     m_name;
    PackageFormat   m_format;
    PackagePlatform m_platform;
    std::string     m_versionString = "1.0.0";
    bool            m_isRelease     = false;
};

class PackagingEditor {
public:
    void setIsShowDraft(bool v)               { m_isShowDraft    = v; }
    void setIsGroupByPlatform(bool v)         { m_isGroupByPlatform = v; }
    void setOutputDir(const std::string& v)   { m_outputDir      = v; }

    bool addPackage(const PackageEntry& p) {
        for (auto& x : m_packages) if (x.id() == p.id()) return false;
        m_packages.push_back(p); return true;
    }
    bool removePackage(uint32_t id) {
        auto it = std::find_if(m_packages.begin(), m_packages.end(),
            [&](const PackageEntry& p){ return p.id() == id; });
        if (it == m_packages.end()) return false;
        m_packages.erase(it); return true;
    }
    [[nodiscard]] PackageEntry* findPackage(uint32_t id) {
        for (auto& p : m_packages) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool              isShowDraft()       const { return m_isShowDraft;      }
    [[nodiscard]] bool              isGroupByPlatform() const { return m_isGroupByPlatform; }
    [[nodiscard]] const std::string& outputDir()        const { return m_outputDir;        }
    [[nodiscard]] size_t            packageCount()      const { return m_packages.size();  }

    [[nodiscard]] size_t countByFormat(PackageFormat f) const {
        size_t n = 0; for (auto& p : m_packages) if (p.format() == f) ++n; return n;
    }
    [[nodiscard]] size_t countByPlatform(PackagePlatform pl) const {
        size_t n = 0; for (auto& p : m_packages) if (p.platform() == pl) ++n; return n;
    }
    [[nodiscard]] size_t countRelease() const {
        size_t n = 0; for (auto& p : m_packages) if (p.isRelease()) ++n; return n;
    }

private:
    std::vector<PackageEntry> m_packages;
    bool        m_isShowDraft      = false;
    bool        m_isGroupByPlatform = false;
    std::string m_outputDir        = "output";
};

} // namespace NF
