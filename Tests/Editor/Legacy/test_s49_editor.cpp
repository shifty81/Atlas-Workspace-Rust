#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── PipelineMonitorPanel basics ─────────────────────────────────

TEST_CASE("PipelineMonitorPanel name returns PipelineMonitor", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    REQUIRE(panel.name() == "PipelineMonitor");
}

TEST_CASE("PipelineMonitorPanel slot returns DockSlot::Bottom", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    REQUIRE(panel.slot() == DockSlot::Bottom);
}

TEST_CASE("PipelineMonitorPanel inherits EditorPanel", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base->name() == "PipelineMonitor");
}

// ── Event management ────────────────────────────────────────────

TEST_CASE("PipelineMonitorPanel starts with zero events", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    REQUIRE(panel.eventCount() == 0);
    REQUIRE(panel.events().empty());
}

TEST_CASE("PipelineMonitorPanel addEvent increases count", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    panel.addEvent("Compile", "ShaderPipeline", "vertex.glsl compiled", 0.1f);
    REQUIRE(panel.eventCount() == 1);

    panel.addEvent("Link", "ShaderPipeline", "program linked", 0.2f);
    REQUIRE(panel.eventCount() == 2);
}

TEST_CASE("PipelineMonitorPanel events() returns entries in order", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    panel.addEvent("A", "src1", "detail1", 1.0f);
    panel.addEvent("B", "src2", "detail2", 2.0f);
    panel.addEvent("C", "src3", "detail3", 3.0f);

    const auto& evts = panel.events();
    REQUIRE(evts.size() == 3);
    REQUIRE(evts[0].type == "A");
    REQUIRE(evts[1].source == "src2");
    REQUIRE(evts[2].details == "detail3");
    REQUIRE(evts[2].timestamp == 3.0f);
}

TEST_CASE("PipelineMonitorPanel clearEvents empties the list", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    panel.addEvent("X", "s", "d", 0.f);
    panel.addEvent("Y", "s", "d", 0.f);
    REQUIRE(panel.eventCount() == 2);

    panel.clearEvents();
    REQUIRE(panel.eventCount() == 0);
    REQUIRE(panel.events().empty());
}

// ── 500-event cap ───────────────────────────────────────────────

TEST_CASE("PipelineMonitorPanel caps events at 500", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    for (int i = 0; i < 501; ++i) {
        panel.addEvent("Evt", "src", "detail " + std::to_string(i),
                       static_cast<float>(i));
    }
    REQUIRE(panel.eventCount() == 500);

    // First event should have been evicted; newest should be present
    const auto& evts = panel.events();
    REQUIRE(evts.front().details == "detail 1");
    REQUIRE(evts.back().details == "detail 500");
}

TEST_CASE("PipelineMonitorPanel events stay at 500 after more additions", "[Editor][S49]") {
    PipelineMonitorPanel panel;
    for (int i = 0; i < 600; ++i) {
        panel.addEvent("T", "S", "d" + std::to_string(i), 0.f);
    }
    REQUIRE(panel.eventCount() == 500);
}
