// S132 editor tests: LocalizationKeyEditor, TranslationEditor, LanguagePackEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LanguagePackEditor.h"
#include "NF/Editor/TranslationEditor.h"
#include "NF/Editor/LocalizationKeyEditor.h"

using namespace NF;

// ── LocalizationKeyEditor ─────────────────────────────────────────────────────

TEST_CASE("L10nKeyCategory names", "[Editor][S132]") {
    REQUIRE(std::string(l10nKeyCategoryName(L10nKeyCategory::UI))        == "UI");
    REQUIRE(std::string(l10nKeyCategoryName(L10nKeyCategory::Gameplay))  == "Gameplay");
    REQUIRE(std::string(l10nKeyCategoryName(L10nKeyCategory::Narrative)) == "Narrative");
    REQUIRE(std::string(l10nKeyCategoryName(L10nKeyCategory::System))    == "System");
    REQUIRE(std::string(l10nKeyCategoryName(L10nKeyCategory::Debug))     == "Debug");
}

TEST_CASE("L10nKeyStatus names", "[Editor][S132]") {
    REQUIRE(std::string(l10nKeyStatusName(L10nKeyStatus::Draft))      == "Draft");
    REQUIRE(std::string(l10nKeyStatusName(L10nKeyStatus::Review))     == "Review");
    REQUIRE(std::string(l10nKeyStatusName(L10nKeyStatus::Approved))   == "Approved");
    REQUIRE(std::string(l10nKeyStatusName(L10nKeyStatus::Deprecated)) == "Deprecated");
    REQUIRE(std::string(l10nKeyStatusName(L10nKeyStatus::Missing))    == "Missing");
}

TEST_CASE("L10nKey defaults", "[Editor][S132]") {
    L10nKey k(1, "ui_start_button", L10nKeyCategory::UI, L10nKeyStatus::Draft);
    REQUIRE(k.id()          == 1u);
    REQUIRE(k.name()        == "ui_start_button");
    REQUIRE(k.category()    == L10nKeyCategory::UI);
    REQUIRE(k.status()      == L10nKeyStatus::Draft);
    REQUIRE(k.defaultText() == "");
    REQUIRE(k.maxLength()   == 256u);
    REQUIRE(k.isEnabled());
}

TEST_CASE("L10nKey mutation", "[Editor][S132]") {
    L10nKey k(2, "gameplay_hint", L10nKeyCategory::Gameplay, L10nKeyStatus::Review);
    k.setDefaultText("Press X to jump");
    k.setMaxLength(128u);
    k.setIsEnabled(false);
    REQUIRE(k.defaultText() == "Press X to jump");
    REQUIRE(k.maxLength()   == 128u);
    REQUIRE(!k.isEnabled());
}

TEST_CASE("LocalizationKeyEditor defaults", "[Editor][S132]") {
    LocalizationKeyEditor ed;
    REQUIRE(!ed.isShowDeprecated());
    REQUIRE(ed.isGroupByCategory());
    REQUIRE(ed.warnMissingTranslation());
    REQUIRE(ed.keyCount() == 0u);
}

TEST_CASE("LocalizationKeyEditor add/remove keys", "[Editor][S132]") {
    LocalizationKeyEditor ed;
    REQUIRE(ed.addKey(L10nKey(1, "k_a", L10nKeyCategory::UI,       L10nKeyStatus::Draft)));
    REQUIRE(ed.addKey(L10nKey(2, "k_b", L10nKeyCategory::Gameplay, L10nKeyStatus::Approved)));
    REQUIRE(ed.addKey(L10nKey(3, "k_c", L10nKeyCategory::System,   L10nKeyStatus::Review)));
    REQUIRE(!ed.addKey(L10nKey(1, "k_a", L10nKeyCategory::UI,      L10nKeyStatus::Draft)));
    REQUIRE(ed.keyCount() == 3u);
    REQUIRE(ed.removeKey(2));
    REQUIRE(ed.keyCount() == 2u);
    REQUIRE(!ed.removeKey(99));
}

