#pragma once
// NF::Editor — Pipeline state editor v1: blend, cull, fill, depth pipeline configurations
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class PsvBlendMode : uint8_t { Opaque, Translucent, Additive, Multiply };
enum class PsvCullMode  : uint8_t { None, Front, Back };
enum class PsvFillMode  : uint8_t { Solid, Wireframe, Point };

struct PsvPipelineState {
    uint32_t     id          = 0;
    std::string  name;
    PsvBlendMode blend       = PsvBlendMode::Opaque;
    PsvCullMode  cull        = PsvCullMode::Back;
    PsvFillMode  fill        = PsvFillMode::Solid;
    bool         depthTest   = true;
    bool         scissorTest = false;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

class PipelineStateEditorV1 {
public:
    bool addState(const PsvPipelineState& state) {
        if (!state.isValid()) return false;
        for (const auto& s : m_states) if (s.id == state.id) return false;
        m_states.push_back(state);
        return true;
    }

    bool removeState(uint32_t id) {
        for (auto it = m_states.begin(); it != m_states.end(); ++it) {
            if (it->id == id) {
                if (m_defaultId == id) m_defaultId = 0;
                m_states.erase(it);
                return true;
            }
        }
        return false;
    }

    bool cloneState(uint32_t srcId, uint32_t newId) {
        const PsvPipelineState* src = findState_(srcId);
        if (!src) return false;
        for (const auto& s : m_states) if (s.id == newId) return false;
        PsvPipelineState copy = *src;
        copy.id = newId;
        copy.name = copy.name + "_copy";
        m_states.push_back(copy);
        return true;
    }

    const PsvPipelineState* findState(const std::string& name) const {
        for (const auto& s : m_states) if (s.name == name) return &s;
        return nullptr;
    }

    bool setDefault(uint32_t id) {
        if (!findState_(id)) return false;
        m_defaultId = id;
        return true;
    }

    const PsvPipelineState* getDefault() const {
        return findState_(m_defaultId);
    }

    [[nodiscard]] size_t stateCount() const { return m_states.size(); }

private:
    const PsvPipelineState* findState_(uint32_t id) const {
        for (const auto& s : m_states) if (s.id == id) return &s;
        return nullptr;
    }

    std::vector<PsvPipelineState> m_states;
    uint32_t                      m_defaultId = 0;
};

} // namespace NF
