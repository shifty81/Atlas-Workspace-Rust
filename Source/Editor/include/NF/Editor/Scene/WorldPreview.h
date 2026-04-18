#pragma once
// NF::Editor — World preview service
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

enum class PreviewState : uint8_t { Idle, Loading, Ready, Error };

class WorldPreviewService {
public:
    void loadPreview(const std::string& path) {
        m_worldPath = path;
        if (path.empty()) {
            m_state = PreviewState::Error;
            m_lastError = "Empty path";
            return;
        }
        m_state = PreviewState::Loading;
        m_state = PreviewState::Ready;
        m_dirty = false;
    }

    void unloadPreview() {
        m_worldPath.clear();
        m_state = PreviewState::Idle;
        m_dirty = false;
        m_lastError.clear();
    }

    [[nodiscard]] PreviewState state() const { return m_state; }
    [[nodiscard]] const std::string& worldPath() const { return m_worldPath; }

    void setViewCenter(const Vec3& center) { m_viewCenter = center; }
    [[nodiscard]] const Vec3& viewCenter() const { return m_viewCenter; }

    void setViewRadius(float r) { m_viewRadius = r; }
    [[nodiscard]] float viewRadius() const { return m_viewRadius; }

    void setDirty() { m_dirty = true; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    [[nodiscard]] const std::string& lastError() const { return m_lastError; }

private:
    PreviewState m_state = PreviewState::Idle;
    std::string m_worldPath;
    Vec3 m_viewCenter;
    float m_viewRadius = 100.0f;
    bool m_dirty = false;
    std::string m_lastError;
};

// ── M3/S2 Play-in-Editor ──────────────────────────────────────────


} // namespace NF
