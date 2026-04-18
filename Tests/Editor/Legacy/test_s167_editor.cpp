// S167 editor tests: XREditorV1, ARMarkerEditorV1, VRInteractionEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/XREditorV1.h"
#include "NF/Editor/ARMarkerEditorV1.h"
#include "NF/Editor/VRInteractionEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── XREditorV1 ───────────────────────────────────────────────────────────────

TEST_CASE("Xrv1Device validity", "[Editor][S167]") {
    Xrv1Device d;
    REQUIRE(!d.isValid());
    d.id = 1; d.name = "QuestPro";
    REQUIRE(d.isValid());
}

TEST_CASE("XREditorV1 addDevice and deviceCount", "[Editor][S167]") {
    XREditorV1 xr;
    REQUIRE(xr.deviceCount() == 0);
    Xrv1Device d; d.id = 1; d.name = "HMD01";
    REQUIRE(xr.addDevice(d));
    REQUIRE(xr.deviceCount() == 1);
}

TEST_CASE("XREditorV1 addDevice invalid fails", "[Editor][S167]") {
    XREditorV1 xr;
    REQUIRE(!xr.addDevice(Xrv1Device{}));
}

TEST_CASE("XREditorV1 addDevice duplicate fails", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d; d.id = 1; d.name = "A";
    xr.addDevice(d);
    REQUIRE(!xr.addDevice(d));
}

TEST_CASE("XREditorV1 removeDevice", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d; d.id = 2; d.name = "B";
    xr.addDevice(d);
    REQUIRE(xr.removeDevice(2));
    REQUIRE(xr.deviceCount() == 0);
    REQUIRE(!xr.removeDevice(2));
}

TEST_CASE("XREditorV1 setState connectedCount", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d; d.id = 1; d.name = "A";
    xr.addDevice(d);
    REQUIRE(xr.setState(1, Xrv1DeviceState::Connected));
    REQUIRE(xr.connectedCount() == 1);
}

TEST_CASE("XREditorV1 setState errorCount", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d; d.id = 1; d.name = "A";
    xr.addDevice(d);
    xr.setState(1, Xrv1DeviceState::Error);
    REQUIRE(xr.errorCount() == 1);
}

TEST_CASE("XREditorV1 setPrimary", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d; d.id = 1; d.name = "A";
    xr.addDevice(d);
    REQUIRE(xr.setPrimary(1, true));
    REQUIRE(xr.findDevice(1)->isPrimary);
}

TEST_CASE("XREditorV1 setRenderMode", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d; d.id = 1; d.name = "A";
    xr.addDevice(d);
    REQUIRE(xr.setRenderMode(1, Xrv1RenderMode::Passthrough));
    REQUIRE(xr.findDevice(1)->renderMode == Xrv1RenderMode::Passthrough);
}

TEST_CASE("XREditorV1 setXRMode and xrMode", "[Editor][S167]") {
    XREditorV1 xr;
    REQUIRE(xr.xrMode() == Xrv1XRMode::None);
    xr.setXRMode(Xrv1XRMode::MixedReality);
    REQUIRE(xr.xrMode() == Xrv1XRMode::MixedReality);
}

TEST_CASE("XREditorV1 countByType", "[Editor][S167]") {
    XREditorV1 xr;
    Xrv1Device d1; d1.id = 1; d1.name = "A"; d1.deviceType = Xrv1DeviceType::HMD;
    Xrv1Device d2; d2.id = 2; d2.name = "B"; d2.deviceType = Xrv1DeviceType::Controller;
    Xrv1Device d3; d3.id = 3; d3.name = "C"; d3.deviceType = Xrv1DeviceType::Controller;
    xr.addDevice(d1); xr.addDevice(d2); xr.addDevice(d3);
    REQUIRE(xr.countByType(Xrv1DeviceType::HMD)        == 1);
    REQUIRE(xr.countByType(Xrv1DeviceType::Controller) == 2);
}

TEST_CASE("xrv1DeviceTypeName covers all values", "[Editor][S167]") {
    REQUIRE(std::string(xrv1DeviceTypeName(Xrv1DeviceType::HMD))          == "HMD");
    REQUIRE(std::string(xrv1DeviceTypeName(Xrv1DeviceType::EyeTracking))  == "EyeTracking");
}

TEST_CASE("xrv1XRModeName covers all values", "[Editor][S167]") {
    REQUIRE(std::string(xrv1XRModeName(Xrv1XRMode::None))         == "None");
    REQUIRE(std::string(xrv1XRModeName(Xrv1XRMode::MixedReality)) == "MixedReality");
}

TEST_CASE("XREditorV1 onChange callback", "[Editor][S167]") {
    XREditorV1 xr;
    uint64_t notified = 0;
    xr.setOnChange([&](uint64_t id) { notified = id; });
    Xrv1Device d; d.id = 5; d.name = "E";
    xr.addDevice(d);
    xr.setState(5, Xrv1DeviceState::Connected);
    REQUIRE(notified == 5);
}

// ── ARMarkerEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Armv1Marker validity", "[Editor][S167]") {
    Armv1Marker m;
    REQUIRE(!m.isValid());
    m.id = 1; m.name = "ImageTarget01";
    REQUIRE(m.isValid());
}

TEST_CASE("ARMarkerEditorV1 addMarker and markerCount", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    REQUIRE(arm.markerCount() == 0);
    Armv1Marker m; m.id = 1; m.name = "M1";
    REQUIRE(arm.addMarker(m));
    REQUIRE(arm.markerCount() == 1);
}

TEST_CASE("ARMarkerEditorV1 addMarker invalid fails", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    REQUIRE(!arm.addMarker(Armv1Marker{}));
}

TEST_CASE("ARMarkerEditorV1 addMarker duplicate fails", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m; m.id = 1; m.name = "A";
    arm.addMarker(m);
    REQUIRE(!arm.addMarker(m));
}

TEST_CASE("ARMarkerEditorV1 removeMarker", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m; m.id = 2; m.name = "B";
    arm.addMarker(m);
    REQUIRE(arm.removeMarker(2));
    REQUIRE(arm.markerCount() == 0);
    REQUIRE(!arm.removeMarker(2));
}

TEST_CASE("ARMarkerEditorV1 setTrackingState trackingCount", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m; m.id = 1; m.name = "A";
    arm.addMarker(m);
    REQUIRE(arm.setTrackingState(1, Armv1TrackingState::Tracking));
    REQUIRE(arm.trackingCount() == 1);
}

TEST_CASE("ARMarkerEditorV1 lostCount", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m1; m1.id = 1; m1.name = "A";
    Armv1Marker m2; m2.id = 2; m2.name = "B";
    arm.addMarker(m1); arm.addMarker(m2);
    // Both start as Lost by default
    REQUIRE(arm.lostCount() == 2);
}

TEST_CASE("ARMarkerEditorV1 setEnabled enabledCount", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m1; m1.id = 1; m1.name = "A"; m1.isEnabled = true;
    Armv1Marker m2; m2.id = 2; m2.name = "B"; m2.isEnabled = false;
    arm.addMarker(m1); arm.addMarker(m2);
    REQUIRE(arm.enabledCount() == 1);
    arm.setEnabled(2, true);
    REQUIRE(arm.enabledCount() == 2);
}

TEST_CASE("ARMarkerEditorV1 setConfidence", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m; m.id = 1; m.name = "A";
    arm.addMarker(m);
    REQUIRE(arm.setConfidence(1, 0.95f));
    REQUIRE(arm.findMarker(1)->confidence == Approx(0.95f));
}

TEST_CASE("ARMarkerEditorV1 countByType", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    Armv1Marker m1; m1.id = 1; m1.name = "A"; m1.markerType = Armv1MarkerType::Image;
    Armv1Marker m2; m2.id = 2; m2.name = "B"; m2.markerType = Armv1MarkerType::QR;
    arm.addMarker(m1); arm.addMarker(m2);
    REQUIRE(arm.countByType(Armv1MarkerType::Image) == 1);
    REQUIRE(arm.countByType(Armv1MarkerType::QR)    == 1);
}

TEST_CASE("armv1MarkerTypeName covers all values", "[Editor][S167]") {
    REQUIRE(std::string(armv1MarkerTypeName(Armv1MarkerType::Image))   == "Image");
    REQUIRE(std::string(armv1MarkerTypeName(Armv1MarkerType::Object))  == "Object");
}

TEST_CASE("armv1TrackingStateName covers all values", "[Editor][S167]") {
    REQUIRE(std::string(armv1TrackingStateName(Armv1TrackingState::Lost))   == "Lost");
    REQUIRE(std::string(armv1TrackingStateName(Armv1TrackingState::Paused)) == "Paused");
}

TEST_CASE("ARMarkerEditorV1 onChange callback", "[Editor][S167]") {
    ARMarkerEditorV1 arm;
    uint64_t notified = 0;
    arm.setOnChange([&](uint64_t id) { notified = id; });
    Armv1Marker m; m.id = 3; m.name = "C";
    arm.addMarker(m);
    arm.setTrackingState(3, Armv1TrackingState::Tracking);
    REQUIRE(notified == 3);
}

// ── VRInteractionEditorV1 ────────────────────────────────────────────────────

TEST_CASE("Vriv1Zone validity", "[Editor][S167]") {
    Vriv1Zone z;
    REQUIRE(!z.isValid());
    z.id = 1; z.name = "GrabZone01"; z.radius = 0.2f;
    REQUIRE(z.isValid());
}

