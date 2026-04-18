// S129 editor tests: TagSystemEditor, FilterEditor, SearchIndexEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SearchIndexEditor.h"
#include "NF/Editor/FilterEditor.h"
#include "NF/Editor/TagSystemEditor.h"

using namespace NF;

// ── TagSystemEditor ───────────────────────────────────────────────────────────

TEST_CASE("TagCategory names", "[Editor][S129]") {
    REQUIRE(std::string(tagCategoryName(TagCategory::Asset))     == "Asset");
    REQUIRE(std::string(tagCategoryName(TagCategory::Scene))     == "Scene");
    REQUIRE(std::string(tagCategoryName(TagCategory::Entity))    == "Entity");
    REQUIRE(std::string(tagCategoryName(TagCategory::Component)) == "Component");
    REQUIRE(std::string(tagCategoryName(TagCategory::Custom))    == "Custom");
}

TEST_CASE("TagScope names", "[Editor][S129]") {
    REQUIRE(std::string(tagScopeName(TagScope::Project)) == "Project");
    REQUIRE(std::string(tagScopeName(TagScope::Scene))   == "Scene");
    REQUIRE(std::string(tagScopeName(TagScope::Local))   == "Local");
    REQUIRE(std::string(tagScopeName(TagScope::Global))  == "Global");
}

TEST_CASE("TagDefinition defaults", "[Editor][S129]") {
    TagDefinition t(1, "enemy", TagCategory::Entity, TagScope::Scene);
    REQUIRE(t.id()             == 1u);
    REQUIRE(t.name()           == "enemy");
    REQUIRE(t.category()       == TagCategory::Entity);
    REQUIRE(t.scope()          == TagScope::Scene);
    REQUIRE(t.color()          == 0u);
    REQUIRE(!t.isHierarchical());
    REQUIRE(t.isEnabled());
}

TEST_CASE("TagDefinition mutation", "[Editor][S129]") {
    TagDefinition t(2, "asset_group", TagCategory::Asset, TagScope::Project);
    t.setColor(0xFF0000u);
    t.setIsHierarchical(true);
    t.setIsEnabled(false);
    REQUIRE(t.color()          == 0xFF0000u);
    REQUIRE(t.isHierarchical());
    REQUIRE(!t.isEnabled());
}

TEST_CASE("TagSystemEditor defaults", "[Editor][S129]") {
    TagSystemEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByCategory());
    REQUIRE(ed.maxTagsPerItem() == 32u);
    REQUIRE(ed.tagCount()       == 0u);
}

TEST_CASE("TagSystemEditor add/remove tags", "[Editor][S129]") {
    TagSystemEditor ed;
    REQUIRE(ed.addTag(TagDefinition(1, "t_a", TagCategory::Entity,    TagScope::Scene)));
    REQUIRE(ed.addTag(TagDefinition(2, "t_b", TagCategory::Asset,     TagScope::Project)));
    REQUIRE(ed.addTag(TagDefinition(3, "t_c", TagCategory::Component, TagScope::Global)));
    REQUIRE(!ed.addTag(TagDefinition(1, "t_a", TagCategory::Entity,   TagScope::Scene)));
    REQUIRE(ed.tagCount() == 3u);
    REQUIRE(ed.removeTag(2));
    REQUIRE(ed.tagCount() == 2u);
    REQUIRE(!ed.removeTag(99));
}

TEST_CASE("TagSystemEditor counts and find", "[Editor][S129]") {
    TagSystemEditor ed;
    TagDefinition t1(1, "t_a", TagCategory::Entity,    TagScope::Scene);
    TagDefinition t2(2, "t_b", TagCategory::Entity,    TagScope::Local);
    TagDefinition t3(3, "t_c", TagCategory::Asset,     TagScope::Project);
    TagDefinition t4(4, "t_d", TagCategory::Component, TagScope::Global); t4.setIsEnabled(false);
    ed.addTag(t1); ed.addTag(t2); ed.addTag(t3); ed.addTag(t4);
    REQUIRE(ed.countByCategory(TagCategory::Entity)    == 2u);
    REQUIRE(ed.countByCategory(TagCategory::Asset)     == 1u);
    REQUIRE(ed.countByCategory(TagCategory::Custom)    == 0u);
    REQUIRE(ed.countByScope(TagScope::Scene)           == 1u);
    REQUIRE(ed.countByScope(TagScope::Project)         == 1u);
    REQUIRE(ed.countEnabled()                          == 3u);
    auto* found = ed.findTag(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == TagCategory::Asset);
    REQUIRE(ed.findTag(99) == nullptr);
}

TEST_CASE("TagSystemEditor settings mutation", "[Editor][S129]") {
    TagSystemEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByCategory(false);
    ed.setMaxTagsPerItem(64u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByCategory());
    REQUIRE(ed.maxTagsPerItem() == 64u);
}

