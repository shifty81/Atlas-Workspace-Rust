#include "LayoutPersistence.h"
#include <fstream>
#include <sstream>

namespace atlas::editor {

// ---- JSON helpers ----

static std::string Indent(int depth) {
    return std::string(static_cast<size_t>(depth * 2), ' ');
}

static std::string EscapeJSON(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

static void SkipWS(const std::string& json, size_t& pos) {
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
           json[pos] == '\n' || json[pos] == '\r'))
        ++pos;
}

static std::string ParseString(const std::string& json, size_t& pos) {
    if (pos >= json.size() || json[pos] != '"') return "";
    ++pos;
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;
        }
        result += json[pos++];
    }
    if (pos < json.size()) ++pos; // skip closing quote
    return result;
}

static std::string ParseToken(const std::string& json, size_t& pos) {
    SkipWS(json, pos);
    if (pos < json.size() && json[pos] == '"')
        return ParseString(json, pos);
    std::string tok;
    while (pos < json.size() && json[pos] != ',' && json[pos] != '}' &&
           json[pos] != ']' && json[pos] != ' ' && json[pos] != '\n' &&
           json[pos] != '\r' && json[pos] != '\t') {
        tok += json[pos++];
    }
    return tok;
}

// ---- Serialization ----

bool LayoutPersistence::SaveLayout(const EditorLayout& layout, const std::string& path) {
    std::string json = Serialize(layout);
    std::ofstream out(path);
    if (!out.is_open()) {
        m_lastError = "Cannot open file for writing: " + path;
        return false;
    }
    out << json;
    return true;
}

bool LayoutPersistence::LoadLayout(EditorLayout& layout, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        m_lastError = "Cannot open file for reading: " + path;
        return false;
    }
    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    return Deserialize(layout, json);
}

