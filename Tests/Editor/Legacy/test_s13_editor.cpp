#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S13 Tests: Localization System ──────────────────────────────

TEST_CASE("LocaleId names", "[Editor][S13]") {
    REQUIRE(std::string(localeIdName(LocaleId::English))  == "English");
    REQUIRE(std::string(localeIdName(LocaleId::Spanish))  == "Spanish");
    REQUIRE(std::string(localeIdName(LocaleId::French))   == "French");
    REQUIRE(std::string(localeIdName(LocaleId::German))   == "German");
    REQUIRE(std::string(localeIdName(LocaleId::Japanese)) == "Japanese");
    REQUIRE(std::string(localeIdName(LocaleId::Chinese))  == "Chinese");
    REQUIRE(std::string(localeIdName(LocaleId::Korean))   == "Korean");
    REQUIRE(std::string(localeIdName(LocaleId::Russian))  == "Russian");
}

TEST_CASE("LocalizedString defaults", "[Editor][S13]") {
    LocalizedString ls;
    REQUIRE_FALSE(ls.isValid());
    REQUIRE_FALSE(ls.isVerified());
    REQUIRE(ls.locale == LocaleId::English);

    ls.key = "greeting";
    ls.value = "Hello";
    REQUIRE(ls.isValid());

    ls.verify();
    REQUIRE(ls.isVerified());
}

TEST_CASE("TranslationEntry set/get/has", "[Editor][S13]") {
    TranslationEntry entry;
    entry.key = "btn_ok";
    REQUIRE(entry.localeCount() == 0);
    REQUIRE_FALSE(entry.has(LocaleId::English));

    entry.set(LocaleId::English, "OK");
    entry.set(LocaleId::Spanish, "Aceptar");
    REQUIRE(entry.localeCount() == 2);
    REQUIRE(entry.has(LocaleId::English));
    REQUIRE(entry.has(LocaleId::Spanish));
    REQUIRE_FALSE(entry.has(LocaleId::French));

    const auto* en = entry.get(LocaleId::English);
    REQUIRE(en != nullptr);
    REQUIRE(*en == "OK");

    const auto* fr = entry.get(LocaleId::French);
    REQUIRE(fr == nullptr);
}

TEST_CASE("TranslationTable addEntry + duplicate rejection", "[Editor][S13]") {
    TranslationTable table("UI");
    REQUIRE(table.addEntry("btn_ok"));
    REQUIRE(table.entryCount() == 1);
    REQUIRE_FALSE(table.addEntry("btn_ok")); // duplicate
    REQUIRE(table.entryCount() == 1);
}

TEST_CASE("TranslationTable removeEntry", "[Editor][S13]") {
    TranslationTable table("UI");
    table.addEntry("btn_ok");
    REQUIRE(table.removeEntry("btn_ok"));
    REQUIRE(table.entryCount() == 0);
    REQUIRE_FALSE(table.removeEntry("nonexistent"));
}

TEST_CASE("TranslationTable findEntry", "[Editor][S13]") {
    TranslationTable table("UI");
    table.addEntry("btn_ok", "Button label");
    auto* e = table.findEntry("btn_ok");
    REQUIRE(e != nullptr);
    REQUIRE(e->context == "Button label");
    REQUIRE(table.findEntry("missing") == nullptr);
}

TEST_CASE("TranslationTable setTranslation + lookup", "[Editor][S13]") {
    TranslationTable table("UI");
    table.addEntry("greeting");
    REQUIRE(table.setTranslation("greeting", LocaleId::English, "Hello"));
    REQUIRE(table.setTranslation("greeting", LocaleId::French, "Bonjour"));

    auto* en = table.lookup("greeting", LocaleId::English);
    REQUIRE(en != nullptr);
    REQUIRE(*en == "Hello");

    auto* fr = table.lookup("greeting", LocaleId::French);
    REQUIRE(fr != nullptr);
    REQUIRE(*fr == "Bonjour");

    REQUIRE(table.lookup("greeting", LocaleId::German) == nullptr);
    REQUIRE(table.lookup("missing", LocaleId::English) == nullptr);
}

TEST_CASE("TranslationTable translatedCount + completionRate", "[Editor][S13]") {
    TranslationTable table("UI");
    table.addEntry("a");
    table.addEntry("b");
    table.addEntry("c");

    table.setTranslation("a", LocaleId::English, "A");
    table.setTranslation("b", LocaleId::English, "B");
    // c not translated to English

    REQUIRE(table.translatedCount(LocaleId::English) == 2);
    REQUIRE(table.completionRate(LocaleId::English) == Catch::Approx(2.f / 3.f));
    REQUIRE(table.completionRate(LocaleId::French) == Catch::Approx(0.f));
}

