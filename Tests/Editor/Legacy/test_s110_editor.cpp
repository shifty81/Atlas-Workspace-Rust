// S110 editor tests: BiomeEditor, WeatherSystemEditor, EcosystemEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/EcosystemEditor.h"
#include "NF/Editor/WeatherSystemEditor.h"
#include "NF/Editor/BiomeEditor.h"

using namespace NF;

// ── BiomeEditor ──────────────────────────────────────────────────────────────

TEST_CASE("BiomeType names", "[Editor][S110]") {
    REQUIRE(std::string(biomeTypeName(BiomeType::Forest))   == "Forest");
    REQUIRE(std::string(biomeTypeName(BiomeType::Desert))   == "Desert");
    REQUIRE(std::string(biomeTypeName(BiomeType::Tundra))   == "Tundra");
    REQUIRE(std::string(biomeTypeName(BiomeType::Swamp))    == "Swamp");
    REQUIRE(std::string(biomeTypeName(BiomeType::Ocean))    == "Ocean");
    REQUIRE(std::string(biomeTypeName(BiomeType::Mountain)) == "Mountain");
    REQUIRE(std::string(biomeTypeName(BiomeType::Plains))   == "Plains");
    REQUIRE(std::string(biomeTypeName(BiomeType::Volcanic)) == "Volcanic");
    REQUIRE(std::string(biomeTypeName(BiomeType::Magical))  == "Magical");
    REQUIRE(std::string(biomeTypeName(BiomeType::Urban))    == "Urban");
}

TEST_CASE("BiomeTemperature names", "[Editor][S110]") {
    REQUIRE(std::string(biomeTemperatureName(BiomeTemperature::Freezing))  == "Freezing");
    REQUIRE(std::string(biomeTemperatureName(BiomeTemperature::Cold))      == "Cold");
    REQUIRE(std::string(biomeTemperatureName(BiomeTemperature::Temperate)) == "Temperate");
    REQUIRE(std::string(biomeTemperatureName(BiomeTemperature::Warm))      == "Warm");
    REQUIRE(std::string(biomeTemperatureName(BiomeTemperature::Hot))       == "Hot");
    REQUIRE(std::string(biomeTemperatureName(BiomeTemperature::Scorching)) == "Scorching");
}

TEST_CASE("BiomeMoistureLevel names", "[Editor][S110]") {
    REQUIRE(std::string(biomeMoistureLevelName(BiomeMoistureLevel::Arid))      == "Arid");
    REQUIRE(std::string(biomeMoistureLevelName(BiomeMoistureLevel::Dry))       == "Dry");
    REQUIRE(std::string(biomeMoistureLevelName(BiomeMoistureLevel::Moderate))  == "Moderate");
    REQUIRE(std::string(biomeMoistureLevelName(BiomeMoistureLevel::Humid))     == "Humid");
    REQUIRE(std::string(biomeMoistureLevelName(BiomeMoistureLevel::Saturated)) == "Saturated");
}

TEST_CASE("BiomeEntry defaults", "[Editor][S110]") {
    BiomeEntry entry(1, "temperate_forest", BiomeType::Forest);
    REQUIRE(entry.id()           == 1u);
    REQUIRE(entry.name()         == "temperate_forest");
    REQUIRE(entry.type()         == BiomeType::Forest);
    REQUIRE(entry.temperature()  == BiomeTemperature::Temperate);
    REQUIRE(entry.moisture()     == BiomeMoistureLevel::Moderate);
    REQUIRE(entry.elevation()    == 0.0f);
    REQUIRE(!entry.isUnderwater());
}

TEST_CASE("BiomeEntry mutation", "[Editor][S110]") {
    BiomeEntry entry(2, "hot_desert", BiomeType::Desert);
    entry.setTemperature(BiomeTemperature::Scorching);
    entry.setMoisture(BiomeMoistureLevel::Arid);
    entry.setElevation(150.0f);
    entry.setUnderwater(false);
    REQUIRE(entry.temperature() == BiomeTemperature::Scorching);
    REQUIRE(entry.moisture()    == BiomeMoistureLevel::Arid);
    REQUIRE(entry.elevation()   == 150.0f);
    REQUIRE(!entry.isUnderwater());
}

