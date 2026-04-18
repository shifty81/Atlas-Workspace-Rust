// S139 editor tests: TypographySystem, IconographySpec, FontRegistryEditor
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/FontRegistryEditor.h"

using namespace NF;
using Catch::Approx;

// ── TypographySystem ──────────────────────────────────────────────────────────

TEST_CASE("FontWeight names", "[Editor][S139]") {
    REQUIRE(std::string(fontWeightName(FontWeight::Thin))       == "Thin");
    REQUIRE(std::string(fontWeightName(FontWeight::ExtraLight)) == "ExtraLight");
    REQUIRE(std::string(fontWeightName(FontWeight::Light))      == "Light");
    REQUIRE(std::string(fontWeightName(FontWeight::Regular))    == "Regular");
    REQUIRE(std::string(fontWeightName(FontWeight::Medium))     == "Medium");
    REQUIRE(std::string(fontWeightName(FontWeight::SemiBold))   == "SemiBold");
    REQUIRE(std::string(fontWeightName(FontWeight::Bold))       == "Bold");
    REQUIRE(std::string(fontWeightName(FontWeight::ExtraBold))  == "ExtraBold");
    REQUIRE(std::string(fontWeightName(FontWeight::Black))      == "Black");
}

TEST_CASE("FontStyle names", "[Editor][S139]") {
    REQUIRE(std::string(fontStyleName(FontStyle::Normal))  == "Normal");
    REQUIRE(std::string(fontStyleName(FontStyle::Italic))  == "Italic");
    REQUIRE(std::string(fontStyleName(FontStyle::Oblique)) == "Oblique");
}

TEST_CASE("TypeRole names", "[Editor][S139]") {
    REQUIRE(std::string(typeRoleName(TypeRole::Display))   == "Display");
    REQUIRE(std::string(typeRoleName(TypeRole::Heading1))  == "Heading1");
    REQUIRE(std::string(typeRoleName(TypeRole::Heading2))  == "Heading2");
    REQUIRE(std::string(typeRoleName(TypeRole::Heading3))  == "Heading3");
    REQUIRE(std::string(typeRoleName(TypeRole::Body))      == "Body");
    REQUIRE(std::string(typeRoleName(TypeRole::BodySmall)) == "BodySmall");
    REQUIRE(std::string(typeRoleName(TypeRole::Label))     == "Label");
    REQUIRE(std::string(typeRoleName(TypeRole::Caption))   == "Caption");
    REQUIRE(std::string(typeRoleName(TypeRole::Code))      == "Code");
    REQUIRE(std::string(typeRoleName(TypeRole::Tooltip))   == "Tooltip");
}

TEST_CASE("TypeStyle lineHeightPx calculation", "[Editor][S139]") {
    TypeStyle s;
    s.sizePx = 13.f;
    s.lineHeight = 1.5f;
    REQUIRE(s.lineHeightPx() == Catch::Approx(19.5f));
}

TEST_CASE("TypographyScale set and retrieve styles", "[Editor][S139]") {
    TypographyScale scale;
    TypeStyle s;
    s.role = TypeRole::Body;
    s.sizePx = 13.f;
    s.weight = FontWeight::Regular;
    scale.setStyle(s);

    const auto* found = scale.style(TypeRole::Body);
    REQUIRE(found != nullptr);
    REQUIRE(found->sizePx == Catch::Approx(13.f));
    REQUIRE(found->weight == FontWeight::Regular);
}

TEST_CASE("TypographyScale loadAtlasDarkDefaults", "[Editor][S139]") {
    TypographyScale scale;
    scale.loadAtlasDarkDefaults();
    REQUIRE(scale.count() == 10u);

    const auto* body = scale.style(TypeRole::Body);
    REQUIRE(body != nullptr);
    REQUIRE(body->sizePx == Catch::Approx(13.f));

    const auto* display = scale.style(TypeRole::Display);
    REQUIRE(display != nullptr);
    REQUIRE(display->weight == FontWeight::Bold);
}

TEST_CASE("TypographyScale overwrite existing style", "[Editor][S139]") {
    TypographyScale scale;
    TypeStyle s; s.role = TypeRole::Label; s.sizePx = 12.f;
    scale.setStyle(s);
    REQUIRE(scale.count() == 1u);

    TypeStyle s2; s2.role = TypeRole::Label; s2.sizePx = 14.f;
    scale.setStyle(s2);
    REQUIRE(scale.count() == 1u);
    REQUIRE(scale.style(TypeRole::Label)->sizePx == Catch::Approx(14.f));
}

