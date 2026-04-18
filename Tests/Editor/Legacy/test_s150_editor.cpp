// S150 editor tests: PackageManagerV1, DependencyGraphV1, VersionResolverV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/VersionResolverV1.h"
#include "NF/Editor/DependencyGraphV1.h"
#include "NF/Editor/PackageManagerV1.h"

using namespace NF;
using Catch::Approx;

// ── PackageManagerV1 ──────────────────────────────────────────────────────────

TEST_CASE("PkgStatus names", "[Editor][S150]") {
    REQUIRE(std::string(pkgStatusName(PkgStatus::NotInstalled)) == "NotInstalled");
    REQUIRE(std::string(pkgStatusName(PkgStatus::Installing))   == "Installing");
    REQUIRE(std::string(pkgStatusName(PkgStatus::Installed))    == "Installed");
    REQUIRE(std::string(pkgStatusName(PkgStatus::Outdated))     == "Outdated");
    REQUIRE(std::string(pkgStatusName(PkgStatus::Failed))       == "Failed");
    REQUIRE(std::string(pkgStatusName(PkgStatus::Removed))      == "Removed");
}

TEST_CASE("PkgEntry validity and isInstalled", "[Editor][S150]") {
    PkgEntry p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "catch2";
    REQUIRE(p.isValid());
    REQUIRE(!p.isInstalled());
    p.status = PkgStatus::Installed;
    REQUIRE(p.isInstalled());
}

TEST_CASE("PackageManagerV1 addPackage and packageCount", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 1; pkg.name = "fmt";
    REQUIRE(pm.addPackage(pkg));
    REQUIRE(pm.packageCount() == 1);
}

TEST_CASE("PackageManagerV1 reject duplicate package", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 2; pkg.name = "spdlog";
    REQUIRE(pm.addPackage(pkg));
    REQUIRE(!pm.addPackage(pkg));
}

TEST_CASE("PackageManagerV1 removePackage", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 3; pkg.name = "nlohmann";
    pm.addPackage(pkg);
    REQUIRE(pm.removePackage(3));
    REQUIRE(pm.packageCount() == 0);
    REQUIRE(!pm.removePackage(3));
}

TEST_CASE("PackageManagerV1 install and getStatus", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 4; pkg.name = "glfw";
    pm.addPackage(pkg);
    REQUIRE(pm.install(4));
    REQUIRE(pm.getStatus(4) == PkgStatus::Installed);
}

TEST_CASE("PackageManagerV1 uninstall", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 5; pkg.name = "vulkan";
    pm.addPackage(pkg);
    pm.install(5);
    REQUIRE(pm.uninstall(5));
    REQUIRE(pm.getStatus(5) == PkgStatus::Removed);
}

TEST_CASE("PackageManagerV1 findPackage by name", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 6; pkg.name = "imgui";
    pm.addPackage(pkg);
    REQUIRE(pm.findPackage("imgui") != nullptr);
    REQUIRE(pm.findPackage("unknown") == nullptr);
}

TEST_CASE("PackageManagerV1 onStatusChange callback", "[Editor][S150]") {
    PackageManagerV1 pm;
    PkgEntry pkg; pkg.id = 7; pkg.name = "asio";
    pm.addPackage(pkg);
    PkgStatus last = PkgStatus::NotInstalled;
    pm.setOnStatusChange([&](uint32_t, PkgStatus s){ last = s; });
    pm.install(7);
    REQUIRE(last == PkgStatus::Installed);
}

// ── DependencyGraphV1 ─────────────────────────────────────────────────────────

TEST_CASE("DgNode validity", "[Editor][S150]") {
    DgNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "libA";
    REQUIRE(n.isValid());
}

TEST_CASE("DependencyGraphV1 addNode and nodeCount", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode n; n.id = 1; n.name = "core";
    REQUIRE(dg.addNode(n));
    REQUIRE(dg.nodeCount() == 1);
}

TEST_CASE("DependencyGraphV1 removeNode", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode n; n.id = 2; n.name = "render";
    dg.addNode(n);
    REQUIRE(dg.removeNode(2));
    REQUIRE(dg.nodeCount() == 0);
    REQUIRE(!dg.removeNode(2));
}

TEST_CASE("DependencyGraphV1 addDependency and getDirectDeps", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode a; a.id = 1; a.name = "A";
    DgNode b; b.id = 2; b.name = "B";
    dg.addNode(a); dg.addNode(b);
    REQUIRE(dg.addDependency(1, 2));
    auto deps = dg.getDirectDeps(1);
    REQUIRE(deps.size() == 1);
    REQUIRE(deps[0] == 2);
}

