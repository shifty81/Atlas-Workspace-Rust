#include "HotReloadConfig.h"
#include <algorithm>

namespace atlas::sim {

HotReloadConfig::HotReloadConfig() = default;

// ---- Parameter Registration ----

void HotReloadConfig::RegisterParam(
    const std::string& key,
    const ConfigValue& defaultValue,
    const std::string& description) {

    ConfigParam param;
    param.key = key;
    param.value = defaultValue;
    param.defaultValue = defaultValue;
    param.description = description;
    param.version = 0;
    param.locked = false;
    m_params[key] = std::move(param);
}

bool HotReloadConfig::HasParam(const std::string& key) const {
    return m_params.find(key) != m_params.end();
}

size_t HotReloadConfig::ParamCount() const {
    return m_params.size();
}

// ---- Get/Set ----

ConfigValue HotReloadConfig::GetValue(const std::string& key) const {
    auto it = m_params.find(key);
    if (it == m_params.end()) return ConfigValue{};
    return it->second.value;
}

ConfigChangeResult HotReloadConfig::SetValue(
    const std::string& key, const ConfigValue& value) {

    ConfigChangeResult result;
    result.key = key;

    auto it = m_params.find(key);
    if (it == m_params.end()) {
        result.error = "Parameter not found: " + key;
        return result;
    }

    if (it->second.locked) {
        result.error = "Parameter is locked: " + key;
        return result;
    }

    if (m_inTick) {
        result.error = "Cannot change parameters during a tick";
        return result;
    }

    ConfigValue oldValue = it->second.value;
    it->second.value = value;
    it->second.version++;
    m_globalVersion++;

    result.success = true;
    result.newVersion = it->second.version;

    m_changedKeys.push_back(key);

    if (m_changeCallback) {
        m_changeCallback(key, oldValue, value);
    }

    return result;
}

// ---- Locking ----

void HotReloadConfig::LockParam(const std::string& key) {
    auto it = m_params.find(key);
    if (it != m_params.end()) {
        it->second.locked = true;
    }
}

void HotReloadConfig::UnlockParam(const std::string& key) {
    auto it = m_params.find(key);
    if (it != m_params.end()) {
        it->second.locked = false;
    }
}

bool HotReloadConfig::IsLocked(const std::string& key) const {
    auto it = m_params.find(key);
    if (it == m_params.end()) return false;
    return it->second.locked;
}

// ---- Tick Gating ----

void HotReloadConfig::BeginTick() {
    m_inTick = true;
}

void HotReloadConfig::EndTick() {
    m_inTick = false;
}

bool HotReloadConfig::IsInTick() const {
    return m_inTick;
}

// ---- Versioning ----

uint32_t HotReloadConfig::GetParamVersion(const std::string& key) const {
    auto it = m_params.find(key);
    if (it == m_params.end()) return 0;
    return it->second.version;
}

uint32_t HotReloadConfig::GetGlobalVersion() const {
    return m_globalVersion;
}

// ---- Reset ----

void HotReloadConfig::ResetToDefault(const std::string& key) {
    auto it = m_params.find(key);
    if (it == m_params.end()) return;
    if (it->second.locked || m_inTick) return;

    ConfigValue oldValue = it->second.value;
    it->second.value = it->second.defaultValue;
    it->second.version++;
    m_globalVersion++;

    m_changedKeys.push_back(key);

    if (m_changeCallback) {
        m_changeCallback(key, oldValue, it->second.defaultValue);
    }
}

void HotReloadConfig::ResetAllToDefaults() {
    if (m_inTick) return;

    for (auto& [key, param] : m_params) {
        if (param.locked) continue;

        ConfigValue oldValue = param.value;
        param.value = param.defaultValue;
        param.version++;
        m_globalVersion++;

        m_changedKeys.push_back(key);

        if (m_changeCallback) {
            m_changeCallback(key, oldValue, param.defaultValue);
        }
    }
}

// ---- Change tracking ----

void HotReloadConfig::SetChangeCallback(ConfigChangeCallback callback) {
    m_changeCallback = std::move(callback);
}

std::vector<std::string> HotReloadConfig::GetChangedKeys() const {
    return m_changedKeys;
}

void HotReloadConfig::ClearChangedKeys() {
    m_changedKeys.clear();
}

// ---- Snapshot ----

std::vector<ConfigParam> HotReloadConfig::GetAllParams() const {
    std::vector<ConfigParam> result;
    result.reserve(m_params.size());
    for (const auto& [key, param] : m_params) {
        result.push_back(param);
    }
    return result;
}

const ConfigParam* HotReloadConfig::GetParam(const std::string& key) const {
    auto it = m_params.find(key);
    if (it == m_params.end()) return nullptr;
    return &it->second;
}

}  // namespace atlas::sim
