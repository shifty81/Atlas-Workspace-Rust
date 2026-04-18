#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/GradientEditor.h"
#include "NF/Editor/CurveEditor.h"

using namespace NF;

// ── S83: CurveEditor + FontEditor + GradientEditor ───────────────

// ── CurveEditor ──────────────────────────────────────────────────

TEST_CASE("CurveType names are correct", "[Editor][S83]") {
    REQUIRE(std::string(curveTypeName(CurveType::Linear))     == "Linear");
    REQUIRE(std::string(curveTypeName(CurveType::Bezier))     == "Bezier");
    REQUIRE(std::string(curveTypeName(CurveType::Hermite))    == "Hermite");
    REQUIRE(std::string(curveTypeName(CurveType::CatmullRom)) == "CatmullRom");
    REQUIRE(std::string(curveTypeName(CurveType::Step))       == "Step");
    REQUIRE(std::string(curveTypeName(CurveType::Sine))       == "Sine");
    REQUIRE(std::string(curveTypeName(CurveType::Cosine))     == "Cosine");
    REQUIRE(std::string(curveTypeName(CurveType::Custom))     == "Custom");
}

TEST_CASE("CurveHandleMode names are correct", "[Editor][S83]") {
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Free))    == "Free");
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Aligned)) == "Aligned");
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Vector))  == "Vector");
    REQUIRE(std::string(curveHandleModeName(CurveHandleMode::Auto))    == "Auto");
}

TEST_CASE("CurveControlPoint select deselect and set", "[Editor][S83]") {
    CurveControlPoint cp;
    cp.setTime(1.0f);
    cp.setValue(0.5f);
    REQUIRE_FALSE(cp.selected);
    cp.select();
    REQUIRE(cp.selected);
    cp.deselect();
    REQUIRE_FALSE(cp.selected);
    REQUIRE(cp.time == 1.0f);
    REQUIRE(cp.value == 0.5f);
}

TEST_CASE("Curve addPoint and pointCount", "[Editor][S83]") {
    Curve curve("speed", CurveType::Bezier);
    CurveControlPoint cp; cp.setTime(0.0f); cp.setValue(0.0f);
    REQUIRE(curve.addPoint(cp));
    REQUIRE(curve.pointCount() == 1);
}

TEST_CASE("Curve addPoint rejects duplicate time", "[Editor][S83]") {
    Curve curve("speed", CurveType::Linear);
    CurveControlPoint cp; cp.setTime(0.5f);
    curve.addPoint(cp);
    REQUIRE_FALSE(curve.addPoint(cp));
    REQUIRE(curve.pointCount() == 1);
}

TEST_CASE("Curve removePoint removes entry", "[Editor][S83]") {
    Curve curve("speed", CurveType::Linear);
    CurveControlPoint cp; cp.setTime(1.0f);
    curve.addPoint(cp);
    REQUIRE(curve.removePoint(1.0f));
    REQUIRE(curve.pointCount() == 0);
}

TEST_CASE("Curve selectAll and deselectAll", "[Editor][S83]") {
    Curve curve("alpha", CurveType::Linear);
    CurveControlPoint p1; p1.setTime(0.f);
    CurveControlPoint p2; p2.setTime(1.f);
    curve.addPoint(p1); curve.addPoint(p2);
    curve.selectAll();
    REQUIRE(curve.selectedCount() == 2);
    curve.deselectAll();
    REQUIRE(curve.selectedCount() == 0);
}

TEST_CASE("Curve duration is max point time", "[Editor][S83]") {
    Curve curve("pos", CurveType::Bezier);
    CurveControlPoint p1; p1.setTime(0.f);
    CurveControlPoint p2; p2.setTime(4.f);
    CurveControlPoint p3; p3.setTime(2.f);
    curve.addPoint(p1); curve.addPoint(p2); curve.addPoint(p3);
    REQUIRE(curve.duration() == 4.0f);
}

