#pragma once
// NF::Editor — Hierarchy panel
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

class HierarchyPanel : public EditorPanel {
public:
    HierarchyPanel() = default;
    explicit HierarchyPanel(SelectionService* sel) : m_selection(sel) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Left; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Hierarchy", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        // Search filter display
        if (!m_searchFilter.empty()) {
            ui.drawText(bounds.x + 8.f, y, "Filter: " + m_searchFilter, theme.propertyLabel);
            y += 18.f;
        }
        // Entity list
        for (auto id : m_entityList) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "Entity #%u", id);
            bool selected = m_selection && m_selection->isSelected(id);
            if (selected) {
                ui.drawRect({bounds.x, y, bounds.w, 18.f}, theme.selectionHighlight);
            }
            ui.drawText(bounds.x + 16.f, y + 2.f, buf,
                         selected ? theme.selectionBorder : theme.panelText);
            y += 18.f;
            if (y > bounds.y + bounds.h - 4.f) break;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Hierarchy", bounds);
        ctx.textInput("Search", m_searchFilter);
        ctx.separator();
        float contentH = static_cast<float>(m_entityList.size()) * 20.f;
        Rect scrollR{bounds.x, bounds.y + 60.f, bounds.w, bounds.h - 64.f};
        ctx.beginScrollArea("hierarchy_scroll", scrollR, contentH);
        for (auto id : m_entityList) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "Entity #%u", id);
            bool expanded = true;
            ctx.treeNode(buf, expanded);
        }
        ctx.endScrollArea();
        ctx.endPanel();
    }

    [[nodiscard]] SelectionService* selectionService() const { return m_selection; }
    void setSelectionService(SelectionService* s) { m_selection = s; }

    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }
    void setSearchFilter(const std::string& f) { m_searchFilter = f; }

    void setEntityList(const std::vector<EntityID>& ids) { m_entityList = ids; }
    [[nodiscard]] const std::vector<EntityID>& entityList() const { return m_entityList; }

private:
    std::string m_name = "Hierarchy";
    SelectionService* m_selection = nullptr;
    std::string m_searchFilter;
    std::vector<EntityID> m_entityList;
};

// ── ConsolePanel ─────────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::ConsolePanel instead (U4).


} // namespace NF