TEST_CASE("LocalizationKeyEditor counts and find", "[Editor][S132]") {
    LocalizationKeyEditor ed;
    L10nKey k1(1, "k_a", L10nKeyCategory::UI,        L10nKeyStatus::Draft);
    L10nKey k2(2, "k_b", L10nKeyCategory::UI,        L10nKeyStatus::Approved);
    L10nKey k3(3, "k_c", L10nKeyCategory::Narrative, L10nKeyStatus::Review);
    L10nKey k4(4, "k_d", L10nKeyCategory::Debug,     L10nKeyStatus::Deprecated); k4.setIsEnabled(false);
    ed.addKey(k1); ed.addKey(k2); ed.addKey(k3); ed.addKey(k4);
    REQUIRE(ed.countByCategory(L10nKeyCategory::UI)        == 2u);
    REQUIRE(ed.countByCategory(L10nKeyCategory::Narrative) == 1u);
    REQUIRE(ed.countByCategory(L10nKeyCategory::System)    == 0u);
    REQUIRE(ed.countByStatus(L10nKeyStatus::Draft)         == 1u);
    REQUIRE(ed.countByStatus(L10nKeyStatus::Approved)      == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findKey(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == L10nKeyCategory::Narrative);
    REQUIRE(ed.findKey(99) == nullptr);
}

TEST_CASE("LocalizationKeyEditor settings mutation", "[Editor][S132]") {
    LocalizationKeyEditor ed;
    ed.setIsShowDeprecated(true);
    ed.setIsGroupByCategory(false);
    ed.setWarnMissingTranslation(false);
    REQUIRE(ed.isShowDeprecated());
    REQUIRE(!ed.isGroupByCategory());
    REQUIRE(!ed.warnMissingTranslation());
}

// ── TranslationEditor ─────────────────────────────────────────────────────────

TEST_CASE("TranslationQuality names", "[Editor][S132]") {
    REQUIRE(std::string(translationQualityName(TranslationQuality::MachineTranslated)) == "MachineTranslated");
    REQUIRE(std::string(translationQualityName(TranslationQuality::Draft))             == "Draft");
    REQUIRE(std::string(translationQualityName(TranslationQuality::Reviewed))          == "Reviewed");
    REQUIRE(std::string(translationQualityName(TranslationQuality::Approved))          == "Approved");
    REQUIRE(std::string(translationQualityName(TranslationQuality::Certified))         == "Certified");
}

TEST_CASE("TranslationDirection names", "[Editor][S132]") {
    REQUIRE(std::string(translationDirectionName(TranslationDirection::LTR)) == "LTR");
    REQUIRE(std::string(translationDirectionName(TranslationDirection::RTL)) == "RTL");
    REQUIRE(std::string(translationDirectionName(TranslationDirection::TTB)) == "TTB");
}

TEST_CASE("TranslationRecord defaults", "[Editor][S132]") {
    TranslationRecord e(1, 42, "en-US", TranslationQuality::Draft);
    REQUIRE(e.id()        == 1u);
    REQUIRE(e.keyId()     == 42u);
    REQUIRE(e.locale()    == "en-US");
    REQUIRE(e.quality()   == TranslationQuality::Draft);
    REQUIRE(e.direction() == TranslationDirection::LTR);
    REQUIRE(!e.isPlural());
    REQUIRE(e.isEnabled());
}

TEST_CASE("TranslationRecord mutation", "[Editor][S132]") {
    TranslationRecord e(2, 10, "ar-SA", TranslationQuality::Approved);
    e.setDirection(TranslationDirection::RTL);
    e.setIsPlural(true);
    e.setIsEnabled(false);
    REQUIRE(e.direction() == TranslationDirection::RTL);
    REQUIRE(e.isPlural());
    REQUIRE(!e.isEnabled());
}

TEST_CASE("TranslationEditor defaults", "[Editor][S132]") {
    TranslationEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByLocale());
    REQUIRE(ed.defaultQuality() == TranslationQuality::Draft);
    REQUIRE(ed.entryCount()     == 0u);
}

