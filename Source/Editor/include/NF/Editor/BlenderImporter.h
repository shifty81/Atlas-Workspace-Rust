#pragma once
// NF::Editor — Blender auto-importer
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
#include "NF/Editor/AssetImporters.h"

namespace NF {

enum class BlenderExportFormat : uint8_t {
    FBX  = 0,
    GLTF = 1,
    OBJ  = 2,
    GLB  = 3
};

inline const char* blenderExportFormatName(BlenderExportFormat f) {
    switch (f) {
        case BlenderExportFormat::FBX:  return "FBX";
        case BlenderExportFormat::GLTF: return "GLTF";
        case BlenderExportFormat::OBJ:  return "OBJ";
        case BlenderExportFormat::GLB:  return "GLB";
    }
    return "Unknown";
}

inline const char* blenderExportFormatExtension(BlenderExportFormat f) {
    switch (f) {
        case BlenderExportFormat::FBX:  return ".fbx";
        case BlenderExportFormat::GLTF: return ".gltf";
        case BlenderExportFormat::OBJ:  return ".obj";
        case BlenderExportFormat::GLB:  return ".glb";
    }
    return "";
}

/// Record of a single Blender export that arrived in the watched directory.
struct BlenderExportEntry {
    std::string sourcePath;                      // path inside export dir
    BlenderExportFormat format = BlenderExportFormat::FBX;
    uint64_t exportedAt        = 0;              // epoch seconds
    bool autoImported          = false;          // true once auto-imported
    AssetGuid importedGuid;                      // GUID after import (null if not yet)
};

/// Watches a Blender export directory and auto-imports new/changed assets
/// into the editor's AssetDatabase via MeshImporter.
class BlenderAutoImporter {
public:
    /// Set the directory to watch for Blender exports.
    void setExportDirectory(const std::string& dir) { m_exportDir = dir; }
    [[nodiscard]] const std::string& exportDirectory() const { return m_exportDir; }

    /// Enable or disable auto-import.
    void setAutoImportEnabled(bool enabled) { m_autoImport = enabled; }
    [[nodiscard]] bool isAutoImportEnabled() const { return m_autoImport; }

    /// Scan the export directory for new or changed files.
    /// Returns number of new exports detected.
    size_t scanExports() {
        if (m_exportDir.empty() || !std::filesystem::exists(m_exportDir)) return 0;

        size_t detected = 0;
        for (auto& entry : std::filesystem::directory_iterator(m_exportDir)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            BlenderExportFormat fmt;
            if      (ext == ".fbx")  fmt = BlenderExportFormat::FBX;
            else if (ext == ".gltf") fmt = BlenderExportFormat::GLTF;
            else if (ext == ".obj")  fmt = BlenderExportFormat::OBJ;
            else if (ext == ".glb")  fmt = BlenderExportFormat::GLB;
            else continue;

            std::string relPath = entry.path().string();
            // Skip already-known files
            bool found = false;
            for (auto& e : m_exports) {
                if (e.sourcePath == relPath) { found = true; break; }
            }
            if (found) continue;

            BlenderExportEntry be;
            be.sourcePath = relPath;
            be.format = fmt;
            auto ftime = std::filesystem::last_write_time(entry);
            be.exportedAt = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());
            m_exports.push_back(be);
            ++detected;
        }
        return detected;
    }

    /// Auto-import all pending exports into the given asset database.
    /// Returns number of assets imported.
    size_t importPending(AssetDatabase& db, MeshImporter& meshImporter) {
        size_t imported = 0;
        for (auto& ex : m_exports) {
            if (ex.autoImported) continue;
            if (!meshImporter.canImport(ex.sourcePath)) continue;
            AssetGuid guid = meshImporter.import(db, ex.sourcePath);
            if (!guid.isNull()) {
                ex.autoImported = true;
                ex.importedGuid = guid;
                ++imported;
                NF_LOG_INFO("BlenderBridge", "Auto-imported: " + ex.sourcePath);
            }
        }
        return imported;
    }

    /// Poll: scan + auto-import in one call (convenience for editor tick).
    size_t poll(AssetDatabase& db, MeshImporter& meshImporter) {
        scanExports();
        if (!m_autoImport) return 0;
        return importPending(db, meshImporter);
    }

    [[nodiscard]] const std::vector<BlenderExportEntry>& exports() const { return m_exports; }
    [[nodiscard]] size_t exportCount() const { return m_exports.size(); }

    [[nodiscard]] size_t importedCount() const {
        size_t n = 0;
        for (auto& e : m_exports) if (e.autoImported) ++n;
        return n;
    }

    [[nodiscard]] size_t pendingCount() const {
        return m_exports.size() - importedCount();
    }

    void clearHistory() { m_exports.clear(); }

private:
    std::string m_exportDir;
    bool m_autoImport = true;
    std::vector<BlenderExportEntry> m_exports;
};

// ── Editor application ───────────────────────────────────────────


} // namespace NF
