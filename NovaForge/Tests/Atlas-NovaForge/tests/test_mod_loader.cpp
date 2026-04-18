#include "../engine/mod/ModLoader.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::mod;

void test_mod_loader_register() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "ships";
    desc.name = "Ships Mod";
    desc.version = "1.0.0";

    ModLoadResult result = loader.RegisterMod(desc);
    assert(result == ModLoadResult::Success);
    assert(loader.ModCount() == 1);
    assert(loader.HasMod("ships"));

    std::cout << "[PASS] test_mod_loader_register" << std::endl;
}

void test_mod_loader_register_invalid() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "";
    desc.name = "";

    ModLoadResult result = loader.RegisterMod(desc);
    assert(result == ModLoadResult::InvalidDescriptor);
    assert(loader.ModCount() == 0);

    std::cout << "[PASS] test_mod_loader_register_invalid" << std::endl;
}

void test_mod_loader_register_duplicate() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "ships";
    desc.name = "Ships Mod";

    loader.RegisterMod(desc);
    ModLoadResult result = loader.RegisterMod(desc);
    assert(result == ModLoadResult::AlreadyLoaded);
    assert(loader.ModCount() == 1);

    std::cout << "[PASS] test_mod_loader_register_duplicate" << std::endl;
}

void test_mod_loader_unregister() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "ships";
    desc.name = "Ships Mod";
    loader.RegisterMod(desc);

    assert(loader.UnregisterMod("ships") == true);
    assert(loader.ModCount() == 0);
    assert(!loader.HasMod("ships"));

    assert(loader.UnregisterMod("ships") == false);

    std::cout << "[PASS] test_mod_loader_unregister" << std::endl;
}

void test_mod_loader_enable_disable() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "ships";
    desc.name = "Ships Mod";
    loader.RegisterMod(desc);

    assert(loader.EnabledMods().empty());

    assert(loader.EnableMod("ships") == true);
    assert(loader.EnabledMods().size() == 1);

    assert(loader.DisableMod("ships") == true);
    assert(loader.EnabledMods().empty());

    assert(loader.EnableMod("nonexistent") == false);
    assert(loader.DisableMod("nonexistent") == false);

    std::cout << "[PASS] test_mod_loader_enable_disable" << std::endl;
}

void test_mod_loader_get_mod() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "ships";
    desc.name = "Ships Mod";
    desc.version = "2.0.0";
    desc.author = "Test Author";
    loader.RegisterMod(desc);

    const ModDescriptor* found = loader.GetMod("ships");
    assert(found != nullptr);
    assert(found->name == "Ships Mod");
    assert(found->version == "2.0.0");
    assert(found->author == "Test Author");

    assert(loader.GetMod("nonexistent") == nullptr);

    std::cout << "[PASS] test_mod_loader_get_mod" << std::endl;
}

void test_mod_loader_dependencies() {
    ModLoader loader;

    ModDescriptor base;
    base.id = "base";
    base.name = "Base Mod";
    loader.RegisterMod(base);

    ModDescriptor ships;
    ships.id = "ships";
    ships.name = "Ships Mod";
    ships.dependencies = {"base", "weapons"};
    loader.RegisterMod(ships);

    assert(loader.ValidateDependencies("base") == true);
    assert(loader.ValidateDependencies("ships") == false);

    auto missing = loader.MissingDependencies("ships");
    assert(missing.size() == 1);
    assert(missing[0] == "weapons");

    // Add the missing dependency
    ModDescriptor weapons;
    weapons.id = "weapons";
    weapons.name = "Weapons Mod";
    loader.RegisterMod(weapons);

    assert(loader.ValidateDependencies("ships") == true);
    assert(loader.MissingDependencies("ships").empty());

    std::cout << "[PASS] test_mod_loader_dependencies" << std::endl;
}

void test_mod_loader_scan_directory() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_mod_scan_test").string();
    std::filesystem::remove_all(tmpDir);
    std::filesystem::create_directories(tmpDir + "/mod_a");
    std::filesystem::create_directories(tmpDir + "/mod_b");
    std::filesystem::create_directories(tmpDir + "/not_a_mod");

    {
        std::ofstream f(tmpDir + "/mod_a/mod.json");
        f << "{}";
    }
    {
        std::ofstream f(tmpDir + "/mod_b/mod.json");
        f << "{}";
    }

    ModLoader loader;
    size_t found = loader.ScanDirectory(tmpDir);
    assert(found == 2);
    assert(loader.ModCount() == 2);

    std::filesystem::remove_all(tmpDir);

    std::cout << "[PASS] test_mod_loader_scan_directory" << std::endl;
}

void test_mod_loader_clear() {
    ModLoader loader;

    ModDescriptor desc;
    desc.id = "ships";
    desc.name = "Ships Mod";
    loader.RegisterMod(desc);
    assert(loader.ModCount() == 1);

    loader.Clear();
    assert(loader.ModCount() == 0);

    std::cout << "[PASS] test_mod_loader_clear" << std::endl;
}
