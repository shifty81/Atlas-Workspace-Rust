// S158 editor tests: CurveLibraryV1, SplineEditorV1, GradientEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/GradientEditorV1.h"
#include "NF/Editor/SplineEditorV1.h"
#include "NF/Editor/CurveLibraryV1.h"

using namespace NF;
using Catch::Approx;

// ── CurveLibraryV1 ────────────────────────────────────────────────────────────

TEST_CASE("Clv1Curve validity", "[Editor][S158]") {
    Clv1Curve c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Ease";
    REQUIRE(c.isValid());
}

TEST_CASE("CurveLibraryV1 addCurve and curveCount", "[Editor][S158]") {
    CurveLibraryV1 lib;
    REQUIRE(lib.curveCount() == 0);
    Clv1Curve c; c.id = 1; c.name = "Bounce";
    REQUIRE(lib.addCurve(c));
    REQUIRE(lib.curveCount() == 1);
}

TEST_CASE("CurveLibraryV1 addCurve invalid fails", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c;
    REQUIRE(!lib.addCurve(c));
}

TEST_CASE("CurveLibraryV1 addCurve duplicate id fails", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c; c.id = 1; c.name = "A";
    lib.addCurve(c);
    REQUIRE(!lib.addCurve(c));
}

TEST_CASE("CurveLibraryV1 removeCurve", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c; c.id = 2; c.name = "Sine";
    lib.addCurve(c);
    REQUIRE(lib.removeCurve(2));
    REQUIRE(lib.curveCount() == 0);
    REQUIRE(!lib.removeCurve(2));
}

TEST_CASE("CurveLibraryV1 renameCurve", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c; c.id = 3; c.name = "Old";
    lib.addCurve(c);
    REQUIRE(lib.renameCurve(3, "New"));
    auto* found = lib.findCurve(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->name == "New");
}

TEST_CASE("CurveLibraryV1 renameCurve empty name fails", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c; c.id = 4; c.name = "X";
    lib.addCurve(c);
    REQUIRE(!lib.renameCurve(4, ""));
}

TEST_CASE("CurveLibraryV1 renameCurve unknown id fails", "[Editor][S158]") {
    CurveLibraryV1 lib;
    REQUIRE(!lib.renameCurve(99, "Y"));
}

TEST_CASE("CurveLibraryV1 addKeyframe", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c; c.id = 5; c.name = "Linear";
    lib.addCurve(c);
    Clv1Keyframe kf; kf.time = 0.5f; kf.value = 1.f;
    REQUIRE(lib.addKeyframe(5, kf));
    auto* found = lib.findCurve(5);
    REQUIRE(found != nullptr);
    REQUIRE(found->keys.size() == 1);
}

TEST_CASE("CurveLibraryV1 addKeyframe unknown curve fails", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Keyframe kf;
    REQUIRE(!lib.addKeyframe(999, kf));
}

TEST_CASE("CurveLibraryV1 onChange callback fires on rename", "[Editor][S158]") {
    CurveLibraryV1 lib;
    Clv1Curve c; c.id = 6; c.name = "Anim";
    lib.addCurve(c);
    uint64_t changedId = 0;
    lib.setOnChange([&](uint64_t id){ changedId = id; });
    lib.renameCurve(6, "NewAnim");
    REQUIRE(changedId == 6);
}

TEST_CASE("CurveLibraryV1 findCurve unknown returns null", "[Editor][S158]") {
    CurveLibraryV1 lib;
    REQUIRE(lib.findCurve(999) == nullptr);
}

// ── SplineEditorV1 ────────────────────────────────────────────────────────────

TEST_CASE("Sev1ControlPoint validity", "[Editor][S158]") {
    Sev1ControlPoint p;
    REQUIRE(!p.isValid());
    p.id = 1;
    REQUIRE(p.isValid());
}

TEST_CASE("SplineEditorV1 addPoint and pointCount", "[Editor][S158]") {
    SplineEditorV1 se;
    REQUIRE(se.pointCount() == 0);
    Sev1ControlPoint p; p.id = 1; p.x = 0.f; p.y = 0.f; p.z = 0.f;
    REQUIRE(se.addPoint(p));
    REQUIRE(se.pointCount() == 1);
}

TEST_CASE("SplineEditorV1 addPoint invalid fails", "[Editor][S158]") {
    SplineEditorV1 se;
    Sev1ControlPoint p;
    REQUIRE(!se.addPoint(p));
}

TEST_CASE("SplineEditorV1 addPoint duplicate id fails", "[Editor][S158]") {
    SplineEditorV1 se;
    Sev1ControlPoint p; p.id = 1;
    se.addPoint(p);
    REQUIRE(!se.addPoint(p));
}

TEST_CASE("SplineEditorV1 removePoint", "[Editor][S158]") {
    SplineEditorV1 se;
    Sev1ControlPoint p; p.id = 2;
    se.addPoint(p);
    REQUIRE(se.removePoint(2));
    REQUIRE(se.pointCount() == 0);
    REQUIRE(!se.removePoint(2));
}

TEST_CASE("SplineEditorV1 movePoint", "[Editor][S158]") {
    SplineEditorV1 se;
    Sev1ControlPoint p; p.id = 3; p.x = 0.f; p.y = 0.f; p.z = 0.f;
    se.addPoint(p);
    REQUIRE(se.movePoint(3, 1.f, 2.f, 3.f));
}