TEST_CASE("BiomeEditor defaults", "[Editor][S110]") {
    BiomeEditor ed;
    REQUIRE(!ed.showTemperatureMap());
    REQUIRE(!ed.showMoistureMap());
    REQUIRE(!ed.showElevationMap());
    REQUIRE(ed.activeBlendRadius() == 50.0f);
    REQUIRE(ed.biomeCount()        == 0u);
}

TEST_CASE("BiomeEditor add/remove biomes", "[Editor][S110]") {
    BiomeEditor ed;
    REQUIRE(ed.addBiome(BiomeEntry(1, "forest",   BiomeType::Forest)));
    REQUIRE(ed.addBiome(BiomeEntry(2, "desert",   BiomeType::Desert)));
    REQUIRE(ed.addBiome(BiomeEntry(3, "ocean",    BiomeType::Ocean)));
    REQUIRE(!ed.addBiome(BiomeEntry(1, "forest",  BiomeType::Forest)));
    REQUIRE(ed.biomeCount() == 3u);
    REQUIRE(ed.removeBiome(2));
    REQUIRE(ed.biomeCount() == 2u);
    REQUIRE(!ed.removeBiome(99));
}

TEST_CASE("BiomeEditor counts and find", "[Editor][S110]") {
    BiomeEditor ed;
    BiomeEntry b1(1, "forest",   BiomeType::Forest);
    BiomeEntry b2(2, "tundra",   BiomeType::Tundra);   b2.setTemperature(BiomeTemperature::Freezing);
    BiomeEntry b3(3, "ocean",    BiomeType::Ocean);     b3.setUnderwater(true);
    BiomeEntry b4(4, "swamp",    BiomeType::Swamp);     b4.setUnderwater(true);
    ed.addBiome(b1); ed.addBiome(b2); ed.addBiome(b3); ed.addBiome(b4);
    REQUIRE(ed.countByType(BiomeType::Forest)                    == 1u);
    REQUIRE(ed.countByType(BiomeType::Ocean)                     == 1u);
    REQUIRE(ed.countByType(BiomeType::Volcanic)                  == 0u);
    REQUIRE(ed.countByTemperature(BiomeTemperature::Freezing)    == 1u);
    REQUIRE(ed.countByTemperature(BiomeTemperature::Temperate)   == 3u);
    REQUIRE(ed.countUnderwater()                                 == 2u);
    auto* found = ed.findBiome(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == BiomeType::Ocean);
    REQUIRE(ed.findBiome(99) == nullptr);
}

TEST_CASE("BiomeEditor mutation", "[Editor][S110]") {
    BiomeEditor ed;
    ed.setShowTemperatureMap(true);
    ed.setShowMoistureMap(true);
    ed.setShowElevationMap(true);
    ed.setActiveBlendRadius(75.0f);
    REQUIRE(ed.showTemperatureMap());
    REQUIRE(ed.showMoistureMap());
    REQUIRE(ed.showElevationMap());
    REQUIRE(ed.activeBlendRadius() == 75.0f);
}

// ── WeatherSystemEditor ──────────────────────────────────────────────────────

TEST_CASE("WeatherCondition names", "[Editor][S110]") {
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Clear))       == "Clear");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Cloudy))      == "Cloudy");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Overcast))    == "Overcast");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Rain))        == "Rain");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Thunderstorm))== "Thunderstorm");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Snow))        == "Snow");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Blizzard))    == "Blizzard");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Fog))         == "Fog");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Sandstorm))   == "Sandstorm");
    REQUIRE(std::string(weatherConditionName(WeatherCondition::Hail))        == "Hail");
}

