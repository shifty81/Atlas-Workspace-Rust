#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI IDEPanel — code navigation and symbol search panel.
/// Replaces the legacy NF::Editor::IDEPanel for the AtlasUI framework.
class IDEPanel final : public PanelBase {
public:
    IDEPanel()
        : PanelBase("atlas.ide", "IDE") {}

    void paint(IPaintContext& context) override;

    void setSearchQuery(const std::string& query) { m_searchQuery = query; }
    [[nodiscard]] const std::string& searchQuery() const { return m_searchQuery; }

    struct SearchResult {
        std::string symbolName;
        std::string filePath;
        int line = 0;
    };

    void clearResults() { m_results.clear(); }
    void addResult(const std::string& symbol, const std::string& path, int line = 0) {
        m_results.push_back({symbol, path, line});
    }
    [[nodiscard]] const std::vector<SearchResult>& results() const { return m_results; }
    [[nodiscard]] size_t resultCount() const { return m_results.size(); }

private:
    std::string m_searchQuery;
    std::vector<SearchResult> m_results;
};

} // namespace NF::UI::AtlasUI
