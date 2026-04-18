// S175 editor tests: AccessibilityEditorV1, LocalizationKeyEditorV1, SubtitleEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AccessibilityEditorV1.h"
#include "NF/Editor/LocalizationKeyEditorV1.h"
#include "NF/Editor/SubtitleEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── AccessibilityEditorV1 ────────────────────────────────────────────────────

TEST_CASE("Accv1Rule validity", "[Editor][S175]") {
    Accv1Rule r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "ContrastRatio";
    REQUIRE(r.isValid());
}

TEST_CASE("AccessibilityEditorV1 addRule and ruleCount", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    REQUIRE(acc.ruleCount() == 0);
    Accv1Rule r; r.id = 1; r.name = "Rule1";
    REQUIRE(acc.addRule(r));
    REQUIRE(acc.ruleCount() == 1);
}

TEST_CASE("AccessibilityEditorV1 addRule invalid fails", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    REQUIRE(!acc.addRule(Accv1Rule{}));
}

TEST_CASE("AccessibilityEditorV1 addRule duplicate fails", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1Rule r; r.id = 1; r.name = "A";
    acc.addRule(r);
    REQUIRE(!acc.addRule(r));
}

TEST_CASE("AccessibilityEditorV1 removeRule", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1Rule r; r.id = 2; r.name = "B";
    acc.addRule(r);
    REQUIRE(acc.removeRule(2));
    REQUIRE(acc.ruleCount() == 0);
    REQUIRE(!acc.removeRule(2));
}

TEST_CASE("AccessibilityEditorV1 runCheck and checkCount", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1CheckResult ch; ch.id = 1; ch.ruleId = 10; ch.state = Accv1CheckState::Pass;
    REQUIRE(acc.runCheck(ch));
    REQUIRE(acc.checkCount() == 1);
}

TEST_CASE("AccessibilityEditorV1 runCheck invalid fails", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    REQUIRE(!acc.runCheck(Accv1CheckResult{}));
}

TEST_CASE("AccessibilityEditorV1 runCheck duplicate fails", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1CheckResult ch; ch.id = 1; ch.ruleId = 10;
    acc.runCheck(ch);
    REQUIRE(!acc.runCheck(ch));
}

TEST_CASE("AccessibilityEditorV1 passCount and failCount", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1CheckResult c1; c1.id = 1; c1.ruleId = 1; c1.state = Accv1CheckState::Pass;
    Accv1CheckResult c2; c2.id = 2; c2.ruleId = 2; c2.state = Accv1CheckState::Fail;
    acc.runCheck(c1); acc.runCheck(c2);
    REQUIRE(acc.passCount() == 1);
    REQUIRE(acc.failCount() == 1);
}

TEST_CASE("AccessibilityEditorV1 findRule returns ptr", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1Rule r; r.id = 5; r.name = "R5";
    acc.addRule(r);
    REQUIRE(acc.findRule(5) != nullptr);
    REQUIRE(acc.findRule(5)->name == "R5");
    REQUIRE(acc.findRule(99) == nullptr);
}

TEST_CASE("AccessibilityEditorV1 findCheck returns ptr", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1CheckResult ch; ch.id = 3; ch.ruleId = 7; ch.state = Accv1CheckState::Warning;
    acc.runCheck(ch);
    REQUIRE(acc.findCheck(3) != nullptr);
    REQUIRE(acc.findCheck(3)->isWarning());
    REQUIRE(acc.findCheck(99) == nullptr);
}

TEST_CASE("AccessibilityEditorV1 countByCategory", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    Accv1Rule r1; r1.id = 1; r1.name = "A"; r1.category = Accv1RuleCategory::Visual;
    Accv1Rule r2; r2.id = 2; r2.name = "B"; r2.category = Accv1RuleCategory::Motor;
    acc.addRule(r1); acc.addRule(r2);
    REQUIRE(acc.countByCategory(Accv1RuleCategory::Visual) == 1);
    REQUIRE(acc.countByCategory(Accv1RuleCategory::Motor)  == 1);
}

TEST_CASE("accv1RuleCategoryName covers all values", "[Editor][S175]") {
    REQUIRE(std::string(accv1RuleCategoryName(Accv1RuleCategory::Visual))   == "Visual");
    REQUIRE(std::string(accv1RuleCategoryName(Accv1RuleCategory::Cognitive)) == "Cognitive");
}

TEST_CASE("accv1CheckStateName covers all values", "[Editor][S175]") {
    REQUIRE(std::string(accv1CheckStateName(Accv1CheckState::Pass))          == "Pass");
    REQUIRE(std::string(accv1CheckStateName(Accv1CheckState::NotApplicable)) == "NotApplicable");
}

TEST_CASE("Accv1CheckResult helpers", "[Editor][S175]") {
    Accv1CheckResult ch; ch.id = 1; ch.ruleId = 1; ch.state = Accv1CheckState::Pass;
    REQUIRE(ch.isPassed());
    ch.state = Accv1CheckState::Fail;
    REQUIRE(ch.isFailed());
    ch.state = Accv1CheckState::Warning;
    REQUIRE(ch.isWarning());
}

