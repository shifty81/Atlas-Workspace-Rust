// S190 editor tests: NavLinkEditorV1, SoundCueEditorV1, LensFlareEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/NavLinkEditorV1.h"
#include "NF/Editor/SoundCueEditorV1.h"
#include "NF/Editor/LensFlareEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── NavLinkEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Nlev1NavLink validity", "[Editor][S190]") {
    Nlev1NavLink l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "JumpLink";
    REQUIRE(l.isValid());
}

TEST_CASE("NavLinkEditorV1 addLink and linkCount", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    REQUIRE(nav.linkCount() == 0);
    Nlev1NavLink l; l.id = 1; l.name = "L1";
    REQUIRE(nav.addLink(l));
    REQUIRE(nav.linkCount() == 1);
}

TEST_CASE("NavLinkEditorV1 addLink invalid fails", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    REQUIRE(!nav.addLink(Nlev1NavLink{}));
}

TEST_CASE("NavLinkEditorV1 addLink duplicate fails", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavLink l; l.id = 1; l.name = "A";
    nav.addLink(l);
    REQUIRE(!nav.addLink(l));
}

TEST_CASE("NavLinkEditorV1 removeLink", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavLink l; l.id = 2; l.name = "B";
    nav.addLink(l);
    REQUIRE(nav.removeLink(2));
    REQUIRE(nav.linkCount() == 0);
    REQUIRE(!nav.removeLink(2));
}

TEST_CASE("NavLinkEditorV1 findLink", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavLink l; l.id = 3; l.name = "C";
    nav.addLink(l);
    REQUIRE(nav.findLink(3) != nullptr);
    REQUIRE(nav.findLink(99) == nullptr);
}

TEST_CASE("NavLinkEditorV1 addArea and areaCount", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavArea a; a.id = 1; a.name = "WaterArea";
    REQUIRE(nav.addArea(a));
    REQUIRE(nav.areaCount() == 1);
}

TEST_CASE("NavLinkEditorV1 removeArea", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavArea a; a.id = 1; a.name = "LavaArea";
    nav.addArea(a);
    REQUIRE(nav.removeArea(1));
    REQUIRE(nav.areaCount() == 0);
}

TEST_CASE("NavLinkEditorV1 activeCount", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavLink l1; l1.id = 1; l1.name = "A"; l1.state = Nlev1LinkState::Active;
    Nlev1NavLink l2; l2.id = 2; l2.name = "B";
    nav.addLink(l1); nav.addLink(l2);
    REQUIRE(nav.activeCount() == 1);
}

TEST_CASE("NavLinkEditorV1 blockedCount", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavLink l1; l1.id = 1; l1.name = "A"; l1.state = Nlev1LinkState::Blocked;
    Nlev1NavLink l2; l2.id = 2; l2.name = "B"; l2.state = Nlev1LinkState::Active;
    nav.addLink(l1); nav.addLink(l2);
    REQUIRE(nav.blockedCount() == 1);
}

TEST_CASE("NavLinkEditorV1 countByType", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    Nlev1NavLink l1; l1.id = 1; l1.name = "A"; l1.linkType = Nlev1LinkType::Jump;
    Nlev1NavLink l2; l2.id = 2; l2.name = "B"; l2.linkType = Nlev1LinkType::Climb;
    Nlev1NavLink l3; l3.id = 3; l3.name = "C"; l3.linkType = Nlev1LinkType::Jump;
    nav.addLink(l1); nav.addLink(l2); nav.addLink(l3);
    REQUIRE(nav.countByType(Nlev1LinkType::Jump) == 2);
    REQUIRE(nav.countByType(Nlev1LinkType::Climb) == 1);
}

TEST_CASE("NavLinkEditorV1 onChange callback", "[Editor][S190]") {
    NavLinkEditorV1 nav;
    uint64_t notified = 0;
    nav.setOnChange([&](uint64_t id) { notified = id; });
    Nlev1NavLink l; l.id = 5; l.name = "E";
    nav.addLink(l);
    REQUIRE(notified == 5);
}

