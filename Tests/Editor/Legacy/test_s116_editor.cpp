// S116 editor tests: ModdingToolkit, ContentPackEditor, ScriptingConsole
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ContentPackEditor.h"
#include "NF/Editor/ModdingToolkit.h"

using namespace NF;

// ── ModdingToolkit ────────────────────────────────────────────────────────────

TEST_CASE("ModType names", "[Editor][S116]") {
    REQUIRE(std::string(modTypeName(ModType::Content))  == "Content");
    REQUIRE(std::string(modTypeName(ModType::Gameplay)) == "Gameplay");
    REQUIRE(std::string(modTypeName(ModType::UI))       == "UI");
    REQUIRE(std::string(modTypeName(ModType::Audio))    == "Audio");
    REQUIRE(std::string(modTypeName(ModType::Visual))   == "Visual");
    REQUIRE(std::string(modTypeName(ModType::Script))   == "Script");
    REQUIRE(std::string(modTypeName(ModType::Total))    == "Total");
    REQUIRE(std::string(modTypeName(ModType::Patch))    == "Patch");
}

TEST_CASE("ModLoadOrder names", "[Editor][S116]") {
    REQUIRE(std::string(modLoadOrderName(ModLoadOrder::Early))    == "Early");
    REQUIRE(std::string(modLoadOrderName(ModLoadOrder::Normal))   == "Normal");
    REQUIRE(std::string(modLoadOrderName(ModLoadOrder::Late))     == "Late");
    REQUIRE(std::string(modLoadOrderName(ModLoadOrder::Override)) == "Override");
}

TEST_CASE("ModCompatibility names", "[Editor][S116]") {
    REQUIRE(std::string(modCompatibilityName(ModCompatibility::Compatible))   == "Compatible");
    REQUIRE(std::string(modCompatibilityName(ModCompatibility::RequiresBase)) == "RequiresBase");
    REQUIRE(std::string(modCompatibilityName(ModCompatibility::Incompatible)) == "Incompatible");
    REQUIRE(std::string(modCompatibilityName(ModCompatibility::Unknown))      == "Unknown");
}

TEST_CASE("ModDef defaults", "[Editor][S116]") {
    ModDef md(1, "hd_textures", ModType::Visual);
    REQUIRE(md.id()            == 1u);
    REQUIRE(md.name()          == "hd_textures");
    REQUIRE(md.type()          == ModType::Visual);
    REQUIRE(md.loadOrder()     == ModLoadOrder::Normal);
    REQUIRE(md.compatibility() == ModCompatibility::Unknown);
    REQUIRE(md.version()       == "1.0.0");
    REQUIRE(md.isEnabled());
    REQUIRE(!md.isRequired());
}

TEST_CASE("ModDef mutation", "[Editor][S116]") {
    ModDef md(2, "gameplay_overhaul", ModType::Gameplay);
    md.setLoadOrder(ModLoadOrder::Early);
    md.setCompatibility(ModCompatibility::RequiresBase);
    md.setVersion("2.1.0");
    md.setIsEnabled(false);
    md.setIsRequired(true);
    REQUIRE(md.loadOrder()     == ModLoadOrder::Early);
    REQUIRE(md.compatibility() == ModCompatibility::RequiresBase);
    REQUIRE(md.version()       == "2.1.0");
    REQUIRE(!md.isEnabled());
    REQUIRE(md.isRequired());
}

TEST_CASE("ModdingToolkit defaults", "[Editor][S116]") {
    ModdingToolkit tk;
    REQUIRE(!tk.isShowDisabled());
    REQUIRE(tk.isShowConflicts());
    REQUIRE(!tk.isAutoResolve());
    REQUIRE(tk.modCount() == 0u);
}

TEST_CASE("ModdingToolkit add/remove mods", "[Editor][S116]") {
    ModdingToolkit tk;
    REQUIRE(tk.addMod(ModDef(1, "hd_tex",   ModType::Visual)));
    REQUIRE(tk.addMod(ModDef(2, "gameplay", ModType::Gameplay)));
    REQUIRE(tk.addMod(ModDef(3, "audio",    ModType::Audio)));
    REQUIRE(!tk.addMod(ModDef(1, "hd_tex",  ModType::Visual)));
    REQUIRE(tk.modCount() == 3u);
    REQUIRE(tk.removeMod(2));
    REQUIRE(tk.modCount() == 2u);
    REQUIRE(!tk.removeMod(99));
}