TEST_CASE("Vriv1Zone zero radius invalid", "[Editor][S167]") {
    Vriv1Zone z; z.id = 1; z.name = "X"; z.radius = 0.f;
    REQUIRE(!z.isValid());
}

TEST_CASE("VRInteractionEditorV1 addZone and zoneCount", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    REQUIRE(vri.zoneCount() == 0);
    Vriv1Zone z; z.id = 1; z.name = "Z1"; z.radius = 0.1f;
    REQUIRE(vri.addZone(z));
    REQUIRE(vri.zoneCount() == 1);
}

TEST_CASE("VRInteractionEditorV1 addZone invalid fails", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    REQUIRE(!vri.addZone(Vriv1Zone{}));
}

TEST_CASE("VRInteractionEditorV1 addZone duplicate fails", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z; z.id = 1; z.name = "A"; z.radius = 0.1f;
    vri.addZone(z);
    REQUIRE(!vri.addZone(z));
}

TEST_CASE("VRInteractionEditorV1 removeZone", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z; z.id = 2; z.name = "B"; z.radius = 0.1f;
    vri.addZone(z);
    REQUIRE(vri.removeZone(2));
    REQUIRE(vri.zoneCount() == 0);
    REQUIRE(!vri.removeZone(2));
}

TEST_CASE("VRInteractionEditorV1 setState activeCount", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z; z.id = 1; z.name = "A"; z.radius = 0.1f;
    vri.addZone(z);
    REQUIRE(vri.setState(1, Vriv1InteractionState::Active));
    REQUIRE(vri.activeCount() == 1);
}

TEST_CASE("VRInteractionEditorV1 setVisible visibleCount", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z1; z1.id = 1; z1.name = "A"; z1.radius = 0.1f;
    Vriv1Zone z2; z2.id = 2; z2.name = "B"; z2.radius = 0.1f; z2.isVisible = false;
    vri.addZone(z1); vri.addZone(z2);
    REQUIRE(vri.visibleCount() == 1);
    vri.setVisible(2, true);
    REQUIRE(vri.visibleCount() == 2);
}

TEST_CASE("VRInteractionEditorV1 setRadius validation", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z; z.id = 1; z.name = "A"; z.radius = 0.1f;
    vri.addZone(z);
    REQUIRE(!vri.setRadius(1, 0.f));
    REQUIRE(vri.setRadius(1, 0.5f));
    REQUIRE(vri.findZone(1)->radius == Approx(0.5f));
}

TEST_CASE("VRInteractionEditorV1 setHapticProfile", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z; z.id = 1; z.name = "A"; z.radius = 0.1f;
    vri.addZone(z);
    REQUIRE(vri.setHapticProfile(1, Vriv1HapticProfile::Heavy));
    REQUIRE(vri.findZone(1)->hapticProfile == Vriv1HapticProfile::Heavy);
}

TEST_CASE("VRInteractionEditorV1 countByType", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z1; z1.id = 1; z1.name = "A"; z1.radius = 0.1f; z1.zoneType = Vriv1ZoneType::Grab;
    Vriv1Zone z2; z2.id = 2; z2.name = "B"; z2.radius = 0.1f; z2.zoneType = Vriv1ZoneType::Teleport;
    vri.addZone(z1); vri.addZone(z2);
    REQUIRE(vri.countByType(Vriv1ZoneType::Grab)     == 1);
    REQUIRE(vri.countByType(Vriv1ZoneType::Teleport) == 1);
}

TEST_CASE("VRInteractionEditorV1 countByHand", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    Vriv1Zone z1; z1.id = 1; z1.name = "A"; z1.radius = 0.1f; z1.handSide = Vriv1HandSide::Left;
    Vriv1Zone z2; z2.id = 2; z2.name = "B"; z2.radius = 0.1f; z2.handSide = Vriv1HandSide::Right;
    vri.addZone(z1); vri.addZone(z2);
    REQUIRE(vri.countByHand(Vriv1HandSide::Left)  == 1);
    REQUIRE(vri.countByHand(Vriv1HandSide::Right) == 1);
}

TEST_CASE("vriv1ZoneTypeName covers all values", "[Editor][S167]") {
    REQUIRE(std::string(vriv1ZoneTypeName(Vriv1ZoneType::Grab))    == "Grab");
    REQUIRE(std::string(vriv1ZoneTypeName(Vriv1ZoneType::Physics)) == "Physics");
}

TEST_CASE("VRInteractionEditorV1 onChange callback", "[Editor][S167]") {
    VRInteractionEditorV1 vri;
    uint64_t notified = 0;
    vri.setOnChange([&](uint64_t id) { notified = id; });
    Vriv1Zone z; z.id = 8; z.name = "H"; z.radius = 0.1f;
    vri.addZone(z);
    vri.setState(8, Vriv1InteractionState::Hovered);
    REQUIRE(notified == 8);
}
