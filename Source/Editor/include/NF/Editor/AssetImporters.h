#pragma once
// NF::Editor — Mesh/texture importers + asset watcher
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
#include "NF/Editor/AssetDatabase.h"

namespace NF {

struct MeshImportSettings {
    float scaleFactor       = 1.0f;
    bool  generateNormals   = true;
    bool  generateTangents  = false;
    bool  flipWindingOrder  = false;
    bool  mergeMeshes       = false;
    int   maxVertices       = 0;  // 0 = no limit
};

/// Import settings for texture assets.
struct TextureImportSettings {
    bool  generateMipmaps    = true;
    bool  sRGB               = true;
    int   maxResolution      = 0;   // 0 = no limit
    bool  premultiplyAlpha   = false;
    bool  flipVertically     = true;
    float compressionQuality = 0.8f; // 0-1
};

/// Mesh importer — validates and "imports" mesh files into the asset database.
class MeshImporter {
public:
    void setSettings(const MeshImportSettings& s) { m_settings = s; }
    [[nodiscard]] const MeshImportSettings& settings() const { return m_settings; }

    /// Validate that the given path is a supported mesh format.
    [[nodiscard]] bool canImport(const std::string& path) const {
        auto ext = std::filesystem::path(path).extension().string();
        return ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb";
    }

    /// Import a mesh into the asset database.  Returns the GUID.
    AssetGuid import(AssetDatabase& db, const std::string& relativePath) {
        if (!canImport(relativePath)) return {};
        AssetGuid guid = db.registerAsset(relativePath, AssetType::Mesh);
        db.markImported(guid);
        ++m_importCount;
        NF_LOG_INFO("MeshImporter", "Imported mesh: " + relativePath);
        return guid;
    }

    [[nodiscard]] size_t importCount() const { return m_importCount; }

private:
    MeshImportSettings m_settings;
    size_t m_importCount = 0;
};

/// Texture importer — validates and "imports" texture files into the asset database.
class TextureImporter {
public:
    void setSettings(const TextureImportSettings& s) { m_settings = s; }
    [[nodiscard]] const TextureImportSettings& settings() const { return m_settings; }

    /// Validate that the given path is a supported texture format.
    [[nodiscard]] bool canImport(const std::string& path) const {
        auto ext = std::filesystem::path(path).extension().string();
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
               ext == ".tga" || ext == ".bmp";
    }

    /// Import a texture into the asset database.  Returns the GUID.
    AssetGuid import(AssetDatabase& db, const std::string& relativePath) {
        if (!canImport(relativePath)) return {};
        AssetGuid guid = db.registerAsset(relativePath, AssetType::Texture);
        db.markImported(guid);
        ++m_importCount;
        NF_LOG_INFO("TextureImporter", "Imported texture: " + relativePath);
        return guid;
    }

    [[nodiscard]] size_t importCount() const { return m_importCount; }

private:
    TextureImportSettings m_settings;
    size_t m_importCount = 0;
};

/// Watches for asset changes and tracks which GUIDs need re-import (hot-reload).
class AssetWatcher {
public:
    /// Poll the asset database for changes.  Returns number of dirty assets detected.
    size_t pollChanges(AssetDatabase& db) {
        size_t detected = 0;
        for (auto& entry : db.entries()) {
            if (!std::filesystem::exists(entry.path)) continue;
            auto ftime = std::filesystem::last_write_time(entry.path);
            uint64_t modTime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());
            if (modTime > entry.lastModified) {
                m_dirtyAssets.insert(entry.guid);
                ++detected;
            }
        }
        return detected;
    }

    /// Mark a GUID as dirty (needs re-import).
    void markDirty(const AssetGuid& guid) { m_dirtyAssets.insert(guid); }

    /// Clear a GUID from dirty set after re-import.
    void clearDirty(const AssetGuid& guid) { m_dirtyAssets.erase(guid); }

    /// Check if an asset is dirty.
    [[nodiscard]] bool isDirty(const AssetGuid& guid) const {
        return m_dirtyAssets.count(guid) > 0;
    }

    [[nodiscard]] size_t dirtyCount() const { return m_dirtyAssets.size(); }

    void clearAll() { m_dirtyAssets.clear(); }

    [[nodiscard]] const std::set<AssetGuid>& dirtyAssets() const { return m_dirtyAssets; }

private:
    std::set<AssetGuid> m_dirtyAssets;
};

// ── S4 Blender Bridge ────────────────────────────────────────────

/// Supported Blender export formats.

} // namespace NF
