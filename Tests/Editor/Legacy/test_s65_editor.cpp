#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── GraphHostCapability ──────────────────────────────────────────

TEST_CASE("GraphHostCapability flags are distinct", "[Editor][S65]") {
    REQUIRE(GraphHostCapability::Undo      != GraphHostCapability::Redo);
    REQUIRE(GraphHostCapability::Zoom      != GraphHostCapability::Pan);
    REQUIRE(GraphHostCapability::Selection != GraphHostCapability::Minimap);
    REQUIRE(GraphHostCapability::Search    != GraphHostCapability::Validation);
    REQUIRE(GraphHostCapability::None      != GraphHostCapability::Undo);
}

TEST_CASE("GraphHostCapability or-combination and hasCapability", "[Editor][S65]") {
    auto caps = GraphHostCapability::Undo | GraphHostCapability::Zoom | GraphHostCapability::Selection;
    REQUIRE(hasCapability(caps, GraphHostCapability::Undo));
    REQUIRE(hasCapability(caps, GraphHostCapability::Zoom));
    REQUIRE(hasCapability(caps, GraphHostCapability::Selection));
    REQUIRE_FALSE(hasCapability(caps, GraphHostCapability::Minimap));
    REQUIRE_FALSE(hasCapability(caps, GraphHostCapability::Search));
}

TEST_CASE("GraphHostCapability None has no flags set", "[Editor][S65]") {
    REQUIRE_FALSE(hasCapability(GraphHostCapability::None, GraphHostCapability::Undo));
    REQUIRE_FALSE(hasCapability(GraphHostCapability::None, GraphHostCapability::Zoom));
}

// ── GraphViewState ───────────────────────────────────────────────

TEST_CASE("GraphViewState defaults", "[Editor][S65]") {
    GraphViewState vs;
    REQUIRE(vs.scrollX   == 0.f);
    REQUIRE(vs.scrollY   == 0.f);
    REQUIRE(vs.zoom      == 1.f);
    REQUIRE(vs.showGrid);
    REQUIRE_FALSE(vs.showMinimap);
}

TEST_CASE("GraphViewState isDefaultView", "[Editor][S65]") {
    GraphViewState vs;
    REQUIRE(vs.isDefaultView());
    vs.scrollX = 10.f;
    REQUIRE_FALSE(vs.isDefaultView());
}

TEST_CASE("GraphViewState reset returns to default", "[Editor][S65]") {
    GraphViewState vs;
    vs.scrollX = 200.f;
    vs.scrollY = 100.f;
    vs.zoom    = 2.0f;
    vs.reset();
    REQUIRE(vs.isDefaultView());
}

TEST_CASE("GraphViewState setZoom clamps to MIN_ZOOM", "[Editor][S65]") {
    GraphViewState vs;
    vs.setZoom(0.001f);
    REQUIRE(vs.zoom >= GraphViewState::MIN_ZOOM);
}

TEST_CASE("GraphViewState setZoom clamps to MAX_ZOOM", "[Editor][S65]") {
    GraphViewState vs;
    vs.setZoom(999.f);
    REQUIRE(vs.zoom <= GraphViewState::MAX_ZOOM);
}

TEST_CASE("GraphViewState setZoom preserves valid value", "[Editor][S65]") {
    GraphViewState vs;
    vs.setZoom(2.0f);
    REQUIRE(vs.zoom == 2.0f);
}

// ── GraphHostRegistry ────────────────────────────────────────────

// Minimal concrete IGraphHost for testing
struct TestGraphHost final : NF::IGraphHost {
    void onAttach() override { attached = true; }
    void onDetach() override { attached = false; }
    void onUpdate(float) override {}
    void onRender(const NF::Rect&) override {}
    void onNodeSelected(uint32_t id) override { lastSelectedNode = id; }
    void onConnected(uint32_t, uint32_t) override {}
    const char* hostName() const override { return "TestGraphHost"; }
    NF::GraphHostCapability capabilities() const override {
        return NF::GraphHostCapability::Undo | NF::GraphHostCapability::Selection;
    }
    size_t nodeCount() const override { return m_nodeCount; }
    bool isDirty() const override { return m_dirty; }

    bool attached = false;
    uint32_t lastSelectedNode = 0;
    size_t m_nodeCount = 0;
    bool m_dirty = false;
};

TEST_CASE("GraphHostRegistry starts empty", "[Editor][S65]") {
    GraphHostRegistry reg;
    REQUIRE(reg.hostCount() == 0);
    REQUIRE(reg.activeHostId().empty());
}

TEST_CASE("GraphHostRegistry registerHost succeeds", "[Editor][S65]") {
    GraphHostRegistry reg;
    auto host = std::make_shared<TestGraphHost>();
    REQUIRE(reg.registerHost("logic", host));
    REQUIRE(reg.hostCount() == 1);
}

TEST_CASE("GraphHostRegistry registerHost rejects null", "[Editor][S65]") {
    GraphHostRegistry reg;
    REQUIRE_FALSE(reg.registerHost("bad", nullptr));
    REQUIRE(reg.hostCount() == 0);
}

TEST_CASE("GraphHostRegistry registerHost rejects empty id", "[Editor][S65]") {
    GraphHostRegistry reg;
    auto host = std::make_shared<TestGraphHost>();
    REQUIRE_FALSE(reg.registerHost("", host));
}

