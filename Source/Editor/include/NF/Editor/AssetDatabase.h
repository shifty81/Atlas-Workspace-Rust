#pragma once
// NF::Editor — Asset database
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

struct AssetGuid {
    uint64_t hi = 0;
    uint64_t lo = 0;

    [[nodiscard]] bool isNull() const { return hi == 0 && lo == 0; }

    bool operator==(const AssetGuid& o) const { return hi == o.hi && lo == o.lo; }
    bool operator!=(const AssetGuid& o) const { return !(*this == o); }
    bool operator<(const AssetGuid& o) const {
        return hi < o.hi || (hi == o.hi && lo < o.lo);
    }

    /// Generate a deterministic GUID from a path string (FNV-1a based).
    static AssetGuid fromPath(const std::string& path) {
        AssetGuid g;
        // FNV-1a 64-bit for hi
        uint64_t h = 14695981039346656037ULL;
        for (char c : path) {
            h ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
            h *= 1099511628211ULL;
        }
        g.hi = h;
        // Second pass with different seed for lo
        h = 17316040143175676883ULL;
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            h ^= static_cast<uint64_t>(static_cast<unsigned char>(*it));
            h *= 1099511628211ULL;
        }
        g.lo = h;
        return g;
    }

    /// Generate a unique GUID from a counter (for testing / new assets).
    static AssetGuid generate(uint64_t counter) {
        AssetGuid g;
        g.hi = 0x4F00FACE00000000ULL | (counter >> 32);
        g.lo = (counter & 0xFFFFFFFFULL) | 0xA55E700000000000ULL;
        return g;
    }

    [[nodiscard]] std::string toString() const {
        char buf[40];
        std::snprintf(buf, sizeof(buf), "%016llx-%016llx",
                      static_cast<unsigned long long>(hi),
                      static_cast<unsigned long long>(lo));
        return buf;
    }
};

enum class AssetType : uint8_t {
    Unknown  = 0,
    Mesh     = 1,
    Texture  = 2,
    Material = 3,
    Sound    = 4,
    Script   = 5,
    Graph    = 6,
    World    = 7
};

inline const char* assetTypeName(AssetType t) {
    switch (t) {
        case AssetType::Mesh:     return "Mesh";
        case AssetType::Texture:  return "Texture";
        case AssetType::Material: return "Material";
        case AssetType::Sound:    return "Sound";
        case AssetType::Script:   return "Script";
        case AssetType::Graph:    return "Graph";
        case AssetType::World:    return "World";
        default:                  return "Unknown";
    }
}

inline AssetType classifyAssetExtension(const std::string& ext) {
    if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") return AssetType::Mesh;
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") return AssetType::Texture;
    if (ext == ".mat" || ext == ".nfmat") return AssetType::Material;
    if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") return AssetType::Sound;
    if (ext == ".lua" || ext == ".nfs") return AssetType::Script;
    if (ext == ".nfg") return AssetType::Graph;
    if (ext == ".nfw") return AssetType::World;
    return AssetType::Unknown;
}

/// An entry in the asset database.
struct AssetEntry {
    AssetGuid   guid;
    std::string path;          // relative to content root
    std::string name;          // filename without extension
    AssetType   type       = AssetType::Unknown;
    uint64_t    lastModified = 0;  // epoch seconds
    size_t      sizeBytes  = 0;
    bool        imported   = false;
};

/// Central GUID-based asset registry.  Maps paths↔GUIDs and tracks import state.
class AssetDatabase {
public:
    /// Register an asset.  If the path already exists the existing entry is updated.
    AssetGuid registerAsset(const std::string& relativePath, AssetType type,
                            size_t sizeBytes = 0, uint64_t lastMod = 0) {
        // Check if path already registered
        for (auto& e : m_entries) {
            if (e.path == relativePath) {
                e.type = type;
                e.sizeBytes = sizeBytes;
                e.lastModified = lastMod;
                return e.guid;
            }
        }
        AssetEntry entry;
        entry.guid = AssetGuid::fromPath(relativePath);
        entry.path = relativePath;
        entry.type = type;
        entry.sizeBytes = sizeBytes;
        entry.lastModified = lastMod;

        // Extract name from path
        auto pos = relativePath.find_last_of("/\\");
        std::string filename = (pos != std::string::npos) ? relativePath.substr(pos + 1) : relativePath;
        auto dotPos = filename.find_last_of('.');
        entry.name = (dotPos != std::string::npos) ? filename.substr(0, dotPos) : filename;

        m_entries.push_back(entry);
        return entry.guid;
    }

    /// Remove an asset by GUID.
    bool removeAsset(const AssetGuid& guid) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->guid == guid) { m_entries.erase(it); return true; }
        }
        return false;
    }

    /// Find by GUID.
    [[nodiscard]] AssetEntry* findByGuid(const AssetGuid& guid) {
        for (auto& e : m_entries) if (e.guid == guid) return &e;
        return nullptr;
    }
    [[nodiscard]] const AssetEntry* findByGuid(const AssetGuid& guid) const {
        for (auto& e : m_entries) if (e.guid == guid) return &e;
        return nullptr;
    }

    /// Find by relative path.
    [[nodiscard]] AssetEntry* findByPath(const std::string& path) {
        for (auto& e : m_entries) if (e.path == path) return &e;
        return nullptr;
    }

    /// Mark an asset as imported.
    bool markImported(const AssetGuid& guid) {
        if (auto* e = findByGuid(guid)) { e->imported = true; return true; }
        return false;
    }

    /// Scan a directory tree and register all recognised asset files.
    size_t scanDirectory(const std::string& rootPath) {
        size_t count = 0;
        if (!std::filesystem::exists(rootPath)) return 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(rootPath)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            AssetType type = classifyAssetExtension(ext);
            if (type == AssetType::Unknown) continue;

            std::string relPath = entry.path().string();
            // Normalise to forward slashes
            for (char& c : relPath) if (c == '\\') c = '/';

            uint64_t lastMod = 0;
            auto ftime = std::filesystem::last_write_time(entry);
            lastMod = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());

            registerAsset(relPath, type, static_cast<size_t>(entry.file_size()), lastMod);
            ++count;
        }
        return count;
    }

    [[nodiscard]] const std::vector<AssetEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t assetCount() const { return m_entries.size(); }

    /// Return all assets of a given type.
    [[nodiscard]] std::vector<const AssetEntry*> assetsOfType(AssetType type) const {
        std::vector<const AssetEntry*> result;
        for (auto& e : m_entries) if (e.type == type) result.push_back(&e);
        return result;
    }

    [[nodiscard]] size_t importedCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.imported) ++n;
        return n;
    }

    void clear() { m_entries.clear(); }

private:
    std::vector<AssetEntry> m_entries;
};

/// Import settings for mesh assets.

} // namespace NF
