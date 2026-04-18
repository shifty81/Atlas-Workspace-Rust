// NF::Pipeline — S2 BlenderGen Bridge
//
// Implements BlenderBridge: the engine-side component that processes
// assets exported from Blender via the pipeline, validates files,
// detects asset types from metadata, and registers them in the Manifest.

#include "NF/Pipeline/Pipeline.h"

#include <chrono>
#include <filesystem>

namespace NF {

namespace {

static int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch()).count();
}

} // anonymous namespace

// ── BlenderBridge ─────────────────────────────────────────────────────────

BlenderBridge::BlenderBridge(Manifest& manifest)
    : m_manifest(manifest) {}

std::string BlenderBridge::detectAssetType(const ChangeEvent& event) {
    // Parse metadata for "type=mesh", "type=rig", or "type=clip".
    const auto& meta = event.metadata;
    const std::string prefix = "type=";
    auto pos = meta.find(prefix);
    if (pos != std::string::npos) {
        pos += prefix.size();
        auto end = meta.find_first_of(";, \t\n", pos);
        if (end == std::string::npos) end = meta.size();
        return meta.substr(pos, end - pos);
    }

    // Fall back to event type heuristics.
    if (event.eventType == ChangeEventType::AnimationExported)
        return "animation";
    if (event.eventType == ChangeEventType::AssetImported) {
        // Infer from file extension.
        auto ext = std::filesystem::path(event.path).extension().string();
        if (ext == ".glb" || ext == ".gltf" || ext == ".obj" || ext == ".fbx")
            return "mesh";
    }
    return "unknown";
}

bool BlenderBridge::validateAssetFile(const std::filesystem::path& fullPath) {
    std::error_code ec;
    if (!std::filesystem::exists(fullPath, ec)) return false;
    if (!std::filesystem::is_regular_file(fullPath, ec)) return false;
    // Ensure the file is non-empty.
    auto size = std::filesystem::file_size(fullPath, ec);
    return !ec && size > 0;
}

std::filesystem::path BlenderBridge::resolveAssetPath(
    const ChangeEvent& event, const PipelineDirectories& dirs) {
    namespace fs = std::filesystem;

    // The event path may be:
    //  1. Absolute path → use directly
    //  2. Relative to pipeline dir (e.g. "pipeline/assets/ship.glb")
    //  3. Relative to workspace root
    fs::path p(event.path);
    if (p.is_absolute()) return p;

    // Try relative to the .atlas directory's parent (workspace root).
    auto candidate = dirs.root / p;
    if (fs::exists(candidate)) return candidate;

    // Try relative to .atlas/pipeline.
    candidate = dirs.pipeline / p;
    if (fs::exists(candidate)) return candidate;

    // Try the path directly under the pipeline directory hierarchy.
    // e.g. "pipeline/assets/foo.glb" → strip "pipeline/" prefix
    const std::string pathStr = event.path;
    if (pathStr.starts_with("pipeline/")) {
        candidate = dirs.dotAtlas / p;
        if (fs::exists(candidate)) return candidate;
    }

    // Return the workspace-root-relative path even if not found;
    // the caller will detect the missing file via validateAssetFile.
    return dirs.root / p;
}

AssetImportResult BlenderBridge::importAsset(const ChangeEvent& event,
                                              const PipelineDirectories& dirs) {
    AssetImportResult result;
    result.sourcePath      = event.path;
    result.importTimestamp  = nowMs();

    // Step 1: Detect asset type.
    result.assetType = detectAssetType(event);

    // Step 2: Resolve and validate the asset file on disk.
    auto fullPath = resolveAssetPath(event, dirs);
    if (!validateAssetFile(fullPath)) {
        result.status       = AssetImportStatus::Failed;
        result.errorMessage = "Asset file not found or empty: " + fullPath.string();
        ++m_failedCount;
        m_history.push_back(result);
        m_pathIndex[result.sourcePath] = m_history.size() - 1;
        return result;
    }
    result.status = AssetImportStatus::Validated;

    // Step 3: Register in the Manifest.
    AssetRecord record;
    record.type       = result.assetType;
    record.path       = event.path;
    record.importDate = result.importTimestamp;
    // No checksum computation yet — deferred to a future milestone.

    result.guid   = m_manifest.registerAsset(std::move(record));
    result.status = AssetImportStatus::Registered;
    ++m_importCount;

    // Step 4: Track in history.
    m_history.push_back(result);
    m_pathIndex[result.sourcePath] = m_history.size() - 1;

    return result;
}

bool BlenderBridge::isImported(const std::string& path) const {
    auto it = m_pathIndex.find(path);
    if (it == m_pathIndex.end()) return false;
    return m_history[it->second].status == AssetImportStatus::Registered;
}

std::string BlenderBridge::guidForPath(const std::string& path) const {
    auto it = m_pathIndex.find(path);
    if (it == m_pathIndex.end()) return {};
    return m_history[it->second].guid;
}

} // namespace NF
