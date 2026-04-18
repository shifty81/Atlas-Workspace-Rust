#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <functional>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI ContentBrowserPanel — displays project file system for asset browsing.
/// Replaces the legacy NF::Editor::ContentBrowserPanel for the AtlasUI framework.
class ContentBrowserPanel final : public PanelBase {
public:
    ContentBrowserPanel()
        : PanelBase("atlas.content_browser", "Content Browser") {}

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    void setCurrentPath(const std::string& path) { m_currentPath = path; }
    [[nodiscard]] const std::string& currentPath() const { return m_currentPath; }

    struct FileEntry {
        std::string name;
        bool isDirectory = false;
    };

    void clearEntries() { m_entries.clear(); }
    void addEntry(const std::string& name, bool isDirectory) {
        m_entries.push_back({name, isDirectory});
    }
    [[nodiscard]] const std::vector<FileEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    /// Called when the user clicks a file entry.
    using SelectCallback = std::function<void(const std::string& name, bool isDirectory)>;
    void setOnSelect(SelectCallback cb) { m_onSelect = std::move(cb); }

private:
    static constexpr float kScrollBarW = 10.f;
    static constexpr float kRowH       = 18.f;
    static constexpr float kHeaderH    = 22.f;
    static constexpr float kPathH      = 20.f; // current-path row
    static constexpr float kTopH       = kHeaderH + kPathH + 5.f; // separator+spacing

    std::string m_currentPath = "/";
    std::vector<FileEntry> m_entries;
    SelectCallback m_onSelect;

    float m_scrollOffset  = 0.f;
    bool  m_thumbDragging = false;
    float m_dragStartY    = 0.f;
    float m_dragStartOff  = 0.f;

    [[nodiscard]] float contentHeight() const { return static_cast<float>(m_entries.size()) * kRowH; }
    [[nodiscard]] float viewportHeight() const { return std::max(0.f, m_bounds.h - kTopH); }
    [[nodiscard]] float maxScroll() const { return std::max(0.f, contentHeight() - viewportHeight()); }
    [[nodiscard]] float thumbHeight() const {
        float vpH = viewportHeight();
        float ctH = contentHeight();
        if (ctH <= vpH) return vpH;
        return std::max(20.f, vpH * std::min(1.f, vpH / ctH));
    }
    [[nodiscard]] float thumbY() const {
        float track = viewportHeight() - thumbHeight();
        float frac  = (maxScroll() > 0.f) ? (m_scrollOffset / maxScroll()) : 0.f;
        return (m_bounds.y + kTopH) + track * frac;
    }
    void clampScroll() {
        m_scrollOffset = std::max(0.f, std::min(m_scrollOffset, maxScroll()));
    }
};

} // namespace NF::UI::AtlasUI
