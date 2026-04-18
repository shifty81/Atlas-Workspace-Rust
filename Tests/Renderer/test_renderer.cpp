#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Renderer/Renderer.h"

using Catch::Matchers::WithinAbs;

// ── Mesh ─────────────────────────────────────────────────────────

TEST_CASE("Mesh vertex and index storage", "[Renderer][Mesh]") {
    NF::Mesh mesh;
    REQUIRE(mesh.vertexCount() == 0);
    REQUIRE(mesh.indexCount() == 0);

    std::vector<NF::Vertex> verts = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}},
        {{1, 0, 0}, {0, 1, 0}, {1, 0}},
        {{0, 1, 0}, {0, 1, 0}, {0, 1}},
    };
    std::vector<uint32_t> indices = {0, 1, 2};

    mesh.setVertices(verts);
    mesh.setIndices(indices);

    REQUIRE(mesh.vertexCount() == 3);
    REQUIRE(mesh.indexCount() == 3);
    REQUIRE(mesh.triangleCount() == 1);
    REQUIRE(mesh.isDirty());

    mesh.clearDirty();
    REQUIRE_FALSE(mesh.isDirty());
}

TEST_CASE("Mesh bounds computation", "[Renderer][Mesh]") {
    NF::Mesh mesh({
        {{-1, -2, -3}, {}, {}},
        {{4, 5, 6}, {}, {}},
        {{0, 0, 0}, {}, {}},
    }, {0, 1, 2});

    auto [lo, hi] = mesh.bounds();
    REQUIRE_THAT(lo.x, WithinAbs(-1.f, 1e-5));
    REQUIRE_THAT(lo.y, WithinAbs(-2.f, 1e-5));
    REQUIRE_THAT(lo.z, WithinAbs(-3.f, 1e-5));
    REQUIRE_THAT(hi.x, WithinAbs(4.f, 1e-5));
    REQUIRE_THAT(hi.y, WithinAbs(5.f, 1e-5));
    REQUIRE_THAT(hi.z, WithinAbs(6.f, 1e-5));
}

TEST_CASE("Mesh constructor with data", "[Renderer][Mesh]") {
    std::vector<NF::Vertex> verts = {
        {{0, 0, 0}, {0, 0, 1}, {0, 0}},
        {{1, 0, 0}, {0, 0, 1}, {1, 0}},
        {{1, 1, 0}, {0, 0, 1}, {1, 1}},
        {{0, 1, 0}, {0, 0, 1}, {0, 1}},
    };
    NF::Mesh mesh(verts, {0, 1, 2, 2, 3, 0});

    REQUIRE(mesh.vertexCount() == 4);
    REQUIRE(mesh.indexCount() == 6);
    REQUIRE(mesh.triangleCount() == 2);
}

// ── Shader ───────────────────────────────────────────────────────

TEST_CASE("Shader compile", "[Renderer][Shader]") {
    NF::Shader shader(NF::StringID("test_shader"), "vertex_src", "fragment_src");
    REQUIRE_FALSE(shader.isCompiled());

    bool ok = shader.compile();
    REQUIRE(ok);
    REQUIRE(shader.isCompiled());
    REQUIRE(shader.name() == NF::StringID("test_shader"));
}

TEST_CASE("Shader empty source fails compile", "[Renderer][Shader]") {
    NF::Shader shader(NF::StringID("empty"), "", "");
    REQUIRE_FALSE(shader.compile());
}

TEST_CASE("Shader uniforms", "[Renderer][Shader]") {
    NF::Shader shader;
    shader.setUniformFloat("brightness", 0.8f);
    shader.setUniformVec3("lightDir", {1, 0, 0});

    REQUIRE(shader.floatUniforms().size() == 1);
    REQUIRE_THAT(shader.floatUniforms().at("brightness"), WithinAbs(0.8f, 1e-5));
    REQUIRE(shader.vec3Uniforms().size() == 1);
}

// ── Material ─────────────────────────────────────────────────────

TEST_CASE("Material properties", "[Renderer][Material]") {
    NF::Material mat(NF::StringID("metal"));
    mat.setColor({0.5f, 0.5f, 0.5f, 1.f});
    mat.setMetallic(0.9f);
    mat.setRoughness(0.2f);

    REQUIRE(mat.name() == NF::StringID("metal"));
    REQUIRE_THAT(mat.metallic(), WithinAbs(0.9f, 1e-5));
    REQUIRE_THAT(mat.roughness(), WithinAbs(0.2f, 1e-5));
    REQUIRE_THAT(mat.color().x, WithinAbs(0.5f, 1e-5));
}

// ── Render Queue ─────────────────────────────────────────────────

