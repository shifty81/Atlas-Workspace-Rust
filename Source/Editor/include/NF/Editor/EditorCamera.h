#pragma once
// NF::Editor — Editor camera orbit + gizmo
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Workspace/GizmoAxis.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

struct EditorCameraOrbit {
    Vec3  target    = {0.f, 0.f, 0.f};
    float distance  = 10.f;
    float yaw       = -90.f;   // degrees
    float pitch     = 30.f;    // degrees
    float fovDeg    = 60.f;
    float nearPlane = 0.1f;
    float farPlane  = 1000.f;

    Vec3 computePosition() const {
        float y = yaw   * (3.14159265f / 180.f);
        float p = pitch * (3.14159265f / 180.f);
        return Vec3{
            target.x + distance * std::cos(y) * std::cos(p),
            target.y + distance * std::sin(p),
            target.z + distance * std::sin(y) * std::cos(p)
        };
    }

    void orbit(float dyaw, float dpitch) {
        yaw   += dyaw;
        pitch += dpitch;
        if (pitch >  89.f) pitch =  89.f;
        if (pitch < -89.f) pitch = -89.f;
    }

    void zoom(float delta) {
        distance -= delta;
        if (distance < 0.5f) distance = 0.5f;
    }

    void pan(float dx, float dy) {
        float y = yaw * (3.14159265f / 180.f);
        Vec3 right{std::cos(y), 0.f, std::sin(y)};
        Vec3 up{0.f, 1.f, 0.f};
        target = target + right * (-dx * distance * 0.001f);
        target = target + up    * ( dy * distance * 0.001f);
    }

    // aspectRatio is not stored on Camera; it is passed per-frame to projectionMatrix().
    Camera buildCamera(float aspectRatio) const {
        Camera cam;
        cam.position = computePosition();
        cam.target   = target;
        cam.up       = Vec3{0.f, 1.f, 0.f};
        cam.fov      = fovDeg;
        cam.nearPlane = nearPlane;
        cam.farPlane  = farPlane;
        (void)aspectRatio;  // stored externally by caller for use with projectionMatrix()
        return cam;
    }
};

// ── Gizmo ─────────────────────────────────────────────────────────

enum class GizmoMode : uint8_t { Translate, Rotate, Scale };

struct GizmoState {
    GizmoMode mode       = GizmoMode::Translate;
    GizmoAxis activeAxis = GizmoAxis::None;
    bool isDragging      = false;
    bool snapEnabled     = false;
    float snapValue      = 0.25f;   // world units for translate, degrees for rotate

    void activate(GizmoAxis axis) { activeAxis = axis; isDragging = true; }
    void deactivate() { activeAxis = GizmoAxis::None; isDragging = false; }
    void setMode(GizmoMode m) { mode = m; deactivate(); }
};

// ── Editor Settings ───────────────────────────────────────────────

struct SnapSettings {
    float gridSize  = 0.25f;   // world units
    float angleStep = 15.f;    // degrees
    float scaleStep = 0.1f;
    bool  enabled   = false;
};


} // namespace NF
