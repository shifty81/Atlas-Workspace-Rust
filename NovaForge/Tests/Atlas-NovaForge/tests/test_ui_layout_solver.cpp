#include "../engine/ui/UILayoutSolver.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_layout_solver_single_entry() {
    UILayoutSolver solver;
    UIConstraint constraint;
    constraint.minWidth = 100;
    constraint.minHeight = 50;
    solver.AddEntry(1, constraint);

    UILayoutRect bounds{0, 0, 800, 600};
    solver.Solve(bounds, LayoutDirection::Horizontal);

    auto* resolved = solver.GetResolved(1);
    assert(resolved != nullptr);
    assert(resolved->x == 0);
    assert(resolved->y == 0);
    assert(resolved->w == 800);
    assert(resolved->h == 600);
    std::cout << "[PASS] test_layout_solver_single_entry" << std::endl;
}

void test_layout_solver_horizontal_split() {
    UILayoutSolver solver;

    UIConstraint c1;
    c1.minWidth = 100;
    c1.minHeight = 50;
    c1.weight = 1.0f;

    UIConstraint c2;
    c2.minWidth = 100;
    c2.minHeight = 50;
    c2.weight = 1.0f;

    solver.AddEntry(1, c1);
    solver.AddEntry(2, c2);

    UILayoutRect bounds{0, 0, 800, 600};
    solver.Solve(bounds, LayoutDirection::Horizontal);

    auto* r1 = solver.GetResolved(1);
    auto* r2 = solver.GetResolved(2);
    assert(r1 != nullptr);
    assert(r2 != nullptr);

    // Each gets 100 min + 300 extra (equal weight) = 400
    assert(r1->x == 0);
    assert(r1->w == 400);
    assert(r2->x == 400);
    assert(r2->w == 400);

    // Full height for both
    assert(r1->h == 600);
    assert(r2->h == 600);
    std::cout << "[PASS] test_layout_solver_horizontal_split" << std::endl;
}

void test_layout_solver_vertical_split() {
    UILayoutSolver solver;

    UIConstraint c1;
    c1.minWidth = 50;
    c1.minHeight = 100;
    c1.weight = 1.0f;

    UIConstraint c2;
    c2.minWidth = 50;
    c2.minHeight = 100;
    c2.weight = 1.0f;

    solver.AddEntry(1, c1);
    solver.AddEntry(2, c2);

    UILayoutRect bounds{0, 0, 800, 600};
    solver.Solve(bounds, LayoutDirection::Vertical);

    auto* r1 = solver.GetResolved(1);
    auto* r2 = solver.GetResolved(2);
    assert(r1 != nullptr);
    assert(r2 != nullptr);

    // Each gets 100 min + 200 extra = 300
    assert(r1->y == 0);
    assert(r1->h == 300);
    assert(r2->y == 300);
    assert(r2->h == 300);

    // Full width
    assert(r1->w == 800);
    assert(r2->w == 800);
    std::cout << "[PASS] test_layout_solver_vertical_split" << std::endl;
}

void test_layout_solver_weighted() {
    UILayoutSolver solver;

    UIConstraint c1;
    c1.minWidth = 0;
    c1.minHeight = 0;
    c1.weight = 1.0f;

    UIConstraint c2;
    c2.minWidth = 0;
    c2.minHeight = 0;
    c2.weight = 3.0f;

    solver.AddEntry(1, c1);
    solver.AddEntry(2, c2);

    UILayoutRect bounds{0, 0, 800, 600};
    solver.Solve(bounds, LayoutDirection::Horizontal);

    auto* r1 = solver.GetResolved(1);
    auto* r2 = solver.GetResolved(2);

    // 1/4 and 3/4 of 800
    assert(r1->w == 200);
    assert(r2->w == 600);
    std::cout << "[PASS] test_layout_solver_weighted" << std::endl;
}

void test_layout_solver_min_size_respect() {
    UILayoutSolver solver;

    UIConstraint c1;
    c1.minWidth = 500;
    c1.weight = 1.0f;

    UIConstraint c2;
    c2.minWidth = 200;
    c2.weight = 1.0f;

    solver.AddEntry(1, c1);
    solver.AddEntry(2, c2);

    UILayoutRect bounds{0, 0, 800, 600};
    solver.Solve(bounds, LayoutDirection::Horizontal);

    auto* r1 = solver.GetResolved(1);
    auto* r2 = solver.GetResolved(2);

    // Min sizes take 700, remaining 100 split evenly (50 each)
    assert(r1->w >= 500);
    assert(r2->w >= 200);
    assert(r1->w + r2->w <= 800);
    std::cout << "[PASS] test_layout_solver_min_size_respect" << std::endl;
}

void test_layout_solver_clear() {
    UILayoutSolver solver;

    UIConstraint c;
    solver.AddEntry(1, c);
    solver.AddEntry(2, c);
    assert(solver.EntryCount() == 2);

    solver.Clear();
    assert(solver.EntryCount() == 0);
    std::cout << "[PASS] test_layout_solver_clear" << std::endl;
}

void test_layout_solver_deterministic() {
    auto solve = [](int w, int h) -> std::vector<UILayoutRect> {
        UILayoutSolver solver;
        UIConstraint c1;
        c1.minWidth = 100;
        c1.weight = 1.0f;
        UIConstraint c2;
        c2.minWidth = 200;
        c2.weight = 2.0f;
        solver.AddEntry(1, c1);
        solver.AddEntry(2, c2);
        solver.Solve({0, 0, w, h}, LayoutDirection::Horizontal);
        std::vector<UILayoutRect> result;
        for (auto& e : solver.Entries()) {
            result.push_back(e.resolved);
        }
        return result;
    };

    auto a = solve(800, 600);
    auto b = solve(800, 600);
    assert(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        assert(a[i].x == b[i].x);
        assert(a[i].y == b[i].y);
        assert(a[i].w == b[i].w);
        assert(a[i].h == b[i].h);
    }
    std::cout << "[PASS] test_layout_solver_deterministic" << std::endl;
}

void test_layout_solver_offset() {
    UILayoutSolver solver;

    UIConstraint c1;
    c1.minWidth = 100;
    c1.weight = 1.0f;

    solver.AddEntry(1, c1);

    UILayoutRect bounds{50, 100, 400, 300};
    solver.Solve(bounds, LayoutDirection::Horizontal);

    auto* r = solver.GetResolved(1);
    assert(r->x == 50);
    assert(r->y == 100);
    std::cout << "[PASS] test_layout_solver_offset" << std::endl;
}