TEST_CASE("Nlev1NavLink state helpers", "[Editor][S190]") {
    Nlev1NavLink l; l.id = 1; l.name = "X";
    l.state = Nlev1LinkState::Active;   REQUIRE(l.isActive());
    l.state = Nlev1LinkState::Blocked;  REQUIRE(l.isBlocked());
    l.state = Nlev1LinkState::Disabled; REQUIRE(l.isDisabled());
}

TEST_CASE("nlev1LinkTypeName all values", "[Editor][S190]") {
    REQUIRE(std::string(nlev1LinkTypeName(Nlev1LinkType::Jump))     == "Jump");
    REQUIRE(std::string(nlev1LinkTypeName(Nlev1LinkType::Climb))    == "Climb");
    REQUIRE(std::string(nlev1LinkTypeName(Nlev1LinkType::Swim))     == "Swim");
    REQUIRE(std::string(nlev1LinkTypeName(Nlev1LinkType::Teleport)) == "Teleport");
    REQUIRE(std::string(nlev1LinkTypeName(Nlev1LinkType::Door))     == "Door");
    REQUIRE(std::string(nlev1LinkTypeName(Nlev1LinkType::Ladder))   == "Ladder");
}

// ── SoundCueEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Scev1SoundCue validity", "[Editor][S190]") {
    Scev1SoundCue c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "ExplosionCue";
    REQUIRE(c.isValid());
}

TEST_CASE("SoundCueEditorV1 addCue and cueCount", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    REQUIRE(sce.cueCount() == 0);
    Scev1SoundCue c; c.id = 1; c.name = "C1";
    REQUIRE(sce.addCue(c));
    REQUIRE(sce.cueCount() == 1);
}

TEST_CASE("SoundCueEditorV1 addCue invalid fails", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    REQUIRE(!sce.addCue(Scev1SoundCue{}));
}

TEST_CASE("SoundCueEditorV1 addCue duplicate fails", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundCue c; c.id = 1; c.name = "A";
    sce.addCue(c);
    REQUIRE(!sce.addCue(c));
}

TEST_CASE("SoundCueEditorV1 removeCue", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundCue c; c.id = 2; c.name = "B";
    sce.addCue(c);
    REQUIRE(sce.removeCue(2));
    REQUIRE(sce.cueCount() == 0);
    REQUIRE(!sce.removeCue(2));
}

TEST_CASE("SoundCueEditorV1 findCue", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundCue c; c.id = 3; c.name = "C";
    sce.addCue(c);
    REQUIRE(sce.findCue(3) != nullptr);
    REQUIRE(sce.findCue(99) == nullptr);
}

TEST_CASE("SoundCueEditorV1 addTrigger and triggerCount", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundTrigger t; t.id = 1; t.cueId = 10; t.event = "OnDamage";
    REQUIRE(sce.addTrigger(t));
    REQUIRE(sce.triggerCount() == 1);
}

TEST_CASE("SoundCueEditorV1 removeTrigger", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundTrigger t; t.id = 1; t.cueId = 10; t.event = "OnDeath";
    sce.addTrigger(t);
    REQUIRE(sce.removeTrigger(1));
    REQUIRE(sce.triggerCount() == 0);
}

TEST_CASE("SoundCueEditorV1 playingCount", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundCue c1; c1.id = 1; c1.name = "A"; c1.state = Scev1CueState::Playing;
    Scev1SoundCue c2; c2.id = 2; c2.name = "B";
    sce.addCue(c1); sce.addCue(c2);
    REQUIRE(sce.playingCount() == 1);
}

TEST_CASE("SoundCueEditorV1 errorCount", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundCue c1; c1.id = 1; c1.name = "A"; c1.state = Scev1CueState::Error;
    Scev1SoundCue c2; c2.id = 2; c2.name = "B"; c2.state = Scev1CueState::Playing;
    sce.addCue(c1); sce.addCue(c2);
    REQUIRE(sce.errorCount() == 1);
}