TEST_CASE("WindDirection names", "[Editor][S110]") {
    REQUIRE(std::string(windDirectionName(WindDirection::N))  == "N");
    REQUIRE(std::string(windDirectionName(WindDirection::NE)) == "NE");
    REQUIRE(std::string(windDirectionName(WindDirection::E))  == "E");
    REQUIRE(std::string(windDirectionName(WindDirection::SE)) == "SE");
    REQUIRE(std::string(windDirectionName(WindDirection::S))  == "S");
    REQUIRE(std::string(windDirectionName(WindDirection::SW)) == "SW");
    REQUIRE(std::string(windDirectionName(WindDirection::W))  == "W");
    REQUIRE(std::string(windDirectionName(WindDirection::NW)) == "NW");
}

TEST_CASE("WeatherTransitionMode names", "[Editor][S110]") {
    REQUIRE(std::string(weatherTransitionModeName(WeatherTransitionMode::Instant))  == "Instant");
    REQUIRE(std::string(weatherTransitionModeName(WeatherTransitionMode::Smooth))   == "Smooth");
    REQUIRE(std::string(weatherTransitionModeName(WeatherTransitionMode::Random))   == "Random");
    REQUIRE(std::string(weatherTransitionModeName(WeatherTransitionMode::Scripted)) == "Scripted");
}

TEST_CASE("WeatherState defaults", "[Editor][S110]") {
    WeatherState state(1, "sunny_day", WeatherCondition::Clear);
    REQUIRE(state.id()                == 1u);
    REQUIRE(state.name()              == "sunny_day");
    REQUIRE(state.condition()         == WeatherCondition::Clear);
    REQUIRE(state.windDirection()     == WindDirection::N);
    REQUIRE(state.windSpeed()         == 0.0f);
    REQUIRE(state.temperature()       == 20.0f);
    REQUIRE(state.precipitation()     == 0.0f);
    REQUIRE(!state.isLightningEnabled());
}

TEST_CASE("WeatherState mutation", "[Editor][S110]") {
    WeatherState state(2, "storm", WeatherCondition::Thunderstorm);
    state.setWindDirection(WindDirection::SW);
    state.setWindSpeed(60.0f);
    state.setTemperature(12.0f);
    state.setPrecipitation(0.9f);
    state.setLightningEnabled(true);
    REQUIRE(state.windDirection()     == WindDirection::SW);
    REQUIRE(state.windSpeed()         == 60.0f);
    REQUIRE(state.temperature()       == 12.0f);
    REQUIRE(state.precipitation()     == 0.9f);
    REQUIRE(state.isLightningEnabled());
}

TEST_CASE("WeatherSystemEditor defaults", "[Editor][S110]") {
    WeatherSystemEditor ed;
    REQUIRE(ed.transitionMode()     == WeatherTransitionMode::Smooth);
    REQUIRE(ed.transitionDuration() == 30.0f);
    REQUIRE(ed.activeStateId()      == 0u);
    REQUIRE(!ed.isRealTimePreview());
    REQUIRE(ed.stateCount()         == 0u);
}

TEST_CASE("WeatherSystemEditor add/remove states", "[Editor][S110]") {
    WeatherSystemEditor ed;
    REQUIRE(ed.addState(WeatherState(1, "sunny",   WeatherCondition::Clear)));
    REQUIRE(ed.addState(WeatherState(2, "rainy",   WeatherCondition::Rain)));
    REQUIRE(ed.addState(WeatherState(3, "stormy",  WeatherCondition::Thunderstorm)));
    REQUIRE(!ed.addState(WeatherState(1, "sunny",  WeatherCondition::Clear)));
    REQUIRE(ed.stateCount() == 3u);
    REQUIRE(ed.removeState(2));
    REQUIRE(ed.stateCount() == 2u);
    REQUIRE(!ed.removeState(99));
}

