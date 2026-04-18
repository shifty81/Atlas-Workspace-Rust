#pragma once
// NF::Editor — AssetDocument: asset metadata document model.
//
// Phase G.2 — Asset Editor full tool wiring.
//
// An AssetDocument is the authoritative in-editor record for a single content
// asset.  It covers:
//   - Identity: GUID, path, display name, asset type tag
//   - LOD / variant list
//   - Dependency table (other GUIDs this asset references)
//   - Reimport metadata (source path, importer settings)
//   - Dirty tracking, save/load contract

#include "NF/Core/Core.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace NF {

// ── Asset type ────────────────────────────────────────────────────────────────

enum class AssetDocType : uint8_t {
    Unknown,
    StaticMesh,
    SkeletalMesh,
    Texture,
    Material,
    AudioClip,
    AnimationClip,
    Prefab,
    DataTable,
    Script,
};

inline const char* assetDocTypeName(AssetDocType t) {
    switch (t) {
    case AssetDocType::StaticMesh:    return "StaticMesh";
    case AssetDocType::SkeletalMesh:  return "SkeletalMesh";
    case AssetDocType::Texture:       return "Texture";
    case AssetDocType::Material:      return "Material";
    case AssetDocType::AudioClip:     return "AudioClip";
    case AssetDocType::AnimationClip: return "AnimationClip";
    case AssetDocType::Prefab:        return "Prefab";
    case AssetDocType::DataTable:     return "DataTable";
    case AssetDocType::Script:        return "Script";
    default:                           return "Unknown";
    }
}

// ── LOD descriptor ────────────────────────────────────────────────────────────

struct AssetLODEntry {
    uint32_t    lodIndex     = 0;
    std::string meshTag;          ///< content-addressable mesh tag at this LOD
    float       screenPercent = 100.f; ///< switch-in threshold
    uint32_t    triangleCount = 0;
};

// ── Variant descriptor ────────────────────────────────────────────────────────

struct AssetVariantEntry {
    std::string variantId;
    std::string displayName;
    std::map<std::string, std::string> overrides; ///< material/texture slot overrides
};

// ── Dependency entry ──────────────────────────────────────────────────────────

struct AssetDependency {
    std::string guid;         ///< GUID of the referenced asset
    std::string relationship; ///< e.g. "material", "texture", "skeleton"
};

// ── Reimport settings ─────────────────────────────────────────────────────────

struct AssetImportSettings {
    std::string sourcePath;          ///< original source file on disk
    bool        generateMipmaps  = true;
    bool        compressTextures = true;
    float       importScale      = 1.f;
    std::string importerClass;       ///< e.g. "FBXImporter", "PNGImporter"
};

// ── AssetDocument ─────────────────────────────────────────────────────────────

class AssetDocument {
public:
    AssetDocument() = default;
    explicit AssetDocument(const std::string& guid, const std::string& path,
                            AssetDocType type = AssetDocType::Unknown)
        : m_guid(guid), m_assetPath(path), m_type(type) {}

    // ── Identity ──────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& guid()        const { return m_guid; }
    [[nodiscard]] const std::string& assetPath()   const { return m_assetPath; }
    [[nodiscard]] const std::string& displayName() const { return m_displayName; }
    [[nodiscard]] AssetDocType       type()         const { return m_type; }

    void setDisplayName(const std::string& name) { m_displayName = name; markDirty(); }
    void setType(AssetDocType t) { m_type = t; markDirty(); }
    void setAssetPath(const std::string& path) { m_assetPath = path; markDirty(); }

    // ── Dirty tracking ────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── LOD management ────────────────────────────────────────────────────────

    void addLOD(const AssetLODEntry& entry) {
        m_lods.push_back(entry);
        markDirty();
    }

    bool removeLOD(uint32_t lodIndex) {
        for (auto it = m_lods.begin(); it != m_lods.end(); ++it) {
            if (it->lodIndex == lodIndex) {
                m_lods.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t lodCount() const {
        return static_cast<uint32_t>(m_lods.size());
    }

    [[nodiscard]] const std::vector<AssetLODEntry>& lods() const { return m_lods; }

    [[nodiscard]] const AssetLODEntry* findLOD(uint32_t lodIndex) const {
        for (const auto& e : m_lods) {
            if (e.lodIndex == lodIndex) return &e;
        }
        return nullptr;
    }

    bool setLODScreenPercent(uint32_t lodIndex, float pct) {
        for (auto& e : m_lods) {
            if (e.lodIndex == lodIndex) {
                e.screenPercent = pct;
                markDirty();
                return true;
            }
        }
        return false;
    }

    // ── Variant management ────────────────────────────────────────────────────

    void addVariant(const AssetVariantEntry& v) {
        m_variants.push_back(v);
        markDirty();
    }

    bool removeVariant(const std::string& variantId) {
        for (auto it = m_variants.begin(); it != m_variants.end(); ++it) {
            if (it->variantId == variantId) {
                m_variants.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t variantCount() const {
        return static_cast<uint32_t>(m_variants.size());
    }

    [[nodiscard]] const AssetVariantEntry* findVariant(const std::string& variantId) const {
        for (const auto& v : m_variants) {
            if (v.variantId == variantId) return &v;
        }
        return nullptr;
    }

    // ── Dependency management ─────────────────────────────────────────────────

    void addDependency(const AssetDependency& dep) {
        m_dependencies.push_back(dep);
        markDirty();
    }

    bool removeDependency(const std::string& guid) {
        for (auto it = m_dependencies.begin(); it != m_dependencies.end(); ++it) {
            if (it->guid == guid) {
                m_dependencies.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t dependencyCount() const {
        return static_cast<uint32_t>(m_dependencies.size());
    }

    [[nodiscard]] const std::vector<AssetDependency>& dependencies() const {
        return m_dependencies;
    }

    // ── Reimport settings ─────────────────────────────────────────────────────

    [[nodiscard]] const AssetImportSettings& importSettings() const { return m_importSettings; }

    void setImportSettings(const AssetImportSettings& s) {
        m_importSettings = s;
        markDirty();
    }

    void setSourcePath(const std::string& path) {
        m_importSettings.sourcePath = path;
        markDirty();
    }

    // ── Save / load ───────────────────────────────────────────────────────────

    bool save() {
        if (m_assetPath.empty()) return false;
        clearDirty();
        return true;
    }

    bool load() {
        clearDirty();
        return true;
    }

    [[nodiscard]] std::string serialize() const {
        std::string out = "{\"guid\":\"" + m_guid + "\",";
        out += "\"type\":\"" + std::string(assetDocTypeName(m_type)) + "\",";
        out += "\"lods\":" + std::to_string(m_lods.size()) + ",";
        out += "\"variants\":" + std::to_string(m_variants.size()) + "}";
        return out;
    }

private:
    std::string          m_guid;
    std::string          m_assetPath;
    std::string          m_displayName;
    AssetDocType         m_type = AssetDocType::Unknown;
    bool                 m_dirty = false;

    std::vector<AssetLODEntry>     m_lods;
    std::vector<AssetVariantEntry> m_variants;
    std::vector<AssetDependency>   m_dependencies;
    AssetImportSettings            m_importSettings;
};

} // namespace NF
