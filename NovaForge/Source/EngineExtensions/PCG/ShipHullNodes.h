#pragma once
#include "ShipHullGraph.h"

namespace atlas::procedural {

/// Generate a hull mesh from a set of control points using cross-section
/// interpolation along a spline path.
MeshData GenerateHullFromControlPoints(const std::vector<HullControlPoint>& points,
                                       int segments);

/// Generate a small turret mount mesh at the given position/direction.
MeshData GenerateTurretMount(const Vec3& position, const Vec3& direction,
                             const std::string& size);

/// Generate a light fixture indicator mesh.
MeshData GenerateLightFixture(const Vec3& position, float range, bool interior);

/// Generate an interior volume bounding mesh (floor, walls, ceiling).
MeshData GenerateInteriorVolume(const Vec3& boundsMin, const Vec3& boundsMax);

/// Generate a visual feature mesh (fin, antenna, vent, etc.).
MeshData GenerateVisualFeature(const VisualFeature& feature);

/// Generate default control points for a ship class using a seed.
std::vector<HullControlPoint> GenerateDefaultControlPoints(uint64_t seed,
                                                            const std::string& shipClass);

/// Generate default hardpoints for a ship class using a seed.
std::vector<TurretHardpoint> GenerateDefaultHardpoints(uint64_t seed,
                                                        const std::string& shipClass,
                                                        int turretSlots,
                                                        int launcherSlots);

/// Generate default interior sections with cockpit for FPS gameplay.
std::vector<InteriorSection> GenerateDefaultInteriors(uint64_t seed,
                                                       const std::string& shipClass);

/// Generate default lights (interior + exterior nav lights).
std::vector<ShipLight> GenerateDefaultLights(uint64_t seed,
                                              const std::string& shipClass);

}
