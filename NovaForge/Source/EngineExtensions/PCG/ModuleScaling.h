#pragma once
#include "InteriorNode.h"
#include <cstdint>

namespace atlas::procedural {

/// Compile-time scaling constants and utility functions for module sizes.
struct ModuleScaling {
    static constexpr uint32_t XS_TO_S = 2;  // 2 XS = 1 S
    static constexpr uint32_t S_TO_M  = 3;  // 3 S  = 1 M
    static constexpr uint32_t S_TO_L  = 5;  // 5 S  = 1 L
    static constexpr uint32_t L_TO_XL = 2;  // 2 L  = 1 XL

    /// Relative physical size multiplier.
    static constexpr float SizeMultiplier(ModuleSize size) {
        switch (size) {
            case ModuleSize::XS: return 0.5f;
            case ModuleSize::S:  return 1.0f;
            case ModuleSize::M:  return 3.0f;
            case ModuleSize::L:  return 5.0f;
            case ModuleSize::XL: return 10.0f;
            default:             return 1.0f;
        }
    }

    /// Cost scaling relative to a small module.
    static constexpr float CostMultiplier(ModuleSize size) {
        switch (size) {
            case ModuleSize::XS: return 0.3f;
            case ModuleSize::S:  return 1.0f;
            case ModuleSize::M:  return 2.5f;
            case ModuleSize::L:  return 5.0f;
            case ModuleSize::XL: return 12.0f;
            default:             return 1.0f;
        }
    }

    /// Power draw for a module of the given size.
    static constexpr float PowerDraw(ModuleSize size) {
        switch (size) {
            case ModuleSize::XS: return 1.0f;
            case ModuleSize::S:  return 2.0f;
            case ModuleSize::M:  return 5.0f;
            case ModuleSize::L:  return 10.0f;
            case ModuleSize::XL: return 25.0f;
            default:             return 2.0f;
        }
    }

    /// Convert a count of modules at the given size into equivalent S units.
    static uint32_t ConvertToSmallUnits(ModuleSize size, uint32_t count) {
        switch (size) {
            case ModuleSize::XS: return count;                    // 1 XS = 0.5 S → but we use integer; see note
            case ModuleSize::S:  return count;
            case ModuleSize::M:  return count * S_TO_M;
            case ModuleSize::L:  return count * S_TO_L;
            case ModuleSize::XL: return count * S_TO_L * L_TO_XL;
            default:             return count;
        }
    }

    /// True if the module size fits within the slot size (module <= slot).
    static bool FitsInSlot(ModuleSize module, ModuleSize slot) {
        return static_cast<uint8_t>(module) <= static_cast<uint8_t>(slot);
    }
};

} // namespace atlas::procedural
