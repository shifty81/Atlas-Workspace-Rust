#pragma once
// NF::Editor — Command palette v1: fuzzy-search command/file/symbol launcher
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Cpv1ItemType : uint8_t { Command, File, Symbol, Action, Snippet, Setting };
inline const char* cpv1ItemTypeName(Cpv1ItemType t) {
    switch(t){
        case Cpv1ItemType::Command: return "Command";
        case Cpv1ItemType::File:    return "File";
        case Cpv1ItemType::Symbol:  return "Symbol";
        case Cpv1ItemType::Action:  return "Action";
        case Cpv1ItemType::Snippet: return "Snippet";
        case Cpv1ItemType::Setting: return "Setting";
    }
    return "Unknown";
}

enum class Cpv1ItemState : uint8_t { Idle, Matched, Selected, Executing };

struct Cpv1Item {
    uint32_t    id           = 0;
    Cpv1ItemType type        = Cpv1ItemType::Command;
    Cpv1ItemState state      = Cpv1ItemState::Idle;
    std::string label;
    std::string description;
    std::string category;
    std::string shortcutDisplay;
    float       score        = 0.f;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty(); }
};

using Cpv1ExecuteCallback = std::function<void(const Cpv1Item&)>;

class CommandPaletteV1 {
public:
    static constexpr size_t MAX_ITEMS = 512;

    bool addItem(const Cpv1Item& item) {
        if (!item.isValid()) return false;
        if (m_items.size() >= MAX_ITEMS) return false;
        for (const auto& i : m_items) if (i.id == item.id) return false;
        m_items.push_back(item);
        return true;
    }

    bool removeItem(uint32_t id) {
        for (auto it = m_items.begin(); it != m_items.end(); ++it) {
            if (it->id == id) { m_items.erase(it); return true; }
        }
        return false;
    }

    void open()  { m_open = true;  m_query.clear(); }
    void close() { m_open = false; }
    [[nodiscard]] bool isOpen() const { return m_open; }

    std::vector<uint32_t> search(const std::string& query) {
        m_query = query;
        std::vector<uint32_t> results;
        for (auto& item : m_items) {
            if (query.empty() ||
                item.label.find(query)       != std::string::npos ||
                item.description.find(query) != std::string::npos ||
                item.category.find(query)    != std::string::npos) {
                item.state = Cpv1ItemState::Matched;
                results.push_back(item.id);
            } else {
                item.state = Cpv1ItemState::Idle;
            }
        }
        return results;
    }

    bool executeItem(uint32_t id) {
        for (auto& item : m_items) {
            if (item.id == id) {
                item.state = Cpv1ItemState::Executing;
                if (m_onExecute) m_onExecute(item);
                close();
                return true;
            }
        }
        return false;
    }

    void setOnExecute(Cpv1ExecuteCallback cb) { m_onExecute = std::move(cb); }

    [[nodiscard]] size_t itemCount()        const { return m_items.size(); }
    [[nodiscard]] const std::string& query() const { return m_query; }

    const Cpv1Item* findItem(uint32_t id) const {
        for (const auto& i : m_items) if (i.id == id) return &i;
        return nullptr;
    }

private:
    std::vector<Cpv1Item>  m_items;
    Cpv1ExecuteCallback    m_onExecute;
    std::string            m_query;
    bool                   m_open = false;
};

} // namespace NF