TEST_CASE("SoundCueEditorV1 countByType", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundCue c1; c1.id = 1; c1.name = "A"; c1.cueType = Scev1CueType::Music;
    Scev1SoundCue c2; c2.id = 2; c2.name = "B"; c2.cueType = Scev1CueType::SFX;
    Scev1SoundCue c3; c3.id = 3; c3.name = "C"; c3.cueType = Scev1CueType::Music;
    sce.addCue(c1); sce.addCue(c2); sce.addCue(c3);
    REQUIRE(sce.countByType(Scev1CueType::Music) == 2);
    REQUIRE(sce.countByType(Scev1CueType::SFX) == 1);
}

TEST_CASE("SoundCueEditorV1 triggersForCue", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    Scev1SoundTrigger t1; t1.id = 1; t1.cueId = 10; t1.event = "A";
    Scev1SoundTrigger t2; t2.id = 2; t2.cueId = 10; t2.event = "B";
    Scev1SoundTrigger t3; t3.id = 3; t3.cueId = 20; t3.event = "C";
    sce.addTrigger(t1); sce.addTrigger(t2); sce.addTrigger(t3);
    REQUIRE(sce.triggersForCue(10) == 2);
    REQUIRE(sce.triggersForCue(20) == 1);
}

TEST_CASE("SoundCueEditorV1 onChange callback", "[Editor][S190]") {
    SoundCueEditorV1 sce;
    uint64_t notified = 0;
    sce.setOnChange([&](uint64_t id) { notified = id; });
    Scev1SoundCue c; c.id = 7; c.name = "G";
    sce.addCue(c);
    REQUIRE(notified == 7);
}

TEST_CASE("Scev1SoundCue state helpers", "[Editor][S190]") {
    Scev1SoundCue c; c.id = 1; c.name = "X";
    c.state = Scev1CueState::Playing; REQUIRE(c.isPlaying());
    c.state = Scev1CueState::Paused;  REQUIRE(c.isPaused());
    c.state = Scev1CueState::Error;   REQUIRE(c.isError());
}

TEST_CASE("scev1CueTypeName all values", "[Editor][S190]") {
    REQUIRE(std::string(scev1CueTypeName(Scev1CueType::OneShot))  == "OneShot");
    REQUIRE(std::string(scev1CueTypeName(Scev1CueType::Looping))  == "Looping");
    REQUIRE(std::string(scev1CueTypeName(Scev1CueType::Ambience)) == "Ambience");
    REQUIRE(std::string(scev1CueTypeName(Scev1CueType::Music))    == "Music");
    REQUIRE(std::string(scev1CueTypeName(Scev1CueType::SFX))      == "SFX");
    REQUIRE(std::string(scev1CueTypeName(Scev1CueType::Voice))    == "Voice");
}

// ── LensFlareEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Lfev1FlareElement validity", "[Editor][S190]") {
    Lfev1FlareElement e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "MainGlow";
    REQUIRE(e.isValid());
}

TEST_CASE("LensFlareEditorV1 addElement and elementCount", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    REQUIRE(lfe.elementCount() == 0);
    Lfev1FlareElement e; e.id = 1; e.name = "E1";
    REQUIRE(lfe.addElement(e));
    REQUIRE(lfe.elementCount() == 1);
}

TEST_CASE("LensFlareEditorV1 addElement invalid fails", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    REQUIRE(!lfe.addElement(Lfev1FlareElement{}));
}

TEST_CASE("LensFlareEditorV1 addElement duplicate fails", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareElement e; e.id = 1; e.name = "A";
    lfe.addElement(e);
    REQUIRE(!lfe.addElement(e));
}

TEST_CASE("LensFlareEditorV1 removeElement", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareElement e; e.id = 2; e.name = "B";
    lfe.addElement(e);
    REQUIRE(lfe.removeElement(2));
    REQUIRE(lfe.elementCount() == 0);
    REQUIRE(!lfe.removeElement(2));
}

