#pragma once
// NF::Editor — PCG tuning panel + noise params
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
#include "NF/Editor/EditorPanel.h"

namespace NF {

struct Vec3i {
    int x = 0, y = 0, z = 0;
    bool operator==(const Vec3i& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3i& o) const { return !(*this == o); }
};

// ── PCG Tuning ──────────────────────────────────────────────────

struct NoiseParams {
    float frequency   = 1.0f;
    float amplitude   = 1.0f;
    int   octaves     = 4;
    float lacunarity  = 2.0f;
    float persistence = 0.5f;
    int   seed        = 42;

    bool operator==(const NoiseParams& o) const {
        return frequency == o.frequency && amplitude == o.amplitude &&
               octaves == o.octaves && lacunarity == o.lacunarity &&
               persistence == o.persistence && seed == o.seed;
    }
    bool operator!=(const NoiseParams& o) const { return !(*this == o); }
};

struct PCGPreset {
    std::string name;
    NoiseParams params;
};

class PCGTuningPanel : public EditorPanel {
public:
    PCGTuningPanel() { m_name = "PCGTuning"; }

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    void setNoiseParams(const NoiseParams& p) { m_params = p; m_dirty = true; }
    [[nodiscard]] const NoiseParams& noiseParams() const { return m_params; }

    void addPreset(const PCGPreset& preset) { m_presets.push_back(preset); }

    bool removePreset(const std::string& presetName) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
                               [&](const PCGPreset& p) { return p.name == presetName; });
        if (it == m_presets.end()) return false;
        m_presets.erase(it);
        return true;
    }

    bool applyPreset(const std::string& presetName) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
                               [&](const PCGPreset& p) { return p.name == presetName; });
        if (it == m_presets.end()) return false;
        m_params = it->params;
        m_dirty = true;
        return true;
    }

    [[nodiscard]] size_t presetCount() const { return m_presets.size(); }
    [[nodiscard]] const std::vector<PCGPreset>& presets() const { return m_presets; }

    void setSeed(int seed) { m_params.seed = seed; m_dirty = true; }

    void randomizeSeed() {
        m_params.seed = static_cast<int>(std::hash<size_t>{}(
            static_cast<size_t>(m_params.seed) ^ 0x9E3779B97F4A7C15ULL));
        m_dirty = true;
    }

    void markDirty() { m_dirty = true; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

private:
    std::string m_name;
    NoiseParams m_params;
    std::vector<PCGPreset> m_presets;
    bool m_dirty = false;
};

// ── Entity Placement ────────────────────────────────────────────


} // namespace NF