TEST_CASE("TranslationEditor add/remove entries", "[Editor][S132]") {
    TranslationEditor ed;
    REQUIRE(ed.addEntry(TranslationRecord(1, 1, "en-US", TranslationQuality::Draft)));
    REQUIRE(ed.addEntry(TranslationRecord(2, 1, "fr-FR", TranslationQuality::Reviewed)));
    REQUIRE(ed.addEntry(TranslationRecord(3, 1, "de-DE", TranslationQuality::Approved)));
    REQUIRE(!ed.addEntry(TranslationRecord(1, 1, "en-US", TranslationQuality::Draft)));
    REQUIRE(ed.entryCount() == 3u);
    REQUIRE(ed.removeEntry(2));
    REQUIRE(ed.entryCount() == 2u);
    REQUIRE(!ed.removeEntry(99));
}

TEST_CASE("TranslationEditor counts and find", "[Editor][S132]") {
    TranslationEditor ed;
    TranslationRecord e1(1, 1, "en-US", TranslationQuality::Draft);
    TranslationRecord e2(2, 1, "fr-FR", TranslationQuality::Draft);
    TranslationRecord e3(3, 1, "ar-SA", TranslationQuality::Approved); e3.setDirection(TranslationDirection::RTL);
    TranslationRecord e4(4, 1, "zh-TW", TranslationQuality::Certified); e4.setDirection(TranslationDirection::TTB); e4.setIsEnabled(false);
    ed.addEntry(e1); ed.addEntry(e2); ed.addEntry(e3); ed.addEntry(e4);
    REQUIRE(ed.countByQuality(TranslationQuality::Draft)     == 2u);
    REQUIRE(ed.countByQuality(TranslationQuality::Approved)  == 1u);
    REQUIRE(ed.countByQuality(TranslationQuality::Reviewed)  == 0u);
    REQUIRE(ed.countByDirection(TranslationDirection::LTR)   == 2u);
    REQUIRE(ed.countByDirection(TranslationDirection::RTL)   == 1u);
    REQUIRE(ed.countEnabled()                                == 3u);
    auto* found = ed.findEntry(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->direction() == TranslationDirection::RTL);
    REQUIRE(ed.findEntry(99) == nullptr);
}

TEST_CASE("TranslationEditor settings mutation", "[Editor][S132]") {
    TranslationEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByLocale(true);
    ed.setDefaultQuality(TranslationQuality::Reviewed);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByLocale());
    REQUIRE(ed.defaultQuality() == TranslationQuality::Reviewed);
}

// ── LanguagePackEditor ────────────────────────────────────────────────────────

TEST_CASE("LangPackFormat names", "[Editor][S132]") {
    REQUIRE(std::string(langPackFormatName(LangPackFormat::JSON))   == "JSON");
    REQUIRE(std::string(langPackFormatName(LangPackFormat::CSV))    == "CSV");
    REQUIRE(std::string(langPackFormatName(LangPackFormat::PO))     == "PO");
    REQUIRE(std::string(langPackFormatName(LangPackFormat::XLIFF))  == "XLIFF");
    REQUIRE(std::string(langPackFormatName(LangPackFormat::Binary)) == "Binary");
}

TEST_CASE("LangPackStatus names", "[Editor][S132]") {
    REQUIRE(std::string(langPackStatusName(LangPackStatus::Incomplete)) == "Incomplete");
    REQUIRE(std::string(langPackStatusName(LangPackStatus::Complete))   == "Complete");
    REQUIRE(std::string(langPackStatusName(LangPackStatus::Verified))   == "Verified");
    REQUIRE(std::string(langPackStatusName(LangPackStatus::Published))  == "Published");
    REQUIRE(std::string(langPackStatusName(LangPackStatus::Archived))   == "Archived");
}

TEST_CASE("LanguagePack defaults", "[Editor][S132]") {
    LanguagePack p(1, "en-US", LangPackFormat::JSON, LangPackStatus::Incomplete);
    REQUIRE(p.id()            == 1u);
    REQUIRE(p.locale()        == "en-US");
    REQUIRE(p.format()        == LangPackFormat::JSON);
    REQUIRE(p.status()        == LangPackStatus::Incomplete);
    REQUIRE(p.completionPct() == 0.0f);
    REQUIRE(!p.isRTL());
    REQUIRE(p.isEnabled());
}

