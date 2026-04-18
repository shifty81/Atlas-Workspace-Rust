# Equipment Screen Controller + UI Adapter Spec v1

## Controller Responsibilities
- request current equipment model
- map gameplay state to screen VM
- validate equip/unequip actions
- forward user actions to gameplay services
- emit success/error notifications
- open linked screens for crafting, repair, or codex help

## Adapter Boundary
UI should depend on adapter interfaces, not on game entity internals.

```cpp
class IEquipmentScreenAdapter {
public:
    virtual ~IEquipmentScreenAdapter() = default;
    virtual EquipmentScreenVM BuildViewModel() const = 0;
    virtual bool EquipItem(const std::string& slotId, const std::string& itemId) = 0;
    virtual bool UnequipItem(const std::string& slotId) = 0;
    virtual std::vector<std::string> GetCompatibleItems(const std::string& slotId) const = 0;
    virtual std::string GetFailureReason() const = 0;
};
```

## Event Flow
UI event -> controller -> adapter -> gameplay/service layer -> refreshed VM -> UI redraw