TEST_CASE("WeatherSystemEditor counts and find", "[Editor][S110]") {
    WeatherSystemEditor ed;
    WeatherState s1(1, "sunny",   WeatherCondition::Clear);
    WeatherState s2(2, "stormy",  WeatherCondition::Thunderstorm); s2.setLightningEnabled(true);
    WeatherState s3(3, "heavy",   WeatherCondition::Thunderstorm); s3.setLightningEnabled(true);
    WeatherState s4(4, "snow",    WeatherCondition::Snow);
    ed.addState(s1); ed.addState(s2); ed.addState(s3); ed.addState(s4);
    REQUIRE(ed.countByCondition(WeatherCondition::Clear)        == 1u);
    REQUIRE(ed.countByCondition(WeatherCondition::Thunderstorm) == 2u);
    REQUIRE(ed.countByCondition(WeatherCondition::Fog)          == 0u);
    REQUIRE(ed.countWithLightning()                             == 2u);
    auto* found = ed.findState(4);
    REQUIRE(found != nullptr);
    REQUIRE(found->condition() == WeatherCondition::Snow);
    REQUIRE(ed.findState(99) == nullptr);
}

TEST_CASE("WeatherSystemEditor mutation", "[Editor][S110]") {
    WeatherSystemEditor ed;
    ed.setTransitionMode(WeatherTransitionMode::Instant);
    ed.setTransitionDuration(10.0f);
    ed.setActiveStateId(3);
    ed.setRealTimePreview(true);
    REQUIRE(ed.transitionMode()     == WeatherTransitionMode::Instant);
    REQUIRE(ed.transitionDuration() == 10.0f);
    REQUIRE(ed.activeStateId()      == 3u);
    REQUIRE(ed.isRealTimePreview());
}

// ── EcosystemEditor ──────────────────────────────────────────────────────────

TEST_CASE("OrganismType names", "[Editor][S110]") {
    REQUIRE(std::string(organismTypeName(OrganismType::Flora))     == "Flora");
    REQUIRE(std::string(organismTypeName(OrganismType::Fauna))     == "Fauna");
    REQUIRE(std::string(organismTypeName(OrganismType::Fungi))     == "Fungi");
    REQUIRE(std::string(organismTypeName(OrganismType::Aquatic))   == "Aquatic");
    REQUIRE(std::string(organismTypeName(OrganismType::Insect))    == "Insect");
    REQUIRE(std::string(organismTypeName(OrganismType::Undead))    == "Undead");
    REQUIRE(std::string(organismTypeName(OrganismType::Construct)) == "Construct");
    REQUIRE(std::string(organismTypeName(OrganismType::Spirit))    == "Spirit");
}

TEST_CASE("FoodChainRole names", "[Editor][S110]") {
    REQUIRE(std::string(foodChainRoleName(FoodChainRole::Producer))          == "Producer");
    REQUIRE(std::string(foodChainRoleName(FoodChainRole::PrimaryConsumer))   == "PrimaryConsumer");
    REQUIRE(std::string(foodChainRoleName(FoodChainRole::SecondaryConsumer)) == "SecondaryConsumer");
    REQUIRE(std::string(foodChainRoleName(FoodChainRole::Decomposer))        == "Decomposer");
    REQUIRE(std::string(foodChainRoleName(FoodChainRole::Apex))              == "Apex");
}

TEST_CASE("SpawnRule names", "[Editor][S110]") {
    REQUIRE(std::string(spawnRuleName(SpawnRule::Cluster))     == "Cluster");
    REQUIRE(std::string(spawnRuleName(SpawnRule::Scattered))   == "Scattered");
    REQUIRE(std::string(spawnRuleName(SpawnRule::Lone))        == "Lone");
    REQUIRE(std::string(spawnRuleName(SpawnRule::Pack))        == "Pack");
    REQUIRE(std::string(spawnRuleName(SpawnRule::Seasonal))    == "Seasonal");
    REQUIRE(std::string(spawnRuleName(SpawnRule::Underground)) == "Underground");
}