TEST_CASE("AccessibilityEditorV1 onCheck callback", "[Editor][S175]") {
    AccessibilityEditorV1 acc;
    uint64_t notified = 0;
    acc.setOnCheck([&](uint64_t id) { notified = id; });
    Accv1CheckResult ch; ch.id = 7; ch.ruleId = 3;
    acc.runCheck(ch);
    REQUIRE(notified == 7);
}

// ── LocalizationKeyEditorV1 ──────────────────────────────────────────────────

TEST_CASE("Lokv1Key validity", "[Editor][S175]") {
    Lokv1Key k;
    REQUIRE(!k.isValid());
    k.id = 1; k.name = "UI_OK_Button";
    REQUIRE(k.isValid());
}

TEST_CASE("LocalizationKeyEditorV1 addKey and keyCount", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    REQUIRE(lok.keyCount() == 0);
    Lokv1Key k; k.id = 1; k.name = "K1";
    REQUIRE(lok.addKey(k));
    REQUIRE(lok.keyCount() == 1);
}

TEST_CASE("LocalizationKeyEditorV1 addKey invalid fails", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    REQUIRE(!lok.addKey(Lokv1Key{}));
}

TEST_CASE("LocalizationKeyEditorV1 addKey duplicate fails", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 1; k.name = "A";
    lok.addKey(k);
    REQUIRE(!lok.addKey(k));
}

TEST_CASE("LocalizationKeyEditorV1 removeKey", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 2; k.name = "B";
    lok.addKey(k);
    REQUIRE(lok.removeKey(2));
    REQUIRE(lok.keyCount() == 0);
    REQUIRE(!lok.removeKey(2));
}

TEST_CASE("LocalizationKeyEditorV1 addTranslation and approvedCount", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 1; k.name = "K";
    lok.addKey(k);
    Lokv1Translation t; t.locale = "en"; t.state = Lokv1TranslationState::Approved;
    REQUIRE(lok.addTranslation(1, t));
    REQUIRE(lok.approvedCount() == 1);
}

TEST_CASE("LocalizationKeyEditorV1 addTranslation duplicate locale fails", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 1; k.name = "K";
    lok.addKey(k);
    Lokv1Translation t; t.locale = "en";
    lok.addTranslation(1, t);
    REQUIRE(!lok.addTranslation(1, t));
}

TEST_CASE("LocalizationKeyEditorV1 missingCount", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 1; k.name = "K";
    lok.addKey(k);
    Lokv1Translation t; t.locale = "fr"; t.state = Lokv1TranslationState::Missing;
    lok.addTranslation(1, t);
    REQUIRE(lok.missingCount() == 1);
}

TEST_CASE("LocalizationKeyEditorV1 findKey returns ptr", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 5; k.name = "K5";
    lok.addKey(k);
    REQUIRE(lok.findKey(5) != nullptr);
    REQUIRE(lok.findKey(5)->name == "K5");
    REQUIRE(lok.findKey(99) == nullptr);
}

TEST_CASE("LocalizationKeyEditorV1 countByKeyType", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k1; k1.id = 1; k1.name = "A"; k1.type = Lokv1KeyType::UI;
    Lokv1Key k2; k2.id = 2; k2.name = "B"; k2.type = Lokv1KeyType::Dialogue;
    lok.addKey(k1); lok.addKey(k2);
    REQUIRE(lok.countByKeyType(Lokv1KeyType::UI)       == 1);
    REQUIRE(lok.countByKeyType(Lokv1KeyType::Dialogue)  == 1);
}

TEST_CASE("LocalizationKeyEditorV1 countByState", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    Lokv1Key k; k.id = 1; k.name = "K";
    lok.addKey(k);
    Lokv1Translation t1; t1.locale = "en"; t1.state = Lokv1TranslationState::Draft;
    Lokv1Translation t2; t2.locale = "fr"; t2.state = Lokv1TranslationState::Review;
    lok.addTranslation(1, t1); lok.addTranslation(1, t2);
    REQUIRE(lok.countByState(Lokv1TranslationState::Draft)  == 1);
    REQUIRE(lok.countByState(Lokv1TranslationState::Review) == 1);
}

TEST_CASE("lokv1TranslationStateName covers all values", "[Editor][S175]") {
    REQUIRE(std::string(lokv1TranslationStateName(Lokv1TranslationState::Missing))  == "Missing");
    REQUIRE(std::string(lokv1TranslationStateName(Lokv1TranslationState::Exported)) == "Exported");
}

TEST_CASE("lokv1KeyTypeName covers all values", "[Editor][S175]") {
    REQUIRE(std::string(lokv1KeyTypeName(Lokv1KeyType::UI))       == "UI");
    REQUIRE(std::string(lokv1KeyTypeName(Lokv1KeyType::Metadata))  == "Metadata");
}

