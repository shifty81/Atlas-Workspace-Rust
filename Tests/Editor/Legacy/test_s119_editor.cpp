// S119 editor tests: AccessibilityEditor, ColorblindSimulator, SubtitleEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SubtitleEditor.h"
#include "NF/Editor/ColorblindSimulator.h"
#include "NF/Editor/AccessibilityEditor.h"

using namespace NF;

// ── AccessibilityEditor ───────────────────────────────────────────────────────

TEST_CASE("AccessibilityFeatureType names", "[Editor][S119]") {
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::HighContrast))      == "HighContrast");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::LargeText))         == "LargeText");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::ReduceMotion))      == "ReduceMotion");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::ScreenReader))      == "ScreenReader");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::ClosedCaptions))    == "ClosedCaptions");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::ColorblindMode))    == "ColorblindMode");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::AudioDescriptions)) == "AudioDescriptions");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::StickyKeys))        == "StickyKeys");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::OneButtonMode))     == "OneButtonMode");
    REQUIRE(std::string(accessibilityFeatureTypeName(AccessibilityFeatureType::HapticFeedback))    == "HapticFeedback");
}

TEST_CASE("AccessibilityCategory names", "[Editor][S119]") {
    REQUIRE(std::string(accessibilityCategoryName(AccessibilityCategory::Visual))    == "Visual");
    REQUIRE(std::string(accessibilityCategoryName(AccessibilityCategory::Auditory))  == "Auditory");
    REQUIRE(std::string(accessibilityCategoryName(AccessibilityCategory::Motor))     == "Motor");
    REQUIRE(std::string(accessibilityCategoryName(AccessibilityCategory::Cognitive)) == "Cognitive");
    REQUIRE(std::string(accessibilityCategoryName(AccessibilityCategory::Speech))    == "Speech");
}

TEST_CASE("AccessibilityComplianceLevel names", "[Editor][S119]") {
    REQUIRE(std::string(accessibilityComplianceLevelName(AccessibilityComplianceLevel::None))     == "None");
    REQUIRE(std::string(accessibilityComplianceLevelName(AccessibilityComplianceLevel::PartialA)) == "PartialA");
    REQUIRE(std::string(accessibilityComplianceLevelName(AccessibilityComplianceLevel::LevelA))   == "LevelA");
    REQUIRE(std::string(accessibilityComplianceLevelName(AccessibilityComplianceLevel::LevelAA))  == "LevelAA");
    REQUIRE(std::string(accessibilityComplianceLevelName(AccessibilityComplianceLevel::LevelAAA)) == "LevelAAA");
}

TEST_CASE("AccessibilityFeature defaults", "[Editor][S119]") {
    AccessibilityFeature f(1, "high_contrast", AccessibilityFeatureType::HighContrast, AccessibilityCategory::Visual);
    REQUIRE(f.id()              == 1u);
    REQUIRE(f.name()            == "high_contrast");
    REQUIRE(f.type()            == AccessibilityFeatureType::HighContrast);
    REQUIRE(f.category()        == AccessibilityCategory::Visual);
    REQUIRE(!f.isEnabled());
    REQUIRE(f.complianceLevel() == AccessibilityComplianceLevel::None);
    REQUIRE(!f.isRequired());
}

TEST_CASE("AccessibilityFeature mutation", "[Editor][S119]") {
    AccessibilityFeature f(2, "screen_reader", AccessibilityFeatureType::ScreenReader, AccessibilityCategory::Visual);
    f.setIsEnabled(true);
    f.setComplianceLevel(AccessibilityComplianceLevel::LevelAA);
    f.setIsRequired(true);
    REQUIRE(f.isEnabled());
    REQUIRE(f.complianceLevel() == AccessibilityComplianceLevel::LevelAA);
    REQUIRE(f.isRequired());
}

