#pragma once
// NF::Workspace — AssetCatalog: authoritative registry of all known assets.
//
// The AssetCatalog is the single source of truth for asset identity and state
// within the workspace. It is NOT a filesystem index — it tracks the workspace's
// view of assets (imported, staged, or virtual) using typed descriptors.
//
// Components:
//   AssetTypeTag    — coarse asset category (Texture/Mesh/Audio/…/Custom)
//   AssetImportState — lifecycle state for an asset (Unknown → Imported → Dirty → Error)
//   AssetMetadata   — string key-value bag for arbitrary per-asset properties
//   AssetDescriptor — primary asset record (id + path + type + state + metadata)
//   AssetCatalog    — owns all descriptors; supports CRUD + query

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Asset Type Tag ─────────────────────────────────────────────────

enum class AssetTypeTag : uint8_t {
    Unknown,
    Texture,
    Mesh,
    Audio,
    Script,
    Shader,
    Scene,
    Font,
    Video,
    Archive,
    Project,
    Material,
    Animation,
    Prefab,
    Custom,
    Data,      // game data / config files (.json, .yaml, .toml, .csv, etc.)
};

inline const char* assetTypeTagName(AssetTypeTag t) {
    switch (t) {
    case AssetTypeTag::Unknown:   return "Unknown";
    case AssetTypeTag::Texture:   return "Texture";
    case AssetTypeTag::Mesh:      return "Mesh";
    case AssetTypeTag::Audio:     return "Audio";
    case AssetTypeTag::Script:    return "Script";
    case AssetTypeTag::Shader:    return "Shader";
    case AssetTypeTag::Scene:     return "Scene";
    case AssetTypeTag::Font:      return "Font";
    case AssetTypeTag::Video:     return "Video";
    case AssetTypeTag::Archive:   return "Archive";
    case AssetTypeTag::Project:   return "Project";
    case AssetTypeTag::Material:  return "Material";
    case AssetTypeTag::Animation: return "Animation";
    case AssetTypeTag::Prefab:    return "Prefab";
    case AssetTypeTag::Custom:    return "Custom";
    case AssetTypeTag::Data:      return "Data";
    }
    return "Unknown";
}

// ── Asset Import State ─────────────────────────────────────────────

enum class AssetImportState : uint8_t {
    Unknown,     // not yet processed
    Staged,      // queued for import, not yet written
    Importing,   // currently being imported
    Imported,    // successfully imported and in catalog
    Dirty,       // source file changed since last import
    Error,       // import failed
    Excluded,    // explicitly excluded from import
};

inline const char* assetImportStateName(AssetImportState s) {
    switch (s) {
    case AssetImportState::Unknown:   return "Unknown";
    case AssetImportState::Staged:    return "Staged";
    case AssetImportState::Importing: return "Importing";
    case AssetImportState::Imported:  return "Imported";
    case AssetImportState::Dirty:     return "Dirty";
    case AssetImportState::Error:     return "Error";
    case AssetImportState::Excluded:  return "Excluded";
    }
    return "Unknown";
}

// ── Asset Metadata ─────────────────────────────────────────────────
// Arbitrary key-value bag attached to an asset descriptor.
// All values are strings; callers convert as needed.

class AssetMetadata {
public:
    static constexpr size_t MAX_ENTRIES = 64;

    void set(const std::string& key, const std::string& value) {
        for (auto& [k, v] : m_entries) {
            if (k == key) { v = value; return; }
        }
        if (m_entries.size() < MAX_ENTRIES)
            m_entries.push_back({key, value});
    }

    [[nodiscard]] const std::string* get(const std::string& key) const {
        for (const auto& [k, v] : m_entries)
            if (k == key) return &v;
        return nullptr;
    }

    [[nodiscard]] std::string getOr(const std::string& key,
                                    const std::string& fallback) const {
        const auto* v = get(key);
        return v ? *v : fallback;
    }

    [[nodiscard]] bool has(const std::string& key) const {
        return get(key) != nullptr;
    }

    bool remove(const std::string& key) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->first == key) { m_entries.erase(it); return true; }
        }
        return false;
    }

    void clear() { m_entries.clear(); }

    [[nodiscard]] size_t count() const { return m_entries.size(); }
    [[nodiscard]] bool empty()   const { return m_entries.empty(); }

    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>& entries() const {
        return m_entries;
    }

private:
    std::vector<std::pair<std::string, std::string>> m_entries;
};

// ── Asset ID ──────────────────────────────────────────────────────
// 32-bit handle; 0 is always invalid.
using AssetId = uint32_t;
static constexpr AssetId INVALID_ASSET_ID = 0;

// ── Asset Descriptor ──────────────────────────────────────────────

struct AssetDescriptor {
    AssetId         id          = INVALID_ASSET_ID;
    std::string     sourcePath;   // absolute path to source file on disk
    std::string     catalogPath;  // logical path within the catalog (e.g. "Textures/rock.png")
    std::string     displayName;  // human-readable label
    AssetTypeTag    typeTag     = AssetTypeTag::Unknown;
    AssetImportState importState = AssetImportState::Unknown;
    uint64_t        sourceSizeBytes = 0;
    uint64_t        importTimestamp = 0;  // Unix epoch seconds
    std::string     importError;          // only set when importState == Error
    AssetMetadata   metadata;

