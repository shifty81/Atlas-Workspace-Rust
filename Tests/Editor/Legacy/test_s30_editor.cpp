#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/GradientEditor.h"

using namespace NF;

TEST_CASE("GradientType names cover all 8 values", "[Editor][S30]") {
    REQUIRE(std::string(gradientTypeName(GradientType::Linear))    == "Linear");
    REQUIRE(std::string(gradientTypeName(GradientType::Radial))    == "Radial");
    REQUIRE(std::string(gradientTypeName(GradientType::Angular))   == "Angular");
    REQUIRE(std::string(gradientTypeName(GradientType::Diamond))   == "Diamond");
    REQUIRE(std::string(gradientTypeName(GradientType::Square))    == "Square");
    REQUIRE(std::string(gradientTypeName(GradientType::Reflected)) == "Reflected");
    REQUIRE(std::string(gradientTypeName(GradientType::Conical))   == "Conical");
    REQUIRE(std::string(gradientTypeName(GradientType::Custom))    == "Custom");
}

TEST_CASE("GradientInterpolation names cover all 4 values", "[Editor][S30]") {
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Linear))   == "Linear");
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Step))     == "Step");
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Spline))   == "Spline");
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Constant)) == "Constant");
}

TEST_CASE("GradientColorStop select and deselect toggles selected", "[Editor][S30]") {
    GradientColorStop stop;
    stop.position = 0.5f;

    REQUIRE_FALSE(stop.selected);
    stop.select();
    REQUIRE(stop.selected);
    stop.deselect();
    REQUIRE_FALSE(stop.selected);
}

TEST_CASE("GradientColorStop setPosition updates position", "[Editor][S30]") {
    GradientColorStop stop;
    stop.setPosition(0.75f);
    REQUIRE(stop.position == Catch::Approx(0.75f));
}

TEST_CASE("GradientRamp addStop and duplicate position rejection", "[Editor][S30]") {
    GradientRamp ramp("sky", GradientType::Linear);

    GradientColorStop a; a.position = 0.0f;
    GradientColorStop b; b.position = 1.0f;
    GradientColorStop dup; dup.position = 0.0f;

    REQUIRE(ramp.addStop(a));
    REQUIRE(ramp.addStop(b));
    REQUIRE_FALSE(ramp.addStop(dup));
    REQUIRE(ramp.stopCount() == 2);
}

TEST_CASE("GradientRamp removeStop returns correct result", "[Editor][S30]") {
    GradientRamp ramp("sunset", GradientType::Radial);

    GradientColorStop s; s.position = 0.3f;
    ramp.addStop(s);

    REQUIRE(ramp.removeStop(0.3f));
    REQUIRE_FALSE(ramp.removeStop(0.3f));
    REQUIRE(ramp.stopCount() == 0);
}

TEST_CASE("GradientRamp findStop returns correct pointer or nullptr", "[Editor][S30]") {
    GradientRamp ramp("fire", GradientType::Conical);

    GradientColorStop s; s.position = 0.6f; s.r = 1.0f; s.g = 0.2f;
    ramp.addStop(s);

    REQUIRE(ramp.findStop(0.6f) != nullptr);
    REQUIRE(ramp.findStop(0.6f)->g == Catch::Approx(0.2f));
    REQUIRE(ramp.findStop(0.9f) == nullptr);
}

TEST_CASE("GradientRamp selectAll and deselectAll update selectedCount", "[Editor][S30]") {
    GradientRamp ramp("aurora", GradientType::Angular);

    GradientColorStop a; a.position = 0.0f;
    GradientColorStop b; b.position = 0.5f;
    GradientColorStop c; c.position = 1.0f;
    ramp.addStop(a);
    ramp.addStop(b);
    ramp.addStop(c);

    REQUIRE(ramp.selectedCount() == 0);
    ramp.selectAll();
    REQUIRE(ramp.selectedCount() == 3);
    ramp.deselectAll();
    REQUIRE(ramp.selectedCount() == 0);
}

