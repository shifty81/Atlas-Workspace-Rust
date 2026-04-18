#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/procedural/ProceduralMaterialGraph.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <sstream>

namespace atlas::editor {

struct MaterialParameter {
    std::string name;
    std::string type;   // "float", "color", "texture"
    std::string value;
};

class MaterialEditorPanel : public EditorPanel {
public:
    const char* Name() const override { return "Material Editor"; }
    void Draw() override;

    void LoadMaterial(const procedural::MaterialData& material) {
        m_material = material;
        m_loaded = true;
    }

    void Clear() {
        m_material.Clear();
        m_loaded = false;
        m_parameters.clear();
        m_selectedParam = -1;
        m_dirty = false;
    }

    bool HasMaterial() const { return m_loaded && m_material.IsValid(); }
    const procedural::MaterialData& GetMaterial() const { return m_material; }

    void AddParameter(const MaterialParameter& param) {
        m_parameters.push_back(param);
        m_dirty = true;
    }

    void RemoveParameter(const std::string& name) {
        for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it) {
            if (it->name == name) {
                m_parameters.erase(it);
                m_dirty = true;
                return;
            }
        }
    }

    const MaterialParameter* GetParameter(const std::string& name) const {
        for (const auto& p : m_parameters) {
            if (p.name == name) return &p;
        }
        return nullptr;
    }

    void SetParameter(const std::string& name, const std::string& value) {
        for (auto& p : m_parameters) {
            if (p.name == name) {
                p.value = value;
                m_dirty = true;
                return;
            }
        }
    }

    const std::vector<MaterialParameter>& Parameters() const { return m_parameters; }

    void SelectParameter(int index) {
        if (index >= 0 && static_cast<size_t>(index) < m_parameters.size()) {
            m_selectedParam = index;
        } else {
            m_selectedParam = -1;
        }
    }
    int SelectedParameter() const { return m_selectedParam; }

    bool IsDirty() const { return m_dirty; }
    void MarkClean() { m_dirty = false; }

    void SetPreviewMode(bool enabled) { m_previewMode = enabled; }
    bool PreviewMode() const { return m_previewMode; }

    std::string Summary() const {
        if (!m_loaded) return "No material loaded";
        std::ostringstream oss;
        oss << "Material " << m_material.width << "x" << m_material.height
            << ", " << m_parameters.size() << " params";
        if (m_dirty) oss << " (modified)";
        return oss.str();
    }

    void SetShaderPath(const std::string& path) { m_shaderPath = path; }
    std::string ShaderPath() const { return m_shaderPath; }

    std::vector<std::string> ParameterNames() const {
        std::vector<std::string> names;
        for (const auto& p : m_parameters) {
            names.push_back(p.name);
        }
        return names;
    }

    size_t ParameterCount() const { return m_parameters.size(); }

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    procedural::MaterialData m_material;
    bool m_loaded = false;
    std::vector<MaterialParameter> m_parameters;
    int m_selectedParam = -1;
    bool m_dirty = false;
    bool m_previewMode = false;
    std::string m_shaderPath;
    atlas::ui::UIDrawList m_drawList;
};

} // namespace atlas::editor
