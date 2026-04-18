#pragma once
// NF::Editor — Panel state serializer: persist and restore individual panel state
#include "NF/Core/Core.h"
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Panel State Entry ─────────────────────────────────────────────
// Key-value state bag for a single panel instance.

struct PanelStateEntry {
    std::string panelId;
    std::string panelType;
    std::unordered_map<std::string, std::string> props;

    [[nodiscard]] bool isValid() const { return !panelId.empty() && !panelType.empty(); }

    void set(const std::string& key, const std::string& val) { props[key] = val; }
    void set(const std::string& key, int val)   { props[key] = std::to_string(val); }
    void set(const std::string& key, float val) { props[key] = std::to_string(val); }
    void set(const std::string& key, bool val)  { props[key] = val ? "1" : "0"; }

    [[nodiscard]] std::string get(const std::string& key, const std::string& def = "") const {
        auto it = props.find(key);
        return it != props.end() ? it->second : def;
    }
    [[nodiscard]] int   getInt(const std::string& key, int def = 0) const {
        auto it = props.find(key);
        if (it == props.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    }
    [[nodiscard]] float getFloat(const std::string& key, float def = 0.f) const {
        auto it = props.find(key);
        if (it == props.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    }
    [[nodiscard]] bool getBool(const std::string& key, bool def = false) const {
        return getInt(key, def ? 1 : 0) != 0;
    }
};

// ── Panel State Serializer ────────────────────────────────────────
// Serializes/deserializes PanelStateEntry collections to a text blob.

class PanelStateSerializer {
public:
    // Format: "panel:<id>|<type>\n<key>=<value>\n...\nend\n"
    static std::string serialize(const std::vector<PanelStateEntry>& entries) {
        std::ostringstream out;
        for (const auto& e : entries) {
            if (!e.isValid()) continue;
            out << "panel:" << e.panelId << "|" << e.panelType << "\n";
            for (const auto& [k, v] : e.props) {
                out << k << "=" << v << "\n";
            }
            out << "end\n";
        }
        return out.str();
    }

    static bool deserialize(const std::string& data,
                            std::vector<PanelStateEntry>& out) {
        out.clear();
        if (data.empty()) return false;

        std::istringstream in(data);
        std::string line;
        PanelStateEntry* current = nullptr;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            if (line.substr(0, 6) == "panel:") {
                out.emplace_back();
                current = &out.back();
                std::string rest = line.substr(6);
                auto pipe = rest.find('|');
                if (pipe == std::string::npos) { out.pop_back(); current = nullptr; continue; }
                current->panelId   = rest.substr(0, pipe);
                current->panelType = rest.substr(pipe + 1);
            } else if (line == "end") {
                current = nullptr;
            } else if (current) {
                auto eq = line.find('=');
                if (eq != std::string::npos) {
                    current->props[line.substr(0, eq)] = line.substr(eq + 1);
                }
            }
        }

        // Remove invalid entries
        out.erase(std::remove_if(out.begin(), out.end(),
            [](const PanelStateEntry& e) { return !e.isValid(); }), out.end());
        return !out.empty();
    }

    // Merge: update existing entries, add new ones
    static void merge(std::vector<PanelStateEntry>& base,
                      const std::vector<PanelStateEntry>& patch) {
        for (const auto& p : patch) {
            bool found = false;
            for (auto& b : base) {
                if (b.panelId == p.panelId) {
                    for (const auto& [k, v] : p.props) b.props[k] = v;
                    found = true; break;
                }
            }
            if (!found) base.push_back(p);
        }
    }
};

} // namespace NF
