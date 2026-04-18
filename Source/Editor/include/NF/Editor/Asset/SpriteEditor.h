#pragma once
// NF::Editor — Sprite asset + editor
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

enum class SpriteOrigin : uint8_t {
    TopLeft, TopCenter, Center, BottomLeft, BottomCenter
};

inline const char* spriteOriginName(SpriteOrigin o) {
    switch (o) {
        case SpriteOrigin::TopLeft:      return "TopLeft";
        case SpriteOrigin::TopCenter:    return "TopCenter";
        case SpriteOrigin::Center:       return "Center";
        case SpriteOrigin::BottomLeft:   return "BottomLeft";
        case SpriteOrigin::BottomCenter: return "BottomCenter";
    }
    return "Unknown";
}

enum class SpriteBlendMode : uint8_t {
    Normal, Additive, Multiply, Screen, Overlay
};

inline const char* spriteBlendModeName(SpriteBlendMode b) {
    switch (b) {
        case SpriteBlendMode::Normal:   return "Normal";
        case SpriteBlendMode::Additive: return "Additive";
        case SpriteBlendMode::Multiply: return "Multiply";
        case SpriteBlendMode::Screen:   return "Screen";
        case SpriteBlendMode::Overlay:  return "Overlay";
    }
    return "Unknown";
}

enum class SpriteAnimState : uint8_t {
    Idle, Playing, Paused, Stopped, Finished
};

inline const char* spriteAnimStateName(SpriteAnimState s) {
    switch (s) {
        case SpriteAnimState::Idle:     return "Idle";
        case SpriteAnimState::Playing:  return "Playing";
        case SpriteAnimState::Paused:   return "Paused";
        case SpriteAnimState::Stopped:  return "Stopped";
        case SpriteAnimState::Finished: return "Finished";
    }
    return "Unknown";
}

class SpriteAsset {
public:
    explicit SpriteAsset(const std::string& name,
                         uint32_t width  = 32,
                         uint32_t height = 32)
        : m_name(name), m_width(width), m_height(height) {}

    void setOrigin(SpriteOrigin o)      { m_origin    = o; }
    void setBlendMode(SpriteBlendMode b){ m_blendMode = b; }
    void setAnimState(SpriteAnimState s){ m_animState = s; }
    void setFrameCount(uint32_t n)      { m_frameCount = n; }
    void setFrameRate(float fps)        { m_frameRate  = fps; }
    void setLooping(bool v)             { m_looping    = v; }
    void setFlippedH(bool v)            { m_flippedH   = v; }
    void setFlippedV(bool v)            { m_flippedV   = v; }
    void setDirty(bool v)               { m_dirty      = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] uint32_t           width()      const { return m_width;      }
    [[nodiscard]] uint32_t           height()     const { return m_height;     }
    [[nodiscard]] SpriteOrigin       origin()     const { return m_origin;     }
    [[nodiscard]] SpriteBlendMode    blendMode()  const { return m_blendMode;  }
    [[nodiscard]] SpriteAnimState    animState()  const { return m_animState;  }
    [[nodiscard]] uint32_t           frameCount() const { return m_frameCount; }
    [[nodiscard]] float              frameRate()  const { return m_frameRate;  }
    [[nodiscard]] bool               isLooping()  const { return m_looping;    }
    [[nodiscard]] bool               isFlippedH() const { return m_flippedH;   }
    [[nodiscard]] bool               isFlippedV() const { return m_flippedV;   }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isAnimated()    const { return m_frameCount > 1; }
    [[nodiscard]] bool isPlaying()     const { return m_animState == SpriteAnimState::Playing; }
    [[nodiscard]] bool isPaused()      const { return m_animState == SpriteAnimState::Paused;  }
    [[nodiscard]] bool isFinished()    const { return m_animState == SpriteAnimState::Finished;}
    [[nodiscard]] uint32_t area()      const { return m_width * m_height; }

private:
    std::string      m_name;
    uint32_t         m_width      = 32;
    uint32_t         m_height     = 32;
    SpriteOrigin     m_origin     = SpriteOrigin::Center;
    SpriteBlendMode  m_blendMode  = SpriteBlendMode::Normal;
    SpriteAnimState  m_animState  = SpriteAnimState::Idle;
    uint32_t         m_frameCount = 1;
    float            m_frameRate  = 24.0f;
    bool             m_looping    = false;
    bool             m_flippedH   = false;
    bool             m_flippedV   = false;
    bool             m_dirty      = false;
};

class SpriteEditor {
public:
    static constexpr size_t MAX_SPRITES = 512;

    [[nodiscard]] bool addSprite(const SpriteAsset& sprite) {
        for (auto& s : m_sprites) if (s.name() == sprite.name()) return false;
        if (m_sprites.size() >= MAX_SPRITES) return false;
        m_sprites.push_back(sprite);
        return true;
    }

    [[nodiscard]] bool removeSprite(const std::string& name) {
        for (auto it = m_sprites.begin(); it != m_sprites.end(); ++it) {
            if (it->name() == name) {
                if (m_activeSprite == name) m_activeSprite.clear();
                m_sprites.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SpriteAsset* findSprite(const std::string& name) {
        for (auto& s : m_sprites) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] bool setActiveSprite(const std::string& name) {
        for (auto& s : m_sprites) {
            if (s.name() == name) { m_activeSprite = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t spriteCount()   const { return m_sprites.size();  }
    [[nodiscard]] const std::string& activeSprite() const { return m_activeSprite; }

    [[nodiscard]] size_t dirtyCount()    const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isDirty())    ++c; return c;
    }
    [[nodiscard]] size_t animatedCount() const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isAnimated()) ++c; return c;
    }
    [[nodiscard]] size_t playingCount()  const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isPlaying())  ++c; return c;
    }
    [[nodiscard]] size_t loopingCount()  const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isLooping())  ++c; return c;
    }
    [[nodiscard]] size_t countByBlendMode(SpriteBlendMode b) const {
        size_t c = 0; for (auto& s : m_sprites) if (s.blendMode() == b) ++c; return c;
    }
    [[nodiscard]] size_t countByOrigin(SpriteOrigin o) const {
        size_t c = 0; for (auto& s : m_sprites) if (s.origin() == o) ++c; return c;
    }

private:
    std::vector<SpriteAsset> m_sprites;
    std::string              m_activeSprite;
};

// ── S39 — Tilemap Editor ─────────────────────────────────────────


} // namespace NF
