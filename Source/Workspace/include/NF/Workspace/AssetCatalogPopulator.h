#pragma once
// NF::Workspace — AssetCatalogPopulator: scan content roots and populate the catalog.
//
// This utility scans the filesystem directories declared by the project adapter's
// `contentRoots()` and adds discovered files to the AssetCatalog as descriptors
// with `AssetImportState::Unknown`.
//
// It is called by WorkspaceShell during `loadProject()` after the adapter is
// initialized.
//
// The populator classifies files by extension into AssetTypeTag categories.
// Files that do not match any known extension are tagged as Custom.
//
// Thread-safety: single-threaded (workspace main thread only).

#include "NF/Workspace/AssetCatalog.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace NF {

// ── Extension → AssetTypeTag classification ─────────────────────

struct ExtensionMapping {
    std::string_view extension;
    AssetTypeTag     tag;
};

// Built-in extension mappings.  Compared case-insensitively.
inline AssetTypeTag classifyExtension(std::string_view ext) {
    // Normalize to lowercase for comparison
    std::string lower(ext);
    for (auto& c : lower)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    // Textures
    if (lower == ".png"  || lower == ".jpg"  || lower == ".jpeg" ||
        lower == ".bmp"  || lower == ".tga"  || lower == ".dds"  ||
        lower == ".hdr"  || lower == ".exr"  || lower == ".tiff" ||
        lower == ".webp" || lower == ".psd")
        return AssetTypeTag::Texture;

    // Meshes
    if (lower == ".fbx"  || lower == ".obj"  || lower == ".gltf" ||
        lower == ".glb"  || lower == ".dae"  || lower == ".blend" ||
        lower == ".3ds"  || lower == ".ply"  || lower == ".stl")
        return AssetTypeTag::Mesh;

    // Audio
    if (lower == ".wav"  || lower == ".mp3"  || lower == ".ogg"  ||
        lower == ".flac" || lower == ".aiff" || lower == ".wma")
        return AssetTypeTag::Audio;

    // Scripts
    if (lower == ".lua"  || lower == ".py"   || lower == ".cs"   ||
        lower == ".js"   || lower == ".ts")
        return AssetTypeTag::Script;

    // Shaders
    if (lower == ".hlsl" || lower == ".glsl" || lower == ".vert" ||
        lower == ".frag" || lower == ".comp" || lower == ".metal" ||
        lower == ".cso"  || lower == ".spv")
        return AssetTypeTag::Shader;

    // Scenes
    if (lower == ".scene" || lower == ".level" || lower == ".map")
        return AssetTypeTag::Scene;

    // Fonts
    if (lower == ".ttf"  || lower == ".otf"  || lower == ".woff" ||
        lower == ".woff2")
        return AssetTypeTag::Font;

    // Videos
    if (lower == ".mp4"  || lower == ".avi"  || lower == ".mov"  ||
        lower == ".webm" || lower == ".mkv")
        return AssetTypeTag::Video;

    // Archives
    if (lower == ".zip"  || lower == ".7z"   || lower == ".tar"  ||
        lower == ".gz"   || lower == ".rar")
        return AssetTypeTag::Archive;

    // Materials
    if (lower == ".mat"  || lower == ".material")
        return AssetTypeTag::Material;

    // Animations
    if (lower == ".anim" || lower == ".animation")
        return AssetTypeTag::Animation;

    // Prefabs
    if (lower == ".prefab")
        return AssetTypeTag::Prefab;

    // Atlas project manifest files only (.atlas, .project)
    if (lower == ".atlas" || lower == ".project")
        return AssetTypeTag::Project;

    // Game data / config files — JSON, YAML, TOML, CSV, XML, INI
    if (lower == ".json" || lower == ".yaml" || lower == ".yml"  ||
        lower == ".toml" || lower == ".csv"  || lower == ".xml"  ||
        lower == ".ini"  || lower == ".cfg"  || lower == ".conf")
        return AssetTypeTag::Data;

    return AssetTypeTag::Custom;
}

// ── Populate result ──────────────────────────────────────────────

struct PopulateResult {
    size_t filesScanned = 0;      // total files found on disk
    size_t assetsAdded  = 0;      // successfully added to catalog
    size_t duplicates   = 0;      // skipped (catalog path already present)
    size_t errors       = 0;      // failed to add (catalog full, invalid, etc.)
    std::vector<std::string> errorPaths;  // paths that could not be added

    [[nodiscard]] bool success() const { return errors == 0; }
};

// ── Build catalog path ──────────────────────────────────────────

// Convert an absolute source path into a catalog-relative path.
// Given rootDir="/project/Content" and filePath="/project/Content/Textures/rock.png",
// the result is "Textures/rock.png".
inline std::string buildCatalogPath(const std::string& rootDir,
                                    const std::string& filePath) {
    // Strip the root directory prefix
    if (filePath.size() > rootDir.size() + 1 &&
        filePath.compare(0, rootDir.size(), rootDir) == 0) {
        size_t start = rootDir.size();
        if (start < filePath.size() && (filePath[start] == '/' || filePath[start] == '\\'))
            ++start;
        return filePath.substr(start);
    }
    // Fallback: use the full path
    return filePath;
}