TEST_CASE("ModdingToolkit counts and find", "[Editor][S116]") {
    ModdingToolkit tk;
    ModDef m1(1, "vis_a",  ModType::Visual);
    ModDef m2(2, "vis_b",  ModType::Visual);  m2.setCompatibility(ModCompatibility::Compatible);
    ModDef m3(3, "play_a", ModType::Gameplay); m3.setIsEnabled(false);
    ModDef m4(4, "aud_a",  ModType::Audio);   m4.setCompatibility(ModCompatibility::Compatible); m4.setIsEnabled(false);
    tk.addMod(m1); tk.addMod(m2); tk.addMod(m3); tk.addMod(m4);
    REQUIRE(tk.countByType(ModType::Visual)                          == 2u);
    REQUIRE(tk.countByType(ModType::Gameplay)                        == 1u);
    REQUIRE(tk.countByType(ModType::Script)                          == 0u);
    REQUIRE(tk.countByCompatibility(ModCompatibility::Unknown)       == 2u);
    REQUIRE(tk.countByCompatibility(ModCompatibility::Compatible)    == 2u);
    REQUIRE(tk.countEnabled()                                        == 2u);
    auto* found = tk.findMod(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == ModType::Gameplay);
    REQUIRE(tk.findMod(99) == nullptr);
}

TEST_CASE("ModdingToolkit settings mutation", "[Editor][S116]") {
    ModdingToolkit tk;
    tk.setShowDisabled(true);
    tk.setShowConflicts(false);
    tk.setAutoResolve(true);
    REQUIRE(tk.isShowDisabled());
    REQUIRE(!tk.isShowConflicts());
    REQUIRE(tk.isAutoResolve());
}

// ── ContentPackEditor ─────────────────────────────────────────────────────────

TEST_CASE("ContentPackCategory names", "[Editor][S116]") {
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::DLC))          == "DLC");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::Expansion))    == "Expansion");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::CosmeticPack)) == "CosmeticPack");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::MapPack))      == "MapPack");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::SoundPack))    == "SoundPack");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::StoryPack))    == "StoryPack");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::SeasonPass))   == "SeasonPass");
    REQUIRE(std::string(contentPackCategoryName(ContentPackCategory::Free))         == "Free");
}

TEST_CASE("ContentPackStatus names", "[Editor][S116]") {
    REQUIRE(std::string(contentPackStatusName(ContentPackStatus::Draft))       == "Draft");
    REQUIRE(std::string(contentPackStatusName(ContentPackStatus::InReview))    == "InReview");
    REQUIRE(std::string(contentPackStatusName(ContentPackStatus::Approved))    == "Approved");
    REQUIRE(std::string(contentPackStatusName(ContentPackStatus::Published))   == "Published");
    REQUIRE(std::string(contentPackStatusName(ContentPackStatus::Deprecated))  == "Deprecated");
    REQUIRE(std::string(contentPackStatusName(ContentPackStatus::Recalled))    == "Recalled");
}

TEST_CASE("ContentPackPlatform names", "[Editor][S116]") {
    REQUIRE(std::string(contentPackPlatformName(ContentPackPlatform::PC))      == "PC");
    REQUIRE(std::string(contentPackPlatformName(ContentPackPlatform::Console)) == "Console");
    REQUIRE(std::string(contentPackPlatformName(ContentPackPlatform::Mobile))  == "Mobile");
    REQUIRE(std::string(contentPackPlatformName(ContentPackPlatform::All))     == "All");
}

TEST_CASE("ContentPackDef defaults", "[Editor][S116]") {
    ContentPackDef cp(1, "dark_forest_dlc", ContentPackCategory::DLC);
    REQUIRE(cp.id()       == 1u);
    REQUIRE(cp.name()     == "dark_forest_dlc");
    REQUIRE(cp.category() == ContentPackCategory::DLC);
    REQUIRE(cp.status()   == ContentPackStatus::Draft);
    REQUIRE(cp.platform() == ContentPackPlatform::All);
    REQUIRE(cp.sizeMB()   == 0u);
    REQUIRE(!cp.isFree());
    REQUIRE(cp.version()  == "1.0.0");
}

TEST_CASE("ContentPackDef mutation", "[Editor][S116]") {
    ContentPackDef cp(2, "free_skins", ContentPackCategory::CosmeticPack);
    cp.setStatus(ContentPackStatus::Published);
    cp.setPlatform(ContentPackPlatform::PC);
    cp.setSizeMB(512u);
    cp.setIsFree(true);
    cp.setVersion("2.0.0");
    REQUIRE(cp.status()   == ContentPackStatus::Published);
    REQUIRE(cp.platform() == ContentPackPlatform::PC);
    REQUIRE(cp.sizeMB()   == 512u);
    REQUIRE(cp.isFree());
    REQUIRE(cp.version()  == "2.0.0");
}