TEST_CASE("GradientEditorPanel addRamp and duplicate name rejection", "[Editor][S30]") {
    GradientEditorPanel panel;

    GradientRamp ra("alpha", GradientType::Linear);
    GradientRamp rb("beta",  GradientType::Radial);
    GradientRamp dup("alpha", GradientType::Diamond);

    REQUIRE(panel.addRamp(ra));
    REQUIRE(panel.addRamp(rb));
    REQUIRE_FALSE(panel.addRamp(dup));
    REQUIRE(panel.rampCount() == 2);
}

TEST_CASE("GradientEditorPanel removeRamp reduces count", "[Editor][S30]") {
    GradientEditorPanel panel;

    GradientRamp r("my-ramp", GradientType::Square);
    panel.addRamp(r);

    REQUIRE(panel.rampCount() == 1);
    REQUIRE(panel.removeRamp("my-ramp"));
    REQUIRE(panel.rampCount() == 0);
    REQUIRE_FALSE(panel.removeRamp("my-ramp"));
}

TEST_CASE("GradientEditorPanel setActiveRamp and activeRamp", "[Editor][S30]") {
    GradientEditorPanel panel;

    GradientRamp ra("warm", GradientType::Linear);
    GradientRamp rb("cool", GradientType::Reflected);
    panel.addRamp(ra);
    panel.addRamp(rb);

    REQUIRE(panel.activeRamp().empty());
    REQUIRE(panel.setActiveRamp("warm"));
    REQUIRE(panel.activeRamp() == "warm");
    REQUIRE_FALSE(panel.setActiveRamp("nonexistent"));
    REQUIRE(panel.activeRamp() == "warm"); // unchanged
}

TEST_CASE("GradientEditorPanel selectAllStops and deselectAllStops propagate to all ramps", "[Editor][S30]") {
    GradientEditorPanel panel;

    GradientRamp ra("ra", GradientType::Linear);
    GradientColorStop p1; p1.position = 0.f; ra.addStop(p1);

    GradientRamp rb("rb", GradientType::Radial);
    GradientColorStop p2; p2.position = 0.5f; rb.addStop(p2);

    panel.addRamp(ra);
    panel.addRamp(rb);

    panel.selectAllStops();
    REQUIRE(panel.findRamp("ra")->selectedCount() == 1);
    REQUIRE(panel.findRamp("rb")->selectedCount() == 1);

    panel.deselectAllStops();
    REQUIRE(panel.findRamp("ra")->selectedCount() == 0);
    REQUIRE(panel.findRamp("rb")->selectedCount() == 0);
}

TEST_CASE("GradientEditorPanel symmetric flag set and read", "[Editor][S30]") {
    GradientEditorPanel panel;

    REQUIRE_FALSE(panel.isSymmetric());
    panel.setSymmetric(true);
    REQUIRE(panel.isSymmetric());
    panel.setSymmetric(false);
    REQUIRE_FALSE(panel.isSymmetric());
}

TEST_CASE("GradientEditorPanel MAX_RAMPS limit is enforced", "[Editor][S30]") {
    GradientEditorPanel panel;

    for (size_t i = 0; i < GradientEditorPanel::MAX_RAMPS; ++i) {
        GradientRamp r("ramp-" + std::to_string(i), GradientType::Linear);
        REQUIRE(panel.addRamp(r));
    }

    GradientRamp overflow("overflow", GradientType::Custom);
    REQUIRE_FALSE(panel.addRamp(overflow));
    REQUIRE(panel.rampCount() == GradientEditorPanel::MAX_RAMPS);
}

TEST_CASE("GradientEditorPanel removeRamp clears activeRamp if active", "[Editor][S30]") {
    GradientEditorPanel panel;

    GradientRamp r("active-ramp", GradientType::Conical);
    panel.addRamp(r);
    panel.setActiveRamp("active-ramp");
    REQUIRE(panel.activeRamp() == "active-ramp");

    panel.removeRamp("active-ramp");
    REQUIRE(panel.activeRamp().empty());
}
