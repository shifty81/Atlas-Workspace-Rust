#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ViewportRenderTarget ─────────────────────────────────────────

TEST_CASE("ViewportRenderTarget defaults invalid", "[Editor][S53]") {
    ViewportRenderTarget target;
    REQUIRE_FALSE(target.isValid());
    REQUIRE(target.width == 0);
    REQUIRE(target.height == 0);
}

TEST_CASE("ViewportRenderTarget resize and validate", "[Editor][S53]") {
    ViewportRenderTarget target;
    target.resize(1920, 1080);
    target.valid = true;
    REQUIRE(target.isValid());
    REQUIRE(target.width == 1920);
    REQUIRE(target.height == 1080);
}

TEST_CASE("ViewportRenderTarget aspectRatio", "[Editor][S53]") {
    ViewportRenderTarget target;
    target.resize(1920, 1080);
    float ratio = target.aspectRatio();
    REQUIRE(ratio > 1.7f);
    REQUIRE(ratio < 1.8f);
}

TEST_CASE("ViewportRenderTarget aspectRatio zero height", "[Editor][S53]") {
    ViewportRenderTarget target;
    target.resize(100, 0);
    REQUIRE(target.aspectRatio() == 1.f);
}

TEST_CASE("ViewportRenderTarget invalidate", "[Editor][S53]") {
    ViewportRenderTarget target;
    target.valid = true;
    target.resize(800, 600);
    target.invalidate();
    REQUIRE_FALSE(target.isValid());
}

// ── ViewportInputState ───────────────────────────────────────────

TEST_CASE("ViewportInputState defaults", "[Editor][S53]") {
    ViewportInputState input;
    REQUIRE_FALSE(input.anyButtonDown());
    REQUIRE_FALSE(input.hasFocus);
    REQUIRE_FALSE(input.hovered);
}

TEST_CASE("ViewportInputState anyButtonDown", "[Editor][S53]") {
    ViewportInputState input;
    input.leftButton = true;
    REQUIRE(input.anyButtonDown());
    input.leftButton = false;
    input.rightButton = true;
    REQUIRE(input.anyButtonDown());
}

// ── GizmoMode (reuses existing from EditorCamera.h) ──────────────

TEST_CASE("GizmoMode values", "[Editor][S53]") {
    REQUIRE(static_cast<uint8_t>(GizmoMode::Translate) == 0);
    REQUIRE(static_cast<uint8_t>(GizmoMode::Rotate) == 1);
    REQUIRE(static_cast<uint8_t>(GizmoMode::Scale) == 2);
}

TEST_CASE("GizmoSpace names", "[Editor][S53]") {
    REQUIRE(std::string(gizmoSpaceName(GizmoSpace::Local)) == "Local");
    REQUIRE(std::string(gizmoSpaceName(GizmoSpace::World)) == "World");
}

// ── Test viewport host implementation ────────────────────────────

class TestViewportHost : public IViewportHost {
public:
    void onAttach(const ViewportRenderTarget&) override { attached = true; }
    void onDetach() override { detached = true; }
    void onUpdate(float dt) override { totalDt += dt; updateCount++; }
    void onRender(const ViewportRenderTarget&) override { renderCount++; }
    void onResize(uint32_t w, uint32_t h) override { lastW = w; lastH = h; }
    void onInput(const ViewportInputState& s) override { lastInput = s; }
    const char* hostName() const override { return "TestHost"; }

    bool attached = false;
    bool detached = false;
    int updateCount = 0;
    int renderCount = 0;
    float totalDt = 0.f;
    uint32_t lastW = 0, lastH = 0;
    ViewportInputState lastInput;
};

// ── ViewportRenderHostRegistry ─────────────────────────────────────────

TEST_CASE("ViewportRenderHostRegistry starts empty", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    REQUIRE(reg.hostCount() == 0);
    REQUIRE(reg.hostIds().empty());
}

TEST_CASE("ViewportRenderHostRegistry registerHost", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    auto host = std::make_shared<TestViewportHost>();
    REQUIRE(reg.registerHost("test", host));
    REQUIRE(reg.hostCount() == 1);
    REQUIRE(reg.findHost("test") == host);
}

TEST_CASE("ViewportRenderHostRegistry registerHost duplicate fails", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    auto host = std::make_shared<TestViewportHost>();
    reg.registerHost("test", host);
    REQUIRE_FALSE(reg.registerHost("test", host));
}

TEST_CASE("ViewportRenderHostRegistry registerHost null fails", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    REQUIRE_FALSE(reg.registerHost("test", nullptr));
}

