#include "EditorLayout.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <fstream>

namespace atlas::editor {

void EditorLayout::RegisterPanel(EditorPanel* panel) {
    m_panels.push_back(panel);
    m_menuBar.RegisterPanel(panel);
}

bool EditorLayout::ClosePanel(const std::string& name) {
    for (auto* panel : m_panels) {
        if (name == panel->Name()) {
            if (!panel->IsClosable()) {
                return false;
            }
            panel->SetVisible(false);
            return true;
        }
    }
    return false;
}

EditorPanel* EditorLayout::FindPanel(const std::string& name) const {
    for (auto* panel : m_panels) {
        if (name == panel->Name()) {
            return panel;
        }
    }
    return nullptr;
}

void EditorLayout::Draw() {
    DrawNode(m_root);
}

void EditorLayout::DrawNode(DockNode& node) {
    if (node.split == DockSplit::None) {
        if (node.panel && node.panel->IsVisible()) {
            node.panel->Draw();
        }
        return;
    }

    if (node.a) DrawNode(*node.a);
    if (node.b) DrawNode(*node.b);
}

// ── JSON serialisation helpers ──────────────────────────────────

static std::string Indent(int depth) {
    return std::string(depth * 2, ' ');
}

static const char* SplitName(DockSplit s) {
    switch (s) {
        case DockSplit::None:       return "None";
        case DockSplit::Horizontal: return "Horizontal";
        case DockSplit::Vertical:   return "Vertical";
        case DockSplit::Tab:        return "Tab";
    }
    return "None";
}

static DockSplit SplitFromName(const std::string& s) {
    if (s == "Horizontal") return DockSplit::Horizontal;
    if (s == "Vertical")   return DockSplit::Vertical;
    if (s == "Tab")        return DockSplit::Tab;
    return DockSplit::None;
}

static void SerializeNode(std::ostringstream& os, const DockNode& node, int depth) {
    os << Indent(depth) << "{\n";
    os << Indent(depth + 1) << "\"split\": \"" << SplitName(node.split) << "\"";

    if (node.split == DockSplit::None && node.panel) {
        os << ",\n" << Indent(depth + 1) << "\"panel\": \"" << node.panel->Name() << "\"";
    }

    if (node.split == DockSplit::Horizontal || node.split == DockSplit::Vertical) {
        os << ",\n" << Indent(depth + 1) << "\"splitRatio\": " << node.splitRatio;
        if (node.a) {
            os << ",\n" << Indent(depth + 1) << "\"a\":\n";
            SerializeNode(os, *node.a, depth + 1);
        }
        if (node.b) {
            os << ",\n" << Indent(depth + 1) << "\"b\":\n";
            SerializeNode(os, *node.b, depth + 1);
        }
    }

    if (node.split == DockSplit::Tab) {
        os << ",\n" << Indent(depth + 1) << "\"tabs\": [";
        for (size_t i = 0; i < node.tabs.size(); ++i) {
            if (i > 0) os << ", ";
            os << "\"" << node.tabs[i]->Name() << "\"";
        }
        os << "]";
        os << ",\n" << Indent(depth + 1) << "\"activeTab\": " << node.activeTab;
    }

    os << "\n" << Indent(depth) << "}";
}

std::string EditorLayout::SerializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"layout\":\n";
    SerializeNode(os, m_root, 1);
    os << "\n}\n";
    return os.str();
}

// ── Minimal JSON parser helpers ─────────────────────────────────

static std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

static size_t FindMatchingBrace(const std::string& json, size_t open) {
    int depth = 0;
    bool inStr = false;
    for (size_t i = open; i < json.size(); ++i) {
        char c = json[i];
        if (c == '"' && (i == 0 || json[i - 1] != '\\')) inStr = !inStr;
        if (inStr) continue;
        if (c == '{') ++depth;
        if (c == '}') { --depth; if (depth == 0) return i; }
    }
    return std::string::npos;
}

static size_t FindMatchingBracket(const std::string& json, size_t open) {
    int depth = 0;
    bool inStr = false;
    for (size_t i = open; i < json.size(); ++i) {
        char c = json[i];
        if (c == '"' && (i == 0 || json[i - 1] != '\\')) inStr = !inStr;
        if (inStr) continue;
        if (c == '[') ++depth;
        if (c == ']') { --depth; if (depth == 0) return i; }
    }
    return std::string::npos;
}

