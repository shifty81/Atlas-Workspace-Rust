#include "../engine/production/CertifiedBuild.h"
#include <iostream>
#include <cassert>

void test_cert_default_level() {
    atlas::production::CertifiedBuildSystem sys;
    assert(sys.GetLevel() == atlas::production::CertificationLevel::None);
    std::cout << "[PASS] test_cert_default_level" << std::endl;
}

void test_cert_set_level() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::AuditGrade);
    assert(sys.GetLevel() == atlas::production::CertificationLevel::AuditGrade);
    std::cout << "[PASS] test_cert_set_level" << std::endl;
}

void test_cert_add_artifact() {
    atlas::production::CertifiedBuildSystem sys;
    atlas::production::VerificationArtifact a;
    a.name = "binary_hash";
    a.type = "hash";
    a.hash = 42;
    a.passed = true;
    sys.AddArtifact(a);
    assert(sys.ArtifactCount() == 1);
    std::cout << "[PASS] test_cert_add_artifact" << std::endl;
}

void test_cert_get_artifact() {
    atlas::production::CertifiedBuildSystem sys;
    atlas::production::VerificationArtifact a;
    a.name = "test_results";
    a.type = "test";
    a.passed = true;
    sys.AddArtifact(a);
    auto* found = sys.GetArtifact("test_results");
    assert(found != nullptr);
    assert(found->type == "test");
    assert(sys.GetArtifact("missing") == nullptr);
    std::cout << "[PASS] test_cert_get_artifact" << std::endl;
}

void test_cert_list_artifacts() {
    atlas::production::CertifiedBuildSystem sys;
    atlas::production::VerificationArtifact a1; a1.name = "a"; a1.passed = true;
    atlas::production::VerificationArtifact a2; a2.name = "b"; a2.passed = true;
    sys.AddArtifact(a1);
    sys.AddArtifact(a2);
    auto list = sys.ListArtifacts();
    assert(list.size() == 2);
    std::cout << "[PASS] test_cert_list_artifacts" << std::endl;
}

void test_cert_verify_all_pass() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::Standard);
    atlas::production::VerificationArtifact a1; a1.name = "hash"; a1.passed = true;
    atlas::production::VerificationArtifact a2; a2.name = "test"; a2.passed = true;
    sys.AddArtifact(a1);
    sys.AddArtifact(a2);
    auto result = sys.RunVerification();
    assert(result.passed);
    assert(result.level == atlas::production::CertificationLevel::Standard);
    std::cout << "[PASS] test_cert_verify_all_pass" << std::endl;
}

void test_cert_verify_with_failure() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::Full);
    atlas::production::VerificationArtifact a1; a1.name = "hash"; a1.passed = true;
    atlas::production::VerificationArtifact a2; a2.name = "test"; a2.passed = false;
    sys.AddArtifact(a1);
    sys.AddArtifact(a2);
    auto result = sys.RunVerification();
    assert(!result.passed);
    std::cout << "[PASS] test_cert_verify_with_failure" << std::endl;
}

void test_cert_generate_report() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::Basic);
    atlas::production::VerificationArtifact a; a.name = "check"; a.type = "integrity"; a.passed = true;
    sys.AddArtifact(a);
    std::string report = sys.GenerateReport();
    assert(!report.empty());
    assert(report.find("check") != std::string::npos);
    std::cout << "[PASS] test_cert_generate_report" << std::endl;
}

void test_cert_export_json() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::AuditGrade);
    atlas::production::VerificationArtifact a;
    a.name = "binary"; a.type = "hash"; a.hash = 123; a.passed = true;
    sys.AddArtifact(a);
    std::string json = sys.ExportJSON();
    assert(!json.empty());
    assert(json.find("binary") != std::string::npos);
    assert(json.find("\"level\"") != std::string::npos);
    std::cout << "[PASS] test_cert_export_json" << std::endl;
}

void test_cert_clear() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::Full);
    atlas::production::VerificationArtifact a; a.name = "x"; a.passed = true;
    sys.AddArtifact(a);
    assert(sys.ArtifactCount() == 1);
    sys.Clear();
    assert(sys.ArtifactCount() == 0);
    assert(sys.GetLevel() == atlas::production::CertificationLevel::None);
    std::cout << "[PASS] test_cert_clear" << std::endl;
}
