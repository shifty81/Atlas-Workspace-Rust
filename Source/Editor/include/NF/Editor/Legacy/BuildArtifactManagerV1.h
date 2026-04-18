#pragma once
// NF::Editor — Build artifact manager v1: artifact registration, versioning, and lifecycle authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Bamv1ArtifactType  : uint8_t { Executable, Library, Bundle, Package, Patch, Symbols };
enum class Bamv1ArtifactState : uint8_t { Pending, Building, Ready, Verified, Published, Stale, Failed };
enum class Bamv1Compression   : uint8_t { None, LZ4, ZStd, LZMA, Brotli };

inline const char* bamv1ArtifactTypeName(Bamv1ArtifactType t) {
    switch (t) {
        case Bamv1ArtifactType::Executable: return "Executable";
        case Bamv1ArtifactType::Library:    return "Library";
        case Bamv1ArtifactType::Bundle:     return "Bundle";
        case Bamv1ArtifactType::Package:    return "Package";
        case Bamv1ArtifactType::Patch:      return "Patch";
        case Bamv1ArtifactType::Symbols:    return "Symbols";
    }
    return "Unknown";
}

inline const char* bamv1ArtifactStateName(Bamv1ArtifactState s) {
    switch (s) {
        case Bamv1ArtifactState::Pending:   return "Pending";
        case Bamv1ArtifactState::Building:  return "Building";
        case Bamv1ArtifactState::Ready:     return "Ready";
        case Bamv1ArtifactState::Verified:  return "Verified";
        case Bamv1ArtifactState::Published: return "Published";
        case Bamv1ArtifactState::Stale:     return "Stale";
        case Bamv1ArtifactState::Failed:    return "Failed";
    }
    return "Unknown";
}

inline const char* bamv1CompressionName(Bamv1Compression c) {
    switch (c) {
        case Bamv1Compression::None:   return "None";
        case Bamv1Compression::LZ4:    return "LZ4";
        case Bamv1Compression::ZStd:   return "ZStd";
        case Bamv1Compression::LZMA:   return "LZMA";
        case Bamv1Compression::Brotli: return "Brotli";
    }
    return "Unknown";
}

struct Bamv1Artifact {
    uint64_t             id           = 0;
    std::string          name;
    std::string          version;
    std::string          outputPath;
    Bamv1ArtifactType    artifactType = Bamv1ArtifactType::Executable;
    Bamv1ArtifactState   state        = Bamv1ArtifactState::Pending;
    Bamv1Compression     compression  = Bamv1Compression::ZStd;
    uint64_t             sizeBytes    = 0;
    std::string          checksum;
    bool                 signed_      = false;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty() && !version.empty(); }
    [[nodiscard]] bool isReady()     const { return state == Bamv1ArtifactState::Ready; }
    [[nodiscard]] bool isVerified()  const { return state == Bamv1ArtifactState::Verified; }
    [[nodiscard]] bool isPublished() const { return state == Bamv1ArtifactState::Published; }
    [[nodiscard]] bool hasFailed()   const { return state == Bamv1ArtifactState::Failed; }
};

using Bamv1ChangeCallback = std::function<void(uint64_t)>;

class BuildArtifactManagerV1 {
public:
    static constexpr size_t MAX_ARTIFACTS = 512;

    bool addArtifact(const Bamv1Artifact& artifact) {
        if (!artifact.isValid()) return false;
        for (const auto& a : m_artifacts) if (a.id == artifact.id) return false;
        if (m_artifacts.size() >= MAX_ARTIFACTS) return false;
        m_artifacts.push_back(artifact);
        return true;
    }

    bool removeArtifact(uint64_t id) {
        for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
            if (it->id == id) { m_artifacts.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Bamv1Artifact* findArtifact(uint64_t id) {
        for (auto& a : m_artifacts) if (a.id == id) return &a;
        return nullptr;
    }

    bool setState(uint64_t id, Bamv1ArtifactState state) {
        auto* a = findArtifact(id);
        if (!a) return false;
        a->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setChecksum(uint64_t id, const std::string& checksum) {
        auto* a = findArtifact(id);
        if (!a || checksum.empty()) return false;
        a->checksum = checksum;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setSize(uint64_t id, uint64_t sizeBytes) {
        auto* a = findArtifact(id);
        if (!a) return false;
        a->sizeBytes = sizeBytes;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setSigned(uint64_t id, bool isSigned) {
        auto* a = findArtifact(id);
        if (!a) return false;
        a->signed_ = isSigned;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t artifactCount()   const { return m_artifacts.size(); }
    [[nodiscard]] size_t readyCount()      const {
        size_t c = 0; for (const auto& a : m_artifacts) if (a.isReady())     ++c; return c;
    }
    [[nodiscard]] size_t verifiedCount()   const {
        size_t c = 0; for (const auto& a : m_artifacts) if (a.isVerified())  ++c; return c;
    }
    [[nodiscard]] size_t publishedCount()  const {
        size_t c = 0; for (const auto& a : m_artifacts) if (a.isPublished()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount()     const {
        size_t c = 0; for (const auto& a : m_artifacts) if (a.hasFailed())   ++c; return c;
    }
    [[nodiscard]] size_t countByType(Bamv1ArtifactType type) const {
        size_t c = 0; for (const auto& a : m_artifacts) if (a.artifactType == type) ++c; return c;
    }
    [[nodiscard]] size_t countByCompression(Bamv1Compression comp) const {
        size_t c = 0; for (const auto& a : m_artifacts) if (a.compression == comp) ++c; return c;
    }

    void setOnChange(Bamv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Bamv1Artifact> m_artifacts;
    Bamv1ChangeCallback        m_onChange;
};

} // namespace NF
