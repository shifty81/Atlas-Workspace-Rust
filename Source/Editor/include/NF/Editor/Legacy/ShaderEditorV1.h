#pragma once
// NF::Editor — Shader editor v1: multi-stage shader management and compilation
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class SevShaderStage : uint8_t { Vertex, Fragment, Geometry, Compute, TessControl, TessEval };
enum class SevShaderLang  : uint8_t { GLSL, HLSL, MSL, SPIR_V };

struct SevUniform {
    uint32_t    id           = 0;
    std::string name;
    std::string type;
    std::string defaultValue;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct SevShader {
    uint32_t               id       = 0;
    SevShaderStage         stage    = SevShaderStage::Vertex;
    SevShaderLang          lang     = SevShaderLang::GLSL;
    std::string            source;
    std::vector<SevUniform> uniforms;
    bool                   compiled = false;
    [[nodiscard]] bool   isValid()       const { return id != 0; }
    [[nodiscard]] size_t uniformCount()  const { return uniforms.size(); }
};

using SevCompileCallback = std::function<void(uint32_t, bool)>;

class ShaderEditorV1 {
public:
    static constexpr size_t MAX_SHADERS = 64;

    bool addShader(const SevShader& shader) {
        if (!shader.isValid()) return false;
        if (m_shaders.size() >= MAX_SHADERS) return false;
        for (const auto& s : m_shaders) if (s.id == shader.id) return false;
        m_shaders.push_back(shader);
        return true;
    }

    bool removeShader(uint32_t id) {
        for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it) {
            if (it->id == id) { m_shaders.erase(it); return true; }
        }
        return false;
    }

    bool compile(uint32_t id) {
        for (auto& s : m_shaders) {
            if (s.id == id) {
                s.compiled = !s.source.empty();
                if (m_onCompile) m_onCompile(id, s.compiled);
                return s.compiled;
            }
        }
        return false;
    }

    const SevShader* findShader(const std::string& stageName) const {
        for (const auto& s : m_shaders)
            if (s.source.find(stageName) != std::string::npos) return &s;
        return nullptr;
    }

    [[nodiscard]] bool allCompiled() const {
        for (const auto& s : m_shaders) if (!s.compiled) return false;
        return !m_shaders.empty();
    }

    void setOnCompile(SevCompileCallback cb) { m_onCompile = std::move(cb); }

    [[nodiscard]] size_t shaderCount() const { return m_shaders.size(); }

    SevShader* findById(uint32_t id) {
        for (auto& s : m_shaders) if (s.id == id) return &s;
        return nullptr;
    }

private:
    std::vector<SevShader> m_shaders;
    SevCompileCallback     m_onCompile;
};

} // namespace NF
