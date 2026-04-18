#pragma once
// NF::Editor — Anim blueprint asset + editor
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

enum class AnimBPNodeType : uint8_t {
    StateMachine, BlendSpace, Selector, Sequence, Pose
};
inline const char* animBPNodeTypeName(AnimBPNodeType t) {
    switch (t) {
        case AnimBPNodeType::StateMachine: return "StateMachine";
        case AnimBPNodeType::BlendSpace:   return "BlendSpace";
        case AnimBPNodeType::Selector:     return "Selector";
        case AnimBPNodeType::Sequence:     return "Sequence";
        case AnimBPNodeType::Pose:         return "Pose";
    }
    return "Unknown";
}

enum class AnimBPState : uint8_t {
    Inactive, Compiling, Ready, Running, Error
};
inline const char* animBPStateName(AnimBPState s) {
    switch (s) {
        case AnimBPState::Inactive:  return "Inactive";
        case AnimBPState::Compiling: return "Compiling";
        case AnimBPState::Ready:     return "Ready";
        case AnimBPState::Running:   return "Running";
        case AnimBPState::Error:     return "Error";
    }
    return "Unknown";
}

enum class AnimBPBlendMode : uint8_t {
    Override, Additive, Layered, Masked, Blended
};
inline const char* animBPBlendModeName(AnimBPBlendMode m) {
    switch (m) {
        case AnimBPBlendMode::Override: return "Override";
        case AnimBPBlendMode::Additive: return "Additive";
        case AnimBPBlendMode::Layered:  return "Layered";
        case AnimBPBlendMode::Masked:   return "Masked";
        case AnimBPBlendMode::Blended:  return "Blended";
    }
    return "Unknown";
}

class AnimBlueprintAsset {
public:
    explicit AnimBlueprintAsset(const std::string& name,
                                 uint32_t nodeCount = 0,
                                 uint32_t layerCount = 0)
        : m_name(name), m_nodeCount(nodeCount), m_layerCount(layerCount) {}

    void setNodeType(AnimBPNodeType t)    { m_nodeType   = t; }
    void setState(AnimBPState s)          { m_state      = s; }
    void setBlendMode(AnimBPBlendMode m)  { m_blendMode  = m; }
    void setNodeCount(uint32_t v)         { m_nodeCount  = v; }
    void setLayerCount(uint32_t v)        { m_layerCount = v; }
    void setLooping(bool v)               { m_looping    = v; }
    void setOptimized(bool v)             { m_optimized  = v; }
    void setDirty(bool v)                 { m_dirty      = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] AnimBPNodeType     nodeType()   const { return m_nodeType;   }
    [[nodiscard]] AnimBPState        state()      const { return m_state;      }
    [[nodiscard]] AnimBPBlendMode    blendMode()  const { return m_blendMode;  }
    [[nodiscard]] uint32_t           nodeCount()  const { return m_nodeCount;  }
    [[nodiscard]] uint32_t           layerCount() const { return m_layerCount; }
    [[nodiscard]] bool               isLooping()  const { return m_looping;    }
    [[nodiscard]] bool               isOptimized()const { return m_optimized;  }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isRunning()  const { return m_state == AnimBPState::Running;  }
    [[nodiscard]] bool hasError()   const { return m_state == AnimBPState::Error;    }
    [[nodiscard]] bool isReady()    const { return m_state == AnimBPState::Ready;    }
    [[nodiscard]] bool isComplex()  const { return m_nodeCount >= 20; }

private:
    std::string       m_name;
    AnimBPNodeType    m_nodeType   = AnimBPNodeType::StateMachine;
    AnimBPState       m_state      = AnimBPState::Inactive;
    AnimBPBlendMode   m_blendMode  = AnimBPBlendMode::Override;
    uint32_t          m_nodeCount;
    uint32_t          m_layerCount;
    bool              m_looping    = false;
    bool              m_optimized  = false;
    bool              m_dirty      = false;
};

class AnimBlueprintEditor {
public:
    static constexpr size_t MAX_BLUEPRINTS = 256;

    [[nodiscard]] bool addBlueprint(const AnimBlueprintAsset& bp) {
        if (m_blueprints.size() >= MAX_BLUEPRINTS) return false;
        for (auto& b : m_blueprints) if (b.name() == bp.name()) return false;
        m_blueprints.push_back(bp);
        return true;
    }

    [[nodiscard]] bool removeBlueprint(const std::string& name) {
        for (auto it = m_blueprints.begin(); it != m_blueprints.end(); ++it) {
            if (it->name() == name) {
                if (m_activeBlueprint == name) m_activeBlueprint.clear();
                m_blueprints.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AnimBlueprintAsset* findBlueprint(const std::string& name) {
        for (auto& b : m_blueprints) if (b.name() == name) return &b;
        return nullptr;
    }

    [[nodiscard]] bool setActiveBlueprint(const std::string& name) {
        for (auto& b : m_blueprints)
            if (b.name() == name) { m_activeBlueprint = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeBlueprint()   const { return m_activeBlueprint;    }
    [[nodiscard]] size_t blueprintCount()                const { return m_blueprints.size();   }
    [[nodiscard]] size_t dirtyCount()                    const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isDirty())      ++n; return n;
    }
    [[nodiscard]] size_t runningCount()                  const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isRunning())    ++n; return n;
    }
    [[nodiscard]] size_t loopingCount()                  const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isLooping())    ++n; return n;
    }
    [[nodiscard]] size_t optimizedCount()                const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isOptimized())  ++n; return n;
    }
    [[nodiscard]] size_t complexCount()                  const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isComplex())    ++n; return n;
    }
    [[nodiscard]] size_t countByNodeType(AnimBPNodeType t) const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.nodeType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByState(AnimBPState s) const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.state() == s)   ++n; return n;
    }

private:
    std::vector<AnimBlueprintAsset> m_blueprints;
    std::string                     m_activeBlueprint;
};

// ── M1-C — NFRenderViewport base class ────────────────────────────


} // namespace NF