TEST_CASE("RenderQueue submit and sort", "[Renderer][Queue]") {
    NF::RenderQueue queue;
    REQUIRE(queue.empty());

    queue.submit({nullptr, nullptr, NF::Mat4::identity(), 3.f});
    queue.submit({nullptr, nullptr, NF::Mat4::identity(), 1.f});
    queue.submit({nullptr, nullptr, NF::Mat4::identity(), 2.f});

    REQUIRE(queue.size() == 3);

    queue.sort();
    auto& cmds = queue.commands();
    REQUIRE_THAT(cmds[0].sortKey, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(cmds[1].sortKey, WithinAbs(2.f, 1e-5));
    REQUIRE_THAT(cmds[2].sortKey, WithinAbs(3.f, 1e-5));

    queue.clear();
    REQUIRE(queue.empty());
}

// ── Camera ───────────────────────────────────────────────────────

TEST_CASE("Camera view and projection matrices", "[Renderer][Camera]") {
    NF::Camera cam;
    cam.position = {0, 0, 5};
    cam.target = {0, 0, 0};

    auto view = cam.viewMatrix();
    // View matrix should transform the target to near origin
    auto result = view.transformPoint({0, 0, 0});
    REQUIRE_THAT(result.z, WithinAbs(-5.f, 1e-3));

    auto proj = cam.projectionMatrix(16.f / 9.f);
    // Perspective matrix should have -1 in [3,2]
    REQUIRE_THAT(proj.at(3, 2), WithinAbs(-1.f, 1e-5));
}

TEST_CASE("Camera forward direction", "[Renderer][Camera]") {
    NF::Camera cam;
    cam.position = {0, 0, 0};
    cam.target = {0, 0, -1};

    auto fwd = cam.forward();
    REQUIRE_THAT(fwd.z, WithinAbs(-1.f, 1e-4));
}

// ── Renderer ─────────────────────────────────────────────────────

TEST_CASE("Renderer init and frame lifecycle", "[Renderer]") {
    NF::Renderer renderer;
    REQUIRE(renderer.init(1920, 1080));

    REQUIRE(renderer.width() == 1920);
    REQUIRE(renderer.height() == 1080);
    REQUIRE_THAT(renderer.aspect(), WithinAbs(1920.f / 1080.f, 1e-3));

    renderer.beginFrame();
    renderer.submit({nullptr, nullptr, NF::Mat4::identity(), 1.f});
    renderer.submit({nullptr, nullptr, NF::Mat4::identity(), 2.f});
    renderer.endFrame();

    REQUIRE(renderer.drawCallCount() == 2);

    renderer.shutdown();
}

// ── G2: Lighting ─────────────────────────────────────────────────

TEST_CASE("LightSource defaults", "[Renderer][Lighting][G2]") {
    NF::LightSource light;
    REQUIRE(light.type == NF::LightType::Directional);
    REQUIRE_THAT(light.position.y, WithinAbs(10.f, 1e-5));
    REQUIRE_THAT(light.direction.y, WithinAbs(-1.f, 1e-5));
    REQUIRE_THAT(light.color.x, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(light.intensity, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(light.range, WithinAbs(50.f, 1e-5));
    REQUIRE_THAT(light.spotAngle, WithinAbs(45.f, 1e-5));
    REQUIRE(light.enabled);
}

TEST_CASE("LightingState add/remove/clear", "[Renderer][Lighting][G2]") {
    NF::LightingState ls;
    REQUIRE(ls.lightCount() == 0);

    NF::LightSource l1;
    l1.color = {1, 0, 0};
    ls.addLight(l1);
    REQUIRE(ls.lightCount() == 1);
    REQUIRE_THAT(ls.light(0).color.x, WithinAbs(1.f, 1e-5));

    NF::LightSource l2;
    l2.color = {0, 1, 0};
    ls.addLight(l2);
    REQUIRE(ls.lightCount() == 2);

    ls.removeLight(0);
    REQUIRE(ls.lightCount() == 1);
    REQUIRE_THAT(ls.light(0).color.y, WithinAbs(1.f, 1e-5));

    ls.clear();
    REQUIRE(ls.lightCount() == 0);
}

TEST_CASE("LightingState MAX_LIGHTS limit", "[Renderer][Lighting][G2]") {
    NF::LightingState ls;
    for (int i = 0; i < NF::LightingState::MAX_LIGHTS + 5; ++i) {
        ls.addLight(NF::LightSource{});
    }
    REQUIRE(ls.lightCount() == NF::LightingState::MAX_LIGHTS);
}

TEST_CASE("LightingState ambient settings", "[Renderer][Lighting][G2]") {
    NF::LightingState ls;
    REQUIRE_THAT(ls.ambientIntensity(), WithinAbs(0.3f, 1e-5));

    ls.setAmbientColor({0.2f, 0.2f, 0.3f});
    ls.setAmbientIntensity(0.5f);

    REQUIRE_THAT(ls.ambientColor().x, WithinAbs(0.2f, 1e-5));
    REQUIRE_THAT(ls.ambientIntensity(), WithinAbs(0.5f, 1e-5));
}

TEST_CASE("LightingState compute lighting directional", "[Renderer][Lighting][G2]") {
    NF::LightingState ls;
    ls.setAmbientColor({0, 0, 0});
    ls.setAmbientIntensity(0.f);

    NF::LightSource sun;
    sun.type = NF::LightType::Directional;
    sun.direction = {0, -1, 0};
    sun.color = {1, 1, 1};
    sun.intensity = 1.f;
    ls.addLight(sun);

    // Surface facing up, lit from above
    NF::Vec3 result = ls.computeLighting({0, 0, 0}, {0, 1, 0}, {0, 5, 0});
    REQUIRE(result.x > 0.f);
    REQUIRE(result.y > 0.f);
    REQUIRE(result.z > 0.f);

    // Surface facing down, lit from above - no diffuse contribution
    NF::Vec3 dark = ls.computeLighting({0, 0, 0}, {0, -1, 0}, {0, 5, 0});
    REQUIRE_THAT(dark.x, WithinAbs(0.f, 1e-3));
}

TEST_CASE("LightingState compute lighting point light", "[Renderer][Lighting][G2]") {
    NF::LightingState ls;
    ls.setAmbientColor({0, 0, 0});
    ls.setAmbientIntensity(0.f);

    NF::LightSource point;
    point.type = NF::LightType::Point;
    point.position = {0, 5, 0};
    point.color = {1, 1, 1};
    point.intensity = 1.f;
    point.range = 50.f;
    ls.addLight(point);

    NF::Vec3 result = ls.computeLighting({0, 0, 0}, {0, 1, 0}, {0, 10, 0});
    REQUIRE(result.x > 0.f);

    // Far away point - attenuation reduces lighting
    NF::Vec3 far = ls.computeLighting({0, -100, 0}, {0, 1, 0}, {0, 10, 0});
    REQUIRE(far.x < result.x);
}

TEST_CASE("LightingState compute lighting disabled light", "[Renderer][Lighting][G2]") {
    NF::LightingState ls;
    ls.setAmbientColor({0, 0, 0});
    ls.setAmbientIntensity(0.f);

    NF::LightSource sun;
    sun.enabled = false;
    sun.direction = {0, -1, 0};
    ls.addLight(sun);

    NF::Vec3 result = ls.computeLighting({0, 0, 0}, {0, 1, 0}, {0, 5, 0});
    REQUIRE_THAT(result.x, WithinAbs(0.f, 1e-5));
}

// ── G2: VoxelShader ─────────────────────────────────────────────

TEST_CASE("VoxelShader init and setup", "[Renderer][VoxelShader][G2]") {
    NF::VoxelShader vs;
    REQUIRE_FALSE(vs.isReady());

    vs.init();
    REQUIRE(vs.isReady());
    REQUIRE(vs.shader().isCompiled());

    vs.setViewProjection(NF::Mat4::identity());
    vs.setModel(NF::Mat4::identity());
    vs.setLightDir({0, -1, 0});
    vs.setLightColor({1, 1, 1});
    vs.setAmbient({0.1f, 0.1f, 0.1f});
    vs.setViewPos({0, 0, 5});

    REQUIRE(vs.shader().vec3Uniforms().count("u_lightDir") == 1);
    REQUIRE(vs.shader().vec3Uniforms().count("u_viewPos") == 1);
}

// ── G2: Frustum Culling ─────────────────────────────────────────

TEST_CASE("FrustumPlane distance to point", "[Renderer][Frustum][G2]") {
    NF::FrustumPlane plane;
    plane.normal = {0, 1, 0};
    plane.distance = -5.f;

    float d = plane.distanceToPoint({0, 10, 0});
    REQUIRE_THAT(d, WithinAbs(5.f, 1e-5));

    float d2 = plane.distanceToPoint({0, 0, 0});
    REQUIRE_THAT(d2, WithinAbs(-5.f, 1e-5));
}

TEST_CASE("Frustum extractFromVP basic", "[Renderer][Frustum][G2]") {
    NF::Camera cam;
    cam.position = {0, 0, 5};
    cam.target = {0, 0, 0};
    cam.fov = 60.f;
    cam.nearPlane = 0.1f;
    cam.farPlane = 1000.f;

    NF::Mat4 vp = cam.projectionMatrix(1.f) * cam.viewMatrix();
    NF::Frustum frustum;
    frustum.extractFromVP(vp);

    // All 6 planes should have normalized normals
    for (int i = 0; i < 6; ++i) {
        float len = frustum.plane(i).normal.length();
        REQUIRE_THAT(len, WithinAbs(1.f, 1e-3));
    }
}

TEST_CASE("Frustum testAABB inside and outside", "[Renderer][Frustum][G2]") {
    NF::Camera cam;
    cam.position = {0, 0, 50};
    cam.target = {0, 0, 0};
    cam.fov = 60.f;
    cam.nearPlane = 0.1f;
    cam.farPlane = 200.f;

    NF::Mat4 vp = cam.projectionMatrix(1.f) * cam.viewMatrix();
    NF::Frustum frustum;
    frustum.extractFromVP(vp);

    // AABB at origin should be visible (in front of camera)
    REQUIRE(frustum.testAABB({-1, -1, -1}, {1, 1, 1}));

    // AABB far behind camera should not be visible
    REQUIRE_FALSE(frustum.testAABB({-1, -1, 500}, {1, 1, 502}));

    // AABB far to the side
    REQUIRE_FALSE(frustum.testAABB({500, 500, 0}, {502, 502, 2}));
}
