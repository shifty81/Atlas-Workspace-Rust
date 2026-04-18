#pragma once
// NF::Editor — Build pipeline editor v1: step-based build orchestration
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Bpev1BuildTarget : uint8_t { Game, Editor, Server, Client, Shipping };
inline const char* bpev1BuildTargetName(Bpev1BuildTarget t) {
    switch(t){
        case Bpev1BuildTarget::Game:     return "Game";
        case Bpev1BuildTarget::Editor:   return "Editor";
        case Bpev1BuildTarget::Server:   return "Server";
        case Bpev1BuildTarget::Client:   return "Client";
        case Bpev1BuildTarget::Shipping: return "Shipping";
    }
    return "Unknown";
}

enum class Bpev1BuildStatus : uint8_t {
    Idle, Preparing, Compiling, Linking, Packaging, Done, Failed
};

struct Bpev1BuildStep {
    uint32_t         id          = 0;
    std::string      name;
    Bpev1BuildTarget target      = Bpev1BuildTarget::Game;
    float            progressPct = 0.f;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using BpeStatusCallback = std::function<void(Bpev1BuildStatus)>;

class BuildPipelineEditorV1 {
public:
    static constexpr size_t MAX_STEPS = 64;

    bool addStep(const Bpev1BuildStep& step) {
        if (!step.isValid()) return false;
        if (m_steps.size() >= MAX_STEPS) return false;
        for (const auto& s : m_steps) if (s.id == step.id) return false;
        m_steps.push_back(step);
        return true;
    }

    bool removeStep(uint32_t id) {
        for (auto it = m_steps.begin(); it != m_steps.end(); ++it) {
            if (it->id == id) { m_steps.erase(it); return true; }
        }
        return false;
    }

    bool startBuild(Bpev1BuildTarget target) {
        if (m_status == Bpev1BuildStatus::Compiling) return false;
        m_activeTarget = target;
        setStatus(Bpev1BuildStatus::Preparing);
        return true;
    }

    void cancelBuild() {
        if (m_status != Bpev1BuildStatus::Idle && m_status != Bpev1BuildStatus::Done)
            setStatus(Bpev1BuildStatus::Failed);
    }

    [[nodiscard]] size_t            stepCount()     const { return m_steps.size();  }
    [[nodiscard]] Bpev1BuildStatus  currentStatus() const { return m_status;        }
    [[nodiscard]] Bpev1BuildTarget  activeTarget()  const { return m_activeTarget;  }

    [[nodiscard]] float totalProgress() const {
        if (m_steps.empty()) return 0.f;
        float sum = 0.f;
        for (const auto& s : m_steps) sum += s.progressPct;
        return sum / static_cast<float>(m_steps.size());
    }

    void setOnStatusChange(BpeStatusCallback cb) { m_onStatusChange = std::move(cb); }

private:
    void setStatus(Bpev1BuildStatus s) {
        m_status = s;
        if (m_onStatusChange) m_onStatusChange(s);
    }

    std::vector<Bpev1BuildStep> m_steps;
    BpeStatusCallback           m_onStatusChange;
    Bpev1BuildStatus            m_status       = Bpev1BuildStatus::Idle;
    Bpev1BuildTarget            m_activeTarget = Bpev1BuildTarget::Game;
};

} // namespace NF
