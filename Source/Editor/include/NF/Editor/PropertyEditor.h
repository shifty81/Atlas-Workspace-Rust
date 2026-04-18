#pragma once
// NF::Editor — Property editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

class PropertyEditor {
public:
    // Read a property value from a struct via offset, returning a JSON-like representation
    static JsonValue readProperty(const void* obj, const PropertyInfo& prop) {
        const uint8_t* base = static_cast<const uint8_t*>(obj) + prop.offset;

        switch (prop.type) {
            case PropertyType::Bool:
                return JsonValue(*reinterpret_cast<const bool*>(base));
            case PropertyType::Int32:
                return JsonValue(*reinterpret_cast<const int32_t*>(base));
            case PropertyType::Float:
                return JsonValue(*reinterpret_cast<const float*>(base));
            case PropertyType::String:
                return JsonValue(*reinterpret_cast<const std::string*>(base));
            case PropertyType::Vec3: {
                const auto* v = reinterpret_cast<const Vec3*>(base);
                auto arr = JsonValue::array();
                arr.push(JsonValue(v->x));
                arr.push(JsonValue(v->y));
                arr.push(JsonValue(v->z));
                return arr;
            }
            case PropertyType::Color: {
                const auto* c = reinterpret_cast<const Color*>(base);
                auto arr = JsonValue::array();
                arr.push(JsonValue(c->r));
                arr.push(JsonValue(c->g));
                arr.push(JsonValue(c->b));
                arr.push(JsonValue(c->a));
                return arr;
            }
            default:
                return JsonValue();
        }
    }

    // Write a property value to a struct via offset
    static bool writeProperty(void* obj, const PropertyInfo& prop, const JsonValue& value) {
        uint8_t* base = static_cast<uint8_t*>(obj) + prop.offset;

        switch (prop.type) {
            case PropertyType::Bool:
                if (!value.isBool()) return false;
                *reinterpret_cast<bool*>(base) = value.asBool();
                return true;
            case PropertyType::Int32:
                if (!value.isNumber()) return false;
                *reinterpret_cast<int32_t*>(base) = value.asInt();
                return true;
            case PropertyType::Float:
                if (!value.isNumber()) return false;
                *reinterpret_cast<float*>(base) = value.asFloat();
                return true;
            case PropertyType::String:
                if (!value.isString()) return false;
                *reinterpret_cast<std::string*>(base) = value.asString();
                return true;
            case PropertyType::Vec3: {
                if (!value.isArray() || value.size() < 3) return false;
                auto* v = reinterpret_cast<Vec3*>(base);
                v->x = value[static_cast<size_t>(0)].asFloat();
                v->y = value[static_cast<size_t>(1)].asFloat();
                v->z = value[static_cast<size_t>(2)].asFloat();
                return true;
            }
            case PropertyType::Color: {
                if (!value.isArray() || value.size() < 4) return false;
                auto* c = reinterpret_cast<Color*>(base);
                c->r = value[static_cast<size_t>(0)].asFloat();
                c->g = value[static_cast<size_t>(1)].asFloat();
                c->b = value[static_cast<size_t>(2)].asFloat();
                c->a = value[static_cast<size_t>(3)].asFloat();
                return true;
            }
            default:
                return false;
        }
    }

    // Create an undo-safe property change for a float property
    static std::unique_ptr<ICommand> makeFloatChange(float* target, float newValue,
                                                      const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<float>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for an int property
    static std::unique_ptr<ICommand> makeIntChange(int32_t* target, int32_t newValue,
                                                    const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<int32_t>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a bool property
    static std::unique_ptr<ICommand> makeBoolChange(bool* target, bool newValue,
                                                     const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<bool>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a Vec3 property
    static std::unique_ptr<ICommand> makeVec3Change(Vec3* target, Vec3 newValue,
                                                     const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<Vec3>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a Color property
    static std::unique_ptr<ICommand> makeColorChange(Color* target, Color newValue,
                                                      const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<Color>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a string property
    static std::unique_ptr<ICommand> makeStringChange(std::string* target,
                                                       const std::string& newValue,
                                                       const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<std::string>>(
            target, *target, newValue, "Change " + propName);
    }
};

// ── S6 — PCG World Tuning ────────────────────────────────────────


} // namespace NF
