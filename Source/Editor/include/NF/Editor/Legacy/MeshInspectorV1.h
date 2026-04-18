#pragma once
// NF::Editor — Mesh inspector v1: LOD management and mesh statistics inspection
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class MivTopology : uint8_t { TriangleList, TriangleStrip, LineList, PointList };

struct MivMeshStats {
    uint32_t vertexCount   = 0;
    uint32_t indexCount    = 0;
    uint32_t triangleCount = 0;
    uint32_t subMeshCount  = 0;
    float    boundingRadius = 0.f;
    [[nodiscard]] bool isValid() const { return vertexCount > 0; }
};

struct MivLod {
    uint32_t      id            = 0;
    float         screenSizePct = 1.f;
    MivMeshStats  stats;
    [[nodiscard]] bool isValid() const { return id != 0; }
};

using MivInspectCallback = std::function<void(const std::string&)>;

class MeshInspectorV1 {
public:
    bool loadMesh(const std::string& name) {
        if (name.empty()) return false;
        m_meshName = name;
        m_loaded = true;
        if (m_onInspect) m_onInspect(name);
        return true;
    }

    bool unloadMesh() {
        if (!m_loaded) return false;
        m_meshName.clear();
        m_loaded = false;
        m_lods.clear();
        return true;
    }

    bool addLod(const MivLod& lod) {
        if (!lod.isValid()) return false;
        for (const auto& l : m_lods) if (l.id == lod.id) return false;
        m_lods.push_back(lod);
        return true;
    }

    bool removeLod(uint32_t id) {
        for (auto it = m_lods.begin(); it != m_lods.end(); ++it) {
            if (it->id == id) { m_lods.erase(it); return true; }
        }
        return false;
    }

    void setTopology(MivTopology t) { m_topology = t; }
    [[nodiscard]] MivTopology getTopology() const { return m_topology; }

    [[nodiscard]] MivMeshStats getStats() const { return m_stats; }
    void setStats(const MivMeshStats& s) { m_stats = s; }

    [[nodiscard]] size_t           lodCount()  const { return m_lods.size();   }
    [[nodiscard]] const std::string& meshName() const { return m_meshName;     }
    [[nodiscard]] bool             isLoaded()  const { return m_loaded;        }

    void setOnInspect(MivInspectCallback cb) { m_onInspect = std::move(cb); }

private:
    std::vector<MivLod>  m_lods;
    MivMeshStats         m_stats;
    MivTopology          m_topology  = MivTopology::TriangleList;
    std::string          m_meshName;
    bool                 m_loaded    = false;
    MivInspectCallback   m_onInspect;
};

} // namespace NF