static std::string ExtractString(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\": \"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        needle = "\"" + key + "\":\"";
        pos = json.find(needle);
    }
    if (pos == std::string::npos) return "";
    pos += needle.size();
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

static float ExtractFloat(const std::string& json, const std::string& key, float def) {
    std::string needle = "\"" + key + "\": ";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        needle = "\"" + key + "\":";
        pos = json.find(needle);
    }
    if (pos == std::string::npos) return def;
    pos += needle.size();
    try { return std::stof(json.substr(pos)); } catch (...) { return def; }
}

static int ExtractInt(const std::string& json, const std::string& key, int def) {
    std::string needle = "\"" + key + "\": ";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        needle = "\"" + key + "\":";
        pos = json.find(needle);
    }
    if (pos == std::string::npos) return def;
    pos += needle.size();
    try { return std::stoi(json.substr(pos)); } catch (...) { return def; }
}

static std::string ExtractObject(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        needle = "\"" + key + "\": ";
        pos = json.find(needle);
    }
    if (pos == std::string::npos) return "";
    size_t brace = json.find('{', pos + needle.size());
    if (brace == std::string::npos) return "";
    size_t end = FindMatchingBrace(json, brace);
    if (end == std::string::npos) return "";
    return json.substr(brace, end - brace + 1);
}

static std::vector<std::string> ExtractStringArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string needle = "\"" + key + "\": [";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        needle = "\"" + key + "\":[";
        pos = json.find(needle);
    }
    if (pos == std::string::npos) return result;
    size_t bracket = json.find('[', pos);
    size_t end = FindMatchingBracket(json, bracket);
    if (end == std::string::npos) return result;
    std::string arr = json.substr(bracket + 1, end - bracket - 1);
    size_t p = 0;
    while (true) {
        size_t q1 = arr.find('"', p);
        if (q1 == std::string::npos) break;
        size_t q2 = arr.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        result.push_back(arr.substr(q1 + 1, q2 - q1 - 1));
        p = q2 + 1;
    }
    return result;
}

static void DeserializeNode(DockNode& node, const std::string& json,
                     const std::vector<EditorPanel*>& panels) {
    std::string splitStr = ExtractString(json, "split");
    node.split = SplitFromName(splitStr);

    if (node.split == DockSplit::None) {
        std::string panelName = ExtractString(json, "panel");
        node.panel = nullptr;
        for (auto* p : panels) {
            if (panelName == p->Name()) { node.panel = p; break; }
        }
    }

    if (node.split == DockSplit::Horizontal || node.split == DockSplit::Vertical) {
        node.splitRatio = ExtractFloat(json, "splitRatio", 0.5f);
        std::string aJson = ExtractObject(json, "a");
        std::string bJson = ExtractObject(json, "b");
        if (!aJson.empty()) {
            node.a = std::make_unique<DockNode>();
            DeserializeNode(*node.a, aJson, panels);
        }
        if (!bJson.empty()) {
            node.b = std::make_unique<DockNode>();
            DeserializeNode(*node.b, bJson, panels);
        }
    }

    if (node.split == DockSplit::Tab) {
        auto tabNames = ExtractStringArray(json, "tabs");
        node.tabs.clear();
        for (auto& name : tabNames) {
            for (auto* p : panels) {
                if (name == p->Name()) { node.tabs.push_back(p); break; }
            }
        }
        node.activeTab = ExtractInt(json, "activeTab", 0);
    }
}

bool EditorLayout::DeserializeFromJSON(const std::string& json) {
    if (json.empty()) return false;
    // Basic validation: must contain "layout"
    if (json.find("\"layout\"") == std::string::npos) return false;

    std::string layoutJson = ExtractObject(json, "layout");
    if (layoutJson.empty()) return false;

    m_root = DockNode{};
    DeserializeNode(m_root, layoutJson, m_panels);
    return true;
}

bool EditorLayout::SaveToFile(const std::string& path) const {
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;
    ofs << SerializeToJSON();
    return ofs.good();
}

bool EditorLayout::LoadFromFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return DeserializeFromJSON(ss.str());
}

}