TEST_CASE("CurveEditorPanel addCurve and curveCount", "[Editor][S83]") {
    CurveEditorPanel panel;
    Curve c("alpha", CurveType::Linear);
    REQUIRE(panel.addCurve(c));
    REQUIRE(panel.curveCount() == 1);
}

TEST_CASE("CurveEditorPanel addCurve rejects duplicate name", "[Editor][S83]") {
    CurveEditorPanel panel;
    Curve c("speed", CurveType::Bezier);
    panel.addCurve(c);
    REQUIRE_FALSE(panel.addCurve(c));
}

TEST_CASE("CurveEditorPanel removeCurve removes entry", "[Editor][S83]") {
    CurveEditorPanel panel;
    Curve c("fade", CurveType::Sine);
    panel.addCurve(c);
    REQUIRE(panel.removeCurve("fade"));
    REQUIRE(panel.curveCount() == 0);
}

TEST_CASE("CurveEditorPanel setActiveCurve updates active", "[Editor][S83]") {
    CurveEditorPanel panel;
    Curve c("ease", CurveType::CatmullRom);
    panel.addCurve(c);
    REQUIRE(panel.setActiveCurve("ease"));
    REQUIRE(panel.activeCurve() == "ease");
}

TEST_CASE("CurveEditorPanel setLooping", "[Editor][S83]") {
    CurveEditorPanel panel;
    REQUIRE_FALSE(panel.isLooping());
    panel.setLooping(true);
    REQUIRE(panel.isLooping());
}

TEST_CASE("CurveEditorPanel MAX_CURVES is 32", "[Editor][S83]") {
    REQUIRE(CurveEditorPanel::MAX_CURVES == 32);
}

// ── FontEditor ───────────────────────────────────────────────────

TEST_CASE("FontStyle names are correct", "[Editor][S83]") {
    REQUIRE(std::string(fontStyleName(FontStyle::Normal))  == "Normal");
    REQUIRE(std::string(fontStyleName(FontStyle::Italic))  == "Italic");
    REQUIRE(std::string(fontStyleName(FontStyle::Oblique)) == "Oblique");
    REQUIRE(std::string(fontStyleName(FontStyle::Inherit)) == "Inherit");
}

TEST_CASE("FontWeight names are correct", "[Editor][S83]") {
    REQUIRE(std::string(fontWeightName(FontWeight::Thin))       == "Thin");
    REQUIRE(std::string(fontWeightName(FontWeight::ExtraLight)) == "ExtraLight");
    REQUIRE(std::string(fontWeightName(FontWeight::Light))      == "Light");
    REQUIRE(std::string(fontWeightName(FontWeight::Regular))    == "Regular");
    REQUIRE(std::string(fontWeightName(FontWeight::Medium))     == "Medium");
    REQUIRE(std::string(fontWeightName(FontWeight::Bold))       == "Bold");
}

TEST_CASE("FontVariant names are correct", "[Editor][S83]") {
    REQUIRE(std::string(fontVariantName(FontVariant::Normal))       == "Normal");
    REQUIRE(std::string(fontVariantName(FontVariant::SmallCaps))    == "SmallCaps");
    REQUIRE(std::string(fontVariantName(FontVariant::AllSmallCaps)) == "AllSmallCaps");
    REQUIRE(std::string(fontVariantName(FontVariant::PetiteCaps))   == "PetiteCaps");
}

TEST_CASE("FontAsset default style and weight", "[Editor][S83]") {
    FontAsset font("Roboto", 14.0f);
    REQUIRE(font.family() == "Roboto");
    REQUIRE(font.size() == 14.0f);
    REQUIRE(font.style() == FontStyle::Normal);
    REQUIRE(font.weight() == FontWeight::Regular);
    REQUIRE(font.variant() == FontVariant::Normal);
    REQUIRE_FALSE(font.isBold());
    REQUIRE_FALSE(font.isItalic());
}

