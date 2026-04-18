#pragma once
// NF::Editor — IDE integration (indexer, navigator, IDE panel)
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

enum class SourceFileType : uint8_t {
    Header, Source, Shader, Script, Data, Config, Unknown
};

struct IndexedFile {
    std::string path;
    SourceFileType fileType = SourceFileType::Unknown;
    std::string moduleName;
    uint32_t lineCount = 0;
    uint64_t lastModified = 0;
    std::vector<std::string> symbols;
};

class ProjectIndexer {
public:
    void indexDirectory(const std::string& rootPath) {
        namespace fs = std::filesystem;
        if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
            NF_LOG_WARN("IDE", "Directory not found: " + rootPath);
            return;
        }
        for (auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            SourceFileType ft = classifyExtension(ext);
            auto rel = fs::relative(entry.path(), rootPath).string();
            std::string mod;
            auto sep = rel.find_first_of("/\\");
            if (sep != std::string::npos) mod = rel.substr(0, sep);
            IndexedFile f;
            f.path = entry.path().string();
            f.fileType = ft;
            f.moduleName = mod;
            f.lineCount = 0;
            f.lastModified = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    entry.last_write_time().time_since_epoch()).count());
            m_files.push_back(std::move(f));
        }
        NF_LOG_INFO("IDE", "Indexed " + std::to_string(m_files.size()) + " files from " + rootPath);
    }

    void indexFile(const std::string& path, SourceFileType type, const std::string& moduleName) {
        IndexedFile f;
        f.path = path;
        f.fileType = type;
        f.moduleName = moduleName;
        m_files.push_back(std::move(f));
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByType(SourceFileType type) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.fileType == type) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByModule(const std::string& moduleName) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.moduleName == moduleName) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByName(const std::string& nameSubstring) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.path.find(nameSubstring) != std::string::npos) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] const std::vector<IndexedFile>& allFiles() const { return m_files; }
    [[nodiscard]] size_t fileCount() const { return m_files.size(); }

    void clear() { m_files.clear(); }

    void addSymbol(const std::string& filePath, const std::string& symbolName) {
        for (auto& f : m_files) {
            if (f.path == filePath) {
                f.symbols.push_back(symbolName);
                return;
            }
        }
    }

    [[nodiscard]] std::vector<const IndexedFile*> findSymbol(const std::string& symbolName) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            for (auto& s : f.symbols) {
                if (s == symbolName) {
                    result.push_back(&f);
                    break;
                }
            }
        }
        return result;
    }

private:
    static SourceFileType classifyExtension(const std::string& ext) {
        if (ext == ".h" || ext == ".hpp") return SourceFileType::Header;
        if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") return SourceFileType::Source;
        if (ext == ".glsl" || ext == ".hlsl") return SourceFileType::Shader;
        if (ext == ".lua" || ext == ".py") return SourceFileType::Script;
        if (ext == ".json") return SourceFileType::Data;
        if (ext == ".cfg" || ext == ".ini") return SourceFileType::Config;
        return SourceFileType::Unknown;
    }

    std::vector<IndexedFile> m_files;
};

// ── Code Navigation ─────────────────────────────────────────────

enum class SymbolKind : uint8_t {
    Function, Class, Struct, Enum, Variable, Namespace, Macro, Type, Unknown
};

struct NavigationTarget {
    std::string filePath;
    uint32_t line = 0;
    uint32_t column = 0;
    std::string symbolName;
    SymbolKind kind = SymbolKind::Unknown;
};

struct NavigationEntry {
    std::string symbol;
    SymbolKind kind = SymbolKind::Unknown;
    std::string filePath;
    uint32_t line = 0;
};

class CodeNavigator {
public:
    void addEntry(NavigationEntry entry) {
        m_entries.push_back(std::move(entry));
    }

    [[nodiscard]] std::optional<NavigationTarget> goToDefinition(const std::string& symbolName) const {
        for (auto& e : m_entries) {
            if (e.symbol == symbolName) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                return t;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<NavigationTarget> findReferences(const std::string& symbolName) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.symbol == symbolName) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<NavigationTarget> findSymbolsByKind(SymbolKind kind) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.kind == kind) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<NavigationTarget> searchSymbols(const std::string& query) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.symbol.find(query) != std::string::npos) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    void clear() { m_entries.clear(); }