TEST_CASE("LocalizationKeyEditorV1 onChange callback", "[Editor][S175]") {
    LocalizationKeyEditorV1 lok;
    uint64_t notified = 0;
    lok.setOnChange([&](uint64_t id) { notified = id; });
    Lokv1Key k; k.id = 4; k.name = "D";
    lok.addKey(k);
    Lokv1Translation t; t.locale = "de";
    lok.addTranslation(4, t);
    REQUIRE(notified == 4);
}

// ── SubtitleEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Subv1Track validity", "[Editor][S175]") {
    Subv1Track t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "English";
    REQUIRE(t.isValid());
}

TEST_CASE("SubtitleEditorV1 addTrack and trackCount", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    REQUIRE(sub.trackCount() == 0);
    Subv1Track t; t.id = 1; t.name = "EN";
    REQUIRE(sub.addTrack(t));
    REQUIRE(sub.trackCount() == 1);
}

TEST_CASE("SubtitleEditorV1 addTrack invalid fails", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    REQUIRE(!sub.addTrack(Subv1Track{}));
}

TEST_CASE("SubtitleEditorV1 addTrack duplicate fails", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Track t; t.id = 1; t.name = "EN";
    sub.addTrack(t);
    REQUIRE(!sub.addTrack(t));
}

TEST_CASE("SubtitleEditorV1 removeTrack", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Track t; t.id = 2; t.name = "FR";
    sub.addTrack(t);
    REQUIRE(sub.removeTrack(2));
    REQUIRE(sub.trackCount() == 0);
    REQUIRE(!sub.removeTrack(2));
}

TEST_CASE("SubtitleEditorV1 addCue and cueCount", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Cue c; c.id = 1; c.trackId = 10; c.text = "Hello"; c.startMs = 0.f; c.endMs = 1000.f;
    REQUIRE(sub.addCue(c));
    REQUIRE(sub.cueCount() == 1);
}

TEST_CASE("SubtitleEditorV1 addCue invalid fails", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    REQUIRE(!sub.addCue(Subv1Cue{}));
}

TEST_CASE("SubtitleEditorV1 removeCue", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Cue c; c.id = 1; c.trackId = 10; c.text = "Hi"; c.startMs = 0.f; c.endMs = 500.f;
    sub.addCue(c);
    REQUIRE(sub.removeCue(1));
    REQUIRE(sub.cueCount() == 0);
    REQUIRE(!sub.removeCue(1));
}

TEST_CASE("SubtitleEditorV1 setState finalCount", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Track t; t.id = 1; t.name = "EN";
    sub.addTrack(t);
    REQUIRE(sub.setState(1, Subv1TrackState::Final));
    REQUIRE(sub.finalCount() == 1);
    REQUIRE(sub.findTrack(1)->isFinal());
}

TEST_CASE("SubtitleEditorV1 exportedCount", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Track t; t.id = 1; t.name = "EN";
    sub.addTrack(t);
    sub.setState(1, Subv1TrackState::Exported);
    REQUIRE(sub.exportedCount() == 1);
    REQUIRE(sub.findTrack(1)->isExported());
}

TEST_CASE("SubtitleEditorV1 countByStyle", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Cue c1; c1.id = 1; c1.trackId = 1; c1.text = "A"; c1.startMs = 0.f; c1.endMs = 100.f; c1.style = Subv1CueStyle::Whisper;
    Subv1Cue c2; c2.id = 2; c2.trackId = 1; c2.text = "B"; c2.startMs = 200.f; c2.endMs = 300.f; c2.style = Subv1CueStyle::Shout;
    sub.addCue(c1); sub.addCue(c2);
    REQUIRE(sub.countByStyle(Subv1CueStyle::Whisper) == 1);
    REQUIRE(sub.countByStyle(Subv1CueStyle::Shout)   == 1);
}

TEST_CASE("SubtitleEditorV1 findTrack returns ptr", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    Subv1Track t; t.id = 3; t.name = "DE";
    sub.addTrack(t);
    REQUIRE(sub.findTrack(3) != nullptr);
    REQUIRE(sub.findTrack(3)->name == "DE");
    REQUIRE(sub.findTrack(99) == nullptr);
}

TEST_CASE("subv1TrackStateName covers all values", "[Editor][S175]") {
    REQUIRE(std::string(subv1TrackStateName(Subv1TrackState::Empty))    == "Empty");
    REQUIRE(std::string(subv1TrackStateName(Subv1TrackState::Exported)) == "Exported");
}

TEST_CASE("subv1CueStyleName covers all values", "[Editor][S175]") {
    REQUIRE(std::string(subv1CueStyleName(Subv1CueStyle::Normal))   == "Normal");
    REQUIRE(std::string(subv1CueStyleName(Subv1CueStyle::Caption))  == "Caption");
}

TEST_CASE("SubtitleEditorV1 onChange callback", "[Editor][S175]") {
    SubtitleEditorV1 sub;
    uint64_t notified = 0;
    sub.setOnChange([&](uint64_t id) { notified = id; });
    Subv1Track t; t.id = 6; t.name = "ES";
    sub.addTrack(t);
    sub.setState(6, Subv1TrackState::Final);
    REQUIRE(notified == 6);
}