TEST_CASE("ContentPackEditor defaults", "[Editor][S116]") {
    ContentPackEditor ed;
    REQUIRE(!ed.isShowDeprecated());
    REQUIRE(ed.isShowPreview());
    REQUIRE(ed.filterPlatform() == ContentPackPlatform::All);
    REQUIRE(ed.packCount()      == 0u);
}

TEST_CASE("ContentPackEditor add/remove packs", "[Editor][S116]") {
    ContentPackEditor ed;
    REQUIRE(ed.addPack(ContentPackDef(1, "dlc1",      ContentPackCategory::DLC)));
    REQUIRE(ed.addPack(ContentPackDef(2, "cosmetic1", ContentPackCategory::CosmeticPack)));
    REQUIRE(ed.addPack(ContentPackDef(3, "mappack1",  ContentPackCategory::MapPack)));
    REQUIRE(!ed.addPack(ContentPackDef(1, "dlc1",     ContentPackCategory::DLC)));
    REQUIRE(ed.packCount() == 3u);
    REQUIRE(ed.removePack(2));
    REQUIRE(ed.packCount() == 2u);
    REQUIRE(!ed.removePack(99));
}

TEST_CASE("ContentPackEditor counts and find", "[Editor][S116]") {
    ContentPackEditor ed;
    ContentPackDef p1(1, "dlc_a",  ContentPackCategory::DLC);
    ContentPackDef p2(2, "dlc_b",  ContentPackCategory::DLC);      p2.setStatus(ContentPackStatus::Published);
    ContentPackDef p3(3, "free_a", ContentPackCategory::Free);      p3.setIsFree(true);
    ContentPackDef p4(4, "map_a",  ContentPackCategory::MapPack);   p4.setStatus(ContentPackStatus::Published); p4.setIsFree(true);
    ed.addPack(p1); ed.addPack(p2); ed.addPack(p3); ed.addPack(p4);
    REQUIRE(ed.countByCategory(ContentPackCategory::DLC)       == 2u);
    REQUIRE(ed.countByCategory(ContentPackCategory::Free)      == 1u);
    REQUIRE(ed.countByCategory(ContentPackCategory::StoryPack) == 0u);
    REQUIRE(ed.countByStatus(ContentPackStatus::Draft)         == 2u);
    REQUIRE(ed.countByStatus(ContentPackStatus::Published)     == 2u);
    REQUIRE(ed.countFree()                                     == 2u);
    auto* found = ed.findPack(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == ContentPackCategory::Free);
    REQUIRE(ed.findPack(99) == nullptr);
}

TEST_CASE("ContentPackEditor settings mutation", "[Editor][S116]") {
    ContentPackEditor ed;
    ed.setShowDeprecated(true);
    ed.setShowPreview(false);
    ed.setFilterPlatform(ContentPackPlatform::Console);
    REQUIRE(ed.isShowDeprecated());
    REQUIRE(!ed.isShowPreview());
    REQUIRE(ed.filterPlatform() == ContentPackPlatform::Console);
}

// ── ScriptingConsole ──────────────────────────────────────────────────────────

TEST_CASE("SConLanguage names", "[Editor][S116]") {
    REQUIRE(std::string(sConLanguageName(SConLanguage::Lua))         == "Lua");
    REQUIRE(std::string(sConLanguageName(SConLanguage::Python))      == "Python");
    REQUIRE(std::string(sConLanguageName(SConLanguage::JavaScript))  == "JavaScript");
    REQUIRE(std::string(sConLanguageName(SConLanguage::AngelScript)) == "AngelScript");
    REQUIRE(std::string(sConLanguageName(SConLanguage::Squirrel))    == "Squirrel");
    REQUIRE(std::string(sConLanguageName(SConLanguage::CSharp))      == "CSharp");
}

TEST_CASE("ScriptExecutionMode names", "[Editor][S116]") {
    REQUIRE(std::string(scriptExecutionModeName(ScriptExecutionMode::Immediate)) == "Immediate");
    REQUIRE(std::string(scriptExecutionModeName(ScriptExecutionMode::Deferred))  == "Deferred");
    REQUIRE(std::string(scriptExecutionModeName(ScriptExecutionMode::Scheduled)) == "Scheduled");
    REQUIRE(std::string(scriptExecutionModeName(ScriptExecutionMode::OnEvent))   == "OnEvent");
}

TEST_CASE("ScriptSandboxLevel names", "[Editor][S116]") {
    REQUIRE(std::string(scriptSandboxLevelName(ScriptSandboxLevel::None))   == "None");
    REQUIRE(std::string(scriptSandboxLevelName(ScriptSandboxLevel::Low))    == "Low");
    REQUIRE(std::string(scriptSandboxLevelName(ScriptSandboxLevel::Medium)) == "Medium");
    REQUIRE(std::string(scriptSandboxLevelName(ScriptSandboxLevel::High))   == "High");
    REQUIRE(std::string(scriptSandboxLevelName(ScriptSandboxLevel::Strict)) == "Strict");
}

