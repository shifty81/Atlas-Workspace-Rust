#include "../engine/camera/Camera.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::camera;

void test_camera_default_state() {
    Camera cam;
    assert(cam.GetMode() == CameraMode::FreeLook);
    assert(cam.GetFOV() > 59.0f && cam.GetFOV() < 61.0f);
    assert(cam.GetNearPlane() > 0.0f);
    assert(cam.GetFarPlane() > cam.GetNearPlane());

    std::cout << "[PASS] test_camera_default_state" << std::endl;
}

void test_camera_set_mode() {
    Camera cam;
    cam.SetMode(CameraMode::Orbital);
    assert(cam.GetMode() == CameraMode::Orbital);

    cam.SetMode(CameraMode::FPS);
    assert(cam.GetMode() == CameraMode::FPS);

    cam.SetMode(CameraMode::Strategy);
    assert(cam.GetMode() == CameraMode::Strategy);

    std::cout << "[PASS] test_camera_set_mode" << std::endl;
}

void test_camera_position() {
    Camera cam;
    cam.SetPosition(10.0f, 20.0f, 30.0f);
    Vec3 pos = cam.GetPosition();
    assert(std::abs(pos.x - 10.0f) < 0.01f);
    assert(std::abs(pos.y - 20.0f) < 0.01f);
    assert(std::abs(pos.z - 30.0f) < 0.01f);

    std::cout << "[PASS] test_camera_position" << std::endl;
}

void test_camera_fov_clamp() {
    Camera cam;
    cam.SetFOV(200.0f);
    assert(cam.GetFOV() <= 179.0f);

    cam.SetFOV(-10.0f);
    assert(cam.GetFOV() >= 1.0f);

    std::cout << "[PASS] test_camera_fov_clamp" << std::endl;
}

void test_camera_orbit() {
    Camera cam;
    cam.SetMode(CameraMode::Orbital);
    cam.SetTarget(0, 0, 0);
    cam.SetOrbitalRadius(10.0f);
    cam.Orbit(90.0f, 0.0f);

    Vec3 pos = cam.GetPosition();
    assert(std::abs(pos.x - 10.0f) < 0.1f);

    std::cout << "[PASS] test_camera_orbit" << std::endl;
}

void test_camera_movement() {
    Camera cam;
    cam.SetPosition(0, 0, 0);
    cam.SetYawPitch(0, 0);
    Vec3 before = cam.GetPosition();
    cam.MoveForward(1.0f);
    Vec3 after = cam.GetPosition();

    float dist = (after - before).Length();
    assert(dist > 0.0f);

    std::cout << "[PASS] test_camera_movement" << std::endl;
}

void test_camera_pitch_clamp() {
    Camera cam;
    cam.SetYawPitch(0, 100.0f);
    assert(cam.GetPitch() <= 89.0f);

    cam.SetYawPitch(0, -100.0f);
    assert(cam.GetPitch() >= -89.0f);

    std::cout << "[PASS] test_camera_pitch_clamp" << std::endl;
}