TEST_CASE("AccessibilityEditor defaults", "[Editor][S119]") {
    AccessibilityEditor ed;
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.filterCategory()    == AccessibilityCategory::Visual);
    REQUIRE(ed.targetCompliance()  == AccessibilityComplianceLevel::LevelAA);
    REQUIRE(ed.featureCount()      == 0u);
}

TEST_CASE("AccessibilityEditor add/remove features", "[Editor][S119]") {
    AccessibilityEditor ed;
    REQUIRE(ed.addFeature(AccessibilityFeature(1, "high_contrast", AccessibilityFeatureType::HighContrast,   AccessibilityCategory::Visual)));
    REQUIRE(ed.addFeature(AccessibilityFeature(2, "captions",      AccessibilityFeatureType::ClosedCaptions, AccessibilityCategory::Auditory)));
    REQUIRE(ed.addFeature(AccessibilityFeature(3, "sticky_keys",   AccessibilityFeatureType::StickyKeys,     AccessibilityCategory::Motor)));
    REQUIRE(!ed.addFeature(AccessibilityFeature(1, "high_contrast",AccessibilityFeatureType::HighContrast,   AccessibilityCategory::Visual)));
    REQUIRE(ed.featureCount() == 3u);
    REQUIRE(ed.removeFeature(2));
    REQUIRE(ed.featureCount() == 2u);
    REQUIRE(!ed.removeFeature(99));
}

TEST_CASE("AccessibilityEditor counts and find", "[Editor][S119]") {
    AccessibilityEditor ed;
    AccessibilityFeature f1(1, "hi_contrast",  AccessibilityFeatureType::HighContrast,   AccessibilityCategory::Visual);    f1.setIsEnabled(true);
    AccessibilityFeature f2(2, "large_text",   AccessibilityFeatureType::LargeText,      AccessibilityCategory::Visual);    f2.setIsRequired(true);
    AccessibilityFeature f3(3, "captions",     AccessibilityFeatureType::ClosedCaptions, AccessibilityCategory::Auditory);  f3.setIsEnabled(true); f3.setIsRequired(true);
    AccessibilityFeature f4(4, "sticky_keys",  AccessibilityFeatureType::StickyKeys,     AccessibilityCategory::Motor);
    ed.addFeature(f1); ed.addFeature(f2); ed.addFeature(f3); ed.addFeature(f4);
    REQUIRE(ed.countByCategory(AccessibilityCategory::Visual)   == 2u);
    REQUIRE(ed.countByCategory(AccessibilityCategory::Auditory) == 1u);
    REQUIRE(ed.countByCategory(AccessibilityCategory::Speech)   == 0u);
    REQUIRE(ed.countEnabled()                                   == 2u);
    REQUIRE(ed.countRequired()                                  == 2u);
    auto* found = ed.findFeature(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == AccessibilityCategory::Auditory);
    REQUIRE(ed.findFeature(99) == nullptr);
}

TEST_CASE("AccessibilityEditor settings mutation", "[Editor][S119]") {
    AccessibilityEditor ed;
    ed.setShowDisabled(false);
    ed.setFilterCategory(AccessibilityCategory::Motor);
    ed.setTargetCompliance(AccessibilityComplianceLevel::LevelAAA);
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.filterCategory()   == AccessibilityCategory::Motor);
    REQUIRE(ed.targetCompliance() == AccessibilityComplianceLevel::LevelAAA);
}

// ── ColorblindSimulator ───────────────────────────────────────────────────────

TEST_CASE("ColorblindType names", "[Editor][S119]") {
    REQUIRE(std::string(colorblindTypeName(ColorblindType::None))          == "None");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Protanopia))    == "Protanopia");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Deuteranopia))  == "Deuteranopia");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Tritanopia))    == "Tritanopia");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Protanomaly))   == "Protanomaly");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Deuteranomaly)) == "Deuteranomaly");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Tritanomaly))   == "Tritanomaly");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Achromatopsia)) == "Achromatopsia");
    REQUIRE(std::string(colorblindTypeName(ColorblindType::Achromatomaly)) == "Achromatomaly");
}

