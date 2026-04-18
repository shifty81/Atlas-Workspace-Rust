#pragma once
// NovaForge::NovaForgeDocumentRegistry — maps document type → schema + load/save paths.
//
// Maintains the global set of document type registrations for the NovaForge project.
// Each registration describes:
//   - The document type enum value
//   - The data root subdirectory (relative to Data/)
//   - The file extension used for documents of that type
//   - The schema root subdirectory (relative to Schema/)

#include "NovaForge/EditorAdapter/NovaForgeDocument.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NovaForge {

struct DocumentTypeRegistration {
    NovaForgeDocumentType type;
    std::string           displayName;
    std::string           dataSubdir;
    std::string           fileExtension;
    std::string           schemaSubdir;
    std::function<std::unique_ptr<NovaForgeDocument>(std::string filePath)> factory;
};

class NovaForgeDocumentRegistry {
public:
    void registerBuiltins() {
        registerType({
            NovaForgeDocumentType::ItemDefinition, "Item Definition",
            "Items", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::ItemDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::StructureArchetype, "Structure Archetype",
            "Structures", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::StructureArchetype, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::BiomeDefinition, "Biome Definition",
            "Biomes", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::BiomeDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::PlanetArchetype, "Planet Archetype",
            "Planets", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::PlanetArchetype, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::FactionDefinition, "Faction Definition",
            "Factions", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::FactionDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::MissionDefinition, "Mission Definition",
            "Missions", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::MissionDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::ProgressionRules, "Progression Rules",
            "Rules", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::ProgressionRules, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::CharacterRules, "Character Rules",
            "Rules", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::CharacterRules, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::EconomyRules, "Economy Rules",
            "Rules", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::EconomyRules, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::CraftingDefinition, "Crafting Definition",
            "Crafting", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::CraftingDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::PCGRuleset, "PCG Ruleset",
            "PCG/RuleSets", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::PCGRuleset, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::WorldDocument, "World Document",
            "Worlds", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::WorldDocument, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::LevelInstance, "Level Instance",
            "Levels", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::LevelInstance, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::EntityTemplate, "Entity Template",
            "Entities", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::EntityTemplate, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::AssetDocument, "Asset Document",
            "Assets", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::AssetDocument, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::MaterialDocument, "Material Document",
            "Materials", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::MaterialDocument, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::AnimationDocument, "Animation Document",
            "Animations", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::AnimationDocument, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::EncounterTemplate, "Encounter Template",
            "Encounters", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::EncounterTemplate, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::SpawnProfile, "Spawn Profile",
            "Spawns", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::SpawnProfile, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::PCGContext, "PCG Context",
            "PCG/Contexts", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::PCGContext, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::PCGPreset, "PCG Preset",
            "PCG/Presets", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::PCGPreset, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::VisualLogicGraph, "Visual Logic Graph",
            "Graphs", ".json", "",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::VisualLogicGraph, std::move(fp)); }
        });
    }

    void registerType(DocumentTypeRegistration reg) {
        m_registrations.emplace(static_cast<uint8_t>(reg.type), std::move(reg));
    }

    [[nodiscard]] const DocumentTypeRegistration* find(NovaForgeDocumentType type) const {
        auto it = m_registrations.find(static_cast<uint8_t>(type));
        return it != m_registrations.end() ? &it->second : nullptr;
    }

    [[nodiscard]] size_t count() const { return m_registrations.size(); }
    [[nodiscard]] bool empty() const { return m_registrations.empty(); }

    [[nodiscard]] std::vector<const DocumentTypeRegistration*> all() const {
        std::vector<const DocumentTypeRegistration*> out;
        for (const auto& [k, v] : m_registrations)
            out.push_back(&v);
        return out;
    }

    [[nodiscard]] std::unique_ptr<NovaForgeDocument> createDocument(
        NovaForgeDocumentType type, std::string filePath) const {
        const auto* reg = find(type);
        if (!reg || !reg->factory) return nullptr;
        return reg->factory(std::move(filePath));
    }

private:
    std::unordered_map<uint8_t, DocumentTypeRegistration> m_registrations;
};

} // namespace NovaForge