TEST_CASE("LocaleManager active/fallback/resolve", "[Editor][S13]") {
    TranslationTable table("UI");
    table.addEntry("greeting");
    table.setTranslation("greeting", LocaleId::English, "Hello");

    LocaleManager mgr(LocaleId::French);
    mgr.setFallback(LocaleId::English);

    // French not available, should fallback to English
    auto* result = mgr.resolve(table, "greeting");
    REQUIRE(result != nullptr);
    REQUIRE(*result == "Hello");

    // Add French translation
    table.setTranslation("greeting", LocaleId::French, "Bonjour");
    result = mgr.resolve(table, "greeting");
    REQUIRE(result != nullptr);
    REQUIRE(*result == "Bonjour");
}

TEST_CASE("LocaleManager switchCount", "[Editor][S13]") {
    LocaleManager mgr;
    REQUIRE(mgr.switchCount() == 0);
    mgr.setActive(LocaleId::Spanish);
    REQUIRE(mgr.switchCount() == 1);
    mgr.setActive(LocaleId::French);
    REQUIRE(mgr.switchCount() == 2);
    REQUIRE(mgr.active() == LocaleId::French);
}

TEST_CASE("LocalizationSystem init/shutdown", "[Editor][S13]") {
    LocalizationSystem ls;
    REQUIRE_FALSE(ls.isInitialized());
    ls.init();
    REQUIRE(ls.isInitialized());
    ls.shutdown();
    REQUIRE_FALSE(ls.isInitialized());
}

TEST_CASE("LocalizationSystem createTable + duplicate rejection", "[Editor][S13]") {
    LocalizationSystem ls;
    ls.init();
    REQUIRE(ls.createTable("UI") == 0);
    REQUIRE(ls.createTable("Game") == 1);
    REQUIRE(ls.createTable("UI") == -1); // duplicate
    REQUIRE(ls.tableCount() == 2);
}

TEST_CASE("LocalizationSystem tableByName", "[Editor][S13]") {
    LocalizationSystem ls;
    ls.init();
    ls.createTable("UI");
    REQUIRE(ls.tableByName("UI") != nullptr);
    REQUIRE(ls.tableByName("UI")->name() == "UI");
    REQUIRE(ls.tableByName("Other") == nullptr);
}

TEST_CASE("LocalizationSystem translate with locale fallback", "[Editor][S13]") {
    LocalizationSystem ls;
    LocalizationConfig config;
    config.defaultLocale = LocaleId::French;
    config.fallbackLocale = LocaleId::English;
    ls.init(config);
    ls.createTable("UI");

    auto* t = ls.tableByName("UI");
    t->addEntry("ok");
    t->setTranslation("ok", LocaleId::English, "OK");

    // French not set, falls back to English
    auto* result = ls.translate("UI", "ok");
    REQUIRE(result != nullptr);
    REQUIRE(*result == "OK");

    // Set French
    t->setTranslation("ok", LocaleId::French, "D'accord");
    result = ls.translate("UI", "ok");
    REQUIRE(result != nullptr);
    REQUIRE(*result == "D'accord");
}

TEST_CASE("LocalizationSystem setLocale + activeLocale", "[Editor][S13]") {
    LocalizationSystem ls;
    ls.init();
    REQUIRE(ls.activeLocale() == LocaleId::English);
    ls.setLocale(LocaleId::Japanese);
    REQUIRE(ls.activeLocale() == LocaleId::Japanese);
}

TEST_CASE("LocalizationSystem tick + totalEntries/totalTranslated", "[Editor][S13]") {
    LocalizationSystem ls;
    ls.init();
    ls.createTable("UI");

    auto* t = ls.tableByName("UI");
    t->addEntry("a");
    t->addEntry("b");
    t->setTranslation("a", LocaleId::English, "A");

    ls.tick(0.016f);
    ls.tick(0.016f);
    REQUIRE(ls.tickCount() == 2);
    REQUIRE(ls.totalEntries() == 2);
    REQUIRE(ls.totalTranslated(LocaleId::English) == 1);
}

TEST_CASE("LocalizationSystem not initialized rejects operations", "[Editor][S13]") {
    LocalizationSystem ls;
    REQUIRE(ls.createTable("UI") == -1);
}
