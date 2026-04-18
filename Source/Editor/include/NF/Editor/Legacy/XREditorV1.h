#pragma once
// NF::Editor — XR editor v1: device configuration and XR mode authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Xrv1DeviceType  : uint8_t { HMD, Controller, Tracker, HandTracking, EyeTracking };
enum class Xrv1RenderMode  : uint8_t { Stereo, Mono, Passthrough, Mixed };
enum class Xrv1XRMode      : uint8_t { None, VR, AR, MixedReality };
enum class Xrv1DeviceState : uint8_t { Disconnected, Connecting, Connected, Error };

inline const char* xrv1DeviceTypeName(Xrv1DeviceType t) {
    switch (t) {
        case Xrv1DeviceType::HMD:          return "HMD";
        case Xrv1DeviceType::Controller:   return "Controller";
        case Xrv1DeviceType::Tracker:      return "Tracker";
        case Xrv1DeviceType::HandTracking: return "HandTracking";
        case Xrv1DeviceType::EyeTracking:  return "EyeTracking";
    }
    return "Unknown";
}

inline const char* xrv1RenderModeName(Xrv1RenderMode m) {
    switch (m) {
        case Xrv1RenderMode::Stereo:      return "Stereo";
        case Xrv1RenderMode::Mono:        return "Mono";
        case Xrv1RenderMode::Passthrough: return "Passthrough";
        case Xrv1RenderMode::Mixed:       return "Mixed";
    }
    return "Unknown";
}

inline const char* xrv1XRModeName(Xrv1XRMode m) {
    switch (m) {
        case Xrv1XRMode::None:         return "None";
        case Xrv1XRMode::VR:           return "VR";
        case Xrv1XRMode::AR:           return "AR";
        case Xrv1XRMode::MixedReality: return "MixedReality";
    }
    return "Unknown";
}

struct Xrv1Device {
    uint64_t         id         = 0;
    std::string      name;
    std::string      manufacturer;
    Xrv1DeviceType   deviceType = Xrv1DeviceType::HMD;
    Xrv1RenderMode   renderMode = Xrv1RenderMode::Stereo;
    Xrv1DeviceState  state      = Xrv1DeviceState::Disconnected;
    bool             isPrimary  = false;

    [[nodiscard]] bool isValid()       const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isConnected()   const { return state == Xrv1DeviceState::Connected; }
    [[nodiscard]] bool hasError()      const { return state == Xrv1DeviceState::Error; }
};

using Xrv1ChangeCallback = std::function<void(uint64_t)>;

class XREditorV1 {
public:
    static constexpr size_t MAX_DEVICES = 16;

    bool addDevice(const Xrv1Device& device) {
        if (!device.isValid()) return false;
        for (const auto& d : m_devices) if (d.id == device.id) return false;
        if (m_devices.size() >= MAX_DEVICES) return false;
        m_devices.push_back(device);
        return true;
    }

    bool removeDevice(uint64_t id) {
        for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
            if (it->id == id) { m_devices.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Xrv1Device* findDevice(uint64_t id) {
        for (auto& d : m_devices) if (d.id == id) return &d;
        return nullptr;
    }

    bool setState(uint64_t id, Xrv1DeviceState state) {
        auto* d = findDevice(id);
        if (!d) return false;
        d->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setPrimary(uint64_t id, bool primary) {
        auto* d = findDevice(id);
        if (!d) return false;
        d->isPrimary = primary;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setRenderMode(uint64_t id, Xrv1RenderMode mode) {
        auto* d = findDevice(id);
        if (!d) return false;
        d->renderMode = mode;
        if (m_onChange) m_onChange(id);
        return true;
    }

    void setXRMode(Xrv1XRMode mode) { m_xrMode = mode; }
    [[nodiscard]] Xrv1XRMode xrMode() const { return m_xrMode; }

    [[nodiscard]] size_t deviceCount()    const { return m_devices.size(); }
    [[nodiscard]] size_t connectedCount() const {
        size_t c = 0; for (const auto& d : m_devices) if (d.isConnected()) ++c; return c;
    }
    [[nodiscard]] size_t errorCount()     const {
        size_t c = 0; for (const auto& d : m_devices) if (d.hasError())    ++c; return c;
    }
    [[nodiscard]] size_t countByType(Xrv1DeviceType type) const {
        size_t c = 0; for (const auto& d : m_devices) if (d.deviceType == type) ++c; return c;
    }

    void setOnChange(Xrv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Xrv1Device> m_devices;
    Xrv1XRMode              m_xrMode = Xrv1XRMode::None;
    Xrv1ChangeCallback      m_onChange;
};

} // namespace NF
