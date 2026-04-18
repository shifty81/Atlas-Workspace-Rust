#pragma once
// NF::Editor — costume editor
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

enum class CostumeSlot : uint8_t {
    Hat, Top, Bottom, Shoes, Gloves, Belt, Necklace, Earring, Backpack, Cape
};

inline const char* costumeSlotName(CostumeSlot s) {
    switch (s) {
        case CostumeSlot::Hat:      return "Hat";
        case CostumeSlot::Top:      return "Top";
        case CostumeSlot::Bottom:   return "Bottom";
        case CostumeSlot::Shoes:    return "Shoes";
        case CostumeSlot::Gloves:   return "Gloves";
        case CostumeSlot::Belt:     return "Belt";
        case CostumeSlot::Necklace: return "Necklace";
        case CostumeSlot::Earring:  return "Earring";
        case CostumeSlot::Backpack: return "Backpack";
        case CostumeSlot::Cape:     return "Cape";
    }
    return "Unknown";
}

enum class CostumeMaterial : uint8_t {
    Cloth, Leather, Metal, Silk, Fur, Synthetic
};

inline const char* costumeMaterialName(CostumeMaterial m) {
    switch (m) {
        case CostumeMaterial::Cloth:     return "Cloth";
        case CostumeMaterial::Leather:   return "Leather";
        case CostumeMaterial::Metal:     return "Metal";
        case CostumeMaterial::Silk:      return "Silk";
        case CostumeMaterial::Fur:       return "Fur";
        case CostumeMaterial::Synthetic: return "Synthetic";
    }
    return "Unknown";
}

enum class CostumeRarity : uint8_t {
    Common, Uncommon, Rare, Epic, Legendary
};

inline const char* costumeRarityName(CostumeRarity r) {
    switch (r) {
        case CostumeRarity::Common:    return "Common";
        case CostumeRarity::Uncommon:  return "Uncommon";
        case CostumeRarity::Rare:      return "Rare";
        case CostumeRarity::Epic:      return "Epic";
        case CostumeRarity::Legendary: return "Legendary";
    }
    return "Unknown";
}

class CostumePiece {
public:
    explicit CostumePiece(uint32_t id, const std::string& name, CostumeSlot slot)
        : m_id(id), m_name(name), m_slot(slot) {}

    void setMaterial(CostumeMaterial v) { m_material   = v; }
    void setRarity(CostumeRarity v)     { m_rarity     = v; }
    void setEquipped(bool v)            { m_isEquipped  = v; }
    void setLocked(bool v)              { m_isLocked    = v; }
    void setDyeable(bool v)             { m_dyeable     = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;        }
    [[nodiscard]] const std::string& name()       const { return m_name;      }
    [[nodiscard]] CostumeSlot        slot()       const { return m_slot;      }
    [[nodiscard]] CostumeMaterial    material()   const { return m_material;  }
    [[nodiscard]] CostumeRarity      rarity()     const { return m_rarity;    }
    [[nodiscard]] bool               isEquipped() const { return m_isEquipped;}
    [[nodiscard]] bool               isLocked()   const { return m_isLocked;  }
    [[nodiscard]] bool               isDyeable()  const { return m_dyeable;   }

private:
    uint32_t        m_id;
    std::string     m_name;
    CostumeSlot     m_slot;
    CostumeMaterial m_material   = CostumeMaterial::Cloth;
    CostumeRarity   m_rarity     = CostumeRarity::Common;
    bool            m_isEquipped  = false;
    bool            m_isLocked    = false;
    bool            m_dyeable     = true;
};

class CostumeEditor {
public:
    void setShowPreview(bool v) { m_showPreview = v; }
    void setShowStats(bool v)   { m_showStats   = v; }
    void setGridView(bool v)    { m_gridView    = v; }

    bool addPiece(const CostumePiece& p) {
        for (auto& e : m_pieces) if (e.id() == p.id()) return false;
        m_pieces.push_back(p); return true;
    }
    bool removePiece(uint32_t id) {
        auto it = std::find_if(m_pieces.begin(), m_pieces.end(),
            [&](const CostumePiece& e){ return e.id() == id; });
        if (it == m_pieces.end()) return false;
        m_pieces.erase(it); return true;
    }
    [[nodiscard]] CostumePiece* findPiece(uint32_t id) {
        for (auto& e : m_pieces) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isShowPreview() const { return m_showPreview;    }
    [[nodiscard]] bool   isShowStats()   const { return m_showStats;      }
    [[nodiscard]] bool   isGridView()    const { return m_gridView;       }
    [[nodiscard]] size_t pieceCount()    const { return m_pieces.size();  }

    [[nodiscard]] size_t countBySlot(CostumeSlot s) const {
        size_t c = 0; for (auto& e : m_pieces) if (e.slot() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByRarity(CostumeRarity r) const {
        size_t c = 0; for (auto& e : m_pieces) if (e.rarity() == r) ++c; return c;
    }
    [[nodiscard]] size_t countEquipped() const {
        size_t c = 0; for (auto& e : m_pieces) if (e.isEquipped()) ++c; return c;
    }

private:
    std::vector<CostumePiece> m_pieces;
    bool m_showPreview = true;
    bool m_showStats   = true;
    bool m_gridView    = false;
};

} // namespace NF