// ── Extract display name ────────────────────────────────────────

inline std::string extractDisplayName(const std::string& catalogPath) {
    auto sep = catalogPath.find_last_of("/\\");
    std::string filename = (sep != std::string::npos) ? catalogPath.substr(sep + 1) : catalogPath;
    auto dot = filename.rfind('.');
    return (dot != std::string::npos) ? filename.substr(0, dot) : filename;
}

// ── AssetCatalogPopulator ────────────────────────────────────────
// Stateless utility — all state is in the AssetCatalog itself.

class AssetCatalogPopulator {
public:
    // Populate the catalog from a list of content root directories.
    // Each root is scanned for files; subdirectories are scanned recursively
    // if `recursive` is true (default).
    //
    // This is a simulated scan — since we run in a sandboxed environment without
    // real filesystem access, this method accepts explicit file lists via
    // addFiles() and then registers them.  In production, it would use
    // std::filesystem::recursive_directory_iterator.

    // Add explicit file paths from a single content root.
    void addFiles(const std::string& rootDir, const std::vector<std::string>& filePaths) {
        for (const auto& fp : filePaths) {
            m_pending.push_back({rootDir, fp});
        }
    }

    // Clear pending files
    void clear() { m_pending.clear(); }

    // Number of pending files
    [[nodiscard]] size_t pendingCount() const { return m_pending.size(); }

    // Commit all pending files to the catalog.
    PopulateResult populate(AssetCatalog& catalog) {
        PopulateResult result;
        result.filesScanned = m_pending.size();

        for (const auto& [rootDir, filePath] : m_pending) {
            std::string catPath = buildCatalogPath(rootDir, filePath);
            std::string ext = "";
            auto dot = filePath.rfind('.');
            if (dot != std::string::npos) ext = filePath.substr(dot);

            AssetDescriptor desc;
            desc.sourcePath   = filePath;
            desc.catalogPath  = catPath;
            desc.displayName  = extractDisplayName(catPath);
            desc.typeTag      = classifyExtension(ext);
            desc.importState  = AssetImportState::Unknown;

            AssetId id = catalog.add(std::move(desc));
            if (id != INVALID_ASSET_ID) {
                ++result.assetsAdded;
            } else {
                // Could be duplicate or catalog full
                if (catalog.findByPath(catPath) != nullptr) {
                    ++result.duplicates;
                } else {
                    ++result.errors;
                    result.errorPaths.push_back(filePath);
                }
            }
        }

        m_pending.clear();
        return result;
    }

    // Populate the catalog by recursively scanning a directory on disk.
    // Returns a PopulateResult describing what was found and registered.
    PopulateResult populateFromDirectory(AssetCatalog& catalog,
                                         const std::string& rootDir,
                                         bool recursive = true) {
        namespace fs = std::filesystem;
        PopulateResult result;

        fs::path root(rootDir);
        std::error_code ec;

        auto processEntry = [&](const fs::directory_entry& entry) {
            if (!entry.is_regular_file(ec)) return;

            // Skip placeholder / VCS-only files (e.g. .gitkeep, .gitignore, .DS_Store).
            std::string filename = entry.path().filename().string();
            if (!filename.empty() && filename[0] == '.') return;

            std::string filePath = entry.path().string();
            ++result.filesScanned;

            std::string catPath = buildCatalogPath(rootDir, filePath);
            std::string ext;
            auto dot = filePath.rfind('.');
            if (dot != std::string::npos) ext = filePath.substr(dot);

            std::string guid = generateGuid(catPath);

            AssetDescriptor desc;
            desc.sourcePath   = filePath;
            desc.catalogPath  = catPath;
            desc.displayName  = extractDisplayName(catPath);
            desc.typeTag      = classifyExtension(ext);
            desc.importState  = AssetImportState::Unknown;
            desc.metadata.set("guid", guid);
            desc.metadata.set("rootDir", rootDir);

            auto sz = entry.file_size(ec);
            if (!ec) desc.sourceSizeBytes = sz;

            AssetId id = catalog.add(std::move(desc));
            if (id != INVALID_ASSET_ID) {
                ++result.assetsAdded;
            } else {
                if (catalog.findByPath(catPath) != nullptr) {
                    ++result.duplicates;
                } else {
                    ++result.errors;
                    result.errorPaths.push_back(filePath);
                }
            }
        };

        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(root, ec)) {
                if (ec) { ++result.errors; break; }
                processEntry(entry);
            }
        } else {
            for (const auto& entry : fs::directory_iterator(root, ec)) {
                if (ec) { ++result.errors; break; }
                processEntry(entry);
            }
        }

        return result;
    }

private:
    struct PendingFile {
        std::string rootDir;
        std::string filePath;
    };
    std::vector<PendingFile> m_pending;

    // Generate a deterministic pseudo-GUID from a catalog path string.
    // Not cryptographic — just unique enough for editor purposes.
    static std::string generateGuid(const std::string& catalogPath) {
        uint32_t hash = 2166136261u;
        for (unsigned char c : catalogPath) {
            hash ^= c;
            hash *= 16777619u;
        }
        char buf[9];
        std::snprintf(buf, sizeof(buf), "%08X", hash);
        return std::string(buf);
    }
};

} // namespace NF