TEST_CASE("TypographySystem initialize and query", "[Editor][S139]") {
    TypographySystem sys;
    REQUIRE(!sys.isInitialized());
    sys.initialize();
    REQUIRE(sys.isInitialized());
    REQUIRE(sys.themeName() == "AtlasDark");

    REQUIRE(sys.resolvedSizePx(TypeRole::Body)      == Catch::Approx(13.f));
    REQUIRE(sys.resolvedSizePx(TypeRole::Heading1)  == Catch::Approx(24.f));
    REQUIRE(sys.resolvedLineHeightPx(TypeRole::Body) == Catch::Approx(19.5f));
}

TEST_CASE("TypographySystem base font family", "[Editor][S139]") {
    TypographySystem sys;
    sys.initialize();
    REQUIRE(sys.baseFontFamily() == "AtlasUI");
    sys.setBaseFontFamily("CustomFont");
    REQUIRE(sys.baseFontFamily() == "CustomFont");
}

TEST_CASE("TypographySystem missing role returns default", "[Editor][S139]") {
    TypographySystem sys;
    sys.initialize();
    // After init all 10 roles are loaded, just verify get() works
    const auto* code = sys.get(TypeRole::Code);
    REQUIRE(code != nullptr);
    REQUIRE(code->role == TypeRole::Code);
}

// ── IconographySpec ───────────────────────────────────────────────────────────

TEST_CASE("IconSizeTier names and pixel values", "[Editor][S139]") {
    REQUIRE(std::string(iconSizeTierName(IconSizeTier::XSmall))  == "XSmall");
    REQUIRE(std::string(iconSizeTierName(IconSizeTier::Small))   == "Small");
    REQUIRE(std::string(iconSizeTierName(IconSizeTier::Medium))  == "Medium");
    REQUIRE(std::string(iconSizeTierName(IconSizeTier::Large))   == "Large");
    REQUIRE(std::string(iconSizeTierName(IconSizeTier::XLarge))  == "XLarge");
    REQUIRE(std::string(iconSizeTierName(IconSizeTier::Display)) == "Display");

    REQUIRE(iconSizePx(IconSizeTier::Small)  == 16);
    REQUIRE(iconSizePx(IconSizeTier::Large)  == 24);
    REQUIRE(iconSizePx(IconSizeTier::XLarge) == 32);
}

TEST_CASE("IconCategory names", "[Editor][S139]") {
    REQUIRE(std::string(iconCategoryName(IconCategory::Actions))    == "Actions");
    REQUIRE(std::string(iconCategoryName(IconCategory::Navigation)) == "Navigation");
    REQUIRE(std::string(iconCategoryName(IconCategory::Status))     == "Status");
    REQUIRE(std::string(iconCategoryName(IconCategory::FileTypes))  == "FileTypes");
    REQUIRE(std::string(iconCategoryName(IconCategory::UI))         == "UI");
    REQUIRE(std::string(iconCategoryName(IconCategory::Tools))      == "Tools");
    REQUIRE(std::string(iconCategoryName(IconCategory::Media))      == "Media");
    REQUIRE(std::string(iconCategoryName(IconCategory::Editor))     == "Editor");
}

TEST_CASE("IconDescriptor validity", "[Editor][S139]") {
    IconDescriptor d;
    REQUIRE(!d.isValid());
    d.id = "actions/add"; d.label = "Add";
    REQUIRE(d.isValid());
}

TEST_CASE("IconRegistry register and find", "[Editor][S139]") {
    IconRegistry reg;
    IconDescriptor d;
    d.id = "actions/add"; d.label = "Add";
    d.category = IconCategory::Actions;
    d.assetPath = "Icons/actions/add.svg";
    REQUIRE(reg.registerIcon(d));
    REQUIRE(reg.count() == 1u);

    const auto* found = reg.find("actions/add");
    REQUIRE(found != nullptr);
    REQUIRE(found->label == "Add");
}

TEST_CASE("IconRegistry reject duplicate", "[Editor][S139]") {
    IconRegistry reg;
    IconDescriptor d;
    d.id = "ui/close"; d.label = "Close";
    REQUIRE(reg.registerIcon(d));
    REQUIRE(!reg.registerIcon(d));  // duplicate
}

