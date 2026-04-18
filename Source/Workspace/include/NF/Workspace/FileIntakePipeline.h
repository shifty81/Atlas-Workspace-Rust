#pragma once
// NF::Editor — File intake pipeline: route dropped/dragged files into workspace workflows
#include "NF/Core/Core.h"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Intake Source ─────────────────────────────────────────────────

enum class IntakeSource : uint8_t {
    FileDrop,      // drag-and-drop onto workspace surface
    FileDialog,    // opened via file open dialog
    CLI,           // command-line argument
    URLScheme,     // atlas:// URL handler
    ClipboardPaste,
};

inline const char* intakeSourceName(IntakeSource s) {
    switch (s) {
        case IntakeSource::FileDrop:       return "FileDrop";
        case IntakeSource::FileDialog:     return "FileDialog";
        case IntakeSource::CLI:            return "CLI";
        case IntakeSource::URLScheme:      return "URLScheme";
        case IntakeSource::ClipboardPaste: return "ClipboardPaste";
    }
    return "Unknown";
}

// ── Intake File Type ──────────────────────────────────────────────

enum class IntakeFileType : uint8_t {
    Unknown,
    Texture,    // .png .jpg .tga .exr .hdr
    Mesh,       // .fbx .obj .gltf .glb
    Audio,      // .wav .mp3 .ogg .flac
    Script,     // .lua .py .cs
    Shader,     // .hlsl .glsl .spv
    Scene,      // .atlasscene .json
    Font,       // .ttf .otf
    Video,      // .mp4 .webm
    Archive,    // .zip .tar .7z
    Project,    // .atlasproject
};

inline const char* intakeFileTypeName(IntakeFileType t) {
    switch (t) {
        case IntakeFileType::Unknown: return "Unknown";
        case IntakeFileType::Texture: return "Texture";
        case IntakeFileType::Mesh:    return "Mesh";
        case IntakeFileType::Audio:   return "Audio";
        case IntakeFileType::Script:  return "Script";
        case IntakeFileType::Shader:  return "Shader";
        case IntakeFileType::Scene:   return "Scene";
        case IntakeFileType::Font:    return "Font";
        case IntakeFileType::Video:   return "Video";
        case IntakeFileType::Archive: return "Archive";
        case IntakeFileType::Project: return "Project";
    }
    return "Unknown";
}

[[nodiscard]] inline IntakeFileType detectIntakeFileType(const std::string& ext) {
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
        ext == ".tga" || ext == ".exr" || ext == ".hdr")   return IntakeFileType::Texture;
    if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" ||
        ext == ".glb")                                      return IntakeFileType::Mesh;
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" ||
        ext == ".flac")                                     return IntakeFileType::Audio;
    if (ext == ".lua" || ext == ".py"  || ext == ".cs")    return IntakeFileType::Script;
    if (ext == ".hlsl"|| ext == ".glsl"|| ext == ".spv")   return IntakeFileType::Shader;
    if (ext == ".atlasscene")                               return IntakeFileType::Scene;
    if (ext == ".ttf" || ext == ".otf")                    return IntakeFileType::Font;
    if (ext == ".mp4" || ext == ".webm")                   return IntakeFileType::Video;
    if (ext == ".zip" || ext == ".tar" || ext == ".7z")    return IntakeFileType::Archive;
    if (ext == ".atlasproject")                            return IntakeFileType::Project;
    return IntakeFileType::Unknown;
}

// ── Intake Item ───────────────────────────────────────────────────

struct IntakeItem {
    uint32_t      id          = 0;
    std::string   path;
    IntakeSource  source      = IntakeSource::FileDrop;
    IntakeFileType fileType   = IntakeFileType::Unknown;
    bool          validated   = false;
    bool          rejected    = false;
    std::string   rejectReason;

    [[nodiscard]] bool isValid() const {
        return id != 0 && !path.empty() && !rejected;
    }

    [[nodiscard]] std::string filename() const {
        auto p = std::filesystem::path(path);
        return p.filename().string();
    }
};

// ── File Intake Pipeline ──────────────────────────────────────────

using IntakeHandler = std::function<bool(IntakeItem&)>;

class FileIntakePipeline {
public:
    static constexpr size_t MAX_HANDLERS = 16;
    static constexpr size_t MAX_ITEMS    = 512;

    // Register a validation/routing handler
    bool addHandler(const std::string& name, IntakeHandler fn) {
        if (m_handlers.size() >= MAX_HANDLERS) return false;
        for (const auto& [n, _] : m_handlers) if (n == name) return false;
        m_handlers.push_back({name, std::move(fn)});
        return true;
    }

    bool removeHandler(const std::string& name) {
        for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
            if (it->first == name) { m_handlers.erase(it); return true; }
        }
        return false;
    }

    // Ingest a single file path and run it through the pipeline
    bool ingest(const std::string& path, IntakeSource source) {
        if (m_pending.size() >= MAX_ITEMS) return false;

        IntakeItem item;
        item.id = ++m_nextId;
        item.path = path;
        item.source = source;

        // Detect file type from extension
        auto p = std::filesystem::path(path);
        item.fileType = detectIntakeFileType(p.extension().string());

        // Run through handlers
        for (auto& [name, fn] : m_handlers) {
            if (!fn(item)) {
                item.rejected = true;
                if (item.rejectReason.empty())
                    item.rejectReason = "Rejected by: " + name;
                ++m_rejectedCount;
                return false;
            }
        }

        item.validated = true;
        m_pending.push_back(std::move(item));
        ++m_ingestedCount;
        return true;
    }

    // Ingest multiple paths at once
    size_t ingestBatch(const std::vector<std::string>& paths, IntakeSource source) {
        size_t accepted = 0;
        for (const auto& p : paths) {
            if (ingest(p, source)) ++accepted;
        }
        return accepted;
    }

    [[nodiscard]] const std::vector<IntakeItem>& pendingItems() const { return m_pending; }
    [[nodiscard]] size_t pendingCount()  const { return m_pending.size();  }
    [[nodiscard]] size_t ingestedCount() const { return m_ingestedCount;   }
    [[nodiscard]] size_t rejectedCount() const { return m_rejectedCount;   }
    [[nodiscard]] size_t handlerCount()  const { return m_handlers.size(); }

    void clearPending() { m_pending.clear(); }

    [[nodiscard]] const IntakeItem* findById(uint32_t id) const {
        for (const auto& item : m_pending)
            if (item.id == id) return &item;
        return nullptr;
    }

private:
    std::vector<std::pair<std::string, IntakeHandler>> m_handlers;
    std::vector<IntakeItem> m_pending;
    uint32_t m_nextId        = 0;
    size_t   m_ingestedCount = 0;
    size_t   m_rejectedCount = 0;
};

} // namespace NF
