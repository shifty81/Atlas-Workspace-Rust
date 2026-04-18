#pragma once
// NF::Editor — Static mesh editor and viewer
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

enum class MeshShadingMode : uint8_t {
    Flat, Smooth, Wireframe, Normals, UV
};

inline const char* meshShadingModeName(MeshShadingMode m) {
    switch (m) {
        case MeshShadingMode::Flat:      return "Flat";
        case MeshShadingMode::Smooth:    return "Smooth";
        case MeshShadingMode::Wireframe: return "Wireframe";
        case MeshShadingMode::Normals:   return "Normals";
        case MeshShadingMode::UV:        return "UV";
    }
    return "Unknown";
}

enum class MeshColliderShape : uint8_t {
    None, Box, Sphere, Capsule, ConvexHull, TriangleMesh
};

inline const char* meshColliderShapeName(MeshColliderShape s) {
    switch (s) {
        case MeshColliderShape::None:         return "None";
        case MeshColliderShape::Box:          return "Box";
        case MeshColliderShape::Sphere:       return "Sphere";
        case MeshColliderShape::Capsule:      return "Capsule";
        case MeshColliderShape::ConvexHull:   return "ConvexHull";
        case MeshColliderShape::TriangleMesh: return "TriangleMesh";
    }
    return "Unknown";
}

enum class MeshImportScale : uint8_t {
    CM, DM, M, Km, Inches, Feet
};

inline const char* meshImportScaleName(MeshImportScale s) {
    switch (s) {
        case MeshImportScale::CM:     return "CM";
        case MeshImportScale::DM:     return "DM";
        case MeshImportScale::M:      return "M";
        case MeshImportScale::Km:     return "Km";
        case MeshImportScale::Inches: return "Inches";
        case MeshImportScale::Feet:   return "Feet";
    }
    return "Unknown";
}

class StaticMeshAsset {
public:
    explicit StaticMeshAsset(const std::string& name)
        : m_name(name) {}

    void setVertexCount(uint32_t n)         { m_vertexCount   = n; }
    void setTriangleCount(uint32_t n)       { m_triangleCount = n; }
    void setSubmeshCount(uint32_t n)        { m_submeshCount  = n; }
    void setShadingMode(MeshShadingMode m)  { m_shadingMode   = m; }
    void setColliderShape(MeshColliderShape s) { m_colliderShape = s; }
    void setImportScale(MeshImportScale s)  { m_importScale   = s; }
    void setCastsShadow(bool v)             { m_castsShadow   = v; }
    void setReceivesShadow(bool v)          { m_receivesShadow = v; }
    void setDirty(bool v)                   { m_dirty         = v; }

    [[nodiscard]] const std::string&  name()            const { return m_name;           }
    [[nodiscard]] uint32_t            vertexCount()     const { return m_vertexCount;    }
    [[nodiscard]] uint32_t            triangleCount()   const { return m_triangleCount;  }
    [[nodiscard]] uint32_t            submeshCount()    const { return m_submeshCount;   }
    [[nodiscard]] MeshShadingMode     shadingMode()     const { return m_shadingMode;    }
    [[nodiscard]] MeshColliderShape   colliderShape()   const { return m_colliderShape;  }
    [[nodiscard]] MeshImportScale     importScale()     const { return m_importScale;    }
    [[nodiscard]] bool                castsShadow()     const { return m_castsShadow;    }
    [[nodiscard]] bool                receivesShadow()  const { return m_receivesShadow; }
    [[nodiscard]] bool                isDirty()         const { return m_dirty;          }

    [[nodiscard]] bool hasCollider()   const { return m_colliderShape != MeshColliderShape::None; }

private:
    std::string       m_name;
    uint32_t          m_vertexCount   = 0;
    uint32_t          m_triangleCount = 0;
    uint32_t          m_submeshCount  = 1;
    MeshShadingMode   m_shadingMode   = MeshShadingMode::Smooth;
    MeshColliderShape m_colliderShape = MeshColliderShape::None;
    MeshImportScale   m_importScale   = MeshImportScale::M;
    bool              m_castsShadow   = true;
    bool              m_receivesShadow = true;
    bool              m_dirty         = false;
};

class StaticMeshEditor {
public:
    static constexpr size_t MAX_MESHES = 256;

    [[nodiscard]] bool addMesh(const StaticMeshAsset& mesh) {
        for (auto& m : m_meshes) if (m.name() == mesh.name()) return false;
        if (m_meshes.size() >= MAX_MESHES) return false;
        m_meshes.push_back(mesh);
        return true;
    }

    [[nodiscard]] bool removeMesh(const std::string& name) {
        for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it) {
            if (it->name() == name) {
                if (m_activeMesh == name) m_activeMesh.clear();
                m_meshes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] StaticMeshAsset* findMesh(const std::string& name) {
        for (auto& m : m_meshes) if (m.name() == name) return &m;
        return nullptr;
    }

    [[nodiscard]] bool setActiveMesh(const std::string& name) {
        for (auto& m : m_meshes) if (m.name() == name) { m_activeMesh = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeMesh() const { return m_activeMesh; }
    [[nodiscard]] size_t meshCount()  const { return m_meshes.size(); }
    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& m : m_meshes) if (m.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t colliderCount() const {
        size_t c = 0; for (auto& m : m_meshes) if (m.hasCollider()) ++c; return c;
    }
    [[nodiscard]] size_t countByShading(MeshShadingMode s) const {
        size_t c = 0; for (auto& m : m_meshes) if (m.shadingMode() == s) ++c; return c;
    }

private:
    std::vector<StaticMeshAsset> m_meshes;
    std::string                  m_activeMesh;
};

} // namespace NF