TEST_CASE("FontAsset isBold for Bold and Medium weights", "[Editor][S83]") {
    FontAsset font("Arial", 12.0f);
    font.setWeight(FontWeight::Bold);
    REQUIRE(font.isBold());
    font.setWeight(FontWeight::Medium);
    REQUIRE(font.isBold());
    font.setWeight(FontWeight::Light);
    REQUIRE_FALSE(font.isBold());
}

TEST_CASE("FontAsset isItalic for Italic and Oblique styles", "[Editor][S83]") {
    FontAsset font("Times", 12.0f);
    font.setStyle(FontStyle::Italic);
    REQUIRE(font.isItalic());
    font.setStyle(FontStyle::Oblique);
    REQUIRE(font.isItalic());
    font.setStyle(FontStyle::Normal);
    REQUIRE_FALSE(font.isItalic());
}

TEST_CASE("FontEditor addFont and fontCount", "[Editor][S83]") {
    FontEditor editor;
    FontAsset font("Roboto", 14.0f);
    REQUIRE(editor.addFont(font));
    REQUIRE(editor.fontCount() == 1);
}

TEST_CASE("FontEditor addFont rejects duplicate family", "[Editor][S83]") {
    FontEditor editor;
    FontAsset font("Roboto", 14.0f);
    editor.addFont(font);
    REQUIRE_FALSE(editor.addFont(font));
}

TEST_CASE("FontEditor removeFont removes entry", "[Editor][S83]") {
    FontEditor editor;
    FontAsset font("Inter", 12.0f);
    editor.addFont(font);
    REQUIRE(editor.removeFont("Inter"));
    REQUIRE(editor.fontCount() == 0);
}

TEST_CASE("FontEditor setActiveFont updates active", "[Editor][S83]") {
    FontEditor editor;
    FontAsset font("Ubuntu", 16.0f);
    editor.addFont(font);
    REQUIRE(editor.setActiveFont("Ubuntu"));
    REQUIRE(editor.activeFont() == "Ubuntu");
}

TEST_CASE("FontEditor boldCount and italicCount", "[Editor][S83]") {
    FontEditor editor;
    FontAsset a("Arial", 12.f); a.setWeight(FontWeight::Bold);
    FontAsset b("Times", 12.f); b.setStyle(FontStyle::Italic);
    FontAsset c("Courier", 12.f);
    editor.addFont(a); editor.addFont(b); editor.addFont(c);
    REQUIRE(editor.boldCount() == 1);
    REQUIRE(editor.italicCount() == 1);
}

TEST_CASE("FontEditor embeddedCount and dirtyCount", "[Editor][S83]") {
    FontEditor editor;
    FontAsset a("A", 10.f); a.setEmbedded(true); a.setDirty(true);
    FontAsset b("B", 10.f); b.setEmbedded(false);
    editor.addFont(a); editor.addFont(b);
    REQUIRE(editor.embeddedCount() == 1);
    REQUIRE(editor.dirtyCount() == 1);
}

TEST_CASE("FontEditor MAX_FONTS is 128", "[Editor][S83]") {
    REQUIRE(FontEditor::MAX_FONTS == 128);
}

// ── GradientEditor ───────────────────────────────────────────────

TEST_CASE("GradientType names are correct", "[Editor][S83]") {
    REQUIRE(std::string(gradientTypeName(GradientType::Linear))    == "Linear");
    REQUIRE(std::string(gradientTypeName(GradientType::Radial))    == "Radial");
    REQUIRE(std::string(gradientTypeName(GradientType::Angular))   == "Angular");
    REQUIRE(std::string(gradientTypeName(GradientType::Diamond))   == "Diamond");
    REQUIRE(std::string(gradientTypeName(GradientType::Square))    == "Square");
    REQUIRE(std::string(gradientTypeName(GradientType::Reflected)) == "Reflected");
    REQUIRE(std::string(gradientTypeName(GradientType::Conical))   == "Conical");
    REQUIRE(std::string(gradientTypeName(GradientType::Custom))    == "Custom");
}