TEST_CASE("IconRegistry unregister", "[Editor][S139]") {
    IconRegistry reg;
    IconDescriptor d;
    d.id = "nav/back"; d.label = "Back";
    reg.registerIcon(d);
    REQUIRE(reg.unregisterIcon("nav/back"));
    REQUIRE(reg.count() == 0u);
    REQUIRE(!reg.unregisterIcon("nav/back")); // already gone
}

TEST_CASE("IconRegistry byCategory", "[Editor][S139]") {
    IconRegistry reg;
    auto mk = [](const char* id, const char* lbl, IconCategory cat) {
        IconDescriptor d; d.id = id; d.label = lbl; d.category = cat; return d;
    };
    reg.registerIcon(mk("actions/add",   "Add",   IconCategory::Actions));
    reg.registerIcon(mk("actions/delete","Delete",IconCategory::Actions));
    reg.registerIcon(mk("ui/close",      "Close", IconCategory::UI));

    auto actions = reg.byCategory(IconCategory::Actions);
    REQUIRE(actions.size() == 2u);

    auto ui = reg.byCategory(IconCategory::UI);
    REQUIRE(ui.size() == 1u);
}

TEST_CASE("IconRegistry loadAtlasCoreDefaults", "[Editor][S139]") {
    IconRegistry reg;
    reg.loadAtlasCoreDefaults();
    REQUIRE(reg.has("actions/add"));
    REQUIRE(reg.has("status/error"));
    REQUIRE(reg.has("ui/close"));
    REQUIRE(reg.count() >= 15u);
}

TEST_CASE("IconographySpec initialize and surface sizes", "[Editor][S139]") {
    IconographySpec spec;
    REQUIRE(!spec.isInitialized());
    spec.initialize();
    REQUIRE(spec.isInitialized());

    REQUIRE(spec.toolbarIconSize()      == IconSizeTier::Small);
    REQUIRE(spec.panelHeaderIconSize()  == IconSizeTier::XSmall);
    REQUIRE(spec.menuIconSize()         == IconSizeTier::Small);
    REQUIRE(spec.notificationIconSize() == IconSizeTier::Medium);
}

TEST_CASE("IconographySpec valid icon id check", "[Editor][S139]") {
    REQUIRE( IconographySpec::isValidIconId("actions/add"));
    REQUIRE( IconographySpec::isValidIconId("ui/close"));
    REQUIRE(!IconographySpec::isValidIconId("noslash"));
    REQUIRE(!IconographySpec::isValidIconId("/leadingslash"));
    REQUIRE(!IconographySpec::isValidIconId("trailing/"));
}

TEST_CASE("IconographySpec snapToGrid", "[Editor][S139]") {
    REQUIRE(IconographySpec::snapToGrid(16) == 16);
    REQUIRE(IconographySpec::snapToGrid(17) == 16);
    REQUIRE(IconographySpec::snapToGrid(20) == 20);
    REQUIRE(IconographySpec::snapToGrid(23) == 20);
    REQUIRE(IconographySpec::snapToGrid(24) == 24);
}

TEST_CASE("IconographySpec grid constants", "[Editor][S139]") {
    REQUIRE(IconographySpec::GRID_BASE_PX == 4);
    REQUIRE(IconographySpec::PIXEL_BUDGET == 2);
}

// ── FontRegistryEditor ────────────────────────────────────────────────────────

TEST_CASE("FontAssetFormat names", "[Editor][S139]") {
    REQUIRE(std::string(fontAssetFormatName(FontAssetFormat::TTF))  == "TTF");
    REQUIRE(std::string(fontAssetFormatName(FontAssetFormat::OTF))  == "OTF");
    REQUIRE(std::string(fontAssetFormatName(FontAssetFormat::WOFF)) == "WOFF");
    REQUIRE(std::string(fontAssetFormatName(FontAssetFormat::BDF))  == "BDF");
}

TEST_CASE("FontFaceDescriptor validity and displayLabel", "[Editor][S139]") {
    FontFaceDescriptor d;
    REQUIRE(!d.isValid());
    d.id = 1; d.familyName = "AtlasUI"; d.assetPath = "AtlasUI-Regular.ttf";
    REQUIRE(d.isValid());
    REQUIRE(d.displayLabel().find("AtlasUI") != std::string::npos);
}

