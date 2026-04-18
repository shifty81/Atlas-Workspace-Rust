#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/CurveEditor.h"

using namespace NF;

TEST_CASE("CurveType names cover all 8 values", "[Editor][S29]") {
    REQUIRE(std::string(curveTypeName(CurveType::Linear))     == "Linear");
    REQUIRE(std::string(curveTypeName(CurveType::Bezier))     == "Bezier");
    REQUIRE(std::string(curveTypeName(CurveType::Hermite))    == "Hermite");
    REQUIRE(std::string(curveTypeName(CurveType::CatmullRom)) == "CatmullRom");
    REQUIRE(std::string(curveTypeName(CurveType::Step))       == "Step");
    REQUIRE(std::string(curveTypeName(CurveType::Sine))       == "Sine");
    REQUIRE(std::string(curveTypeName(CurveType::Cosine))     == "Cosine");
    REQUIRE(std::string(curveTypeName(CurveType::Custom))     == "Custom");
}

TEST_CASE("CurveHandleMode names cover all 4 values", "[Editor][S29]") {
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Free))    == "Free");
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Aligned)) == "Aligned");
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Vector))  == "Vector");
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Auto))    == "Auto");
}

TEST_CASE("CurveControlPoint select and deselect toggles selected", "[Editor][S29]") {
    CurveControlPoint cp;
    cp.time = 1.0f; cp.value = 0.5f;

    REQUIRE_FALSE(cp.selected);
    cp.select();
    REQUIRE(cp.selected);
    cp.deselect();
    REQUIRE_FALSE(cp.selected);
}

TEST_CASE("CurveControlPoint setTime and setValue update fields", "[Editor][S29]") {
    CurveControlPoint cp;
    cp.setTime(3.0f);
    cp.setValue(0.8f);

    REQUIRE(cp.time  == Catch::Approx(3.0f));
    REQUIRE(cp.value == Catch::Approx(0.8f));
}

TEST_CASE("Curve addPoint and duplicate time rejection", "[Editor][S29]") {
    Curve curve("ease-out", CurveType::Bezier);

    CurveControlPoint a; a.time = 0.0f; a.value = 0.f;
    CurveControlPoint b; b.time = 1.0f; b.value = 1.f;
    CurveControlPoint dup; dup.time = 0.0f; dup.value = 9.f;

    REQUIRE(curve.addPoint(a));
    REQUIRE(curve.addPoint(b));
    REQUIRE_FALSE(curve.addPoint(dup));
    REQUIRE(curve.pointCount() == 2);
}

TEST_CASE("Curve removePoint returns correct result", "[Editor][S29]") {
    Curve curve("ramp", CurveType::Linear);

    CurveControlPoint cp; cp.time = 2.0f; cp.value = 1.0f;
    curve.addPoint(cp);

    REQUIRE(curve.removePoint(2.0f));
    REQUIRE_FALSE(curve.removePoint(2.0f));
    REQUIRE(curve.pointCount() == 0);
}

TEST_CASE("Curve findPoint returns correct pointer or nullptr", "[Editor][S29]") {
    Curve curve("sine-wave", CurveType::Sine);

    CurveControlPoint cp; cp.time = 0.5f; cp.value = 0.7f;
    curve.addPoint(cp);

    REQUIRE(curve.findPoint(0.5f) != nullptr);
    REQUIRE(curve.findPoint(0.5f)->value == Catch::Approx(0.7f));
    REQUIRE(curve.findPoint(9.9f) == nullptr);
}

TEST_CASE("Curve selectAll and deselectAll update selectedCount", "[Editor][S29]") {
    Curve curve("bounce", CurveType::Hermite);

    CurveControlPoint a; a.time = 0.0f;
    CurveControlPoint b; b.time = 0.5f;
    CurveControlPoint c; c.time = 1.0f;
    curve.addPoint(a);
    curve.addPoint(b);
    curve.addPoint(c);

    REQUIRE(curve.selectedCount() == 0);
    curve.selectAll();
    REQUIRE(curve.selectedCount() == 3);
    curve.deselectAll();
    REQUIRE(curve.selectedCount() == 0);
}

