#pragma once

#include "NF/Core/Core.h"
#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace NF::UI::AtlasUI {

class IWidget;

struct PanelState {
    std::string panelId;
    std::string title;
    bool visible = true;
    bool active = false;
    NF::Rect bounds{};
};

struct ILayoutContext {
    virtual ~ILayoutContext() = default;
    [[nodiscard]] virtual float dpiScale() const = 0;
    [[nodiscard]] virtual NF::Vec2 availableSize() const = 0;
    [[nodiscard]] virtual NF::Vec2 measureText(std::string_view text, float fontSize) const = 0;
    virtual void invalidateLayout() = 0;
};

struct IPaintContext {
    virtual ~IPaintContext() = default;
    virtual void drawRect(const NF::Rect& rect, Color color) = 0;
    virtual void fillRect(const NF::Rect& rect, Color color) = 0;
    virtual void drawText(const NF::Rect& rect, std::string_view text, FontId font, Color color) = 0;
    virtual void drawImage(const NF::Rect& rect, uint32_t textureId, Color tint) = 0;
    virtual void pushClip(const NF::Rect& rect) = 0;
    virtual void popClip() = 0;
    [[nodiscard]] virtual DrawList& drawList() = 0;
};

struct IInputContext {
    virtual ~IInputContext() = default;
    [[nodiscard]] virtual NF::Vec2 mousePosition() const = 0;
    [[nodiscard]] virtual bool primaryDown() const = 0;
    [[nodiscard]] virtual bool secondaryDown() const = 0;
    [[nodiscard]] virtual bool keyDown(int keyCode) const = 0;
    // Characters typed this frame (from the platform WM_CHAR / key event path).
    // '\b' represents Backspace, '\r' represents Enter/Return.
    [[nodiscard]] virtual std::string_view typedText() const = 0;
    // true only on the first frame the primary button transitions from up→down.
    // Defaults to false so existing test mocks need no changes.
    [[nodiscard]] virtual bool primaryPressed() const { return false; }
    // Mouse-wheel scroll delta for this frame (positive = scroll up/forward).
    // Defaults to 0 so existing test mocks need no changes.
    [[nodiscard]] virtual float scrollDelta() const { return 0.f; }
    virtual void requestFocus(IWidget* widget) = 0;
    virtual void capturePointer(IWidget* widget) = 0;
    virtual void releasePointer(IWidget* widget) = 0;
};

class IWidget {
public:
    virtual ~IWidget() = default;

    virtual void measure(ILayoutContext& context) = 0;
    virtual void arrange(const NF::Rect& bounds) = 0;
    virtual void paint(IPaintContext& context) = 0;
    virtual bool handleInput(IInputContext& context) = 0;

    virtual void setVisible(bool visible) = 0;
    [[nodiscard]] virtual bool isVisible() const = 0;
    [[nodiscard]] virtual NF::Rect bounds() const = 0;
};

class IPanel : public IWidget {
public:
    ~IPanel() override = default;

    [[nodiscard]] virtual const char* panelId() const = 0;
    [[nodiscard]] virtual const char* title() const = 0;

    virtual void initialize() = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;

    [[nodiscard]] virtual PanelState saveState() const = 0;
    virtual void loadState(const PanelState& state) = 0;
};

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void drawCommandBuffer(const DrawList& drawList) = 0;
};

class IWindowHost {
public:
    virtual ~IWindowHost() = default;

    virtual void attachPanel(std::shared_ptr<IPanel> panel) = 0;
    virtual void detachPanel(std::string_view panelId) = 0;
    [[nodiscard]] virtual std::shared_ptr<IPanel> findPanel(std::string_view panelId) const = 0;
};

class IPopupHost {
public:
    virtual ~IPopupHost() = default;
    virtual void openPopup(const NF::Rect& anchor, std::shared_ptr<IWidget> content) = 0;
    virtual void closePopup() = 0;
    [[nodiscard]] virtual bool isPopupOpen() const = 0;
};

} // namespace NF::UI::AtlasUI
