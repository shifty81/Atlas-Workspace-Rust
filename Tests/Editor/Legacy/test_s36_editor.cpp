#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("FontStyle names cover all 4 values", "[Editor][S36]") {
    REQUIRE(std::string(fontStyleName(FontStyle::Normal))  == "Normal");
    REQUIRE(std::string(fontStyleName(FontStyle::Italic))  == "Italic");
    REQUIRE(std::string(fontStyleName(FontStyle::Oblique)) == "Oblique");
    REQUIRE(std::string(fontStyleName(FontStyle::Inherit)) == "Inherit");
}

TEST_CASE("FontWeight names cover all 6 values", "[Editor][S36]") {
    REQUIRE(std::string(fontWeightName(FontWeight::Thin))       == "Thin");
    REQUIRE(std::string(fontWeightName(FontWeight::ExtraLight)) == "ExtraLight");
    REQUIRE(std::string(fontWeightName(FontWeight::Light))      == "Light");
    REQUIRE(std::string(fontWeightName(FontWeight::Regular))    == "Regular");
    REQUIRE(std::string(fontWeightName(FontWeight::Medium))     == "Medium");
    REQUIRE(std::string(fontWeightName(FontWeight::Bold))       == "Bold");
}

TEST_CASE("FontVariant names cover all 4 values", "[Editor][S36]") {
    REQUIRE(std::string(fontVariantName(FontVariant::Normal))       == "Normal");
    REQUIRE(std::string(fontVariantName(FontVariant::SmallCaps))    == "SmallCaps");
    REQUIRE(std::string(fontVariantName(FontVariant::AllSmallCaps)) == "AllSmallCaps");
    REQUIRE(std::string(fontVariantName(FontVariant::PetiteCaps))   == "PetiteCaps");
}

TEST_CASE("FontAsset default values", "[Editor][S36]") {
    FontAsset font("Arial", 14.0f);
    REQUIRE(font.family()   == "Arial");
    REQUIRE(font.size()     == Catch::Approx(14.0f));
    REQUIRE(font.style()    == FontStyle::Normal);
    REQUIRE(font.weight()   == FontWeight::Regular);
    REQUIRE(font.variant()  == FontVariant::Normal);
    REQUIRE(font.lineHeight()    == Catch::Approx(1.2f));
    REQUIRE(font.letterSpacing() == Catch::Approx(0.0f));
    REQUIRE_FALSE(font.isEmbedded());
    REQUIRE_FALSE(font.isDirty());
}

TEST_CASE("FontAsset isBold detects Medium and Bold weights", "[Editor][S36]") {
    FontAsset font("Test", 12.0f);
    font.setWeight(FontWeight::Regular);
    REQUIRE_FALSE(font.isBold());

    font.setWeight(FontWeight::Medium);
    REQUIRE(font.isBold());

    font.setWeight(FontWeight::Bold);
    REQUIRE(font.isBold());
}

TEST_CASE("FontAsset isItalic detects Italic and Oblique styles", "[Editor][S36]") {
    FontAsset font("Test", 12.0f);
    font.setStyle(FontStyle::Normal);
    REQUIRE_FALSE(font.isItalic());

    font.setStyle(FontStyle::Italic);
    REQUIRE(font.isItalic());

    font.setStyle(FontStyle::Oblique);
    REQUIRE(font.isItalic());
}

TEST_CASE("FontAsset setters round-trip", "[Editor][S36]") {
    FontAsset font("Verdana", 16.0f);
    font.setStyle(FontStyle::Italic);
    font.setWeight(FontWeight::Light);
    font.setVariant(FontVariant::SmallCaps);
    font.setSize(18.0f);
    font.setLineHeight(1.5f);
    font.setLetterSpacing(0.5f);
    font.setEmbedded(true);
    font.setDirty(true);

    REQUIRE(font.style()         == FontStyle::Italic);
    REQUIRE(font.weight()        == FontWeight::Light);
    REQUIRE(font.variant()       == FontVariant::SmallCaps);
    REQUIRE(font.size()          == Catch::Approx(18.0f));
    REQUIRE(font.lineHeight()    == Catch::Approx(1.5f));
    REQUIRE(font.letterSpacing() == Catch::Approx(0.5f));
    REQUIRE(font.isEmbedded());
    REQUIRE(font.isDirty());
}

TEST_CASE("FontEditor addFont and duplicate rejection", "[Editor][S36]") {
    FontEditor editor;
    FontAsset a("Arial",   12.0f);
    FontAsset b("Roboto",  14.0f);
    FontAsset dup("Arial", 16.0f);

    REQUIRE(editor.addFont(a));
    REQUIRE(editor.addFont(b));
    REQUIRE_FALSE(editor.addFont(dup));
    REQUIRE(editor.fontCount() == 2);
}

TEST_CASE("FontEditor removeFont clears activeFont", "[Editor][S36]") {
    FontEditor editor;
    FontAsset a("Helvetica");
    editor.addFont(a);
    editor.setActiveFont("Helvetica");
    REQUIRE(editor.activeFont() == "Helvetica");

    editor.removeFont("Helvetica");
    REQUIRE(editor.fontCount()       == 0);
    REQUIRE(editor.activeFont().empty());
}

TEST_CASE("FontEditor findFont returns pointer or nullptr", "[Editor][S36]") {
    FontEditor editor;
    FontAsset a("Comic Sans");
    editor.addFont(a);

    REQUIRE(editor.findFont("Comic Sans") != nullptr);
    REQUIRE(editor.findFont("Comic Sans")->family() == "Comic Sans");
    REQUIRE(editor.findFont("Unknown") == nullptr);
}

TEST_CASE("FontEditor setActiveFont returns false for missing font", "[Editor][S36]") {
    FontEditor editor;
    REQUIRE_FALSE(editor.setActiveFont("Ghost"));
    REQUIRE(editor.activeFont().empty());
}

TEST_CASE("FontEditor dirtyCount, embeddedCount, boldCount, italicCount", "[Editor][S36]") {
    FontEditor editor;
    FontAsset a("A"); a.setDirty(true); a.setEmbedded(true); a.setWeight(FontWeight::Bold);
    FontAsset b("B"); b.setDirty(true); b.setStyle(FontStyle::Italic);
    FontAsset c("C"); c.setEmbedded(true); c.setWeight(FontWeight::Medium);

    editor.addFont(a);
    editor.addFont(b);
    editor.addFont(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.embeddedCount() == 2);
    REQUIRE(editor.boldCount()     == 2); // A (Bold) and C (Medium)
    REQUIRE(editor.italicCount()   == 1); // B only
}

TEST_CASE("FontEditor MAX_FONTS limit enforced", "[Editor][S36]") {
    FontEditor editor;
    for (size_t i = 0; i < FontEditor::MAX_FONTS; ++i) {
        FontAsset f("Font" + std::to_string(i));
        REQUIRE(editor.addFont(f));
    }
    FontAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addFont(overflow));
    REQUIRE(editor.fontCount() == FontEditor::MAX_FONTS);
}

TEST_CASE("FontAsset line height and letter spacing defaults", "[Editor][S36]") {
    FontAsset font("Mono", 10.0f);
    font.setLineHeight(2.0f);
    font.setLetterSpacing(-0.1f);
    REQUIRE(font.lineHeight()    == Catch::Approx(2.0f));
    REQUIRE(font.letterSpacing() == Catch::Approx(-0.1f));
}