TEST_CASE("Curve duration returns max control point time", "[Editor][S29]") {
    Curve curve("fade", CurveType::CatmullRom);

    CurveControlPoint a; a.time = 0.0f;
    CurveControlPoint b; b.time = 5.0f;
    CurveControlPoint c; c.time = 2.5f;
    curve.addPoint(a);
    curve.addPoint(b);
    curve.addPoint(c);

    REQUIRE(curve.duration() == Catch::Approx(5.0f));
}

TEST_CASE("CurveEditorPanel addCurve and duplicate name rejection", "[Editor][S29]") {
    CurveEditorPanel panel;

    Curve ca("alpha", CurveType::Linear);
    Curve cb("beta",  CurveType::Bezier);
    Curve dup("alpha", CurveType::Step);

    REQUIRE(panel.addCurve(ca));
    REQUIRE(panel.addCurve(cb));
    REQUIRE_FALSE(panel.addCurve(dup));
    REQUIRE(panel.curveCount() == 2);
}

TEST_CASE("CurveEditorPanel removeCurve reduces count", "[Editor][S29]") {
    CurveEditorPanel panel;

    Curve c("my-curve", CurveType::Sine);
    panel.addCurve(c);

    REQUIRE(panel.curveCount() == 1);
    REQUIRE(panel.removeCurve("my-curve"));
    REQUIRE(panel.curveCount() == 0);
    REQUIRE_FALSE(panel.removeCurve("my-curve"));
}

TEST_CASE("CurveEditorPanel setActiveCurve and activeCurve", "[Editor][S29]") {
    CurveEditorPanel panel;

    Curve ca("speed",  CurveType::Linear);
    Curve cb("weight", CurveType::Hermite);
    panel.addCurve(ca);
    panel.addCurve(cb);

    REQUIRE(panel.activeCurve().empty());
    REQUIRE(panel.setActiveCurve("speed"));
    REQUIRE(panel.activeCurve() == "speed");
    REQUIRE_FALSE(panel.setActiveCurve("nonexistent"));
    REQUIRE(panel.activeCurve() == "speed"); // unchanged
}

TEST_CASE("CurveEditorPanel selectAll and deselectAll propagate to all curves", "[Editor][S29]") {
    CurveEditorPanel panel;

    Curve ca("ca", CurveType::Linear);
    CurveControlPoint p1; p1.time = 0.f; ca.addPoint(p1);

    Curve cb("cb", CurveType::Bezier);
    CurveControlPoint p2; p2.time = 1.f; cb.addPoint(p2);

    panel.addCurve(ca);
    panel.addCurve(cb);

    panel.selectAllPoints();
    REQUIRE(panel.findCurve("ca")->selectedCount() == 1);
    REQUIRE(panel.findCurve("cb")->selectedCount() == 1);

    panel.deselectAllPoints();
    REQUIRE(panel.findCurve("ca")->selectedCount() == 0);
    REQUIRE(panel.findCurve("cb")->selectedCount() == 0);
}

TEST_CASE("CurveEditorPanel looping flag set and read", "[Editor][S29]") {
    CurveEditorPanel panel;

    REQUIRE_FALSE(panel.isLooping());
    panel.setLooping(true);
    REQUIRE(panel.isLooping());
    panel.setLooping(false);
    REQUIRE_FALSE(panel.isLooping());
}

TEST_CASE("CurveEditorPanel MAX_CURVES limit is enforced", "[Editor][S29]") {
    CurveEditorPanel panel;

    for (size_t i = 0; i < CurveEditorPanel::MAX_CURVES; ++i) {
        Curve c("curve-" + std::to_string(i), CurveType::Linear);
        REQUIRE(panel.addCurve(c));
    }

    Curve overflow("overflow", CurveType::Custom);
    REQUIRE_FALSE(panel.addCurve(overflow));
    REQUIRE(panel.curveCount() == CurveEditorPanel::MAX_CURVES);
}