TEST_CASE("ScriptEntry defaults", "[Editor][S116]") {
    ScriptEntry se(1, "spawn_handler", SConLanguage::Lua);
    REQUIRE(se.id()            == 1u);
    REQUIRE(se.name()          == "spawn_handler");
    REQUIRE(se.language()      == SConLanguage::Lua);
    REQUIRE(se.executionMode() == ScriptExecutionMode::Immediate);
    REQUIRE(se.sandboxLevel()  == ScriptSandboxLevel::Medium);
    REQUIRE(se.isEnabled());
    REQUIRE(se.source()        == "");
}

TEST_CASE("ScriptEntry mutation", "[Editor][S116]") {
    ScriptEntry se(2, "event_hook", SConLanguage::Python);
    se.setExecutionMode(ScriptExecutionMode::OnEvent);
    se.setSandboxLevel(ScriptSandboxLevel::Strict);
    se.setIsEnabled(false);
    se.setSource("def on_event(e): pass");
    REQUIRE(se.executionMode() == ScriptExecutionMode::OnEvent);
    REQUIRE(se.sandboxLevel()  == ScriptSandboxLevel::Strict);
    REQUIRE(!se.isEnabled());
    REQUIRE(se.source()        == "def on_event(e): pass");
}

TEST_CASE("ScriptingConsole defaults", "[Editor][S116]") {
    ScriptingConsole con;
    REQUIRE(con.activeLanguage()    == SConLanguage::Lua);
    REQUIRE(con.isShowLineNumbers());
    REQUIRE(con.isShowAutoComplete());
    REQUIRE(con.sandboxLevel()      == ScriptSandboxLevel::Medium);
    REQUIRE(con.scriptCount()       == 0u);
}

TEST_CASE("ScriptingConsole add/remove scripts", "[Editor][S116]") {
    ScriptingConsole con;
    REQUIRE(con.addScript(ScriptEntry(1, "lua_script1",  SConLanguage::Lua)));
    REQUIRE(con.addScript(ScriptEntry(2, "py_script1",   SConLanguage::Python)));
    REQUIRE(con.addScript(ScriptEntry(3, "js_script1",   SConLanguage::JavaScript)));
    REQUIRE(!con.addScript(ScriptEntry(1, "lua_script1", SConLanguage::Lua)));
    REQUIRE(con.scriptCount() == 3u);
    REQUIRE(con.removeScript(2));
    REQUIRE(con.scriptCount() == 2u);
    REQUIRE(!con.removeScript(99));
}

TEST_CASE("ScriptingConsole counts and find", "[Editor][S116]") {
    ScriptingConsole con;
    ScriptEntry s1(1, "lua_a",  SConLanguage::Lua);
    ScriptEntry s2(2, "lua_b",  SConLanguage::Lua);    s2.setExecutionMode(ScriptExecutionMode::Deferred);
    ScriptEntry s3(3, "py_a",   SConLanguage::Python); s3.setIsEnabled(false);
    ScriptEntry s4(4, "ang_a",  SConLanguage::AngelScript); s4.setExecutionMode(ScriptExecutionMode::Deferred); s4.setIsEnabled(false);
    con.addScript(s1); con.addScript(s2); con.addScript(s3); con.addScript(s4);
    REQUIRE(con.countByLanguage(SConLanguage::Lua)                     == 2u);
    REQUIRE(con.countByLanguage(SConLanguage::Python)                  == 1u);
    REQUIRE(con.countByLanguage(SConLanguage::CSharp)                  == 0u);
    REQUIRE(con.countByExecutionMode(ScriptExecutionMode::Immediate)     == 2u);
    REQUIRE(con.countByExecutionMode(ScriptExecutionMode::Deferred)      == 2u);
    REQUIRE(con.countEnabled()                                           == 2u);
    auto* found = con.findScript(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->language() == SConLanguage::Python);
    REQUIRE(con.findScript(99) == nullptr);
}

TEST_CASE("ScriptingConsole settings mutation", "[Editor][S116]") {
    ScriptingConsole con;
    con.setActiveLanguage(SConLanguage::Python);
    con.setShowLineNumbers(false);
    con.setShowAutoComplete(false);
    con.setSandboxLevel(ScriptSandboxLevel::Strict);
    REQUIRE(con.activeLanguage()    == SConLanguage::Python);
    REQUIRE(!con.isShowLineNumbers());
    REQUIRE(!con.isShowAutoComplete());
    REQUIRE(con.sandboxLevel()      == ScriptSandboxLevel::Strict);
}
