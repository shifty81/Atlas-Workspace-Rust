#pragma once
// NF::Editor — Procedural mesh editor
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

enum class ProceduralMeshPrimitive : uint8_t {
    Plane, Box, Sphere, Cylinder, Cone, Torus, Capsule, Custom
};

inline const char* proceduralMeshPrimitiveName(ProceduralMeshPrimitive p) {
    switch (p) {
        case ProceduralMeshPrimitive::Plane:    return "Plane";
        case ProceduralMeshPrimitive::Box:      return "Box";
        case ProceduralMeshPrimitive::Sphere:   return "Sphere";
        case ProceduralMeshPrimitive::Cylinder: return "Cylinder";
        case ProceduralMeshPrimitive::Cone:     return "Cone";
        case ProceduralMeshPrimitive::Torus:    return "Torus";
        case ProceduralMeshPrimitive::Capsule:  return "Capsule";
        case ProceduralMeshPrimitive::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class ProceduralMeshNormalMode : uint8_t {
    Smooth, Flat, WeightedSmooth
};

inline const char* proceduralMeshNormalModeName(ProceduralMeshNormalMode m) {
    switch (m) {
        case ProceduralMeshNormalMode::Smooth:         return "Smooth";
        case ProceduralMeshNormalMode::Flat:           return "Flat";
        case ProceduralMeshNormalMode::WeightedSmooth: return "WeightedSmooth";
    }
    return "Unknown";
}

enum class ProceduralMeshUVMode : uint8_t {
    Planar, Box, Spherical, Cylindrical, Custom
};

inline const char* proceduralMeshUVModeName(ProceduralMeshUVMode m) {
    switch (m) {
        case ProceduralMeshUVMode::Planar:      return "Planar";
        case ProceduralMeshUVMode::Box:         return "Box";
        case ProceduralMeshUVMode::Spherical:   return "Spherical";
        case ProceduralMeshUVMode::Cylindrical: return "Cylindrical";
        case ProceduralMeshUVMode::Custom:      return "Custom";
    }
    return "Unknown";
}

class ProceduralMeshConfig {
public:
    explicit ProceduralMeshConfig(const std::string& name, ProceduralMeshPrimitive prim)
        : m_name(name), m_primitive(prim) {}

    void setNormalMode(ProceduralMeshNormalMode m){ m_normalMode = m; }
    void setUVMode(ProceduralMeshUVMode m)        { m_uvMode     = m; }
    void setSubdivisions(uint32_t n)              { m_subdivisions = n; }
    void setGenerateTangents(bool v)              { m_genTangents = v; }
    void setGenerateCollision(bool v)             { m_genCollision= v; }
    void setDirty(bool v)                         { m_dirty       = v; }

    [[nodiscard]] const std::string&        name()              const { return m_name;         }
    [[nodiscard]] ProceduralMeshPrimitive   primitive()         const { return m_primitive;    }
    [[nodiscard]] ProceduralMeshNormalMode  normalMode()        const { return m_normalMode;   }
    [[nodiscard]] ProceduralMeshUVMode      uvMode()            const { return m_uvMode;       }
    [[nodiscard]] uint32_t                  subdivisions()      const { return m_subdivisions; }
    [[nodiscard]] bool                      generatesTangents() const { return m_genTangents;  }
    [[nodiscard]] bool                      generatesCollision()const { return m_genCollision; }
    [[nodiscard]] bool                      isDirty()           const { return m_dirty;        }

private:
    std::string                 m_name;
    ProceduralMeshPrimitive     m_primitive    = ProceduralMeshPrimitive::Box;
    ProceduralMeshNormalMode    m_normalMode   = ProceduralMeshNormalMode::Smooth;
    ProceduralMeshUVMode        m_uvMode       = ProceduralMeshUVMode::Box;
    uint32_t                    m_subdivisions = 1;
    bool                        m_genTangents  = true;
    bool                        m_genCollision = false;
    bool                        m_dirty        = false;
};

class ProceduralMeshEditor {
public:
    static constexpr size_t MAX_MESHES = 512;

    [[nodiscard]] bool addMesh(const ProceduralMeshConfig& mesh) {
        for (auto& m : m_meshes) if (m.name() == mesh.name()) return false;
        if (m_meshes.size() >= MAX_MESHES) return false;
        m_meshes.push_back(mesh);
        return true;
    }

    [[nodiscard]] bool removeMesh(const std::string& name) {
        for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it) {
            if (it->name() == name) { m_meshes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ProceduralMeshConfig* findMesh(const std::string& name) {
        for (auto& m : m_meshes) if (m.name() == name) return &m;
        return nullptr;
    }

    [[nodiscard]] size_t meshCount()      const { return m_meshes.size(); }
    [[nodiscard]] size_t dirtyCount()     const {
        size_t c = 0; for (auto& m : m_meshes) if (m.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t collisionCount() const {
        size_t c = 0; for (auto& m : m_meshes) if (m.generatesCollision()) ++c; return c;
    }
    [[nodiscard]] size_t countByPrimitive(ProceduralMeshPrimitive p) const {
        size_t c = 0; for (auto& m : m_meshes) if (m.primitive() == p) ++c; return c;
    }

private:
    std::vector<ProceduralMeshConfig> m_meshes;
};

} // namespace NF