    [[nodiscard]] bool isValid() const {
        return id != INVALID_ASSET_ID
            && !sourcePath.empty()
            && !catalogPath.empty()
            && typeTag != AssetTypeTag::Unknown;
    }

    [[nodiscard]] bool isImported() const {
        return importState == AssetImportState::Imported;
    }

    [[nodiscard]] bool needsReimport() const {
        return importState == AssetImportState::Dirty
            || importState == AssetImportState::Error
            || importState == AssetImportState::Unknown;
    }

    [[nodiscard]] std::string extension() const {
        auto dot = sourcePath.rfind('.');
        return dot != std::string::npos ? sourcePath.substr(dot) : "";
    }
};

// ── Asset Catalog ──────────────────────────────────────────────────
// Primary registry. Owns all AssetDescriptor records.
// Thread-safety: single-threaded (workspace main thread only).

class AssetCatalog {
public:
    static constexpr size_t MAX_ASSETS = 65536;

    // ── Registration ────────────────────────────────────────────

    // Add a new asset descriptor. Returns the assigned id, or INVALID_ASSET_ID on failure.
    AssetId add(AssetDescriptor desc) {
        if (m_assets.size() >= MAX_ASSETS) return INVALID_ASSET_ID;
        if (desc.sourcePath.empty() || desc.catalogPath.empty()) return INVALID_ASSET_ID;
        if (desc.typeTag == AssetTypeTag::Unknown) return INVALID_ASSET_ID;
        // Reject duplicate catalog path
        if (m_byPath.count(desc.catalogPath)) return INVALID_ASSET_ID;

        desc.id = ++m_nextId;
        m_byPath[desc.catalogPath] = desc.id;
        m_assets[desc.id] = std::move(desc);
        ++m_addCount;
        return m_nextId;
    }

    bool remove(AssetId id) {
        auto it = m_assets.find(id);
        if (it == m_assets.end()) return false;
        m_byPath.erase(it->second.catalogPath);
        m_assets.erase(it);
        return true;
    }

    // ── Mutation ─────────────────────────────────────────────────

    bool setImportState(AssetId id, AssetImportState state) {
        auto it = m_assets.find(id);
        if (it == m_assets.end()) return false;
        it->second.importState = state;
        return true;
    }

    bool setImportError(AssetId id, const std::string& error) {
        auto it = m_assets.find(id);
        if (it == m_assets.end()) return false;
        it->second.importState  = AssetImportState::Error;
        it->second.importError  = error;
        return true;
    }

    bool setMetadata(AssetId id, const std::string& key, const std::string& value) {
        auto it = m_assets.find(id);
        if (it == m_assets.end()) return false;
        it->second.metadata.set(key, value);
        return true;
    }

    bool markDirty(AssetId id) {
        return setImportState(id, AssetImportState::Dirty);
    }

    // ── Query ─────────────────────────────────────────────────────

    [[nodiscard]] const AssetDescriptor* find(AssetId id) const {
        auto it = m_assets.find(id);
        return it != m_assets.end() ? &it->second : nullptr;
    }

    [[nodiscard]] const AssetDescriptor* findByPath(const std::string& catalogPath) const {
        auto it = m_byPath.find(catalogPath);
        if (it == m_byPath.end()) return nullptr;
        return find(it->second);
    }

    [[nodiscard]] bool contains(AssetId id) const {
        return m_assets.count(id) > 0;
    }

    [[nodiscard]] size_t count()    const { return m_assets.size();  }
    [[nodiscard]] bool   empty()    const { return m_assets.empty(); }
    [[nodiscard]] size_t addCount() const { return m_addCount;       }

    // Count assets with a given import state
    [[nodiscard]] size_t countByState(AssetImportState state) const {
        size_t n = 0;
        for (const auto& [id, desc] : m_assets)
            if (desc.importState == state) ++n;
        return n;
    }

    // Count assets with a given type tag
    [[nodiscard]] size_t countByType(AssetTypeTag tag) const {
        size_t n = 0;
        for (const auto& [id, desc] : m_assets)
            if (desc.typeTag == tag) ++n;
        return n;
    }

    // Collect all assets matching a predicate
    [[nodiscard]] std::vector<const AssetDescriptor*>
    query(std::function<bool(const AssetDescriptor&)> pred) const {
        std::vector<const AssetDescriptor*> out;
        for (const auto& [id, desc] : m_assets)
            if (pred(desc)) out.push_back(&desc);
        return out;
    }

    // Collect all descriptors as a flat list (order not guaranteed)
    [[nodiscard]] std::vector<const AssetDescriptor*> all() const {
        std::vector<const AssetDescriptor*> out;
        out.reserve(m_assets.size());
        for (const auto& [id, desc] : m_assets)
            out.push_back(&desc);
        return out;
    }

    void clear() {
        m_assets.clear();
        m_byPath.clear();
        m_nextId    = 0;
        m_addCount  = 0;
    }

private:
    std::unordered_map<AssetId, AssetDescriptor> m_assets;
    std::unordered_map<std::string, AssetId>     m_byPath;
    AssetId m_nextId   = 0;
    size_t  m_addCount = 0;
};

} // namespace NF
