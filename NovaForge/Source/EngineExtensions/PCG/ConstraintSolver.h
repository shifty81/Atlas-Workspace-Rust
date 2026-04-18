#pragma once
#include "DeterministicRNG.h"
#include <cstdint>
#include <vector>
#include <functional>
#include <string>

namespace atlas::procedural {

/// A single item to be fitted by the constraint solver.
/// Each item has a required capacity cost and a value (fitness).
struct FitItem {
    std::string name;
    float cost    = 0;   // e.g. powergrid / CPU usage
    float value   = 0;   // fitness contribution
    int32_t group = -1;  // optional group tag (for mutual exclusion etc.)
};

/// Configuration for a constraint solving run.
struct ConstraintConfig {
    float maxCapacity     = 100.0f;  // total budget
    int32_t maxItems      = 10;      // maximum items that can be selected
    int32_t populationSize = 50;     // GA population size
    int32_t generations    = 100;    // GA iteration count
    float mutationRate     = 0.1f;   // probability of flipping a gene
    float crossoverRate    = 0.7f;   // probability of crossover
};

/// Result of a constraint solver run.
struct ConstraintResult {
    std::vector<size_t> selectedIndices;  // indices into the item pool
    float totalCost   = 0;
    float totalValue  = 0;
    int32_t itemCount = 0;
    bool feasible     = false;
};

/// Genetic-algorithm-style constraint solver for fitting items within
/// capacity and count limits.  Used to generate valid ship loadouts,
/// module configurations, and other fitting problems.
///
/// The solver is deterministic — given the same seed, items, and
/// config it produces the same result on every platform.
///
/// Usage:
///   ConstraintSolver solver(seed);
///   solver.AddItem({"Laser", 25, 10});
///   solver.AddItem({"Shield", 40, 15});
///   auto result = solver.Solve(config);
///
class ConstraintSolver {
public:
    explicit ConstraintSolver(uint64_t seed = 1);

    /// Add an item to the available pool.
    void AddItem(const FitItem& item);

    /// Clear all items.
    void ClearItems();

    /// Number of items in the pool.
    size_t ItemCount() const { return m_items.size(); }

    /// Run the solver with the given configuration.
    ConstraintResult Solve(const ConstraintConfig& config);

    /// Convenience: check whether a result satisfies the constraints.
    static bool IsFeasible(const ConstraintResult& result,
                           const ConstraintConfig& config);

private:
    DeterministicRNG m_rng;
    std::vector<FitItem> m_items;

    // GA internals
    using Chromosome = std::vector<bool>;

    Chromosome RandomChromosome(size_t len, const ConstraintConfig& cfg);
    float Fitness(const Chromosome& chr, const ConstraintConfig& cfg) const;
    ConstraintResult Decode(const Chromosome& chr) const;
    Chromosome Crossover(const Chromosome& a, const Chromosome& b);
    void Mutate(Chromosome& chr, float rate);
};

} // namespace atlas::procedural
