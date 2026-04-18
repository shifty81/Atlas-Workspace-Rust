#pragma once
// NF::Editor — Content browser panel
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
#include "NF/Editor/ContentBrowser.h"
#include "NF/Workspace/AssetCatalog.h"

namespace NF {

enum class ContentViewMode : uint8_t { Grid, List };

class ContentBrowserPanel : public EditorPanel {
public:
    ContentBrowserPanel() = default;
    explicit ContentBrowserPanel(ContentBrowser* browser) : m_browser(browser) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Bottom; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Content Browser", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        if (m_browser) {
            ui.drawText(bounds.x + 8.f, y, m_browser->currentPath(), theme.propertyLabel);
            y += 18.f;
            for (auto& entry : m_browser->entries()) {
                if (y > bounds.y + bounds.h - 4.f) break;
                std::string icon = entry.isDirectory ? "[D] " : "[F] ";
                ui.drawText(bounds.x + 16.f, y, icon + entry.name, theme.panelText);
                y += 18.f;
            }
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Content Browser", bounds);
        if (m_browser) {
            ctx.label(m_browser->currentPath());
            ctx.beginHorizontal();
            if (ctx.button("Up")) m_browser->navigateUp();
            ctx.endHorizontal();
            ctx.separator();
            for (auto& entry : m_browser->entries()) {
                std::string icon = entry.isDirectory ? "[D] " : "[F] ";
                if (ctx.button(icon + entry.name)) {
                    if (entry.isDirectory)
                        m_browser->navigateTo(entry.name);
                }
            }
        }
        ctx.endPanel();
    }

    [[nodiscard]] ContentBrowser* contentBrowser() const { return m_browser; }
    void setContentBrowser(ContentBrowser* b) { m_browser = b; }

    [[nodiscard]] ContentViewMode viewMode() const { return m_viewMode; }
    void setViewMode(ContentViewMode m) { m_viewMode = m; }

    void setAssetCatalog(const NF::AssetCatalog* catalog) { m_assetCatalog = catalog; }
    [[nodiscard]] const NF::AssetCatalog* assetCatalog() const { return m_assetCatalog; }

private:
    std::string m_name = "ContentBrowser";
    ContentBrowser* m_browser = nullptr;
    ContentViewMode m_viewMode = ContentViewMode::Grid;
    const NF::AssetCatalog* m_assetCatalog = nullptr;
};

// ── EditorToolbar ────────────────────────────────────────────────


} // namespace NF
