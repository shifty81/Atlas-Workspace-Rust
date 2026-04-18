#pragma once
// NF::Editor — Inspector panel
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
#include "NF/Workspace/AssetCatalog.h"

namespace NF {

class InspectorPanel : public EditorPanel {
public:
    InspectorPanel() = default;
    InspectorPanel(SelectionService* sel, TypeRegistry* reg)
        : m_selection(sel), m_typeRegistry(reg) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Inspector", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        if (m_selection && m_selection->hasSelection()) {
            EntityID id = m_selection->primarySelection();
            char idBuf[32];
            std::snprintf(idBuf, sizeof(idBuf), "Entity #%u", id);
            ui.drawText(bounds.x + 8.f, y, idBuf, theme.panelText);
            y += 20.f;
            ui.drawRect({bounds.x + 8.f, y, bounds.w - 16.f, 1.f}, theme.propertySeparator);
            y += 8.f;
            ui.drawText(bounds.x + 8.f, y, "Transform", theme.propertyLabel);
            y += 18.f;
            const char* axes[] = {"X: 0.00", "Y: 0.00", "Z: 0.00"};
            for (auto* a : axes) {
                ui.drawText(bounds.x + 16.f, y, a, theme.propertyValue);
                y += 16.f;
            }
        } else {
            ui.drawText(bounds.x + 8.f, y, "No entity selected", theme.propertyLabel);
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Inspector", bounds);
        if (m_selection && m_selection->hasSelection()) {
            char idBuf[32];
            std::snprintf(idBuf, sizeof(idBuf), "Entity #%u", m_selection->primarySelection());
            ctx.headerLabel(idBuf);
            ctx.separator();
            bool tExpanded = true;
            if (ctx.treeNode("Transform", tExpanded)) {
                ctx.indent(12.f);
                ctx.label("X: 0.00");
                ctx.label("Y: 0.00");
                ctx.label("Z: 0.00");
                ctx.unindent(12.f);
            }
        } else {
            ctx.label("No entity selected");
        }
        ctx.endPanel();
    }

    [[nodiscard]] SelectionService* selectionService() const { return m_selection; }
    void setSelectionService(SelectionService* s) { m_selection = s; }

    [[nodiscard]] TypeRegistry* typeRegistry() const { return m_typeRegistry; }
    void setTypeRegistry(TypeRegistry* r) { m_typeRegistry = r; }

    void setSelectedAsset(const NF::AssetDescriptor* desc) { m_selectedAsset = desc; }
    [[nodiscard]] const NF::AssetDescriptor* selectedAsset() const { return m_selectedAsset; }

private:
    std::string m_name = "Inspector";
    SelectionService* m_selection = nullptr;
    TypeRegistry* m_typeRegistry = nullptr;
    const NF::AssetDescriptor* m_selectedAsset = nullptr;
};

// ── HierarchyPanel ──────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::HierarchyPanel instead (U2).


} // namespace NF