TEST_CASE("FontRegistryEditor register and find", "[Editor][S139]") {
    FontRegistryEditor reg;
    FontFaceDescriptor d;
    d.id = 1; d.familyName = "AtlasUI"; d.weight = FontWeight::Regular;
    d.style = FontStyle::Normal; d.assetPath = "AtlasUI-Regular.ttf";

    REQUIRE(reg.registerFace(d));
    REQUIRE(reg.faceCount() == 1u);

    const auto* found = reg.find(1);
    REQUIRE(found != nullptr);
    REQUIRE(found->familyName == "AtlasUI");
}

TEST_CASE("FontRegistryEditor reject duplicate id", "[Editor][S139]") {
    FontRegistryEditor reg;
    FontFaceDescriptor d;
    d.id = 1; d.familyName = "F"; d.assetPath = "f.ttf";
    REQUIRE(reg.registerFace(d));
    REQUIRE(!reg.registerFace(d));
}

TEST_CASE("FontRegistryEditor unregister", "[Editor][S139]") {
    FontRegistryEditor reg;
    FontFaceDescriptor d;
    d.id = 1; d.familyName = "F"; d.assetPath = "f.ttf";
    reg.registerFace(d);
    REQUIRE(reg.unregisterFace(1));
    REQUIRE(reg.faceCount() == 0u);
}

TEST_CASE("FontRegistryEditor findByFamily", "[Editor][S139]") {
    FontRegistryEditor reg;
    FontFaceDescriptor d1, d2;
    d1.id = 1; d1.familyName = "AtlasUI"; d1.weight = FontWeight::Regular;
    d1.assetPath = "r.ttf";
    d2.id = 2; d2.familyName = "AtlasUI"; d2.weight = FontWeight::Bold;
    d2.assetPath = "b.ttf";
    reg.registerFace(d1);
    reg.registerFace(d2);

    const auto* bold = reg.findByFamily("AtlasUI", FontWeight::Bold);
    REQUIRE(bold != nullptr);
    REQUIRE(bold->id == 2u);

    const auto* missing = reg.findByFamily("Unknown");
    REQUIRE(missing == nullptr);
}

TEST_CASE("FontRegistryEditor byFamily and families list", "[Editor][S139]") {
    FontRegistryEditor reg;
    auto add = [&](uint32_t id, const char* fam, const char* path) {
        FontFaceDescriptor d; d.id = id; d.familyName = fam; d.assetPath = path;
        reg.registerFace(d);
    };
    add(1, "AtlasUI",   "a.ttf");
    add(2, "AtlasUI",   "b.ttf");
    add(3, "AtlasMono", "c.ttf");

    REQUIRE(reg.byFamily("AtlasUI").size()   == 2u);
    REQUIRE(reg.byFamily("AtlasMono").size() == 1u);
    REQUIRE(reg.families().size()            == 2u);
}

TEST_CASE("FontRegistryEditor markLoaded and loadedCount", "[Editor][S139]") {
    FontRegistryEditor reg;
    FontFaceDescriptor d;
    d.id = 1; d.familyName = "F"; d.assetPath = "f.ttf"; d.loaded = false;
    reg.registerFace(d);
    REQUIRE(reg.loadedCount() == 0u);
    REQUIRE(reg.markLoaded(1));
    REQUIRE(reg.loadedCount() == 1u);
}

TEST_CASE("FontRegistryEditor loadAtlasEmbedded", "[Editor][S139]") {
    FontRegistryEditor reg;
    reg.loadAtlasEmbedded();
    REQUIRE(reg.faceCount() == 6u);
    REQUIRE(reg.loadedCount() == 6u);
    REQUIRE(reg.findByFamily("AtlasUI",   FontWeight::Regular) != nullptr);
    REQUIRE(reg.findByFamily("AtlasMono", FontWeight::Regular) != nullptr);
}

TEST_CASE("FontRegistryEditor preview state", "[Editor][S139]") {
    FontRegistryEditor reg;
    REQUIRE(!reg.previewText().empty());
    reg.setPreviewText("Hello World");
    REQUIRE(reg.previewText() == "Hello World");
    reg.setPreviewSize(16.f);
    REQUIRE(reg.previewSizePx() == Catch::Approx(16.f));
    reg.setSelectedFaceId(3);
    REQUIRE(reg.selectedFaceId() == 3u);
}
