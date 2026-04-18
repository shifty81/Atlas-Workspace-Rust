#pragma once
// NF::Editor — Shader variant editor
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

enum class ShaderVariantStage : uint8_t {
    Vertex, Fragment, Geometry, TessControl, TessEval, Compute
};

inline const char* shaderVariantStageName(ShaderVariantStage s) {
    switch (s) {
        case ShaderVariantStage::Vertex:      return "Vertex";
        case ShaderVariantStage::Fragment:    return "Fragment";
        case ShaderVariantStage::Geometry:    return "Geometry";
        case ShaderVariantStage::TessControl: return "TessControl";
        case ShaderVariantStage::TessEval:    return "TessEval";
        case ShaderVariantStage::Compute:     return "Compute";
    }
    return "Unknown";
}

enum class ShaderVariantCompileStatus : uint8_t {
    NotCompiled, Compiling, Ok, Error, Warning
};

inline const char* shaderVariantCompileStatusName(ShaderVariantCompileStatus s) {
    switch (s) {
        case ShaderVariantCompileStatus::NotCompiled: return "NotCompiled";
        case ShaderVariantCompileStatus::Compiling:   return "Compiling";
        case ShaderVariantCompileStatus::Ok:          return "Ok";
        case ShaderVariantCompileStatus::Error:       return "Error";
        case ShaderVariantCompileStatus::Warning:     return "Warning";
    }
    return "Unknown";
}

enum class ShaderTargetAPI : uint8_t {
    Vulkan, DX12, Metal, OpenGL, WebGPU
};

inline const char* shaderTargetAPIName(ShaderTargetAPI a) {
    switch (a) {
        case ShaderTargetAPI::Vulkan:  return "Vulkan";
        case ShaderTargetAPI::DX12:    return "DX12";
        case ShaderTargetAPI::Metal:   return "Metal";
        case ShaderTargetAPI::OpenGL:  return "OpenGL";
        case ShaderTargetAPI::WebGPU:  return "WebGPU";
    }
    return "Unknown";
}

class ShaderVariant {
public:
    explicit ShaderVariant(uint32_t id, ShaderVariantStage stage, ShaderTargetAPI api)
        : m_id(id), m_stage(stage), m_api(api) {}

    void setStatus(ShaderVariantCompileStatus s) { m_status  = s; }
    void setDefines(const std::string& d)        { m_defines = d; }
    void setEnabled(bool v)                      { m_enabled = v; }
    void setEntryPoint(const std::string& ep)    { m_entryPoint = ep; }

    [[nodiscard]] uint32_t                   id()         const { return m_id;         }
    [[nodiscard]] ShaderVariantStage         stage()      const { return m_stage;      }
    [[nodiscard]] ShaderTargetAPI            api()        const { return m_api;        }
    [[nodiscard]] ShaderVariantCompileStatus status()     const { return m_status;     }
    [[nodiscard]] const std::string&         defines()    const { return m_defines;    }
    [[nodiscard]] const std::string&         entryPoint() const { return m_entryPoint; }
    [[nodiscard]] bool                       isEnabled()  const { return m_enabled;    }
    [[nodiscard]] bool                       isCompiled() const {
        return m_status == ShaderVariantCompileStatus::Ok;
    }

private:
    uint32_t                    m_id;
    ShaderVariantStage          m_stage;
    ShaderTargetAPI             m_api;
    ShaderVariantCompileStatus  m_status     = ShaderVariantCompileStatus::NotCompiled;
    std::string                 m_defines;
    std::string                 m_entryPoint = "main";
    bool                        m_enabled    = true;
};

class ShaderVariantEditor {
public:
    [[nodiscard]] bool addVariant(const ShaderVariant& variant) {
        for (auto& v : m_variants) if (v.id() == variant.id()) return false;
        m_variants.push_back(variant);
        return true;
    }

    [[nodiscard]] bool removeVariant(uint32_t id) {
        for (auto it = m_variants.begin(); it != m_variants.end(); ++it) {
            if (it->id() == id) { m_variants.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ShaderVariant* findVariant(uint32_t id) {
        for (auto& v : m_variants) if (v.id() == id) return &v;
        return nullptr;
    }

    [[nodiscard]] size_t variantCount()  const { return m_variants.size(); }
    [[nodiscard]] size_t enabledCount()  const {
        size_t c = 0; for (auto& v : m_variants) if (v.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t compiledCount() const {
        size_t c = 0; for (auto& v : m_variants) if (v.isCompiled()) ++c; return c;
    }
    [[nodiscard]] size_t countByStage(ShaderVariantStage s) const {
        size_t c = 0; for (auto& v : m_variants) if (v.stage() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByAPI(ShaderTargetAPI a) const {
        size_t c = 0; for (auto& v : m_variants) if (v.api() == a) ++c; return c;
    }
    [[nodiscard]] size_t countByStatus(ShaderVariantCompileStatus s) const {
        size_t c = 0; for (auto& v : m_variants) if (v.status() == s) ++c; return c;
    }

private:
    std::vector<ShaderVariant> m_variants;
};

} // namespace NF