TEST_CASE("ColorblindSimMode names", "[Editor][S119]") {
    REQUIRE(std::string(colorblindSimModeName(ColorblindSimMode::RealTime))   == "RealTime");
    REQUIRE(std::string(colorblindSimModeName(ColorblindSimMode::Screenshot)) == "Screenshot");
    REQUIRE(std::string(colorblindSimModeName(ColorblindSimMode::SideBySide)) == "SideBySide");
    REQUIRE(std::string(colorblindSimModeName(ColorblindSimMode::Overlay))    == "Overlay");
    REQUIRE(std::string(colorblindSimModeName(ColorblindSimMode::Palette))    == "Palette");
}

TEST_CASE("ColorblindSeverity names", "[Editor][S119]") {
    REQUIRE(std::string(colorblindSeverityName(ColorblindSeverity::Mild))     == "Mild");
    REQUIRE(std::string(colorblindSeverityName(ColorblindSeverity::Moderate)) == "Moderate");
    REQUIRE(std::string(colorblindSeverityName(ColorblindSeverity::Severe))   == "Severe");
    REQUIRE(std::string(colorblindSeverityName(ColorblindSeverity::Complete)) == "Complete");
}

TEST_CASE("ColorblindSimProfile defaults", "[Editor][S119]") {
    ColorblindSimProfile p(1, "deuteranopia_profile", ColorblindType::Deuteranopia);
    REQUIRE(p.id()          == 1u);
    REQUIRE(p.name()        == "deuteranopia_profile");
    REQUIRE(p.type()        == ColorblindType::Deuteranopia);
    REQUIRE(p.simMode()     == ColorblindSimMode::SideBySide);
    REQUIRE(p.severity()    == ColorblindSeverity::Severe);
    REQUIRE(!p.isActive());
    REQUIRE(p.blendFactor() == 1.0f);
}

TEST_CASE("ColorblindSimProfile mutation", "[Editor][S119]") {
    ColorblindSimProfile p(2, "protanopia_profile", ColorblindType::Protanopia);
    p.setSimMode(ColorblindSimMode::Overlay);
    p.setSeverity(ColorblindSeverity::Moderate);
    p.setIsActive(true);
    p.setBlendFactor(0.75f);
    REQUIRE(p.simMode()     == ColorblindSimMode::Overlay);
    REQUIRE(p.severity()    == ColorblindSeverity::Moderate);
    REQUIRE(p.isActive());
    REQUIRE(p.blendFactor() == 0.75f);
}

TEST_CASE("ColorblindSimulator defaults", "[Editor][S119]") {
    ColorblindSimulator sim;
    REQUIRE(sim.activeType()   == ColorblindType::None);
    REQUIRE(sim.activeMode()   == ColorblindSimMode::SideBySide);
    REQUIRE(sim.isShowStats());
    REQUIRE(sim.profileCount() == 0u);
}

TEST_CASE("ColorblindSimulator add/remove profiles", "[Editor][S119]") {
    ColorblindSimulator sim;
    REQUIRE(sim.addProfile(ColorblindSimProfile(1, "deut", ColorblindType::Deuteranopia)));
    REQUIRE(sim.addProfile(ColorblindSimProfile(2, "prot", ColorblindType::Protanopia)));
    REQUIRE(sim.addProfile(ColorblindSimProfile(3, "trit", ColorblindType::Tritanopia)));
    REQUIRE(!sim.addProfile(ColorblindSimProfile(1, "deut",ColorblindType::Deuteranopia)));
    REQUIRE(sim.profileCount() == 3u);
    REQUIRE(sim.removeProfile(2));
    REQUIRE(sim.profileCount() == 2u);
    REQUIRE(!sim.removeProfile(99));
}

