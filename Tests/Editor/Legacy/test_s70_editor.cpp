#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── S70: Localization ────────────────────────────────────────────

TEST_CASE("LocaleId names are correct", "[Editor][S70]") {
    REQUIRE(std::string(localeIdName(LocaleId::English))  == "English");
    REQUIRE(std::string(localeIdName(LocaleId::Spanish))  == "Spanish");
    REQUIRE(std::string(localeIdName(LocaleId::French))   == "French");
    REQUIRE(std::string(localeIdName(LocaleId::German))   == "German");
    REQUIRE(std::string(localeIdName(LocaleId::Japanese)) == "Japanese");
    REQUIRE(std::string(localeIdName(LocaleId::Chinese))  == "Chinese");
    REQUIRE(std::string(localeIdName(LocaleId::Korean))   == "Korean");
    REQUIRE(std::string(localeIdName(LocaleId::Russian))  == "Russian");
}

TEST_CASE("LocalizedString defaults are invalid", "[Editor][S70]") {
    LocalizedString s;
    REQUIRE_FALSE(s.isValid());
}

TEST_CASE("LocalizedString valid when key and value set", "[Editor][S70]") {
    LocalizedString s;
    s.key = "menu.file";
    s.value = "File";
    REQUIRE(s.isValid());
}

TEST_CASE("LocalizedString defaults to English locale", "[Editor][S70]") {
    LocalizedString s;
    REQUIRE(s.locale == LocaleId::English);
}

TEST_CASE("LocalizedString verify sets verified flag", "[Editor][S70]") {
    LocalizedString s;
    s.key = "k";
    s.value = "v";
    REQUIRE_FALSE(s.isVerified());
    s.verify();
    REQUIRE(s.isVerified());
}

TEST_CASE("TranslationEntry set and get roundtrip", "[Editor][S70]") {
    TranslationEntry entry;
    entry.key = "menu.file";
    entry.set(LocaleId::English, "File");
    entry.set(LocaleId::Spanish, "Archivo");
    const std::string* en = entry.get(LocaleId::English);
    const std::string* es = entry.get(LocaleId::Spanish);
    REQUIRE(en != nullptr);
    REQUIRE(*en == "File");
    REQUIRE(es != nullptr);
    REQUIRE(*es == "Archivo");
}

TEST_CASE("TranslationEntry get returns nullptr for missing locale", "[Editor][S70]") {
    TranslationEntry entry;
    entry.key = "k";
    REQUIRE(entry.get(LocaleId::French) == nullptr);
}

TEST_CASE("TranslationEntry has returns true only when set", "[Editor][S70]") {
    TranslationEntry entry;
    entry.key = "k";
    REQUIRE_FALSE(entry.has(LocaleId::English));
    entry.set(LocaleId::English, "value");
    REQUIRE(entry.has(LocaleId::English));
}

TEST_CASE("TranslationEntry localeCount tracks translations", "[Editor][S70]") {
    TranslationEntry entry;
    entry.key = "k";
    REQUIRE(entry.localeCount() == 0);
    entry.set(LocaleId::English, "v");
    entry.set(LocaleId::French, "v2");
    REQUIRE(entry.localeCount() == 2);
}

TEST_CASE("TranslationTable starts empty", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE(table.entryCount() == 0);
    REQUIRE(table.name() == "ui");
}

TEST_CASE("TranslationTable addEntry increases count", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE(table.addEntry("menu.file"));
    REQUIRE(table.entryCount() == 1);
}

TEST_CASE("TranslationTable addEntry rejects duplicate key", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("menu.file");
    REQUIRE_FALSE(table.addEntry("menu.file"));
    REQUIRE(table.entryCount() == 1);
}

TEST_CASE("TranslationTable removeEntry removes existing entry", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("menu.file");
    REQUIRE(table.removeEntry("menu.file"));
    REQUIRE(table.entryCount() == 0);
}