TEST_CASE("LanguagePack mutation", "[Editor][S132]") {
    LanguagePack p(2, "ar-SA", LangPackFormat::PO, LangPackStatus::Verified);
    p.setCompletionPct(95.0f);
    p.setIsRTL(true);
    p.setIsEnabled(false);
    REQUIRE(p.completionPct() == 95.0f);
    REQUIRE(p.isRTL());
    REQUIRE(!p.isEnabled());
}

TEST_CASE("LanguagePackEditor defaults", "[Editor][S132]") {
    LanguagePackEditor ed;
    REQUIRE(!ed.isShowArchived());
    REQUIRE(!ed.isGroupByFormat());
    REQUIRE(ed.minCompletionPct() == 80.0f);
    REQUIRE(ed.packCount()        == 0u);
}

TEST_CASE("LanguagePackEditor add/remove packs", "[Editor][S132]") {
    LanguagePackEditor ed;
    REQUIRE(ed.addPack(LanguagePack(1, "en-US", LangPackFormat::JSON,  LangPackStatus::Complete)));
    REQUIRE(ed.addPack(LanguagePack(2, "fr-FR", LangPackFormat::PO,    LangPackStatus::Verified)));
    REQUIRE(ed.addPack(LanguagePack(3, "de-DE", LangPackFormat::XLIFF, LangPackStatus::Published)));
    REQUIRE(!ed.addPack(LanguagePack(1, "en-US", LangPackFormat::JSON, LangPackStatus::Complete)));
    REQUIRE(ed.packCount() == 3u);
    REQUIRE(ed.removePack(2));
    REQUIRE(ed.packCount() == 2u);
    REQUIRE(!ed.removePack(99));
}

TEST_CASE("LanguagePackEditor counts and find", "[Editor][S132]") {
    LanguagePackEditor ed;
    LanguagePack p1(1, "en-US", LangPackFormat::JSON,  LangPackStatus::Complete);   p1.setCompletionPct(100.0f);
    LanguagePack p2(2, "fr-FR", LangPackFormat::JSON,  LangPackStatus::Verified);   p2.setCompletionPct(90.0f);
    LanguagePack p3(3, "de-DE", LangPackFormat::PO,    LangPackStatus::Incomplete); p3.setCompletionPct(50.0f);
    LanguagePack p4(4, "ja-JP", LangPackFormat::XLIFF, LangPackStatus::Archived);   p4.setCompletionPct(85.0f); p4.setIsEnabled(false);
    ed.addPack(p1); ed.addPack(p2); ed.addPack(p3); ed.addPack(p4);
    REQUIRE(ed.countByFormat(LangPackFormat::JSON)          == 2u);
    REQUIRE(ed.countByFormat(LangPackFormat::PO)            == 1u);
    REQUIRE(ed.countByFormat(LangPackFormat::CSV)           == 0u);
    REQUIRE(ed.countByStatus(LangPackStatus::Complete)      == 1u);
    REQUIRE(ed.countByStatus(LangPackStatus::Verified)      == 1u);
    REQUIRE(ed.countComplete()                              == 3u); // p1(100), p2(90), p4(85) >= 80
    REQUIRE(ed.countEnabled()                               == 3u);
    auto* found = ed.findPack(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->format() == LangPackFormat::PO);
    REQUIRE(ed.findPack(99) == nullptr);
}

TEST_CASE("LanguagePackEditor settings mutation", "[Editor][S132]") {
    LanguagePackEditor ed;
    ed.setIsShowArchived(true);
    ed.setIsGroupByFormat(true);
    ed.setMinCompletionPct(95.0f);
    REQUIRE(ed.isShowArchived());
    REQUIRE(ed.isGroupByFormat());
    REQUIRE(ed.minCompletionPct() == 95.0f);
}
