# Equipment GUI ViewModel + AtlasUI Component Contract v1

## ViewModel Types
```cpp
struct EquipmentSlotVM {
    std::string slotId;
    std::string displayName;
    std::string category;
    bool unlocked;
    bool occupied;
    bool compatible;
    bool damaged;
    float powerDrawKw;
    std::string equippedItemId;
    std::vector<std::string> unmetDependencies;
};

struct EquipmentScreenVM {
    std::string rigTier;
    float powerBudgetKw;
    float usedPowerKw;
    std::vector<EquipmentSlotVM> slots;
    std::string focusedSlotId;
    std::string focusedItemId;
    std::vector<std::string> unlockedSystems;
    std::vector<std::string> warnings;
};
```

## AtlasUI Components
- `DockPanel`
- `InspectorPanel`
- `PropertyList`
- `SlotGrid`
- `StatDeltaCard`
- `RequirementList`
- `ActionBar`
- `SearchInput`
- `FilterChipRow`

## Binding Rules
- the VM is the sole source for screen rendering
- AtlasUI components do not query gameplay objects directly
- state updates arrive through adapter/controller layer only