TEST_CASE("ColorblindSimulator counts and find", "[Editor][S119]") {
    ColorblindSimulator sim;
    ColorblindSimProfile p1(1, "deut_a", ColorblindType::Deuteranopia);
    ColorblindSimProfile p2(2, "deut_b", ColorblindType::Deuteranopia); p2.setIsActive(true);
    ColorblindSimProfile p3(3, "prot_a", ColorblindType::Protanopia);
    ColorblindSimProfile p4(4, "trit_a", ColorblindType::Tritanopia);   p4.setIsActive(true);
    sim.addProfile(p1); sim.addProfile(p2); sim.addProfile(p3); sim.addProfile(p4);
    REQUIRE(sim.countByType(ColorblindType::Deuteranopia) == 2u);
    REQUIRE(sim.countByType(ColorblindType::Protanopia)   == 1u);
    REQUIRE(sim.countByType(ColorblindType::None)         == 0u);
    REQUIRE(sim.countActive()                             == 2u);
    auto* found = sim.findProfile(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == ColorblindType::Protanopia);
    REQUIRE(sim.findProfile(99) == nullptr);
}

TEST_CASE("ColorblindSimulator settings mutation", "[Editor][S119]") {
    ColorblindSimulator sim;
    sim.setActiveType(ColorblindType::Deuteranopia);
    sim.setActiveMode(ColorblindSimMode::RealTime);
    sim.setShowStats(false);
    REQUIRE(sim.activeType() == ColorblindType::Deuteranopia);
    REQUIRE(sim.activeMode() == ColorblindSimMode::RealTime);
    REQUIRE(!sim.isShowStats());
}

// ── SubtitleEditor ────────────────────────────────────────────────────────────

TEST_CASE("SubtitleDisplayMode names", "[Editor][S119]") {
    REQUIRE(std::string(subtitleDisplayModeName(SubtitleDisplayMode::Standard))     == "Standard");
    REQUIRE(std::string(subtitleDisplayModeName(SubtitleDisplayMode::Karaoke))      == "Karaoke");
    REQUIRE(std::string(subtitleDisplayModeName(SubtitleDisplayMode::WordByWord))   == "WordByWord");
    REQUIRE(std::string(subtitleDisplayModeName(SubtitleDisplayMode::Narrative))    == "Narrative");
    REQUIRE(std::string(subtitleDisplayModeName(SubtitleDisplayMode::SignLanguage)) == "SignLanguage");
}

TEST_CASE("SubtitlePosition names", "[Editor][S119]") {
    REQUIRE(std::string(subtitlePositionName(SubtitlePosition::Bottom)) == "Bottom");
    REQUIRE(std::string(subtitlePositionName(SubtitlePosition::Top))    == "Top");
    REQUIRE(std::string(subtitlePositionName(SubtitlePosition::Left))   == "Left");
    REQUIRE(std::string(subtitlePositionName(SubtitlePosition::Right))  == "Right");
    REQUIRE(std::string(subtitlePositionName(SubtitlePosition::Center)) == "Center");
    REQUIRE(std::string(subtitlePositionName(SubtitlePosition::Custom)) == "Custom");
}

TEST_CASE("SubtitleSpeakerTag names", "[Editor][S119]") {
    REQUIRE(std::string(subtitleSpeakerTagName(SubtitleSpeakerTag::None))    == "None");
    REQUIRE(std::string(subtitleSpeakerTagName(SubtitleSpeakerTag::Name))    == "Name");
    REQUIRE(std::string(subtitleSpeakerTagName(SubtitleSpeakerTag::Color))   == "Color");
    REQUIRE(std::string(subtitleSpeakerTagName(SubtitleSpeakerTag::Icon))    == "Icon");
    REQUIRE(std::string(subtitleSpeakerTagName(SubtitleSpeakerTag::Bracket)) == "Bracket");
}

TEST_CASE("SubtitleTrack defaults", "[Editor][S119]") {
    SubtitleTrack t(1, "english_subs", "en-US");
    REQUIRE(t.id()          == 1u);
    REQUIRE(t.name()        == "english_subs");
    REQUIRE(t.language()    == "en-US");
    REQUIRE(t.displayMode() == SubtitleDisplayMode::Standard);
    REQUIRE(t.position()    == SubtitlePosition::Bottom);
    REQUIRE(t.speakerTag()  == SubtitleSpeakerTag::Name);
    REQUIRE(t.isEnabled());
    REQUIRE(t.fontSize()    == 14.0f);
}