TEST_CASE("TranslationTable removeEntry returns false for missing key", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE_FALSE(table.removeEntry("nonexistent"));
}

TEST_CASE("TranslationTable findEntry returns entry", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("menu.file", "File menu");
    REQUIRE(table.findEntry("menu.file") != nullptr);
}

TEST_CASE("TranslationTable findEntry returns nullptr for missing", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE(table.findEntry("missing") == nullptr);
}

TEST_CASE("TranslationTable setTranslation stores value", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("menu.file");
    REQUIRE(table.setTranslation("menu.file", LocaleId::English, "File"));
    const std::string* val = table.lookup("menu.file", LocaleId::English);
    REQUIRE(val != nullptr);
    REQUIRE(*val == "File");
}

TEST_CASE("TranslationTable setTranslation returns false for missing key", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE_FALSE(table.setTranslation("nonexistent", LocaleId::English, "val"));
}

TEST_CASE("TranslationTable lookup returns nullptr for missing key", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE(table.lookup("missing", LocaleId::English) == nullptr);
}

TEST_CASE("TranslationTable translatedCount counts per locale", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("k1");
    table.addEntry("k2");
    table.setTranslation("k1", LocaleId::English, "en1");
    table.setTranslation("k2", LocaleId::English, "en2");
    table.setTranslation("k1", LocaleId::French, "fr1");
    REQUIRE(table.translatedCount(LocaleId::English) == 2);
    REQUIRE(table.translatedCount(LocaleId::French) == 1);
    REQUIRE(table.translatedCount(LocaleId::German) == 0);
}

TEST_CASE("TranslationTable completionRate is 1.0 when all translated", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("k1");
    table.addEntry("k2");
    table.setTranslation("k1", LocaleId::Spanish, "s1");
    table.setTranslation("k2", LocaleId::Spanish, "s2");
    REQUIRE(table.completionRate(LocaleId::Spanish) == Approx(1.0f));
}

TEST_CASE("TranslationTable completionRate is 0.5 when half translated", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("k1");
    table.addEntry("k2");
    table.setTranslation("k1", LocaleId::French, "f1");
    REQUIRE(table.completionRate(LocaleId::French) == Approx(0.5f));
}

TEST_CASE("TranslationTable completionRate is 0.0 for empty table", "[Editor][S70]") {
    TranslationTable table("ui");
    REQUIRE(table.completionRate(LocaleId::English) == Approx(0.0f));
}

TEST_CASE("LocaleManager defaults to English active", "[Editor][S70]") {
    LocaleManager mgr;
    REQUIRE(mgr.active() == LocaleId::English);
    REQUIRE(mgr.fallback() == LocaleId::English);
}

TEST_CASE("LocaleManager setActive updates active locale", "[Editor][S70]") {
    LocaleManager mgr;
    mgr.setActive(LocaleId::French);
    REQUIRE(mgr.active() == LocaleId::French);
}

TEST_CASE("LocaleManager setActive increments switchCount", "[Editor][S70]") {
    LocaleManager mgr;
    mgr.setActive(LocaleId::German);
    REQUIRE(mgr.switchCount() == 1);
    mgr.setActive(LocaleId::Japanese);
    REQUIRE(mgr.switchCount() == 2);
}

TEST_CASE("LocaleManager resolve returns active locale value", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("k");
    table.setTranslation("k", LocaleId::English, "en_val");
    table.setTranslation("k", LocaleId::French, "fr_val");
    LocaleManager mgr(LocaleId::French);
    const std::string* val = mgr.resolve(table, "k");
    REQUIRE(val != nullptr);
    REQUIRE(*val == "fr_val");
}

TEST_CASE("LocaleManager resolve falls back to English when active locale missing", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("k");
    table.setTranslation("k", LocaleId::English, "en_fallback");
    LocaleManager mgr(LocaleId::Japanese);
    mgr.setFallback(LocaleId::English);
    const std::string* val = mgr.resolve(table, "k");
    REQUIRE(val != nullptr);
    REQUIRE(*val == "en_fallback");
}

