#pragma once
// NF::Editor — Asset bundle editor v1: bundle definition and asset entry management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Abev1BundleMode  : uint8_t { Explicit, Automatic, Addressable, Streaming };
enum class Abev1BundleState : uint8_t { Draft, Building, Ready, Published, Stale };

inline const char* abev1BundleModeName(Abev1BundleMode m) {
    switch (m) {
        case Abev1BundleMode::Explicit:    return "Explicit";
        case Abev1BundleMode::Automatic:   return "Automatic";
        case Abev1BundleMode::Addressable: return "Addressable";
        case Abev1BundleMode::Streaming:   return "Streaming";
    }
    return "Unknown";
}

inline const char* abev1BundleStateName(Abev1BundleState s) {
    switch (s) {
        case Abev1BundleState::Draft:     return "Draft";
        case Abev1BundleState::Building:  return "Building";
        case Abev1BundleState::Ready:     return "Ready";
        case Abev1BundleState::Published: return "Published";
        case Abev1BundleState::Stale:     return "Stale";
    }
    return "Unknown";
}

struct Abev1Bundle {
    uint64_t          id         = 0;
    std::string       name;
    Abev1BundleMode   mode       = Abev1BundleMode::Explicit;
    Abev1BundleState  state      = Abev1BundleState::Draft;
    uint64_t          sizeBytes  = 0;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isReady()     const { return state == Abev1BundleState::Ready; }
    [[nodiscard]] bool isPublished() const { return state == Abev1BundleState::Published; }
    [[nodiscard]] bool isStale()     const { return state == Abev1BundleState::Stale; }
};

struct Abev1AssetEntry {
    uint64_t    id       = 0;
    uint64_t    bundleId = 0;
    std::string assetPath;

    [[nodiscard]] bool isValid() const { return id != 0 && bundleId != 0 && !assetPath.empty(); }
};

using Abev1ChangeCallback = std::function<void(uint64_t)>;

class AssetBundleEditorV1 {
public:
    static constexpr size_t MAX_BUNDLES = 256;
    static constexpr size_t MAX_ENTRIES = 8192;

    bool addBundle(const Abev1Bundle& bundle) {
        if (!bundle.isValid()) return false;
        for (const auto& b : m_bundles) if (b.id == bundle.id) return false;
        if (m_bundles.size() >= MAX_BUNDLES) return false;
        m_bundles.push_back(bundle);
        if (m_onChange) m_onChange(bundle.id);
        return true;
    }

    bool removeBundle(uint64_t id) {
        for (auto it = m_bundles.begin(); it != m_bundles.end(); ++it) {
            if (it->id == id) { m_bundles.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Abev1Bundle* findBundle(uint64_t id) {
        for (auto& b : m_bundles) if (b.id == id) return &b;
        return nullptr;
    }

    bool addEntry(const Abev1AssetEntry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        if (m_onChange) m_onChange(entry.bundleId);
        return true;
    }

    bool removeEntry(uint64_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t bundleCount() const { return m_bundles.size(); }
    [[nodiscard]] size_t entryCount()  const { return m_entries.size(); }

    [[nodiscard]] size_t readyCount() const {
        size_t c = 0; for (const auto& b : m_bundles) if (b.isReady()) ++c; return c;
    }
    [[nodiscard]] size_t countByMode(Abev1BundleMode mode) const {
        size_t c = 0; for (const auto& b : m_bundles) if (b.mode == mode) ++c; return c;
    }
    [[nodiscard]] size_t entriesForBundle(uint64_t bundleId) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.bundleId == bundleId) ++c; return c;
    }
    [[nodiscard]] uint64_t totalSizeBytes() const {
        uint64_t sum = 0; for (const auto& b : m_bundles) sum += b.sizeBytes; return sum;
    }

    void setOnChange(Abev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Abev1Bundle>     m_bundles;
    std::vector<Abev1AssetEntry> m_entries;
    Abev1ChangeCallback          m_onChange;
};

} // namespace NF