TEST_CASE("SplineEditorV1 movePoint unknown id fails", "[Editor][S158]") {
    SplineEditorV1 se;
    REQUIRE(!se.movePoint(99, 0.f, 0.f, 0.f));
}

TEST_CASE("SplineEditorV1 setSplineType and getSplineType", "[Editor][S158]") {
    SplineEditorV1 se;
    se.setSplineType(Sev1SplineType::Bezier);
    REQUIRE(se.splineType() == Sev1SplineType::Bezier);
}

TEST_CASE("SplineEditorV1 setLoopMode and loopMode", "[Editor][S158]") {
    SplineEditorV1 se;
    se.setLoopMode(Sev1LoopMode::Loop);
    REQUIRE(se.loopMode() == Sev1LoopMode::Loop);
}

TEST_CASE("SplineEditorV1 length with two points", "[Editor][S158]") {
    SplineEditorV1 se;
    Sev1ControlPoint p1; p1.id = 1; p1.x = 0.f; p1.y = 0.f; p1.z = 0.f;
    Sev1ControlPoint p2; p2.id = 2; p2.x = 3.f; p2.y = 4.f; p2.z = 0.f;
    se.addPoint(p1); se.addPoint(p2);
    REQUIRE(se.length() == Approx(5.f));
}

TEST_CASE("SplineEditorV1 length single point is zero", "[Editor][S158]") {
    SplineEditorV1 se;
    Sev1ControlPoint p; p.id = 1;
    se.addPoint(p);
    REQUIRE(se.length() == Approx(0.f));
}

TEST_CASE("SplineEditorV1 onChange fires on addPoint", "[Editor][S158]") {
    SplineEditorV1 se;
    bool changed = false;
    se.setOnChange([&](){ changed = true; });
    Sev1ControlPoint p; p.id = 1;
    se.addPoint(p);
    REQUIRE(changed);
}

// ── GradientEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Gev1ColorStop validity", "[Editor][S158]") {
    Gev1ColorStop s;
    REQUIRE(!s.isValid());
    s.id = 1; s.position = 0.f;
    REQUIRE(s.isValid());
}

TEST_CASE("Gev1ColorStop invalid position > 1 fails", "[Editor][S158]") {
    Gev1ColorStop s;
    s.id = 1; s.position = 1.5f;
    REQUIRE(!s.isValid());
}

TEST_CASE("GradientEditorV1 addStop and stopCount", "[Editor][S158]") {
    GradientEditorV1 ge;
    REQUIRE(ge.stopCount() == 0);
    Gev1ColorStop s; s.id = 1; s.position = 0.f;
    REQUIRE(ge.addStop(s));
    REQUIRE(ge.stopCount() == 1);
}

TEST_CASE("GradientEditorV1 addStop invalid fails", "[Editor][S158]") {
    GradientEditorV1 ge;
    Gev1ColorStop s;
    REQUIRE(!ge.addStop(s));
}

TEST_CASE("GradientEditorV1 addStop duplicate id fails", "[Editor][S158]") {
    GradientEditorV1 ge;
    Gev1ColorStop s; s.id = 1; s.position = 0.f;
    ge.addStop(s);
    REQUIRE(!ge.addStop(s));
}

TEST_CASE("GradientEditorV1 removeStop", "[Editor][S158]") {
    GradientEditorV1 ge;
    Gev1ColorStop s; s.id = 2; s.position = 0.5f;
    ge.addStop(s);
    REQUIRE(ge.removeStop(2));
    REQUIRE(ge.stopCount() == 0);
    REQUIRE(!ge.removeStop(2));
}

TEST_CASE("GradientEditorV1 moveStop", "[Editor][S158]") {
    GradientEditorV1 ge;
    Gev1ColorStop s; s.id = 3; s.position = 0.f;
    ge.addStop(s);
    REQUIRE(ge.moveStop(3, 0.8f));
}

TEST_CASE("GradientEditorV1 moveStop out-of-range fails", "[Editor][S158]") {
    GradientEditorV1 ge;
    Gev1ColorStop s; s.id = 4; s.position = 0.f;
    ge.addStop(s);
    REQUIRE(!ge.moveStop(4, 1.5f));
    REQUIRE(!ge.moveStop(4, -0.1f));
}

TEST_CASE("GradientEditorV1 moveStop unknown id fails", "[Editor][S158]") {
    GradientEditorV1 ge;
    REQUIRE(!ge.moveStop(99, 0.5f));
}

TEST_CASE("GradientEditorV1 setGradientType", "[Editor][S158]") {
    GradientEditorV1 ge;
    ge.setGradientType(Gev1GradientType::Radial);
    REQUIRE(ge.gradientType() == Gev1GradientType::Radial);
}

TEST_CASE("GradientEditorV1 setInterp", "[Editor][S158]") {
    GradientEditorV1 ge;
    ge.setInterp(Gev1Interp::Step);
    REQUIRE(ge.interp() == Gev1Interp::Step);
}

TEST_CASE("GradientEditorV1 onChange fires on addStop", "[Editor][S158]") {
    GradientEditorV1 ge;
    bool changed = false;
    ge.setOnChange([&](){ changed = true; });
    Gev1ColorStop s; s.id = 1; s.position = 0.f;
    ge.addStop(s);
    REQUIRE(changed);
}
