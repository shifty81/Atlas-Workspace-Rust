#include "../engine/ai/WebAggregationKB.h"
#include <iostream>
#include <cassert>

void test_kb_add_entry() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry entry;
    entry.title = "Test Entry";
    entry.content = "Some content";
    entry.source = "https://example.com";
    entry.category = "tutorial";
    entry.tags = {"cpp", "engine"};
    entry.relevanceScore = 0.8;
    uint64_t id = kb.AddEntry(entry);
    assert(id > 0);
    assert(kb.EntryCount() == 1);
    std::cout << "[PASS] test_kb_add_entry" << std::endl;
}

void test_kb_get_entry() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry entry;
    entry.title = "Get Test";
    entry.content = "Content";
    uint64_t id = kb.AddEntry(entry);
    auto* found = kb.GetEntry(id);
    assert(found != nullptr);
    assert(found->title == "Get Test");
    assert(kb.GetEntry(999) == nullptr);
    std::cout << "[PASS] test_kb_get_entry" << std::endl;
}

void test_kb_remove_entry() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry entry;
    entry.title = "Remove Me";
    uint64_t id = kb.AddEntry(entry);
    assert(kb.EntryCount() == 1);
    kb.RemoveEntry(id);
    assert(kb.EntryCount() == 0);
    assert(kb.GetEntry(id) == nullptr);
    std::cout << "[PASS] test_kb_remove_entry" << std::endl;
}

void test_kb_search() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e1;
    e1.title = "Atlas Engine Guide";
    e1.content = "How to use the engine";
    e1.relevanceScore = 0.9;
    atlas::ai::KBEntry e2;
    e2.title = "Unrelated";
    e2.content = "Nothing about engines";
    e2.relevanceScore = 0.5;
    atlas::ai::KBEntry e3;
    e3.title = "Engine Advanced";
    e3.content = "Advanced engine topics";
    e3.relevanceScore = 0.7;
    kb.AddEntry(e1);
    kb.AddEntry(e2);
    kb.AddEntry(e3);

    auto result = kb.Search("engine");
    assert(result.totalMatches == 3);
    // Results should be sorted by relevance (highest first)
    assert(result.entries[0].relevanceScore >= result.entries[1].relevanceScore);
    std::cout << "[PASS] test_kb_search" << std::endl;
}

void test_kb_search_by_category() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e1;
    e1.title = "A"; e1.category = "tutorial";
    atlas::ai::KBEntry e2;
    e2.title = "B"; e2.category = "reference";
    atlas::ai::KBEntry e3;
    e3.title = "C"; e3.category = "tutorial";
    kb.AddEntry(e1);
    kb.AddEntry(e2);
    kb.AddEntry(e3);

    auto result = kb.SearchByCategory("tutorial");
    assert(result.totalMatches == 2);
    std::cout << "[PASS] test_kb_search_by_category" << std::endl;
}

void test_kb_search_by_tag() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e1;
    e1.title = "A"; e1.tags = {"cpp", "ai"};
    atlas::ai::KBEntry e2;
    e2.title = "B"; e2.tags = {"python"};
    atlas::ai::KBEntry e3;
    e3.title = "C"; e3.tags = {"cpp", "graphics"};
    kb.AddEntry(e1);
    kb.AddEntry(e2);
    kb.AddEntry(e3);

    auto result = kb.SearchByTag("cpp");
    assert(result.totalMatches == 2);
    std::cout << "[PASS] test_kb_search_by_tag" << std::endl;
}

void test_kb_categories() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e1; e1.category = "tutorial";
    atlas::ai::KBEntry e2; e2.category = "reference";
    atlas::ai::KBEntry e3; e3.category = "tutorial";
    kb.AddEntry(e1);
    kb.AddEntry(e2);
    kb.AddEntry(e3);
    assert(kb.CategoryCount() == 2);
    auto cats = kb.ListCategories();
    assert(cats.size() == 2);
    std::cout << "[PASS] test_kb_categories" << std::endl;
}

void test_kb_tags() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e1; e1.tags = {"a", "b"};
    atlas::ai::KBEntry e2; e2.tags = {"b", "c"};
    kb.AddEntry(e1);
    kb.AddEntry(e2);
    auto tags = kb.ListTags();
    assert(tags.size() == 3);
    std::cout << "[PASS] test_kb_tags" << std::endl;
}

void test_kb_export_import() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e;
    e.title = "Export Test";
    e.content = "Content here";
    e.source = "https://test.com";
    e.category = "test";
    e.tags = {"a", "b"};
    e.timestamp = 42;
    e.relevanceScore = 0.75;
    kb.AddEntry(e);

    std::string json = kb.ExportJSON();
    assert(!json.empty());
    assert(json.find("Export Test") != std::string::npos);

    atlas::ai::WebAggregationKB kb2;
    assert(kb2.ImportJSON(json));
    assert(kb2.EntryCount() == 1);
    std::cout << "[PASS] test_kb_export_import" << std::endl;
}

void test_kb_clear() {
    atlas::ai::WebAggregationKB kb;
    atlas::ai::KBEntry e;
    e.title = "Clear Me";
    kb.AddEntry(e);
    assert(kb.EntryCount() == 1);
    kb.Clear();
    assert(kb.EntryCount() == 0);
    std::cout << "[PASS] test_kb_clear" << std::endl;
}
