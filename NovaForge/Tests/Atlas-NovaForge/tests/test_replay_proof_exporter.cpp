#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <filesystem>

#include "../engine/sim/ReplayProofExporter.h"

void test_proof_exporter_empty() {
    atlas::sim::ReplayProofExporter exporter;
    assert(exporter.EntryCount() == 0);
    assert(exporter.GetTrace().entries.empty());
    std::cout << "[PASS] test_proof_exporter_empty" << std::endl;
}

void test_proof_exporter_add_entries() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.AddEntry(1, 100, 0, 10);
    exporter.AddEntry(2, 200, 100, 20);
    exporter.AddEntry(3, 300, 200, 30);
    assert(exporter.EntryCount() == 3);
    assert(exporter.GetTrace().tickCount == 3);
    assert(exporter.GetTrace().initialHash == 100);
    assert(exporter.GetTrace().finalHash == 300);
    std::cout << "[PASS] test_proof_exporter_add_entries" << std::endl;
}

void test_proof_exporter_validate_chain_valid() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.AddEntry(1, 100, 0, 10);
    exporter.AddEntry(2, 200, 100, 20);
    exporter.AddEntry(3, 300, 200, 30);
    assert(exporter.ValidateChain());
    std::cout << "[PASS] test_proof_exporter_validate_chain_valid" << std::endl;
}

void test_proof_exporter_validate_chain_invalid() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.AddEntry(1, 100, 0, 10);
    exporter.AddEntry(2, 200, 999, 20); // broken: prevHash != 100
    assert(!exporter.ValidateChain());
    std::cout << "[PASS] test_proof_exporter_validate_chain_invalid" << std::endl;
}

void test_proof_exporter_export_tla() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.SetSpecName("test");
    exporter.AddEntry(1, 100, 0, 10);
    exporter.AddEntry(2, 200, 100, 20);

    std::string path = (std::filesystem::temp_directory_path() / "test_proof_export.tla").string();
    auto result = exporter.ExportTLA(path);
    assert(result.success);
    assert(result.entriesExported == 2);

    std::ifstream in(path);
    assert(in.good());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("MODULE ReplayTrace") != std::string::npos);
    assert(content.find("Spec: test") != std::string::npos);
    assert(content.find("ChainValid") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_proof_exporter_export_tla" << std::endl;
}

void test_proof_exporter_export_json() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.AddEntry(1, 100, 0, 10);
    exporter.AddEntry(2, 200, 100, 20);

    std::string path = (std::filesystem::temp_directory_path() / "test_proof_export.json").string();
    auto result = exporter.ExportJSON(path);
    assert(result.success);
    assert(result.entriesExported == 2);

    std::ifstream in(path);
    assert(in.good());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"tick\"") != std::string::npos);
    assert(content.find("\"stateHash\"") != std::string::npos);
    assert(content.find("[") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_proof_exporter_export_json" << std::endl;
}

void test_proof_exporter_export_csv() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.AddEntry(1, 100, 0, 10, true);

    std::string path = (std::filesystem::temp_directory_path() / "test_proof_export.csv").string();
    auto result = exporter.ExportCSV(path);
    assert(result.success);
    assert(result.entriesExported == 1);

    std::ifstream in(path);
    assert(in.good());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("tick,stateHash,prevHash,inputHash,isSavePoint") != std::string::npos);
    assert(content.find("true") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_proof_exporter_export_csv" << std::endl;
}

void test_proof_exporter_save_points() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.AddEntry(1, 100, 0, 10, true);
    exporter.AddEntry(2, 200, 100, 20, false);
    exporter.AddEntry(3, 300, 200, 30, true);
    assert(exporter.GetTrace().savePointCount == 2);
    auto sp = exporter.SavePointTicks();
    assert(sp.size() == 2);
    assert(sp[0] == 1);
    assert(sp[1] == 3);
    std::cout << "[PASS] test_proof_exporter_save_points" << std::endl;
}

void test_proof_exporter_format_entry() {
    atlas::sim::ProofTraceEntry entry;
    entry.tick = 5;
    entry.stateHash = 500;
    entry.prevHash = 400;
    entry.inputHash = 50;
    entry.isSavePoint = true;
    std::string formatted = atlas::sim::ReplayProofExporter::FormatEntry(entry);
    assert(formatted.find("tick=5") != std::string::npos);
    assert(formatted.find("stateHash=500") != std::string::npos);
    assert(formatted.find("savePoint=true") != std::string::npos);
    std::cout << "[PASS] test_proof_exporter_format_entry" << std::endl;
}

void test_proof_exporter_clear() {
    atlas::sim::ReplayProofExporter exporter;
    exporter.SetSpecName("test");
    exporter.AddEntry(1, 100, 0, 10);
    exporter.AddEntry(2, 200, 100, 20);
    assert(exporter.EntryCount() == 2);
    exporter.Clear();
    assert(exporter.EntryCount() == 0);
    assert(exporter.GetSpecName().empty());
    std::cout << "[PASS] test_proof_exporter_clear" << std::endl;
}