TEST_CASE("LocaleManager resolve returns nullptr when no translation at all", "[Editor][S70]") {
    TranslationTable table("ui");
    table.addEntry("k");
    LocaleManager mgr(LocaleId::Korean);
    mgr.setFallback(LocaleId::English);
    REQUIRE(mgr.resolve(table, "k") == nullptr);
}

TEST_CASE("LocalizationSystem starts uninitialized", "[Editor][S70]") {
    LocalizationSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.tableCount() == 0);
}

TEST_CASE("LocalizationSystem init sets initialized", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    REQUIRE(sys.isInitialized());
}

TEST_CASE("LocalizationSystem createTable before init returns -1", "[Editor][S70]") {
    LocalizationSystem sys;
    REQUIRE(sys.createTable("ui") == -1);
}

TEST_CASE("LocalizationSystem createTable returns index", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    int idx = sys.createTable("ui");
    REQUIRE(idx == 0);
    REQUIRE(sys.tableCount() == 1);
}

TEST_CASE("LocalizationSystem createTable rejects duplicate name", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    sys.createTable("ui");
    REQUIRE(sys.createTable("ui") == -1);
    REQUIRE(sys.tableCount() == 1);
}

TEST_CASE("LocalizationSystem table returns valid pointer", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    int idx = sys.createTable("menus");
    REQUIRE(sys.table(idx) != nullptr);
    REQUIRE(sys.table(idx)->name() == "menus");
}

TEST_CASE("LocalizationSystem table returns nullptr for invalid index", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    REQUIRE(sys.table(-1) == nullptr);
    REQUIRE(sys.table(99) == nullptr);
}

TEST_CASE("LocalizationSystem tableByName finds table", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    sys.createTable("ui");
    REQUIRE(sys.tableByName("ui") != nullptr);
    REQUIRE(sys.tableByName("other") == nullptr);
}

TEST_CASE("LocalizationSystem translate returns correct value", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    int idx = sys.createTable("ui");
    sys.table(idx)->addEntry("file");
    sys.table(idx)->setTranslation("file", LocaleId::English, "File");
    const std::string* val = sys.translate("ui", "file");
    REQUIRE(val != nullptr);
    REQUIRE(*val == "File");
}

TEST_CASE("LocalizationSystem translate returns nullptr for missing table", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    REQUIRE(sys.translate("nonexistent", "key") == nullptr);
}

TEST_CASE("LocalizationSystem setLocale updates active locale", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    sys.setLocale(LocaleId::German);
    REQUIRE(sys.activeLocale() == LocaleId::German);
}

TEST_CASE("LocalizationSystem tick increments tickCount", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    REQUIRE(sys.tickCount() == 0);
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 1);
}

TEST_CASE("LocalizationSystem tick does nothing when uninitialized", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 0);
}

TEST_CASE("LocalizationSystem shutdown clears tables", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    sys.createTable("ui");
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.tableCount() == 0);
}

TEST_CASE("LocalizationSystem totalEntries sums across tables", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    int idx0 = sys.createTable("ui");
    int idx1 = sys.createTable("game");
    sys.table(idx0)->addEntry("k1");
    sys.table(idx0)->addEntry("k2");
    sys.table(idx1)->addEntry("k3");
    REQUIRE(sys.totalEntries() == 3);
}

TEST_CASE("LocalizationSystem totalTranslated counts across tables", "[Editor][S70]") {
    LocalizationSystem sys;
    sys.init();
    int idx = sys.createTable("ui");
    sys.table(idx)->addEntry("k1");
    sys.table(idx)->addEntry("k2");
    sys.table(idx)->setTranslation("k1", LocaleId::Spanish, "s1");
    sys.table(idx)->setTranslation("k2", LocaleId::Spanish, "s2");
    REQUIRE(sys.totalTranslated(LocaleId::Spanish) == 2);
    REQUIRE(sys.totalTranslated(LocaleId::Russian) == 0);
}