TEST_CASE("GradientInterpolation names are correct", "[Editor][S83]") {
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Linear))   == "Linear");
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Step))     == "Step");
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Spline))   == "Spline");
    REQUIRE(std::string(gradientInterpolationName(GradientInterpolation::Constant)) == "Constant");
}

TEST_CASE("GradientColorStop select deselect and setPosition", "[Editor][S83]") {
    GradientColorStop stop;
    stop.setPosition(0.5f);
    REQUIRE(stop.position == 0.5f);
    REQUIRE_FALSE(stop.selected);
    stop.select();
    REQUIRE(stop.selected);
    stop.deselect();
    REQUIRE_FALSE(stop.selected);
}

TEST_CASE("GradientRamp addStop and stopCount", "[Editor][S83]") {
    GradientRamp ramp("sky", GradientType::Linear);
    GradientColorStop s; s.setPosition(0.0f);
    REQUIRE(ramp.addStop(s));
    REQUIRE(ramp.stopCount() == 1);
}

TEST_CASE("GradientRamp addStop rejects duplicate position", "[Editor][S83]") {
    GradientRamp ramp("sky", GradientType::Linear);
    GradientColorStop s; s.setPosition(0.5f);
    ramp.addStop(s);
    REQUIRE_FALSE(ramp.addStop(s));
}

TEST_CASE("GradientRamp removeStop removes entry", "[Editor][S83]") {
    GradientRamp ramp("fire", GradientType::Radial);
    GradientColorStop s; s.setPosition(0.3f);
    ramp.addStop(s);
    REQUIRE(ramp.removeStop(0.3f));
    REQUIRE(ramp.stopCount() == 0);
}

TEST_CASE("GradientRamp selectAll and deselectAll", "[Editor][S83]") {
    GradientRamp ramp("ice", GradientType::Linear);
    GradientColorStop s1; s1.setPosition(0.f);
    GradientColorStop s2; s2.setPosition(1.f);
    ramp.addStop(s1); ramp.addStop(s2);
    ramp.selectAll();
    REQUIRE(ramp.selectedCount() == 2);
    ramp.deselectAll();
    REQUIRE(ramp.selectedCount() == 0);
}

TEST_CASE("GradientEditorPanel addRamp and rampCount", "[Editor][S83]") {
    GradientEditorPanel panel;
    GradientRamp ramp("sunset", GradientType::Linear);
    REQUIRE(panel.addRamp(ramp));
    REQUIRE(panel.rampCount() == 1);
}

TEST_CASE("GradientEditorPanel addRamp rejects duplicate name", "[Editor][S83]") {
    GradientEditorPanel panel;
    GradientRamp ramp("sunset", GradientType::Linear);
    panel.addRamp(ramp);
    REQUIRE_FALSE(panel.addRamp(ramp));
}

TEST_CASE("GradientEditorPanel removeRamp removes entry", "[Editor][S83]") {
    GradientEditorPanel panel;
    GradientRamp ramp("ocean", GradientType::Radial);
    panel.addRamp(ramp);
    REQUIRE(panel.removeRamp("ocean"));
    REQUIRE(panel.rampCount() == 0);
}

TEST_CASE("GradientEditorPanel setActiveRamp updates active", "[Editor][S83]") {
    GradientEditorPanel panel;
    GradientRamp ramp("fire", GradientType::Conical);
    panel.addRamp(ramp);
    REQUIRE(panel.setActiveRamp("fire"));
    REQUIRE(panel.activeRamp() == "fire");
}

TEST_CASE("GradientEditorPanel setSymmetric", "[Editor][S83]") {
    GradientEditorPanel panel;
    REQUIRE_FALSE(panel.isSymmetric());
    panel.setSymmetric(true);
    REQUIRE(panel.isSymmetric());
}

TEST_CASE("GradientRamp MAX_STOPS is 64", "[Editor][S83]") {
    REQUIRE(GradientRamp::MAX_STOPS == 64);
}