TEST_CASE("DependencyGraphV1 removeDependency", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode a; a.id = 1; a.name = "A";
    DgNode b; b.id = 2; b.name = "B";
    dg.addNode(a); dg.addNode(b);
    dg.addDependency(1, 2);
    REQUIRE(dg.removeDependency(1, 2));
    REQUIRE(dg.getDirectDeps(1).empty());
}

TEST_CASE("DependencyGraphV1 hasCycle detects cycle", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode a; a.id = 1; a.name = "A";
    DgNode b; b.id = 2; b.name = "B";
    dg.addNode(a); dg.addNode(b);
    dg.addDependency(1, 2);
    dg.addDependency(2, 1);
    REQUIRE(dg.hasCycle());
}

TEST_CASE("DependencyGraphV1 hasCycle no cycle", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode a; a.id = 1; a.name = "A";
    DgNode b; b.id = 2; b.name = "B";
    dg.addNode(a); dg.addNode(b);
    dg.addDependency(1, 2);
    REQUIRE(!dg.hasCycle());
}

TEST_CASE("DependencyGraphV1 topologicalOrder", "[Editor][S150]") {
    DependencyGraphV1 dg;
    DgNode a; a.id = 1; a.name = "A";
    DgNode b; b.id = 2; b.name = "B";
    DgNode c; c.id = 3; c.name = "C";
    dg.addNode(a); dg.addNode(b); dg.addNode(c);
    dg.addDependency(1, 2);
    dg.addDependency(2, 3);
    auto order = dg.topologicalOrder();
    REQUIRE(order.size() == 3);
}

// ── VersionResolverV1 ─────────────────────────────────────────────────────────

TEST_CASE("VersionSpec toString", "[Editor][S150]") {
    VersionSpec v; v.major = 1; v.minor = 2; v.patch = 3;
    REQUIRE(v.toString() == "1.2.3");
}

TEST_CASE("VersionSpec toString with prerelease", "[Editor][S150]") {
    VersionSpec v; v.major = 2; v.minor = 0; v.patch = 0; v.prerelease = "alpha";
    REQUIRE(v.toString() == "2.0.0-alpha");
}

TEST_CASE("VersionSpec operator< and operator==", "[Editor][S150]") {
    VersionSpec v1; v1.major = 1; v1.minor = 0; v1.patch = 0;
    VersionSpec v2; v2.major = 1; v2.minor = 1; v2.patch = 0;
    REQUIRE(v1 < v2);
    REQUIRE(!(v2 < v1));
    VersionSpec v3 = v1;
    REQUIRE(v3 == v1);
}

TEST_CASE("VersionSpec satisfies patch constraint", "[Editor][S150]") {
    VersionSpec required; required.major = 1; required.minor = 0; required.patch = 5;
    VersionSpec candidate; candidate.major = 1; candidate.minor = 0; candidate.patch = 7;
    REQUIRE(candidate.satisfies(required, true, false));
    VersionSpec old; old.major = 1; old.minor = 0; old.patch = 3;
    REQUIRE(!old.satisfies(required, true, false));
}

TEST_CASE("VersionResolverV1 addConstraint and constraintCount", "[Editor][S150]") {
    VersionResolverV1 vr;
    VersionSpec lo; lo.major = 1; lo.minor = 0; lo.patch = 0;
    VersionSpec hi; hi.major = 2; hi.minor = 0; hi.patch = 0;
    vr.addConstraint("mylib", lo, hi);
    REQUIRE(vr.constraintCount() == 1);
}

TEST_CASE("VersionResolverV1 removeConstraint", "[Editor][S150]") {
    VersionResolverV1 vr;
    VersionSpec lo, hi;
    vr.addConstraint("pkg", lo, hi);
    REQUIRE(vr.removeConstraint("pkg"));
    REQUIRE(vr.constraintCount() == 0);
    REQUIRE(!vr.removeConstraint("pkg"));
}

TEST_CASE("VersionResolverV1 resolve picks best within constraint", "[Editor][S150]") {
    VersionResolverV1 vr;
    VersionSpec lo; lo.major = 1; lo.minor = 0; lo.patch = 0;
    VersionSpec hi; hi.major = 1; hi.minor = 9; hi.patch = 99;
    vr.addConstraint("lib", lo, hi);

    VersionSpec v1; v1.major = 1; v1.minor = 0; v1.patch = 0;
    VersionSpec v2; v2.major = 1; v2.minor = 5; v2.patch = 0;
    VersionSpec v3; v3.major = 2; v3.minor = 0; v3.patch = 0;
    auto best = vr.resolve("lib", {v1, v2, v3});
    REQUIRE(best == v2);
}