// ── FilterEditor ──────────────────────────────────────────────────────────────

TEST_CASE("FilterOperator names", "[Editor][S129]") {
    REQUIRE(std::string(filterOperatorName(FilterOperator::Equals))     == "Equals");
    REQUIRE(std::string(filterOperatorName(FilterOperator::NotEquals))  == "NotEquals");
    REQUIRE(std::string(filterOperatorName(FilterOperator::Contains))   == "Contains");
    REQUIRE(std::string(filterOperatorName(FilterOperator::StartsWith)) == "StartsWith");
    REQUIRE(std::string(filterOperatorName(FilterOperator::EndsWith))   == "EndsWith");
    REQUIRE(std::string(filterOperatorName(FilterOperator::Regex))      == "Regex");
    REQUIRE(std::string(filterOperatorName(FilterOperator::Range))      == "Range");
}

TEST_CASE("FilterLogic names", "[Editor][S129]") {
    REQUIRE(std::string(filterLogicName(FilterLogic::And)) == "And");
    REQUIRE(std::string(filterLogicName(FilterLogic::Or))  == "Or");
    REQUIRE(std::string(filterLogicName(FilterLogic::Not)) == "Not");
    REQUIRE(std::string(filterLogicName(FilterLogic::Xor)) == "Xor");
}

TEST_CASE("FilterRule defaults", "[Editor][S129]") {
    FilterRule r(1, "name_filter", FilterOperator::Contains);
    REQUIRE(r.id()        == 1u);
    REQUIRE(r.name()      == "name_filter");
    REQUIRE(r.op()        == FilterOperator::Contains);
    REQUIRE(r.logic()     == FilterLogic::And);
    REQUIRE(!r.isNegated());
    REQUIRE(r.isEnabled());
}

TEST_CASE("FilterRule mutation", "[Editor][S129]") {
    FilterRule r(2, "regex_filter", FilterOperator::Regex);
    r.setLogic(FilterLogic::Or);
    r.setIsNegated(true);
    r.setIsEnabled(false);
    REQUIRE(r.logic()     == FilterLogic::Or);
    REQUIRE(r.isNegated());
    REQUIRE(!r.isEnabled());
}

TEST_CASE("FilterEditor defaults", "[Editor][S129]") {
    FilterEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByLogic());
    REQUIRE(ed.maxFiltersPerQuery() == 20u);
    REQUIRE(ed.filterRuleCount()    == 0u);
}

TEST_CASE("FilterEditor add/remove rules", "[Editor][S129]") {
    FilterEditor ed;
    REQUIRE(ed.addFilterRule(FilterRule(1, "f_a", FilterOperator::Equals)));
    REQUIRE(ed.addFilterRule(FilterRule(2, "f_b", FilterOperator::Contains)));
    REQUIRE(ed.addFilterRule(FilterRule(3, "f_c", FilterOperator::Regex)));
    REQUIRE(!ed.addFilterRule(FilterRule(1, "f_a", FilterOperator::Equals)));
    REQUIRE(ed.filterRuleCount() == 3u);
    REQUIRE(ed.removeFilterRule(2));
    REQUIRE(ed.filterRuleCount() == 2u);
    REQUIRE(!ed.removeFilterRule(99));
}

TEST_CASE("FilterEditor counts and find", "[Editor][S129]") {
    FilterEditor ed;
    FilterRule r1(1, "f_a", FilterOperator::Contains);
    FilterRule r2(2, "f_b", FilterOperator::Contains); r2.setLogic(FilterLogic::Or);
    FilterRule r3(3, "f_c", FilterOperator::Equals);   r3.setLogic(FilterLogic::Or);
    FilterRule r4(4, "f_d", FilterOperator::Regex);    r4.setLogic(FilterLogic::Not); r4.setIsEnabled(false);
    ed.addFilterRule(r1); ed.addFilterRule(r2); ed.addFilterRule(r3); ed.addFilterRule(r4);
    REQUIRE(ed.countByOperator(FilterOperator::Contains) == 2u);
    REQUIRE(ed.countByOperator(FilterOperator::Equals)   == 1u);
    REQUIRE(ed.countByOperator(FilterOperator::Range)    == 0u);
    REQUIRE(ed.countByLogic(FilterLogic::And)            == 1u);
    REQUIRE(ed.countByLogic(FilterLogic::Or)             == 2u);
    REQUIRE(ed.countEnabled()                            == 3u);
    auto* found = ed.findFilterRule(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->op() == FilterOperator::Equals);
    REQUIRE(ed.findFilterRule(99) == nullptr);
}

TEST_CASE("FilterEditor settings mutation", "[Editor][S129]") {
    FilterEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByLogic(true);
    ed.setMaxFiltersPerQuery(50u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByLogic());
    REQUIRE(ed.maxFiltersPerQuery() == 50u);
}

