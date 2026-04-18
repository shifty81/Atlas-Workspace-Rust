#ifndef NOVAFORGE_PCG_ASSET_STYLE_H
#define NOVAFORGE_PCG_ASSET_STYLE_H

#include "pcg_context.h"
#include "generation_style.h"
#include "ship_generator.h"
#include "station_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

// ── Shape control point (deformation vertex) ────────────────────────

/**
 * @brief A single control point that deforms PCG-generated geometry.
 *
 * Designers place these in the editor to push/pull/scale regions of a
 * procedural mesh.  Each point has a position, scale modification, and
 * an influence weight that controls falloff.
 */
struct ShapeControlPoint {
    float posX, posY, posZ;            ///< Position on the asset (local space).
    float scaleX, scaleY, scaleZ;      ///< Scale modification at this point.
    float weight;                       ///< Influence weight [0, 1].
};

// ── Shape profile (collection of control points) ────────────────────

/**
 * @brief Defines a shape modification applied to PCG geometry.
 *
 * A shape profile is a curve/surface of control points that warp the
 * generated hull, room boundaries, or station modules.  Designers
 * author these in the editor, and the PCG pipeline applies them after
 * the base geometry is generated.
 */
struct ShapeProfile {
    std::string                    name;
    std::vector<ShapeControlPoint> controlPoints;
    bool  mirrorX;      ///< Mirror across X axis (hull symmetry).
    bool  mirrorY;      ///< Mirror across Y axis.
    float smoothing;    ///< Smoothing factor for interpolation [0, 1].
};

// ── Style color entry ───────────────────────────────────────────────

struct StyleColor {
    float       r, g, b, a;
    std::string regionName;    ///< Which region this color applies to.
};

// ── Material properties ─────────────────────────────────────────────

struct StyleMaterial {
    std::string name;
    float       metallic;      ///< [0, 1]
    float       roughness;     ///< [0, 1]
    float       emissive;      ///< Emissive intensity [0, 1].
    std::string texturePath;   ///< Optional texture file override.
};

// ── Surface treatment ───────────────────────────────────────────────

enum class SurfaceTreatment : uint32_t {
    None          = 0,
    PanelLines    = 1,
    Greeble       = 2,
    Weathered     = 3,
    BattleScarred = 4,
    Pristine      = 5,
};

static constexpr int SURFACE_TREATMENT_COUNT = 6;

// ── Style palette (visual style definition) ─────────────────────────

/**
 * @brief Complete visual style that can be applied to any PCG asset.
 *
 * Contains colors, materials, surface treatment, and a detail level
 * that controls density of surface decorations.  Designers create
 * palettes in the editor and attach them to generation styles.
 */
struct StylePalette {
    std::string                  name;
    std::vector<StyleColor>      colors;
    std::vector<StyleMaterial>   materials;
    SurfaceTreatment             surfaceTreatment;
    float                        detailLevel;   ///< [0, 1]
};

// ── Asset style (shape + visual style combined) ─────────────────────

/**
 * @brief A complete asset modification blueprint.
 *
 * Pairs a ShapeProfile (geometry warping) with a StylePalette (visual
 * look), targeting a specific generation type.  Designers author these
 * in the AssetStylePanel and attach them to GenerationStyles so that
 * PCG output matches their creative vision.
 */
struct AssetStyle {
    std::string          name;
    GenerationStyleType  targetType;
    ShapeProfile         shape;
    StylePalette         palette;
    uint32_t             version;
    bool                 valid;
};

// ── Asset style library (reusable style collection) ─────────────────

/**
 * @brief In-memory library of reusable asset styles.
 *
 * Designers can build up a library of styles and apply them to
 * different generation runs.  The library is serialisable so it
 * persists between editor sessions.
 */
class AssetStyleLibrary {
public:
    AssetStyleLibrary();

    /// Add a style to the library (replaces if name already exists).
    void addStyle(const AssetStyle& style);

    /// Remove a style by name.
    bool removeStyle(const std::string& name);

    /// Find a style by name (nullptr if not found).
    const AssetStyle* findStyle(const std::string& name) const;

    /// List all style names that target the given generation type.
    std::vector<std::string> listStyles(GenerationStyleType type) const;

    /// List all style names in the library.
    std::vector<std::string> listAll() const;

    /// Number of styles in the library.
    size_t size() const;

    /// Clear all styles.
    void clear();

    // ── Shape application ───────────────────────────────────────────

    /// Apply a shape profile to a generated ship's geometry parameters.
    static void applyShapeToShip(GeneratedShip& ship,
                                 const ShapeProfile& shape);

    /// Apply a shape profile to station modules (position/dimension warping).
    static void applyShapeToStation(GeneratedStation& station,
                                    const ShapeProfile& shape);

    // ── Palette application ─────────────────────────────────────────

    /// Apply a style palette to a generated ship.
    static void applyPaletteToShip(GeneratedShip& ship,
                                   const StylePalette& palette);

    /// Apply a style palette to a generated station.
    static void applyPaletteToStation(GeneratedStation& station,
                                      const StylePalette& palette);

    // ── Combined application ────────────────────────────────────────

    /// Apply a full asset style to a ship.
    static void applyToShip(GeneratedShip& ship,
                            const AssetStyle& style);

    /// Apply a full asset style to a station.
    static void applyToStation(GeneratedStation& station,
                               const AssetStyle& style);

    // ── Serialisation ───────────────────────────────────────────────

    static std::string serialize(const AssetStyle& style);
    static AssetStyle  deserialize(const std::string& data);

    /// Human-readable surface treatment name.
    static const char* surfaceTreatmentName(SurfaceTreatment t);

private:
    std::vector<AssetStyle> styles_;
};

} // namespace pcg
} // namespace atlas

#endif // NOVAFORGE_PCG_ASSET_STYLE_H
