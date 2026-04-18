#pragma once
// NF::Editor — Texture atlas editor
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

enum class AtlasPackAlgorithm : uint8_t {
    MaxRects, Guillotine, Shelf, SkylineSimple, OptimalPacking
};

inline const char* atlasPackAlgorithmName(AtlasPackAlgorithm a) {
    switch (a) {
        case AtlasPackAlgorithm::MaxRects:        return "MaxRects";
        case AtlasPackAlgorithm::Guillotine:      return "Guillotine";
        case AtlasPackAlgorithm::Shelf:           return "Shelf";
        case AtlasPackAlgorithm::SkylineSimple:   return "SkylineSimple";
        case AtlasPackAlgorithm::OptimalPacking:  return "OptimalPacking";
    }
    return "Unknown";
}

enum class AtlasExportFormat : uint8_t {
    PNG, JPEG, WEBP, KTX, DDS
};

inline const char* atlasExportFormatName(AtlasExportFormat f) {
    switch (f) {
        case AtlasExportFormat::PNG:  return "PNG";
        case AtlasExportFormat::JPEG: return "JPEG";
        case AtlasExportFormat::WEBP: return "WEBP";
        case AtlasExportFormat::KTX:  return "KTX";
        case AtlasExportFormat::DDS:  return "DDS";
    }
    return "Unknown";
}

class AtlasSprite {
public:
    explicit AtlasSprite(const std::string& name, uint32_t w, uint32_t h)
        : m_name(name), m_width(w), m_height(h) {}

    void setPadding(uint32_t p)  { m_padding  = p; }
    void setRotated(bool v)      { m_rotated  = v; }
    void setTrimmed(bool v)      { m_trimmed  = v; }

    [[nodiscard]] const std::string& name()     const { return m_name;    }
    [[nodiscard]] uint32_t           width()    const { return m_width;   }
    [[nodiscard]] uint32_t           height()   const { return m_height;  }
    [[nodiscard]] uint32_t           padding()  const { return m_padding; }
    [[nodiscard]] bool               isRotated()const { return m_rotated; }
    [[nodiscard]] bool               isTrimmed()const { return m_trimmed; }

private:
    std::string  m_name;
    uint32_t     m_width;
    uint32_t     m_height;
    uint32_t     m_padding  = 2;
    bool         m_rotated  = false;
    bool         m_trimmed  = false;
};

class TextureAtlasEditor {
public:
    explicit TextureAtlasEditor(uint32_t maxW = 4096, uint32_t maxH = 4096)
        : m_maxWidth(maxW), m_maxHeight(maxH) {}

    void setPackAlgorithm(AtlasPackAlgorithm a) { m_algorithm   = a; }
    void setExportFormat(AtlasExportFormat f)   { m_exportFormat= f; }
    void setAllowRotation(bool v)               { m_allowRotation = v; }
    void setAllowTrimming(bool v)               { m_allowTrimming = v; }

    [[nodiscard]] bool addSprite(const AtlasSprite& sprite) {
        for (auto& s : m_sprites) if (s.name() == sprite.name()) return false;
        m_sprites.push_back(sprite);
        return true;
    }

    [[nodiscard]] bool removeSprite(const std::string& name) {
        for (auto it = m_sprites.begin(); it != m_sprites.end(); ++it) {
            if (it->name() == name) { m_sprites.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] AtlasSprite* findSprite(const std::string& name) {
        for (auto& s : m_sprites) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] AtlasPackAlgorithm packAlgorithm()  const { return m_algorithm;    }
    [[nodiscard]] AtlasExportFormat  exportFormat()   const { return m_exportFormat; }
    [[nodiscard]] uint32_t           maxWidth()       const { return m_maxWidth;     }
    [[nodiscard]] uint32_t           maxHeight()      const { return m_maxHeight;    }
    [[nodiscard]] bool               allowsRotation() const { return m_allowRotation;}
    [[nodiscard]] bool               allowsTrimming() const { return m_allowTrimming;}
    [[nodiscard]] size_t             spriteCount()    const { return m_sprites.size();}

    [[nodiscard]] size_t rotatedSpriteCount() const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isRotated()) ++c; return c;
    }
    [[nodiscard]] size_t trimmedSpriteCount() const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isTrimmed()) ++c; return c;
    }

private:
    std::vector<AtlasSprite> m_sprites;
    AtlasPackAlgorithm       m_algorithm    = AtlasPackAlgorithm::MaxRects;
    AtlasExportFormat        m_exportFormat = AtlasExportFormat::PNG;
    uint32_t                 m_maxWidth;
    uint32_t                 m_maxHeight;
    bool                     m_allowRotation = true;
    bool                     m_allowTrimming = true;
};

} // namespace NF