TEST_CASE("Organism defaults", "[Editor][S110]") {
    Organism org(1, "oak_tree", OrganismType::Flora);
    REQUIRE(org.id()              == 1u);
    REQUIRE(org.name()            == "oak_tree");
    REQUIRE(org.type()            == OrganismType::Flora);
    REQUIRE(org.role()            == FoodChainRole::Producer);
    REQUIRE(org.spawnRule()       == SpawnRule::Scattered);
    REQUIRE(org.density()         == 1.0f);
    REQUIRE(org.territoryRadius() == 10.0f);
    REQUIRE(!org.isNocturnal());
}

TEST_CASE("Organism mutation", "[Editor][S110]") {
    Organism org(2, "wolf", OrganismType::Fauna);
    org.setRole(FoodChainRole::Apex);
    org.setSpawnRule(SpawnRule::Pack);
    org.setDensity(0.3f);
    org.setTerritoryRadius(50.0f);
    org.setNocturnal(true);
    REQUIRE(org.role()            == FoodChainRole::Apex);
    REQUIRE(org.spawnRule()       == SpawnRule::Pack);
    REQUIRE(org.density()         == 0.3f);
    REQUIRE(org.territoryRadius() == 50.0f);
    REQUIRE(org.isNocturnal());
}

TEST_CASE("EcosystemEditor defaults", "[Editor][S110]") {
    EcosystemEditor ed;
    REQUIRE(ed.isShowFoodChain());
    REQUIRE(!ed.isShowTerritories());
    REQUIRE(!ed.isShowSpawnZones());
    REQUIRE(ed.simulationSpeed()  == 1.0f);
    REQUIRE(ed.organismCount()    == 0u);
}

TEST_CASE("EcosystemEditor add/remove organisms", "[Editor][S110]") {
    EcosystemEditor ed;
    REQUIRE(ed.addOrganism(Organism(1, "oak",   OrganismType::Flora)));
    REQUIRE(ed.addOrganism(Organism(2, "wolf",  OrganismType::Fauna)));
    REQUIRE(ed.addOrganism(Organism(3, "mushroom", OrganismType::Fungi)));
    REQUIRE(!ed.addOrganism(Organism(1, "oak",  OrganismType::Flora)));
    REQUIRE(ed.organismCount() == 3u);
    REQUIRE(ed.removeOrganism(2));
    REQUIRE(ed.organismCount() == 2u);
    REQUIRE(!ed.removeOrganism(99));
}

TEST_CASE("EcosystemEditor counts and find", "[Editor][S110]") {
    EcosystemEditor ed;
    Organism o1(1, "oak",      OrganismType::Flora);
    Organism o2(2, "wolf",     OrganismType::Fauna);  o2.setRole(FoodChainRole::Apex);  o2.setNocturnal(true);
    Organism o3(3, "deer",     OrganismType::Fauna);  o3.setRole(FoodChainRole::PrimaryConsumer);
    Organism o4(4, "mushroom", OrganismType::Fungi);  o4.setNocturnal(true);
    ed.addOrganism(o1); ed.addOrganism(o2); ed.addOrganism(o3); ed.addOrganism(o4);
    REQUIRE(ed.countByType(OrganismType::Flora)            == 1u);
    REQUIRE(ed.countByType(OrganismType::Fauna)            == 2u);
    REQUIRE(ed.countByType(OrganismType::Spirit)           == 0u);
    REQUIRE(ed.countByRole(FoodChainRole::Producer)        == 2u);
    REQUIRE(ed.countByRole(FoodChainRole::Apex)            == 1u);
    REQUIRE(ed.countNocturnal()                            == 2u);
    auto* found = ed.findOrganism(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == OrganismType::Fauna);
    REQUIRE(ed.findOrganism(99) == nullptr);
}

TEST_CASE("EcosystemEditor mutation", "[Editor][S110]") {
    EcosystemEditor ed;
    ed.setShowFoodChain(false);
    ed.setShowTerritories(true);
    ed.setShowSpawnZones(true);
    ed.setSimulationSpeed(2.0f);
    REQUIRE(!ed.isShowFoodChain());
    REQUIRE(ed.isShowTerritories());
    REQUIRE(ed.isShowSpawnZones());
    REQUIRE(ed.simulationSpeed() == 2.0f);
}