TEST_CASE("LensFlareEditorV1 findElement", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareElement e; e.id = 3; e.name = "C";
    lfe.addElement(e);
    REQUIRE(lfe.findElement(3) != nullptr);
    REQUIRE(lfe.findElement(99) == nullptr);
}

TEST_CASE("LensFlareEditorV1 addProfile and profileCount", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareProfile p; p.id = 1; p.name = "SunFlare";
    REQUIRE(lfe.addProfile(p));
    REQUIRE(lfe.profileCount() == 1);
}

TEST_CASE("LensFlareEditorV1 removeProfile", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareProfile p; p.id = 1; p.name = "LampFlare";
    lfe.addProfile(p);
    REQUIRE(lfe.removeProfile(1));
    REQUIRE(lfe.profileCount() == 0);
}

TEST_CASE("LensFlareEditorV1 visibleCount", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareElement e1; e1.id = 1; e1.name = "A"; e1.state = Lfev1ElementState::Visible;
    Lfev1FlareElement e2; e2.id = 2; e2.name = "B";
    lfe.addElement(e1); lfe.addElement(e2);
    REQUIRE(lfe.visibleCount() == 1);
}

TEST_CASE("LensFlareEditorV1 previewingCount", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareElement e1; e1.id = 1; e1.name = "A"; e1.state = Lfev1ElementState::Previewing;
    Lfev1FlareElement e2; e2.id = 2; e2.name = "B"; e2.state = Lfev1ElementState::Visible;
    lfe.addElement(e1); lfe.addElement(e2);
    REQUIRE(lfe.previewingCount() == 1);
}

TEST_CASE("LensFlareEditorV1 countByType", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    Lfev1FlareElement e1; e1.id = 1; e1.name = "A"; e1.elementType = Lfev1ElementType::Halo;
    Lfev1FlareElement e2; e2.id = 2; e2.name = "B"; e2.elementType = Lfev1ElementType::Ghost;
    Lfev1FlareElement e3; e3.id = 3; e3.name = "C"; e3.elementType = Lfev1ElementType::Halo;
    lfe.addElement(e1); lfe.addElement(e2); lfe.addElement(e3);
    REQUIRE(lfe.countByType(Lfev1ElementType::Halo) == 2);
    REQUIRE(lfe.countByType(Lfev1ElementType::Ghost) == 1);
}

TEST_CASE("LensFlareEditorV1 onChange callback", "[Editor][S190]") {
    LensFlareEditorV1 lfe;
    uint64_t notified = 0;
    lfe.setOnChange([&](uint64_t id) { notified = id; });
    Lfev1FlareElement e; e.id = 9; e.name = "I";
    lfe.addElement(e);
    REQUIRE(notified == 9);
}

TEST_CASE("Lfev1FlareElement state helpers", "[Editor][S190]") {
    Lfev1FlareElement e; e.id = 1; e.name = "X";
    e.state = Lfev1ElementState::Visible;    REQUIRE(e.isVisible());
    e.state = Lfev1ElementState::Previewing; REQUIRE(e.isPreviewing());
}

TEST_CASE("lfev1ElementTypeName all values", "[Editor][S190]") {
    REQUIRE(std::string(lfev1ElementTypeName(Lfev1ElementType::Streak))  == "Streak");
    REQUIRE(std::string(lfev1ElementTypeName(Lfev1ElementType::Halo))    == "Halo");
    REQUIRE(std::string(lfev1ElementTypeName(Lfev1ElementType::Glow))    == "Glow");
    REQUIRE(std::string(lfev1ElementTypeName(Lfev1ElementType::Ring))    == "Ring");
    REQUIRE(std::string(lfev1ElementTypeName(Lfev1ElementType::Shimmer)) == "Shimmer");
    REQUIRE(std::string(lfev1ElementTypeName(Lfev1ElementType::Ghost))   == "Ghost");
}
