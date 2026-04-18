#include "../engine/assets/AssetCategoryRegistry.h"
#include <iostream>
#include <cassert>

using namespace atlas::asset;

void test_category_registry_empty() {
    AssetCategoryRegistry reg;
    assert(reg.EntryCount() == 0);
    assert(reg.Entries().empty());
    assert(reg.Find(AssetType::Mesh) == nullptr);

    std::cout << "[PASS] test_category_registry_empty" << std::endl;
}

void test_category_registry_register() {
    AssetCategoryRegistry reg;
    reg.Register({AssetType::Mesh, ContentCategory::Visual, SystemUsage::Rendering,
                  "Mesh", "3D geometry"});

    assert(reg.EntryCount() == 1);
    const auto* entry = reg.Find(AssetType::Mesh);
    assert(entry != nullptr);
    assert(entry->content == ContentCategory::Visual);
    assert(entry->system == SystemUsage::Rendering);
    assert(entry->label == "Mesh");

    std::cout << "[PASS] test_category_registry_register" << std::endl;
}

void test_category_registry_replace() {
    AssetCategoryRegistry reg;
    reg.Register({AssetType::Audio, ContentCategory::Audio, SystemUsage::General,
                  "Audio", "Sound"});
    reg.Register({AssetType::Audio, ContentCategory::Audio, SystemUsage::Gameplay,
                  "Audio", "Gameplay sound"});

    assert(reg.EntryCount() == 1);
    const auto* entry = reg.Find(AssetType::Audio);
    assert(entry->system == SystemUsage::Gameplay);
    assert(entry->description == "Gameplay sound");

    std::cout << "[PASS] test_category_registry_replace" << std::endl;
}

void test_category_registry_unregister() {
    AssetCategoryRegistry reg;
    reg.Register({AssetType::Font, ContentCategory::Font, SystemUsage::UI,
                  "Font", "Font file"});
    assert(reg.EntryCount() == 1);

    bool removed = reg.Unregister(AssetType::Font);
    assert(removed);
    assert(reg.EntryCount() == 0);

    // Removing non-existent returns false.
    assert(!reg.Unregister(AssetType::Font));

    std::cout << "[PASS] test_category_registry_unregister" << std::endl;
}

void test_category_registry_by_content() {
    AssetCategoryRegistry reg;
    reg.RegisterDefaults();

    auto visuals = reg.ByContent(ContentCategory::Visual);
    assert(visuals.size() >= 3);  // Mesh, Texture, Material, SpriteSheet

    auto audio = reg.ByContent(ContentCategory::Audio);
    assert(audio.size() == 1);

    auto fonts = reg.ByContent(ContentCategory::Font);
    assert(fonts.size() == 1);

    std::cout << "[PASS] test_category_registry_by_content" << std::endl;
}

void test_category_registry_by_system() {
    AssetCategoryRegistry reg;
    reg.RegisterDefaults();

    auto rendering = reg.BySystem(SystemUsage::Rendering);
    assert(rendering.size() >= 3);  // Mesh, Texture, Material

    auto ui = reg.BySystem(SystemUsage::UI);
    assert(ui.size() >= 2);  // Font, SpriteSheet

    auto gameplay = reg.BySystem(SystemUsage::Gameplay);
    assert(gameplay.size() >= 1);  // Mechanic, GameType

    std::cout << "[PASS] test_category_registry_by_system" << std::endl;
}

void test_category_registry_content_for() {
    AssetCategoryRegistry reg;
    reg.RegisterDefaults();

    assert(reg.ContentFor(AssetType::Mesh) == ContentCategory::Visual);
    assert(reg.ContentFor(AssetType::Audio) == ContentCategory::Audio);
    assert(reg.ContentFor(AssetType::Font) == ContentCategory::Font);
    assert(reg.ContentFor(AssetType::World) == ContentCategory::Composite);

    std::cout << "[PASS] test_category_registry_content_for" << std::endl;
}

void test_category_registry_system_for() {
    AssetCategoryRegistry reg;
    reg.RegisterDefaults();

    assert(reg.SystemFor(AssetType::Mesh) == SystemUsage::Rendering);
    assert(reg.SystemFor(AssetType::Font) == SystemUsage::UI);
    assert(reg.SystemFor(AssetType::Mechanic) == SystemUsage::Gameplay);

    std::cout << "[PASS] test_category_registry_system_for" << std::endl;
}

void test_category_registry_defaults() {
    AssetCategoryRegistry reg;
    reg.RegisterDefaults();

    assert(reg.EntryCount() == 14);  // All 14 built-in types

    // Verify every built-in type is registered.
    assert(reg.Find(AssetType::Graph) != nullptr);
    assert(reg.Find(AssetType::World) != nullptr);
    assert(reg.Find(AssetType::Planet) != nullptr);
    assert(reg.Find(AssetType::Galaxy) != nullptr);
    assert(reg.Find(AssetType::Mesh) != nullptr);
    assert(reg.Find(AssetType::Material) != nullptr);
    assert(reg.Find(AssetType::Mechanic) != nullptr);
    assert(reg.Find(AssetType::VoxelSchema) != nullptr);
    assert(reg.Find(AssetType::GameType) != nullptr);
    assert(reg.Find(AssetType::EditorProfile) != nullptr);
    assert(reg.Find(AssetType::Texture) != nullptr);
    assert(reg.Find(AssetType::Audio) != nullptr);
    assert(reg.Find(AssetType::Font) != nullptr);
    assert(reg.Find(AssetType::SpriteSheet) != nullptr);

    std::cout << "[PASS] test_category_registry_defaults" << std::endl;
}

void test_category_registry_clear() {
    AssetCategoryRegistry reg;
    reg.RegisterDefaults();
    assert(reg.EntryCount() > 0);

    reg.Clear();
    assert(reg.EntryCount() == 0);

    std::cout << "[PASS] test_category_registry_clear" << std::endl;
}

void test_category_registry_unregistered_defaults() {
    AssetCategoryRegistry reg;
    // Without registering defaults, ContentFor/SystemFor return defaults.
    assert(reg.ContentFor(AssetType::Mesh) == ContentCategory::Data);
    assert(reg.SystemFor(AssetType::Mesh) == SystemUsage::General);

    std::cout << "[PASS] test_category_registry_unregistered_defaults" << std::endl;
}
