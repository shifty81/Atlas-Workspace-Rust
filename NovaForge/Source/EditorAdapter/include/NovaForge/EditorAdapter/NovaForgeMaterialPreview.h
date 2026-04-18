#pragma once
// NovaForge::NovaForgeMaterialPreview — material preview scene for MaterialEditorTool.
//
// Implements NF::IViewportSceneProvider so MaterialEditorTool can render a
// real-time material preview on standard preview meshes (sphere, cube, plane)
// without requiring a full scene or game boot.
//
// Material parameters are stored as a flat string→string map and updated live;
// each update marks the preview dirty so the render loop knows to refresh.
//
// Phase D.4 — Material Preview Viewport

#include "NF/Workspace/IViewportSceneProvider.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"
#include <cstdio>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace NovaForge {

// ── PreviewMeshType ───────────────────────────────────────────────────────────

enum class PreviewMeshType : uint8_t {
    Sphere,
    Cube,
    Plane,
};

inline const char* previewMeshTypeName(PreviewMeshType t) {
    switch (t) {
        case PreviewMeshType::Sphere: return "Sphere";
        case PreviewMeshType::Cube:   return "Cube";
        case PreviewMeshType::Plane:  return "Plane";
    }
    return "Unknown";
}

inline const char* previewMeshTypeTag(PreviewMeshType t) {
    switch (t) {
        case PreviewMeshType::Sphere: return "mesh/__preview_sphere";
        case PreviewMeshType::Cube:   return "mesh/__preview_cube";
        case PreviewMeshType::Plane:  return "mesh/__preview_plane";
    }
    return "mesh/__preview_sphere";
}

// ── MaterialParameterType ─────────────────────────────────────────────────────

enum class MaterialParameterType : uint8_t {
    Float,
    Vec4,  ///< RGBA color or arbitrary 4-component value
    Texture,
    Bool,
};

inline const char* materialParameterTypeName(MaterialParameterType t) {
    switch (t) {
        case MaterialParameterType::Float:   return "Float";
        case MaterialParameterType::Vec4:    return "Vec4";
        case MaterialParameterType::Texture: return "Texture";
        case MaterialParameterType::Bool:    return "Bool";
    }
    return "Unknown";
}

// ── MaterialParameter ─────────────────────────────────────────────────────────

struct MaterialParameter {
    std::string           name;
    MaterialParameterType type  = MaterialParameterType::Float;
    std::string           value;   ///< string-encoded value (float, "r,g,b,a", path, "true"/"false")
    std::string           defaultValue;
};

// ── MaterialPreviewDescriptor ─────────────────────────────────────────────────

struct MaterialPreviewDescriptor {
    std::string  materialPath;
    std::string  shaderTag;
    PreviewMeshType previewMesh = PreviewMeshType::Sphere;
    std::vector<MaterialParameter> parameters;
};

// ── NovaForgeMaterialPreview ──────────────────────────────────────────────────

class NovaForgeMaterialPreview : public NF::IViewportSceneProvider {
public:
    NovaForgeMaterialPreview()  = default;
    ~NovaForgeMaterialPreview() override = default;

    // ── IViewportSceneProvider ────────────────────────────────────────────

    NF::ViewportSceneState provideScene(NF::ViewportHandle       /*handle*/,
                                        const NF::ViewportSlot&  /*slot*/) override {
        NF::ViewportSceneState st;
        st.hasContent  = !m_descriptor.materialPath.empty();
        st.entityCount = m_world.entityCount();
        st.clearColor  = 0x222222FFu;
        return st;
    }

    // ── Material binding ──────────────────────────────────────────────────

    void bindMaterial(const MaterialPreviewDescriptor& descriptor) {
        m_descriptor  = descriptor;
        m_lastApplied = descriptor;
        m_dirty       = false;
        rebuildPreview();
    }

    void clearMaterial() {
        m_descriptor  = {};
        m_lastApplied = {};
        m_dirty       = false;
        m_world.clearEntities();
        m_previewEntity = kInvalidEntityId;
    }

    [[nodiscard]] bool hasMaterial() const { return !m_descriptor.materialPath.empty(); }

    [[nodiscard]] const MaterialPreviewDescriptor& descriptor() const { return m_descriptor; }

    // ── Preview mesh ──────────────────────────────────────────────────────