std::string LayoutPersistence::Serialize(const EditorLayout& layout) const {
    std::ostringstream ss;
    ss << "{\n";

    // Panel visibility
    ss << "  \"panels\": [\n";
    const auto& panels = layout.Panels();
    for (size_t i = 0; i < panels.size(); ++i) {
        ss << "    {\"name\": \"" << EscapeJSON(panels[i]->Name())
           << "\", \"visible\": " << (panels[i]->IsVisible() ? "true" : "false")
           << ", \"closable\": " << (panels[i]->IsClosable() ? "true" : "false")
           << "}";
        if (i + 1 < panels.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";

    // Dock tree
    ss << "  \"dockTree\": ";
    ss << SerializeDockNode(layout.Root(), 1);
    ss << "\n}\n";

    return ss.str();
}

std::string LayoutPersistence::SerializeDockNode(const DockNode& node, int depth) const {
    std::ostringstream ss;
    ss << "{\n";

    std::string ind = Indent(depth + 1);

    // Split type
    const char* splitStr = "none";
    if (node.split == DockSplit::Horizontal) splitStr = "horizontal";
    else if (node.split == DockSplit::Vertical) splitStr = "vertical";
    ss << ind << "\"split\": \"" << splitStr << "\",\n";

    // Split ratio
    ss << ind << "\"splitRatio\": " << node.splitRatio << ",\n";

    // Panel name
    if (node.panel) {
        ss << ind << "\"panel\": \"" << EscapeJSON(node.panel->Name()) << "\"";
    } else {
        ss << ind << "\"panel\": null";
    }

    // Children
    if (node.a) {
        ss << ",\n" << ind << "\"a\": " << SerializeDockNode(*node.a, depth + 1);
    }
    if (node.b) {
        ss << ",\n" << ind << "\"b\": " << SerializeDockNode(*node.b, depth + 1);
    }

    ss << "\n" << Indent(depth) << "}";
    return ss.str();
}

bool LayoutPersistence::Deserialize(EditorLayout& layout, const std::string& json) {
    size_t pos = 0;
    SkipWS(json, pos);
    if (pos >= json.size() || json[pos] != '{') {
        m_lastError = "Expected '{' at start of layout JSON";
        return false;
    }
    ++pos;

    while (pos < json.size()) {
        SkipWS(json, pos);
        if (pos >= json.size() || json[pos] == '}') break;
        if (json[pos] == ',') { ++pos; continue; }

        std::string key = ParseString(json, pos);
        SkipWS(json, pos);
        if (pos < json.size() && json[pos] == ':') ++pos;
        SkipWS(json, pos);

        if (key == "panels" && pos < json.size() && json[pos] == '[') {
            ++pos;
            while (pos < json.size()) {
                SkipWS(json, pos);
                if (json[pos] == ']') { ++pos; break; }
                if (json[pos] == ',') { ++pos; continue; }
                if (json[pos] == '{') {
                    ++pos;
                    std::string name;
                    bool visible = true;
                    bool closable = true;
                    while (pos < json.size() && json[pos] != '}') {
                        SkipWS(json, pos);
                        if (json[pos] == ',' || json[pos] == '}') { ++pos; continue; }
                        std::string k = ParseString(json, pos);
                        SkipWS(json, pos);
                        if (pos < json.size() && json[pos] == ':') ++pos;
                        SkipWS(json, pos);
                        std::string v = ParseToken(json, pos);
                        if (k == "name") name = v;
                        else if (k == "visible") visible = (v == "true");
                        else if (k == "closable") closable = (v == "true");
                    }
                    if (pos < json.size()) ++pos; // skip '}'

                    // Apply to matching panel
                    EditorPanel* panel = layout.FindPanel(name);
                    if (panel) {
                        panel->SetVisible(visible);
                        panel->SetClosable(closable);
                    }
                }
            }
        } else if (key == "dockTree" && pos < json.size() && json[pos] == '{') {
            if (!DeserializeDockNode(layout.Root(), layout, json, pos)) {
                return false;
            }
        } else {
            // Skip unknown value
            ParseToken(json, pos);
        }
    }

    return true;
}

bool LayoutPersistence::DeserializeDockNode(DockNode& node, EditorLayout& layout,
                                             const std::string& json, size_t& pos) {
    SkipWS(json, pos);
    if (pos >= json.size() || json[pos] != '{') {
        m_lastError = "Expected '{' for dock node";
        return false;
    }
    ++pos;

    node.split = DockSplit::None;
    node.splitRatio = 0.5f;
    node.panel = nullptr;
    node.a.reset();
    node.b.reset();

    while (pos < json.size()) {
        SkipWS(json, pos);
        if (pos >= json.size() || json[pos] == '}') { ++pos; break; }
        if (json[pos] == ',') { ++pos; continue; }

        std::string key = ParseString(json, pos);
        SkipWS(json, pos);
        if (pos < json.size() && json[pos] == ':') ++pos;
        SkipWS(json, pos);

        if (key == "split") {
            std::string v = ParseToken(json, pos);
            if (v == "horizontal") node.split = DockSplit::Horizontal;
            else if (v == "vertical") node.split = DockSplit::Vertical;
            else node.split = DockSplit::None;
        } else if (key == "splitRatio") {
            std::string v = ParseToken(json, pos);
            try { node.splitRatio = std::stof(v); } catch (...) {}
        } else if (key == "panel") {
            std::string v = ParseToken(json, pos);
            if (v != "null") {
                node.panel = layout.FindPanel(v);
            }
        } else if (key == "a" && pos < json.size() && json[pos] == '{') {
            node.a = std::make_unique<DockNode>();
            if (!DeserializeDockNode(*node.a, layout, json, pos)) return false;
        } else if (key == "b" && pos < json.size() && json[pos] == '{') {
            node.b = std::make_unique<DockNode>();
            if (!DeserializeDockNode(*node.b, layout, json, pos)) return false;
        } else {
            ParseToken(json, pos);
        }
    }

    return true;
}

}