private:
    std::vector<NavigationEntry> m_entries;
};

// ── Breadcrumb Trail ─────────────────────────────────────────────

struct BreadcrumbItem {
    std::string label;
    std::string filePath;
    uint32_t line = 0;
};

class BreadcrumbTrail {
public:
    void push(BreadcrumbItem item) {
        if (m_trail.size() >= maxDepth) {
            m_trail.erase(m_trail.begin());
        }
        m_trail.push_back(std::move(item));
    }

    std::optional<BreadcrumbItem> pop() {
        if (m_trail.empty()) return std::nullopt;
        auto item = std::move(m_trail.back());
        m_trail.pop_back();
        return item;
    }

    [[nodiscard]] const BreadcrumbItem* current() const {
        if (m_trail.empty()) return nullptr;
        return &m_trail.back();
    }

    [[nodiscard]] const std::vector<BreadcrumbItem>& trail() const { return m_trail; }
    [[nodiscard]] size_t depth() const { return m_trail.size(); }
    void clear() { m_trail.clear(); }

private:
    std::vector<BreadcrumbItem> m_trail;
    static constexpr size_t maxDepth = 50;
};

// ── IDE Panel ────────────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::IDEPanel instead (U5).

class IDEPanel : public EditorPanel {
public:
    IDEPanel(ProjectIndexer* indexer, CodeNavigator* navigator)
        : m_indexer(indexer), m_navigator(navigator) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "IDE", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 30.f;
        if (!searchQuery.empty()) {
            ui.drawText(bounds.x + 8.f, y, "Search: " + searchQuery, theme.propertyLabel);
            y += 18.f;
        }
        for (auto& r : searchResults) {
            if (y > bounds.y + bounds.h - 4.f) break;
            ui.drawText(bounds.x + 8.f, y, r.symbolName + " @ " + r.filePath, theme.panelText);
            y += 16.f;
        }
    }

    [[nodiscard]] ProjectIndexer* indexer() const { return m_indexer; }
    [[nodiscard]] CodeNavigator* navigator() const { return m_navigator; }

    std::string searchQuery;
    std::vector<NavigationTarget> searchResults;

private:
    std::string m_name = "IDE";
    ProjectIndexer* m_indexer = nullptr;
    CodeNavigator* m_navigator = nullptr;
};

// ── IDE Service ──────────────────────────────────────────────────

class IDEService {
public:
    void init() {
        m_indexer = ProjectIndexer{};
        m_navigator = CodeNavigator{};
        m_breadcrumbs = BreadcrumbTrail{};
        m_initialized = true;
        NF_LOG_INFO("IDE", "IDEService initialized");
    }

    void shutdown() {
        m_indexer.clear();
        m_navigator.clear();
        m_breadcrumbs.clear();
        m_initialized = false;
        NF_LOG_INFO("IDE", "IDEService shutdown");
    }

    [[nodiscard]] ProjectIndexer& indexer() { return m_indexer; }
    [[nodiscard]] CodeNavigator& navigator() { return m_navigator; }
    [[nodiscard]] BreadcrumbTrail& breadcrumbs() { return m_breadcrumbs; }

    void navigateTo(const std::string& filePath, uint32_t line, const std::string& symbolName) {
        BreadcrumbItem item;
        item.label = symbolName;
        item.filePath = filePath;
        item.line = line;
        m_breadcrumbs.push(std::move(item));
        NF_LOG_INFO("IDE", "Navigate to " + symbolName + " at " + filePath + ":" + std::to_string(line));
    }

    bool goBack() {
        auto item = m_breadcrumbs.pop();
        if (!item) return false;
        NF_LOG_INFO("IDE", "Go back to " + item->label);
        return true;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

private:
    ProjectIndexer m_indexer;
    CodeNavigator m_navigator;
    BreadcrumbTrail m_breadcrumbs;
    bool m_initialized = false;
};

// ── MenuBar ──────────────────────────────────────────────────────


} // namespace NF