    bool setPreviewMesh(PreviewMeshType mesh) {
        if (!hasMaterial()) return false;
        m_descriptor.previewMesh = mesh;
        m_world.setMesh(m_previewEntity, previewMeshTypeTag(mesh));
        m_dirty = true;
        return true;
    }

    [[nodiscard]] PreviewMeshType previewMesh() const { return m_descriptor.previewMesh; }

    // ── Shader tag ────────────────────────────────────────────────────────

    bool setShaderTag(const std::string& tag) {
        if (!hasMaterial()) return false;
        m_descriptor.shaderTag = tag;
        m_dirty = true;
        return true;
    }

    [[nodiscard]] const std::string& shaderTag() const { return m_descriptor.shaderTag; }

    // ── Material parameters ───────────────────────────────────────────────

    /// Add a new parameter (or replace if name exists). Marks dirty.
    bool setParameter(const std::string& name, const std::string& value,
                      MaterialParameterType type = MaterialParameterType::Float) {
        if (!hasMaterial() || name.empty()) return false;
        for (auto& p : m_descriptor.parameters) {
            if (p.name == name) { p.value = value; m_dirty = true; return true; }
        }
        MaterialParameter p;
        p.name         = name;
        p.type         = type;
        p.value        = value;
        p.defaultValue = value;
        m_descriptor.parameters.push_back(std::move(p));
        m_dirty = true;
        return true;
    }

    bool removeParameter(const std::string& name) {
        if (!hasMaterial()) return false;
        auto& v = m_descriptor.parameters;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->name == name) { v.erase(it); m_dirty = true; return true; }
        }
        return false;
    }

    bool resetParameterToDefault(const std::string& name) {
        if (!hasMaterial()) return false;
        for (auto& p : m_descriptor.parameters) {
            if (p.name == name) { p.value = p.defaultValue; m_dirty = true; return true; }
        }
        return false;
    }

    bool resetAllParametersToDefault() {
        if (!hasMaterial()) return false;
        for (auto& p : m_descriptor.parameters) p.value = p.defaultValue;
        m_dirty = true;
        return true;
    }

    [[nodiscard]] std::string getParameter(const std::string& name,
                                           const std::string& fallback = "") const {
        for (const auto& p : m_descriptor.parameters)
            if (p.name == name) return p.value;
        return fallback;
    }

    [[nodiscard]] const std::vector<MaterialParameter>& parameters() const {
        return m_descriptor.parameters;
    }

    [[nodiscard]] uint32_t parameterCount() const {
        return static_cast<uint32_t>(m_descriptor.parameters.size());
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty()                  { m_dirty = false; }

    // ── Save/revert ───────────────────────────────────────────────────────

    bool apply() {
        if (!hasMaterial()) return false;
        m_lastApplied = m_descriptor;
        m_dirty       = false;
        return true;
    }

    bool revert() {
        if (!hasMaterial()) return false;
        m_descriptor = m_lastApplied;
        m_dirty      = false;
        rebuildPreview();
        return true;
    }

    // ── Inspector data ────────────────────────────────────────────────────

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> properties() const {
        std::vector<std::pair<std::string, std::string>> props;
        props.push_back({"materialPath", m_descriptor.materialPath});
        props.push_back({"shaderTag",    m_descriptor.shaderTag});
        props.push_back({"previewMesh",  previewMeshTypeName(m_descriptor.previewMesh)});
        for (const auto& p : m_descriptor.parameters)
            props.push_back({std::string("param.") + p.name, p.value});
        return props;
    }

    // ── Preview world access ──────────────────────────────────────────────

    [[nodiscard]] const NovaForgePreviewWorld& previewWorld() const { return m_world; }

private:
    MaterialPreviewDescriptor m_descriptor;
    MaterialPreviewDescriptor m_lastApplied;
    NovaForgePreviewWorld     m_world;
    EntityId                  m_previewEntity = kInvalidEntityId;
    bool                      m_dirty         = false;

    void rebuildPreview() {
        m_world.clearEntities();
        if (!m_descriptor.materialPath.empty()) {
            m_previewEntity = m_world.createEntity("PreviewMesh");
            m_world.setMesh(m_previewEntity, previewMeshTypeTag(m_descriptor.previewMesh));
            m_world.setMaterial(m_previewEntity, m_descriptor.materialPath);
            m_world.clearDirty();
        }
    }
};

} // namespace NovaForge