TEST_CASE("ViewportRenderHostRegistry registerHost empty id fails", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    auto host = std::make_shared<TestViewportHost>();
    REQUIRE_FALSE(reg.registerHost("", host));
}

TEST_CASE("ViewportRenderHostRegistry unregisterHost", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    auto host = std::make_shared<TestViewportHost>();
    reg.registerHost("test", host);
    REQUIRE(reg.unregisterHost("test"));
    REQUIRE(reg.hostCount() == 0);
    REQUIRE_FALSE(reg.unregisterHost("test"));
}

TEST_CASE("ViewportRenderHostRegistry hostIds", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    reg.registerHost("a", std::make_shared<TestViewportHost>());
    reg.registerHost("b", std::make_shared<TestViewportHost>());
    auto ids = reg.hostIds();
    REQUIRE(ids.size() == 2);
}

TEST_CASE("ViewportRenderHostRegistry activeHost", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    auto host = std::make_shared<TestViewportHost>();
    reg.registerHost("test", host);
    reg.setActiveHost("test");
    REQUIRE(reg.activeHostId() == "test");
    REQUIRE(reg.activeHost() == host);
}

TEST_CASE("ViewportRenderHostRegistry MAX_HOSTS limit", "[Editor][S53]") {
    ViewportRenderHostRegistry reg;
    for (size_t i = 0; i < ViewportRenderHostRegistry::MAX_HOSTS; ++i) {
        reg.registerHost("h" + std::to_string(i), std::make_shared<TestViewportHost>());
    }
    REQUIRE_FALSE(reg.registerHost("extra", std::make_shared<TestViewportHost>()));
}

// ── ViewportHostSession ──────────────────────────────────────────

TEST_CASE("ViewportHostSession starts detached", "[Editor][S53]") {
    ViewportHostSession session;
    REQUIRE_FALSE(session.isAttached());
    REQUIRE(session.host() == nullptr);
}

TEST_CASE("ViewportHostSession attach calls onAttach", "[Editor][S53]") {
    ViewportHostSession session;
    auto host = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.resize(800, 600);
    target.valid = true;
    session.attach(host, target);
    REQUIRE(session.isAttached());
    REQUIRE(host->attached);
}

TEST_CASE("ViewportHostSession detach calls onDetach", "[Editor][S53]") {
    ViewportHostSession session;
    auto host = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.valid = true;
    session.attach(host, target);
    session.detach();
    REQUIRE_FALSE(session.isAttached());
    REQUIRE(host->detached);
}

TEST_CASE("ViewportHostSession update calls host", "[Editor][S53]") {
    ViewportHostSession session;
    auto host = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.valid = true;
    session.attach(host, target);
    session.update(0.016f);
    REQUIRE(host->updateCount == 1);
}

TEST_CASE("ViewportHostSession render calls host", "[Editor][S53]") {
    ViewportHostSession session;
    auto host = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.valid = true;
    session.attach(host, target);
    session.render();
    REQUIRE(host->renderCount == 1);
}

TEST_CASE("ViewportHostSession resize calls host", "[Editor][S53]") {
    ViewportHostSession session;
    auto host = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.valid = true;
    session.attach(host, target);
    session.resize(1920, 1080);
    REQUIRE(host->lastW == 1920);
    REQUIRE(host->lastH == 1080);
    REQUIRE(session.target().width == 1920);
}

TEST_CASE("ViewportHostSession deliverInput calls host", "[Editor][S53]") {
    ViewportHostSession session;
    auto host = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.valid = true;
    session.attach(host, target);
    ViewportInputState input;
    input.mouseX = 100.f;
    input.leftButton = true;
    session.deliverInput(input);
    REQUIRE(host->lastInput.mouseX == 100.f);
    REQUIRE(host->lastInput.leftButton);
}

TEST_CASE("ViewportHostSession gizmo mode/space", "[Editor][S53]") {
    ViewportHostSession session;
    session.setGizmoMode(GizmoMode::Rotate);
    REQUIRE(session.gizmoMode() == GizmoMode::Rotate);
    session.setGizmoSpace(GizmoSpace::Local);
    REQUIRE(session.gizmoSpace() == GizmoSpace::Local);
}

TEST_CASE("ViewportHostSession reattach detaches old", "[Editor][S53]") {
    ViewportHostSession session;
    auto host1 = std::make_shared<TestViewportHost>();
    auto host2 = std::make_shared<TestViewportHost>();
    ViewportRenderTarget target;
    target.valid = true;
    session.attach(host1, target);
    session.attach(host2, target);
    REQUIRE(host1->detached);
    REQUIRE(host2->attached);
    REQUIRE(session.host() == host2);
}