// ── SearchIndexEditor ─────────────────────────────────────────────────────────

TEST_CASE("IndexType names", "[Editor][S129]") {
    REQUIRE(std::string(indexTypeName(IndexType::FullText)) == "FullText");
    REQUIRE(std::string(indexTypeName(IndexType::Keyword))  == "Keyword");
    REQUIRE(std::string(indexTypeName(IndexType::Numeric))  == "Numeric");
    REQUIRE(std::string(indexTypeName(IndexType::Spatial))  == "Spatial");
    REQUIRE(std::string(indexTypeName(IndexType::Vector))   == "Vector");
}

TEST_CASE("IndexStatus names", "[Editor][S129]") {
    REQUIRE(std::string(indexStatusName(IndexStatus::Idle))     == "Idle");
    REQUIRE(std::string(indexStatusName(IndexStatus::Building)) == "Building");
    REQUIRE(std::string(indexStatusName(IndexStatus::Ready))    == "Ready");
    REQUIRE(std::string(indexStatusName(IndexStatus::Stale))    == "Stale");
    REQUIRE(std::string(indexStatusName(IndexStatus::Error))    == "Error");
}

TEST_CASE("SearchIndex defaults", "[Editor][S129]") {
    SearchIndex i(1, "name_index", IndexType::FullText);
    REQUIRE(i.id()            == 1u);
    REQUIRE(i.name()          == "name_index");
    REQUIRE(i.type()          == IndexType::FullText);
    REQUIRE(i.status()        == IndexStatus::Idle);
    REQUIRE(i.entryCount()    == 0u);
    REQUIRE(i.isAutoRebuild());
    REQUIRE(i.isEnabled());
}

TEST_CASE("SearchIndex mutation", "[Editor][S129]") {
    SearchIndex i(2, "spatial_index", IndexType::Spatial);
    i.setStatus(IndexStatus::Ready);
    i.setEntryCount(1000u);
    i.setIsAutoRebuild(false);
    i.setIsEnabled(false);
    REQUIRE(i.status()        == IndexStatus::Ready);
    REQUIRE(i.entryCount()    == 1000u);
    REQUIRE(!i.isAutoRebuild());
    REQUIRE(!i.isEnabled());
}

TEST_CASE("SearchIndexEditor defaults", "[Editor][S129]") {
    SearchIndexEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.rebuildIntervalMin() == 60u);
    REQUIRE(ed.indexCount()         == 0u);
}

TEST_CASE("SearchIndexEditor add/remove indices", "[Editor][S129]") {
    SearchIndexEditor ed;
    REQUIRE(ed.addIndex(SearchIndex(1, "i_a", IndexType::FullText)));
    REQUIRE(ed.addIndex(SearchIndex(2, "i_b", IndexType::Keyword)));
    REQUIRE(ed.addIndex(SearchIndex(3, "i_c", IndexType::Numeric)));
    REQUIRE(!ed.addIndex(SearchIndex(1, "i_a", IndexType::FullText)));
    REQUIRE(ed.indexCount() == 3u);
    REQUIRE(ed.removeIndex(2));
    REQUIRE(ed.indexCount() == 2u);
    REQUIRE(!ed.removeIndex(99));
}

TEST_CASE("SearchIndexEditor counts and find", "[Editor][S129]") {
    SearchIndexEditor ed;
    SearchIndex i1(1, "i_a", IndexType::FullText);
    SearchIndex i2(2, "i_b", IndexType::FullText); i2.setStatus(IndexStatus::Ready);
    SearchIndex i3(3, "i_c", IndexType::Keyword);  i3.setStatus(IndexStatus::Building);
    SearchIndex i4(4, "i_d", IndexType::Spatial);  i4.setStatus(IndexStatus::Error); i4.setIsEnabled(false);
    ed.addIndex(i1); ed.addIndex(i2); ed.addIndex(i3); ed.addIndex(i4);
    REQUIRE(ed.countByType(IndexType::FullText)      == 2u);
    REQUIRE(ed.countByType(IndexType::Keyword)       == 1u);
    REQUIRE(ed.countByType(IndexType::Vector)        == 0u);
    REQUIRE(ed.countByStatus(IndexStatus::Idle)      == 1u);
    REQUIRE(ed.countByStatus(IndexStatus::Ready)     == 1u);
    REQUIRE(ed.countEnabled()                        == 3u);
    auto* found = ed.findIndex(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == IndexType::Keyword);
    REQUIRE(ed.findIndex(99) == nullptr);
}

TEST_CASE("SearchIndexEditor settings mutation", "[Editor][S129]") {
    SearchIndexEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(true);
    ed.setRebuildIntervalMin(120u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.rebuildIntervalMin() == 120u);
}
