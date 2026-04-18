#ifndef NOVAFORGE_NAME_GENERATOR_H
#define NOVAFORGE_NAME_GENERATOR_H

#include <string>
#include <vector>
#include <random>

namespace atlas {
namespace utils {

/**
 * @brief Random name generator for game elements
 * 
 * Generates Astralis-like names for characters, ships, corporations, etc.
 */
class NameGenerator {
public:
    NameGenerator();
    
    // Character names
    std::string generateCharacterName(bool male = true);
    
    // Ship names
    enum class ShipStyle {
        Heroic,
        Celestial,
        Mythic,
        Descriptive,
        Random
    };
    std::string generateShipName(ShipStyle style = ShipStyle::Random);
    
    // Organization names
    std::string generateCorporationName();
    
    // Location names
    std::string generateSystemName();
    std::string generateStationName(const std::string& system_name = "");
    
    // Mission and content names
    std::string generateMissionName();
    std::string generateExplorationSiteName();
    
    // NPC names
    std::string generatePirateName();
    std::string generatePilotCallsign();
    
    // Resource names
    std::string generateAsteroidDesignation();
    
private:
    std::mt19937 rng_;
    
    // Name components
    std::vector<std::string> first_names_male_;
    std::vector<std::string> first_names_female_;
    std::vector<std::string> last_names_;
    
    std::vector<std::string> ship_prefixes_;
    std::vector<std::string> ship_names_heroic_;
    std::vector<std::string> ship_names_celestial_;
    std::vector<std::string> ship_names_mythic_;
    std::vector<std::string> ship_names_descriptive_;
    
    std::vector<std::string> corp_prefixes_;
    std::vector<std::string> corp_types_;
    
    std::vector<std::string> system_prefixes_;
    std::vector<std::string> system_cores_;
    std::vector<std::string> system_suffixes_;
    
    std::vector<std::string> station_prefixes_;
    std::vector<std::string> station_descriptors_;
    
    std::vector<std::string> mission_adjectives_;
    std::vector<std::string> mission_nouns_;
    
    std::vector<std::string> asteroid_types_;
    
    // Helper methods
    template<typename T>
    const T& randomChoice(const std::vector<T>& vec);
    
    int randomInt(int min, int max);
    bool randomBool(float probability = 0.5f);
    
    void initializeNameData();
};

} // namespace utils
} // namespace atlas

#endif // NOVAFORGE_NAME_GENERATOR_H