TEST_CASE("SubtitleTrack mutation", "[Editor][S119]") {
    SubtitleTrack t(2, "spanish_subs", "es-ES");
    t.setDisplayMode(SubtitleDisplayMode::Karaoke);
    t.setPosition(SubtitlePosition::Top);
    t.setSpeakerTag(SubtitleSpeakerTag::Color);
    t.setIsEnabled(false);
    t.setFontSize(18.0f);
    REQUIRE(t.displayMode() == SubtitleDisplayMode::Karaoke);
    REQUIRE(t.position()    == SubtitlePosition::Top);
    REQUIRE(t.speakerTag()  == SubtitleSpeakerTag::Color);
    REQUIRE(!t.isEnabled());
    REQUIRE(t.fontSize()    == 18.0f);
}

TEST_CASE("SubtitleEditor defaults", "[Editor][S119]") {
    SubtitleEditor ed;
    REQUIRE(ed.isShowPreview());
    REQUIRE(ed.defaultPosition() == SubtitlePosition::Bottom);
    REQUIRE(ed.defaultFontSize() == 14.0f);
    REQUIRE(ed.trackCount()      == 0u);
}

TEST_CASE("SubtitleEditor add/remove tracks", "[Editor][S119]") {
    SubtitleEditor ed;
    REQUIRE(ed.addTrack(SubtitleTrack(1, "en", "en-US")));
    REQUIRE(ed.addTrack(SubtitleTrack(2, "es", "es-ES")));
    REQUIRE(ed.addTrack(SubtitleTrack(3, "fr", "fr-FR")));
    REQUIRE(!ed.addTrack(SubtitleTrack(1, "en","en-US")));
    REQUIRE(ed.trackCount() == 3u);
    REQUIRE(ed.removeTrack(2));
    REQUIRE(ed.trackCount() == 2u);
    REQUIRE(!ed.removeTrack(99));
}

TEST_CASE("SubtitleEditor counts and find", "[Editor][S119]") {
    SubtitleEditor ed;
    SubtitleTrack t1(1, "en_a", "en-US");
    SubtitleTrack t2(2, "en_b", "en-GB");  t2.setDisplayMode(SubtitleDisplayMode::Karaoke);
    SubtitleTrack t3(3, "es",   "es-ES");  t3.setPosition(SubtitlePosition::Top); t3.setIsEnabled(false);
    SubtitleTrack t4(4, "fr",   "fr-FR");  t4.setDisplayMode(SubtitleDisplayMode::Karaoke); t4.setIsEnabled(false);
    ed.addTrack(t1); ed.addTrack(t2); ed.addTrack(t3); ed.addTrack(t4);
    REQUIRE(ed.countByDisplayMode(SubtitleDisplayMode::Standard) == 2u);
    REQUIRE(ed.countByDisplayMode(SubtitleDisplayMode::Karaoke)  == 2u);
    REQUIRE(ed.countByPosition(SubtitlePosition::Bottom)         == 3u);
    REQUIRE(ed.countByPosition(SubtitlePosition::Top)            == 1u);
    REQUIRE(ed.countEnabled()                                    == 2u);
    auto* found = ed.findTrack(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->language() == "es-ES");
    REQUIRE(ed.findTrack(99) == nullptr);
}

TEST_CASE("SubtitleEditor settings mutation", "[Editor][S119]") {
    SubtitleEditor ed;
    ed.setShowPreview(false);
    ed.setDefaultPosition(SubtitlePosition::Top);
    ed.setDefaultFontSize(20.0f);
    REQUIRE(!ed.isShowPreview());
    REQUIRE(ed.defaultPosition() == SubtitlePosition::Top);
    REQUIRE(ed.defaultFontSize() == 20.0f);
}
