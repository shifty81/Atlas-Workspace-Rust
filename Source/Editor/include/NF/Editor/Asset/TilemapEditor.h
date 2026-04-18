#pragma once
// NF::Editor — Tile asset + tilemap editor
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

enum class TileFlipMode : uint8_t {
    None, Horizontal, Vertical, Both, Rotate90
};
inline const char* tileFlipModeName(TileFlipMode m) {
    switch (m) {
        case TileFlipMode::None:       return "None";
        case TileFlipMode::Horizontal: return "Horizontal";
        case TileFlipMode::Vertical:   return "Vertical";
        case TileFlipMode::Both:       return "Both";
        case TileFlipMode::Rotate90:   return "Rotate90";
    }
    return "Unknown";
}

enum class TileLayerType : uint8_t {
    Background, Midground, Foreground, Object, Collision
};
inline const char* tileLayerTypeName(TileLayerType t) {
    switch (t) {
        case TileLayerType::Background: return "Background";
        case TileLayerType::Midground:  return "Midground";
        case TileLayerType::Foreground: return "Foreground";
        case TileLayerType::Object:     return "Object";
        case TileLayerType::Collision:  return "Collision";
    }
    return "Unknown";
}

enum class TileAnimMode : uint8_t {
    Static, Loop, PingPong, Once, Random
};
inline const char* tileAnimModeName(TileAnimMode m) {
    switch (m) {
        case TileAnimMode::Static:   return "Static";
        case TileAnimMode::Loop:     return "Loop";
        case TileAnimMode::PingPong: return "PingPong";
        case TileAnimMode::Once:     return "Once";
        case TileAnimMode::Random:   return "Random";
    }
    return "Unknown";
}

class TileAsset {
public:
    explicit TileAsset(const std::string& name,
                       uint32_t tileWidth  = 16,
                       uint32_t tileHeight = 16)
        : m_name(name), m_tileWidth(tileWidth), m_tileHeight(tileHeight) {}

    void setTilesetName(const std::string& ts) { m_tilesetName = ts; }
    void setTileId(uint32_t id)                { m_tileId      = id; }
    void setFlipMode(TileFlipMode f)           { m_flipMode    = f;  }
    void setLayerType(TileLayerType l)         { m_layerType   = l;  }
    void setAnimMode(TileAnimMode a)           { m_animMode    = a;  }
    void setAnimated(bool v)                   { m_animated    = v;  }
    void setCollider(bool v)                   { m_collider    = v;  }
    void setDirty(bool v)                      { m_dirty       = v;  }

    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] const std::string& tilesetName() const { return m_tilesetName; }
    [[nodiscard]] uint32_t           tileId()      const { return m_tileId;      }
    [[nodiscard]] uint32_t           tileWidth()   const { return m_tileWidth;   }
    [[nodiscard]] uint32_t           tileHeight()  const { return m_tileHeight;  }
    [[nodiscard]] TileFlipMode       flipMode()    const { return m_flipMode;    }
    [[nodiscard]] TileLayerType      layerType()   const { return m_layerType;   }
    [[nodiscard]] TileAnimMode       animMode()    const { return m_animMode;    }
    [[nodiscard]] bool               isAnimated()  const { return m_animated;    }
    [[nodiscard]] bool               hasCollider() const { return m_collider;    }
    [[nodiscard]] bool               isDirty()     const { return m_dirty;       }

    [[nodiscard]] uint32_t area() const { return m_tileWidth * m_tileHeight; }

private:
    std::string   m_name;
    std::string   m_tilesetName;
    uint32_t      m_tileId      = 0;
    uint32_t      m_tileWidth;
    uint32_t      m_tileHeight;
    TileFlipMode  m_flipMode    = TileFlipMode::None;
    TileLayerType m_layerType   = TileLayerType::Background;
    TileAnimMode  m_animMode    = TileAnimMode::Static;
    bool          m_animated    = false;
    bool          m_collider    = false;
    bool          m_dirty       = false;
};

class TilemapEditor {
public:
    static constexpr size_t MAX_TILES = 256;

    [[nodiscard]] bool addTile(const TileAsset& tile) {
        if (m_tiles.size() >= MAX_TILES) return false;
        for (auto& t : m_tiles) if (t.name() == tile.name()) return false;
        m_tiles.push_back(tile);
        return true;
    }

    [[nodiscard]] bool removeTile(const std::string& name) {
        for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
            if (it->name() == name) {
                if (m_activeTile == name) m_activeTile.clear();
                m_tiles.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TileAsset* findTile(const std::string& name) {
        for (auto& t : m_tiles) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveTile(const std::string& name) {
        for (auto& t : m_tiles)
            if (t.name() == name) { m_activeTile = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeTile() const { return m_activeTile; }
    [[nodiscard]] size_t tileCount()   const { return m_tiles.size(); }
    [[nodiscard]] size_t dirtyCount()  const {
        size_t c = 0; for (auto& t : m_tiles) if (t.isDirty())     ++c; return c;
    }
    [[nodiscard]] size_t animatedCount() const {
        size_t c = 0; for (auto& t : m_tiles) if (t.isAnimated())  ++c; return c;
    }
    [[nodiscard]] size_t colliderCount() const {
        size_t c = 0; for (auto& t : m_tiles) if (t.hasCollider()) ++c; return c;
    }
    [[nodiscard]] size_t countByLayerType(TileLayerType l) const {
        size_t c = 0; for (auto& t : m_tiles) if (t.layerType() == l) ++c; return c;
    }
    [[nodiscard]] size_t countByFlipMode(TileFlipMode f) const {
        size_t c = 0; for (auto& t : m_tiles) if (t.flipMode()    == f) ++c; return c;
    }

private:
    std::vector<TileAsset> m_tiles;
    std::string            m_activeTile;
};

// ── S40 — Audio Clip Editor ──────────────────────────────────────


} // namespace NF