TEST_CASE("GraphHostRegistry registerHost rejects duplicate id", "[Editor][S65]") {
    GraphHostRegistry reg;
    auto h1 = std::make_shared<TestGraphHost>();
    auto h2 = std::make_shared<TestGraphHost>();
    REQUIRE(reg.registerHost("graph", h1));
    REQUIRE_FALSE(reg.registerHost("graph", h2));
    REQUIRE(reg.hostCount() == 1);
}

TEST_CASE("GraphHostRegistry unregisterHost removes entry", "[Editor][S65]") {
    GraphHostRegistry reg;
    auto host = std::make_shared<TestGraphHost>();
    reg.registerHost("scene", host);
    REQUIRE(reg.unregisterHost("scene"));
    REQUIRE(reg.hostCount() == 0);
}

TEST_CASE("GraphHostRegistry findHost returns correct host", "[Editor][S65]") {
    GraphHostRegistry reg;
    auto host = std::make_shared<TestGraphHost>();
    reg.registerHost("behavior", host);
    auto found = reg.findHost("behavior");
    REQUIRE(found != nullptr);
    REQUIRE(std::string(found->hostName()) == "TestGraphHost");
}

TEST_CASE("GraphHostRegistry findHost returns nullptr for unknown id", "[Editor][S65]") {
    GraphHostRegistry reg;
    REQUIRE(reg.findHost("nonexistent") == nullptr);
}

TEST_CASE("GraphHostRegistry hostIds returns all ids", "[Editor][S65]") {
    GraphHostRegistry reg;
    reg.registerHost("a", std::make_shared<TestGraphHost>());
    reg.registerHost("b", std::make_shared<TestGraphHost>());
    auto ids = reg.hostIds();
    REQUIRE(ids.size() == 2);
    bool hasA = false, hasB = false;
    for (const auto& id : ids) {
        if (id == "a") hasA = true;
        if (id == "b") hasB = true;
    }
    REQUIRE(hasA);
    REQUIRE(hasB);
}

TEST_CASE("GraphHostRegistry setActiveHost and activeHost", "[Editor][S65]") {
    GraphHostRegistry reg;
    auto host = std::make_shared<TestGraphHost>();
    reg.registerHost("main", host);
    reg.setActiveHost("main");
    REQUIRE(reg.activeHostId() == "main");
    REQUIRE(reg.activeHost() != nullptr);
}

TEST_CASE("GraphHostRegistry activeHost returns nullptr when id not registered", "[Editor][S65]") {
    GraphHostRegistry reg;
    reg.setActiveHost("missing");
    REQUIRE(reg.activeHost() == nullptr);
}

TEST_CASE("GraphHostRegistry MAX_HOSTS enforced", "[Editor][S65]") {
    GraphHostRegistry reg;
    for (size_t i = 0; i < GraphHostRegistry::MAX_HOSTS; ++i)
        REQUIRE(reg.registerHost("h" + std::to_string(i), std::make_shared<TestGraphHost>()));
    REQUIRE_FALSE(reg.registerHost("overflow", std::make_shared<TestGraphHost>()));
    REQUIRE(reg.hostCount() == GraphHostRegistry::MAX_HOSTS);
}

// ── GraphHostSession ─────────────────────────────────────────────

TEST_CASE("GraphHostSession starts detached", "[Editor][S65]") {
    GraphHostSession session;
    REQUIRE_FALSE(session.isAttached());
    REQUIRE(session.host() == nullptr);
}

TEST_CASE("GraphHostSession attach calls onAttach on host", "[Editor][S65]") {
    GraphHostSession session;
    auto host = std::make_shared<TestGraphHost>();
    session.attach(host);
    REQUIRE(session.isAttached());
    REQUIRE(host->attached);
}

TEST_CASE("GraphHostSession detach calls onDetach on host", "[Editor][S65]") {
    GraphHostSession session;
    auto host = std::make_shared<TestGraphHost>();
    session.attach(host);
    session.detach();
    REQUIRE_FALSE(session.isAttached());
    REQUIRE_FALSE(host->attached);
    REQUIRE(session.host() == nullptr);
}

TEST_CASE("GraphHostSession selectNode updates selectedNodeId and calls host", "[Editor][S65]") {
    GraphHostSession session;
    auto host = std::make_shared<TestGraphHost>();
    session.attach(host);
    session.selectNode(42u);
    REQUIRE(session.selectedNodeId() == 42u);
    REQUIRE(host->lastSelectedNode == 42u);
}

TEST_CASE("GraphHostSession viewState zoom is adjustable", "[Editor][S65]") {
    GraphHostSession session;
    session.viewState().setZoom(1.5f);
    REQUIRE(session.viewState().zoom == 1.5f);
}

TEST_CASE("GraphHostSession re-attach replaces old host", "[Editor][S65]") {
    GraphHostSession session;
    auto h1 = std::make_shared<TestGraphHost>();
    auto h2 = std::make_shared<TestGraphHost>();
    session.attach(h1);
    REQUIRE(h1->attached);
    session.attach(h2);
    REQUIRE_FALSE(h1->attached);  // old host detached
    REQUIRE(h2->attached);
}
